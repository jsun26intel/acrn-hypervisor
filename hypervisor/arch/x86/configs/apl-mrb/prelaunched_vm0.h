/*
 * Copyright (C) 2018 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PRELAUNCHED_VM0_H
#define PRELAUNCHED_VM0_H

#define	VM0_CONFIGURED

#define VM0_CONFIG_NAME				"ACRN PARTITION VM 1 for APL-MRB"
#define VM0_CONFIG_TYPE				PRE_LAUNCHED_VM
#define VM0_CONFIG_PCPU_BITMAP			(PLUG_CPU(0) | PLUG_CPU(2))
#define VM0_CONFIG_FLAGS			0U
#define VM0_CONFIG_MEM_START_HPA		0x100000000UL
#define VM0_CONFIG_MEM_SIZE			0x20000000UL

#define VM0_CONFIG_OS_NAME			"ClearLinux 26600"
#define VM0_CONFIG_OS_BOOTARGS			"root=/dev/sda3 rw rootwait noxsave maxcpus=2 nohpet console=hvc0 \
						console=ttyS2 no_timer_check ignore_loglevel log_buf_len=16M \
						consoleblank=0 tsc=reliable xapic_phys"

#define VM0_CONFIG_PCI_PTDEV_NUM		2U

#define VM0_CONFIG_PCI_PTDEV0_CONFIGURED
#define VM0_CONFIG_PCI_PTDEV0_NAME		"Hostbridge"
#define VM0_CONFIG_PCI_PTDEV0_VBUS		0x00U
#define VM0_CONFIG_PCI_PTDEV0_VDEV		0x00U
#define VM0_CONFIG_PCI_PTDEV0_VFUN		0x00U
#define VM0_CONFIG_PCI_PTDEV0_PBUS		0x00U
#define VM0_CONFIG_PCI_PTDEV0_PDEV		0x00U
#define VM0_CONFIG_PCI_PTDEV0_PFUN		0x00U

#define VM0_CONFIG_PCI_PTDEV1_CONFIGURED
#define VM0_CONFIG_PCI_PTDEV1_NAME		"SATA Controller"
#define VM0_CONFIG_PCI_PTDEV1_VBUS		0x00U
#define VM0_CONFIG_PCI_PTDEV1_VDEV		0x01U
#define VM0_CONFIG_PCI_PTDEV1_VFUN		0x00U
#define VM0_CONFIG_PCI_PTDEV1_PBUS		0x00U
#define VM0_CONFIG_PCI_PTDEV1_PDEV		0x12U
#define VM0_CONFIG_PCI_PTDEV1_PFUN		0x00U

#endif /* PRELAUNCHED_VM0_H */
