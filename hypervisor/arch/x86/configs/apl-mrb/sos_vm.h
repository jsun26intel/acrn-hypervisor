/*
 * Copyright (C) 2018 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOS_VM_CONFIG_H
#define SOS_VM_CONFIG_H

#define SOS_VM_CONFIG_NAME		"ACRN SOS VM for APL-MRB"
#define SOS_VM_CONFIG_MEM_SIZE		0x200000000UL
#define SOS_VM_CONFIG_PCPU_BITMAP	(PLUG_CPU(1) | PLUG_CPU(2) | PLUG_CPU(3) | PLUG_CPU(4))

#define SOS_VM_CONFIG_OS_NAME		"ClearLinux 26600"

#endif /* SOS_VM_CONFIG_H */
