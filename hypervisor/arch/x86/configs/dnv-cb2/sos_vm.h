/*
 * Copyright (C) 2018 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SOS_VM_CONFIG_H
#define SOS_VM_CONFIG_H

#define SOS_VM_CONFIG_NAME		"ACRN SOS VM for DNV-CB2"
#define SOS_VM_CONFIG_MEM_SIZE		0x400000000UL
#define SOS_VM_CONFIG_PCPU_BITMAP	(PLUG_CPU(1) | PLUG_CPU(2) | PLUG_CPU(3) | PLUG_CPU(4)	\
					| PLUG_CPU(5) | PLUG_CPU(6) | PLUG_CPU(7) | PLUG_CPU(8))

#define SOS_VM_CONFIG_OS_NAME		"ClearLinux 26600"

#endif /* SOS_VM_CONFIG_H */
