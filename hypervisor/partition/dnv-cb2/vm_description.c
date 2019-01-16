/*
 * Copyright (C) 2018 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <hypervisor.h>
#include <e820.h>

/* Number of CPUs in VM1*/
#define VM1_NUM_CPUS    4U

/* Logical CPU IDs assigned to this VM */
uint16_t VM1_CPUS[VM1_NUM_CPUS] = {0U, 2U, 4U, 6U};

/* Number of CPUs in VM2*/
#define VM2_NUM_CPUS    4U

/* Logical CPU IDs assigned with this VM */
uint16_t VM2_CPUS[VM2_NUM_CPUS] = {7U, 5U, 3U, 1U};

static struct acrn_vm_pci_ptdev_config vm0_pci_ptdevs[3] = {
	{
		.vbdf.bits = {.b = 0x00U, .d = 0x00U, .f = 0x00U},
		.pbdf.bits = {.b = 0x00U, .d = 0x00U, .f = 0x00U},
	},
	{
		.vbdf.bits = {.b = 0x00U, .d = 0x01U, .f = 0x00U},
		.pbdf.bits = {.b = 0x03U, .d = 0x00U, .f = 0x01U},
	},
	{
		.vbdf.bits = {.b = 0x00U, .d = 0x02U, .f = 0x00U},
		.pbdf.bits = {.b = 0x00U, .d = 0x15U, .f = 0x00U},
	},
};

static struct acrn_vm_pci_ptdev_config vm1_pci_ptdevs[3] = {
	{
		.vbdf.bits = {.b = 0x00U, .d = 0x00U, .f = 0x00U},
		.pbdf.bits = {.b = 0x00U, .d = 0x00U, .f = 0x00U},
	},
	{
		.vbdf.bits = {.b = 0x00U, .d = 0x01U, .f = 0x00U},
		.pbdf.bits = {.b = 0x00U, .d = 0x14U, .f = 0x00U},
	},
	{
		.vbdf.bits = {.b = 0x00U, .d = 0x02U, .f = 0x00U},
		.pbdf.bits = {.b = 0x03U, .d = 0x00U, .f = 0x00U},
	},
};

/*******************************/
/* User Defined VM definitions */
/*******************************/
struct vm_config_array vm_config_partition = {

		/* Virtual Machine descriptions */
		.vm_config_array = {
			{
				.type = PRE_LAUNCHED_VM,
				.pcpu_bitmap = (PLUG_CPU(0) | PLUG_CPU(2) | PLUG_CPU(4) | PLUG_CPU(6)),
				.guest_flags = LAPIC_PASSTHROUGH,
				.memory.start_hpa = 0x100000000UL,
				.memory.size = 0x80000000UL, /* uses contiguous memory from host */
				.os_config.bootargs = "root=/dev/sda rw rootwait noxsave maxcpus=4 nohpet console=hvc0 " \
						"console=ttyS0 no_timer_check ignore_loglevel log_buf_len=16M "\
						"consoleblank=0 tsc=reliable xapic_phys  apic_debug",
				.pci_ptdev_num = 3,
				.pci_ptdevs = vm0_pci_ptdevs,
				.vpci_vdev_array = &vpci_vdev_array1,
			},

			{
				.type = PRE_LAUNCHED_VM,
				.pcpu_bitmap = (PLUG_CPU(1) | PLUG_CPU(3) | PLUG_CPU(5) | PLUG_CPU(7)),
				.guest_flags = LAPIC_PASSTHROUGH,
				.memory.start_hpa = 0x180000000UL,
				.memory.size = 0x80000000UL, /* uses contiguous memory from host */
				.os_config.bootargs = "root=/dev/sda2 rw rootwait noxsave maxcpus=4 nohpet console=hvc0 "\
						"console=ttyS0 no_timer_check ignore_loglevel log_buf_len=16M "\
						"consoleblank=0 tsc=reliable xapic_phys apic_debug",
				.pci_ptdev_num = 3,
				.pci_ptdevs = vm1_pci_ptdevs,
				.vpci_vdev_array = &vpci_vdev_array2,
			},
		}
};

const struct pcpu_vm_config_mapping pcpu_vm_config_map[] = {
	{
		.vm_config_ptr = &vm_config_partition.vm_config_array[0],
		.is_bsp = true,
	},
	{
		.vm_config_ptr = &vm_config_partition.vm_config_array[1],
		.is_bsp = false,
	},
	{
		.vm_config_ptr = &vm_config_partition.vm_config_array[0],
		.is_bsp = false,
	},
	{
		.vm_config_ptr = &vm_config_partition.vm_config_array[1],
		.is_bsp = false,
	},
	{
		.vm_config_ptr = &vm_config_partition.vm_config_array[0],
		.is_bsp = false,
	},
	{
		.vm_config_ptr = &vm_config_partition.vm_config_array[1],
		.is_bsp = false,
	},
	{
		.vm_config_ptr = &vm_config_partition.vm_config_array[0],
		.is_bsp = false,
	},
	{
		.vm_config_ptr = &vm_config_partition.vm_config_array[1],
		.is_bsp = true,
	},
};
