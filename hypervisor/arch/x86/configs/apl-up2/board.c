/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <board.h>
#include <msr.h>
#include <vtd.h>

#ifndef CONFIG_ACPI_PARSE_ENABLED
#error "DMAR info is not available, please set ACPI_PARSE_ENABLED to y in Kconfig. \
	Or use acrn-config tool to generate platform DMAR info."
#endif

struct dmar_info plat_dmar_info;

struct platform_clos_info platform_clos_array[MAX_PLATFORM_CLOS_NUM] = {
	{
		.clos_mask = 0xff,
		.msr_index = MSR_IA32_L2_MASK_BASE,
	},
	{
		.clos_mask = 0xff,
		.msr_index = MSR_IA32_L2_MASK_BASE + 1U,
	},
	{
		.clos_mask = 0xff,
		.msr_index = MSR_IA32_L2_MASK_BASE + 2U,
	},
	{
		.clos_mask = 0xff,
		.msr_index = MSR_IA32_L2_MASK_BASE + 3U,
	},
};

const struct cpu_state_table board_cpu_state_tbl;
