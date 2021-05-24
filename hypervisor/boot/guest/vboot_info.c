/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <types.h>
#include <rtl.h>
#include <errno.h>
#include <asm/per_cpu.h>
#include <asm/irq.h>
#include <boot.h>
#include <asm/pgtable.h>
#include <asm/zeropage.h>
#include <asm/seed.h>
#include <asm/mmu.h>
#include <asm/guest/vm.h>
#include <logmsg.h>
#include <vboot_info.h>
#include <vacpi.h>

#define DBG_LEVEL_BOOT	6U

/**
 * @pre vm != NULL && mod != NULL
 */
static void init_vm_ramdisk_info(struct acrn_vm *vm, const struct abi_module *mod)
{
	if (mod->start != NULL) {
		vm->sw.ramdisk_info.src_addr = mod->start;
		vm->sw.ramdisk_info.load_addr = vm->sw.kernel_info.kernel_load_addr + vm->sw.kernel_info.kernel_size;
		vm->sw.ramdisk_info.load_addr = (void *)round_page_up((uint64_t)vm->sw.ramdisk_info.load_addr);
		vm->sw.ramdisk_info.size = mod->size;
	}
}

/**
 * @pre vm != NULL && mod != NULL
 */
static void init_vm_acpi_info(struct acrn_vm *vm, const struct abi_module *mod)
{
	vm->sw.acpi_info.src_addr = mod->start;
	vm->sw.acpi_info.load_addr = (void *)VIRT_ACPI_DATA_ADDR;
	vm->sw.acpi_info.size = ACPI_MODULE_SIZE;
}

/**
 * @pre vm != NULL
 */
static void *get_kernel_load_addr(struct acrn_vm *vm)
{
	void *load_addr = NULL;
	struct vm_sw_info *sw_info = &vm->sw;
	struct zero_page *zeropage;
	struct acrn_vm_config *vm_config = get_vm_config(vm->vm_id);

	switch (sw_info->kernel_type) {
	case KERNEL_BZIMAGE:
		/* According to the explaination for pref_address
		 * in Documentation/x86/boot.txt, a relocating
		 * bootloader should attempt to load kernel at pref_address
		 * if possible. A non-relocatable kernel will unconditionally
		 * move itself and to run at this address, so no need to copy
		 * kernel to perf_address by bootloader, if kernel is
		 * non-relocatable.
		 */
		zeropage = (struct zero_page *)sw_info->kernel_info.kernel_src_addr;
		if (zeropage->hdr.relocatable_kernel != 0U) {
			zeropage = (struct zero_page *)zeropage->hdr.pref_addr;
		}
		load_addr = (void *)zeropage;
		break;
	case KERNEL_ZEPHYR:
		load_addr = (void *)vm_config->os_config.kernel_load_addr;
		break;
	default:
		pr_err("Unsupported Kernel type.");
		break;
	}
	if (load_addr == NULL) {
		pr_err("Could not get kernel load addr of VM %d .", vm->vm_id);
	}
	return load_addr;
}

/**
 * @pre vm != NULL && mod != NULL
 */
static int32_t init_vm_kernel_info(struct acrn_vm *vm, const struct abi_module *mod)
{
	struct acrn_vm_config *vm_config = get_vm_config(vm->vm_id);

	dev_dbg(DBG_LEVEL_BOOT, "kernel mod start=0x%x, size=0x%x",
			(uint64_t)mod->start, mod->size);

	vm->sw.kernel_type = vm_config->os_config.kernel_type;
	vm->sw.kernel_info.kernel_src_addr = mod->start;
	if (vm->sw.kernel_info.kernel_src_addr != NULL) {
		vm->sw.kernel_info.kernel_size = mod->size;
		vm->sw.kernel_info.kernel_load_addr = get_kernel_load_addr(vm);
	}

	return (vm->sw.kernel_info.kernel_load_addr == NULL) ? (-EINVAL) : 0;
}

/* cmdline parsed from abi module string, for pre-launched VMs and SOS VM only. */
static char mod_cmdline[PRE_VM_NUM + SOS_VM_NUM][MAX_BOOTARGS_SIZE] = { '\0' };

/**
 * @pre vm != NULL && abi != NULL
 */
static void init_vm_bootargs_info(struct acrn_vm *vm, const struct acrn_boot_info *abi)
{
	struct acrn_vm_config *vm_config = get_vm_config(vm->vm_id);

	vm->sw.bootargs_info.src_addr = vm_config->os_config.bootargs;
	/* If module string of the kernel module exists, it would OVERRIDE the pre-configured build-in VM bootargs,
	 * which means we give user a chance to re-configure VM bootargs at bootloader runtime. e.g. GRUB menu
	 */
	if (mod_cmdline[vm->vm_id][0] != '\0') {
		vm->sw.bootargs_info.src_addr = &mod_cmdline[vm->vm_id][0];
	}

	if (vm_config->load_order == SOS_VM) {
		if (strncat_s((char *)vm->sw.bootargs_info.src_addr, MAX_BOOTARGS_SIZE, " ", 1U) == 0) {
			char seed_args[MAX_SEED_ARG_SIZE] = "";

			fill_seed_arg(seed_args, MAX_SEED_ARG_SIZE);
			/* Fill seed argument for SOS
			 * seed_args string ends with a white space and '\0', so no additional delimiter is needed
			 */
			if (strncat_s((char *)vm->sw.bootargs_info.src_addr, MAX_BOOTARGS_SIZE,
					seed_args, (MAX_BOOTARGS_SIZE - 1U)) != 0) {
				pr_err("failed to fill seed arg to SOS bootargs!");
			}

			/* If there is cmdline from abi->cmdline, merge it with configured SOS bootargs.
			 * This is very helpful when one of configured bootargs need to be revised at GRUB runtime
			 * (e.g. "root="), since the later one would override the previous one if multiple bootargs exist.
			 */
			if (abi->cmdline[0] != '\0') {
				if (strncat_s((char *)vm->sw.bootargs_info.src_addr, MAX_BOOTARGS_SIZE,
						abi->cmdline, (MAX_BOOTARGS_SIZE - 1U)) != 0) {
					pr_err("failed to merge mbi cmdline to SOS bootargs!");
				}
			}
		} else {
			pr_err("no space to append SOS bootargs!");
		}

	}

	vm->sw.bootargs_info.size = strnlen_s((const char *)vm->sw.bootargs_info.src_addr, MAX_BOOTARGS_SIZE);

	/* Kernel bootarg and zero page are right before the kernel image */
	if (vm->sw.bootargs_info.size > 0U) {
		vm->sw.bootargs_info.load_addr = vm->sw.kernel_info.kernel_load_addr - (MEM_1K * 8U);
	} else {
		vm->sw.bootargs_info.load_addr = NULL;
	}
}

/* @pre abi != NULL && tag != NULL
 */
static struct abi_module *get_mod_by_tag(const struct acrn_boot_info *abi, const char *tag)
{
	uint8_t i;
	struct abi_module *mod = NULL;
	struct abi_module *mods = (struct abi_module *)(&abi->mods[0]);
	uint32_t tag_len = strnlen_s(tag, MAX_MOD_TAG_LEN);

	for (i = 0U; i < abi->mods_count; i++) {
		const char *string = (char *)hpa2hva((uint64_t)(mods + i)->string);
		uint32_t str_len = strnlen_s(string, MAX_MOD_TAG_LEN);
		const char *p_chr = string + tag_len; /* point to right after the end of tag */

		/* The tag must be located at the first word in string and end with SPACE/TAB or EOL since
		 * when do file stitch by tool, the tag in string might be followed by EOL(0x0d/0x0a).
		 */
		if ((str_len >= tag_len) && (strncmp(string, tag, tag_len) == 0)
				&& (is_space(*p_chr) || is_eol(*p_chr))) {
			mod = mods + i;
			break;
		}
	}
	/* GRUB might put module at address 0 or under 1MB in the case that the module size is less then 1MB
	 * ACRN will not support these cases
	 */
	if ((mod != NULL) && (mod->start == NULL)) {
		pr_err("Unsupported module: start at HPA 0, size 0x%x .", mod->size);
		mod = NULL;
	}

	return mod;
}

/* @pre vm != NULL && abi != NULL
 */
static int32_t init_vm_sw_load(struct acrn_vm *vm, const struct acrn_boot_info *abi)
{
	struct acrn_vm_config *vm_config = get_vm_config(vm->vm_id);
	struct abi_module *mod;
	int32_t ret = -EINVAL;

	dev_dbg(DBG_LEVEL_BOOT, "mod counts=%d\n", abi->mods_count);

	/* find kernel module first */
	mod = get_mod_by_tag(abi, vm_config->os_config.kernel_mod_tag);
	if (mod != NULL) {
		const char *string = (char *)hpa2hva((uint64_t)mod->string);
		uint32_t str_len = strnlen_s(string, MAX_BOOTARGS_SIZE);
		uint32_t tag_len = strnlen_s(vm_config->os_config.kernel_mod_tag, MAX_MOD_TAG_LEN);
		const char *p_chr = string + tag_len + 1; /* point to the possible start of cmdline */

		/* check whether there is a cmdline configured in module string */
		if (((str_len > (tag_len + 1U))) && (is_space(*(p_chr - 1))) && (!is_eol(*p_chr))) {
			(void)strncpy_s(&mod_cmdline[vm->vm_id][0], MAX_BOOTARGS_SIZE,
					p_chr, (MAX_BOOTARGS_SIZE - 1U));
		}

		ret = init_vm_kernel_info(vm, mod);
	}

	if (ret == 0) {
		/* Currently VM bootargs only support Linux guest */
		if (vm->sw.kernel_type == KERNEL_BZIMAGE) {
			init_vm_bootargs_info(vm, abi);
		}
		/* check whether there is a ramdisk module */
		mod = get_mod_by_tag(abi, vm_config->os_config.ramdisk_mod_tag);
		if (mod != NULL) {
			init_vm_ramdisk_info(vm, mod);
		}

		if (is_prelaunched_vm(vm)) {
			mod = get_mod_by_tag(abi, vm_config->acpi_config.acpi_mod_tag);
			if ((mod != NULL) && (mod->size == ACPI_MODULE_SIZE)) {
				init_vm_acpi_info(vm, mod);
			} else {
				pr_err("failed to load VM %d acpi module", vm->vm_id);
			}
		}

	} else {
		pr_err("failed to load VM %d kernel module", vm->vm_id);
	}
	return ret;
}

/**
 * @param[inout] vm pointer to a vm descriptor
 *
 * @retval 0 on success
 * @retval -EINVAL on invalid parameters
 *
 * @pre vm != NULL
 */
int32_t init_vm_boot_info(struct acrn_vm *vm)
{
	struct acrn_boot_info *abi = get_acrn_boot_info();
	int32_t ret = -EINVAL;

	stac();
	ret = init_vm_sw_load(vm, abi);
	clac();

	return ret;
}
