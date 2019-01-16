/*
 * Copyright (C) 2018 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <hypervisor.h>
#include <e820.h>

/* Number of CPUs in VM1 */
#define VM1_NUM_CPUS    2U

/* Logical CPU IDs assigned to this VM */
uint16_t VM1_CPUS[VM1_NUM_CPUS] = {0U, 2U};

/* Number of CPUs in VM2 */
#define VM2_NUM_CPUS    2U

/* Logical CPU IDs assigned with this VM */
uint16_t VM2_CPUS[VM2_NUM_CPUS] = {3U, 1U};

static struct acrn_vm_pci_ptdev_config vm0_pci_ptdevs[2] = {
	{
		.vbdf.bits = {.b = 0x00U, .d = 0x00U, .f = 0x00U},
		.pbdf.bits = {.b = 0x00U, .d = 0x00U, .f = 0x00U},
	},
	{
		.vbdf.bits = {.b = 0x00U, .d = 0x01U, .f = 0x00U},
		.pbdf.bits = {.b = 0x00U, .d = 0x12U, .f = 0x00U},
	},
};

static struct acrn_vm_pci_ptdev_config vm1_pci_ptdevs[3] = {
	{
		.vbdf.bits = {.b = 0x00U, .d = 0x00U, .f = 0x00U},
		.pbdf.bits = {.b = 0x00U, .d = 0x00U, .f = 0x00U},
	},
	{
		.vbdf.bits = {.b = 0x00U, .d = 0x01U, .f = 0x00U},
		.pbdf.bits = {.b = 0x00U, .d = 0x15U, .f = 0x00U},
	},
	{
		.vbdf.bits = {.b = 0x00U, .d = 0x02U, .f = 0x00U},
		.pbdf.bits = {.b = 0x02U, .d = 0x00U, .f = 0x00U},
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
				.pcpu_bitmap = (PLUG_CPU(0) | PLUG_CPU(2)),
				.memory.start_hpa = 0x100000000UL,
				.memory.size = 0x20000000UL, /* uses contiguous memory from host */
				.os_config.bootargs = "root=/dev/sda3 rw rootwait noxsave maxcpus=2 nohpet console=hvc0 \
						console=ttyS2 no_timer_check ignore_loglevel log_buf_len=16M \
						consoleblank=0 tsc=reliable xapic_phys",
				.pci_ptdev_num = 2,
				.pci_ptdevs = vm0_pci_ptdevs,
			},

			{
				.type = PRE_LAUNCHED_VM,
				.pcpu_bitmap = (PLUG_CPU(1) | PLUG_CPU(3)),
				.guest_flags = LAPIC_PASSTHROUGH,
				.memory.start_hpa = 0x120000000UL,
				.memory.size = 0x20000000UL, /* uses contiguous memory from host */
				.os_config.bootargs = "root=/dev/sda3 rw rootwait noxsave maxcpus=2 nohpet console=hvc0 \
						console=ttyS2 no_timer_check ignore_loglevel log_buf_len=16M \
						consoleblank=0 tsc=reliable xapic_phys",
				.pci_ptdev_num = 3,
				.pci_ptdevs = vm1_pci_ptdevs,
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
		.is_bsp = true,
	},
};
