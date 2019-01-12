/*
 * Copyright (C) 2018 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <hypervisor.h>
#include <prelaunched_vms.h>

#define INIT_VM_PCI_PTDEV_BDF(vm_idx, dev_idx, type)	\
	{	\
		.b = VM##vm_idx##_CONFIG_PCI_PTDEV##dev_idx##_##type##BUS,	\
		.d = VM##vm_idx##_CONFIG_PCI_PTDEV##dev_idx##_##type##DEV,	\
		.f = VM##vm_idx##_CONFIG_PCI_PTDEV##dev_idx##_##type##FUN,	\
	}

#define INIT_VM_PCI_PTDEV(vm_idx, dev_idx)	\
	{	\
		.vbdf.bits = INIT_VM_PCI_PTDEV_BDF(vm_idx, dev_idx, V),	\
		.pbdf.bits = INIT_VM_PCI_PTDEV_BDF(vm_idx, dev_idx, P),	\
	},

#define INIT_VM_CONFIG(idx)	\
	{		\
		.type = VM##idx##_CONFIG_TYPE,	\
		.name = VM##idx##_CONFIG_NAME,	\
		.pcpu_bitmap = VM##idx##_CONFIG_PCPU_BITMAP,	\
		.guest_flags = VM##idx##_CONFIG_FLAGS,	\
		.memory = {	\
			.start_hpa = VM##idx##_CONFIG_MEM_START_HPA,	\
			.size = VM##idx##_CONFIG_MEM_SIZE,	\
		},	\
		.pci_ptdev_num = VM##idx##_CONFIG_PCI_PTDEV_NUM,	\
		.pci_ptdevs = vm##idx##_pci_ptdevs,	\
		.os_config = {	\
			.name = VM##idx##_CONFIG_OS_NAME,	\
			.bootargs = VM##idx##_CONFIG_OS_BOOTARGS,	\
		},	\
	},

#if VM0_CONFIG_PCI_PTDEV_NUM != 0
static struct acrn_vm_pci_ptdev_config vm0_pci_ptdevs[VM0_CONFIG_PCI_PTDEV_NUM] = {
#ifdef VM0_CONFIG_PCI_PTDEV0_CONFIGURED
	INIT_VM_PCI_PTDEV(0, 0)
#endif
#ifdef VM0_CONFIG_PCI_PTDEV1_CONFIGURED
	INIT_VM_PCI_PTDEV(0, 1)
#endif
#ifdef VM0_CONFIG_PCI_PTDEV2_CONFIGURED
	INIT_VM_PCI_PTDEV(0, 2)
#endif
#ifdef VM0_CONFIG_PCI_PTDEV3_CONFIGURED
	INIT_VM_PCI_PTDEV(0, 3)
#endif
};
#endif

#if VM1_CONFIG_PCI_PTDEV_NUM != 0
static struct acrn_vm_pci_ptdev_config vm1_pci_ptdevs[VM1_CONFIG_PCI_PTDEV_NUM] = {
#ifdef VM1_CONFIG_PCI_PTDEV0_CONFIGURED
	INIT_VM_PCI_PTDEV(1, 0)
#endif
#ifdef VM1_CONFIG_PCI_PTDEV1_CONFIGURED
	INIT_VM_PCI_PTDEV(1, 1)
#endif
#ifdef VM1_CONFIG_PCI_PTDEV2_CONFIGURED
	INIT_VM_PCI_PTDEV(1, 2)
#endif
#ifdef VM1_CONFIG_PCI_PTDEV3_CONFIGURED
	INIT_VM_PCI_PTDEV(1, 3)
#endif
};
#endif

#if VM2_CONFIG_PCI_PTDEV_NUM != 0
static struct acrn_vm_pci_ptdev_config vm2_pci_ptdevs[VM2_CONFIG_PCI_PTDEV_NUM] = {
#ifdef VM2_CONFIG_PCI_PTDEV0_CONFIGURED
	INIT_VM_PCI_PTDEV(2, 0)
#endif
#ifdef VM2_CONFIG_PCI_PTDEV1_CONFIGURED
	INIT_VM_PCI_PTDEV(2, 1)
#endif
#ifdef VM2_CONFIG_PCI_PTDEV2_CONFIGURED
	INIT_VM_PCI_PTDEV(2, 2)
#endif
#ifdef VM2_CONFIG_PCI_PTDEV3_CONFIGURED
	INIT_VM_PCI_PTDEV(2, 3)
#endif
};
#endif

#if VM3_CONFIG_PCI_PTDEV_NUM != 0

static struct acrn_vm_pci_ptdev_config vm3_pci_ptdevs[VM3_CONFIG_PCI_PTDEV_NUM] = {
#ifdef VM3_CONFIG_PCI_PTDEV0_CONFIGURED
	INIT_VM_PCI_PTDEV(3, 0)
#endif
#ifdef VM3_CONFIG_PCI_PTDEV1_CONFIGURED
	INIT_VM_PCI_PTDEV(3, 1)
#endif
#ifdef VM3_CONFIG_PCI_PTDEV2_CONFIGURED
	INIT_VM_PCI_PTDEV(3, 2)
#endif
#ifdef VM3_CONFIG_PCI_PTDEV3_CONFIGURED
	INIT_VM_PCI_PTDEV(3, 3)
#endif
};
#endif

struct acrn_vm_config vm_configs[CONFIG_MAX_VM_NUM] = {
#ifdef VM0_CONFIGURED
	INIT_VM_CONFIG(0)
#endif

#ifdef VM1_CONFIGURED
	INIT_VM_CONFIG(1)
#endif

#ifdef VM2_CONFIGURED
	INIT_VM_CONFIG(2)
#endif

#ifdef VM3_CONFIGURED
	INIT_VM_CONFIG(3)
#endif
};
