/*-
* Copyright (c) 2011 NetApp, Inc.
* Copyright (c) 2018 Intel Corporation
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY NETAPP, INC ``AS IS'' AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL NETAPP, INC OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
* OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
* SUCH DAMAGE.
*
* $FreeBSD$
*/

/* Virtual PCI device related operations (read/write, etc) */

#include <hypervisor.h>
#include "pci_priv.h"

static struct pci_vdev *partition_mode_find_vdev(struct acrn_vpci *vpci, union pci_bdf vbdf)
{
	struct pci_vdev *vdev = NULL;
	int32_t i;

	for (i = 0; i < vpci->vm->vm_config->pci_ptdev_num; i++) {
		vdev = vpci->vm->pci_ptdevs + i;
		if (vdev->vbdf.value == vbdf.value) {
			break;
		}
		vdev = NULL;
	}

	return vdev;
}

/* get pdev bar address and load bar size and type to vdev */
void partition_mode_bar_init(struct pci_vdev *vdev)
{
	uint64_t base, size;
	uint32_t base_hi, size_hi, bar_offset;
	uint16_t orig_cmd;
	uint8_t	type, idx;

	orig_cmd = pci_pdev_read_cfg(vdev->pdev.bdf, PCIR_COMMAND, 2U);
	if ((orig_cmd & PCIM_CMD_MEMORY) != 0U ){
		/* Disable memory decode before sizing BAR */
		pci_pdev_write_cfg(vdev->pdev.bdf, PCIR_COMMAND, 2U, orig_cmd & ~PCIM_CMD_MEMORY);
	}

	for (idx = 0; idx < PCI_BAR_COUNT; idx++) {
		bar_offset = pci_bar_offset(idx);
		base = pci_pdev_read_cfg(vdev->pdev.bdf, bar_offset, 4U);
		if ((base & PCIM_BAR_SPACE) == PCIM_BAR_IO_SPACE) {
			continue;
		}
		type = (uint8_t)(base & PCIM_BAR_MEM_TYPE);
		base &= PCIM_BAR_MEM_BASE;
		if (type == PCIM_BAR_MEM_32) {
			pci_pdev_write_cfg(vdev->pdev.bdf, bar_offset, 4U, (uint32_t)~0U);
			size = pci_pdev_read_cfg(vdev->pdev.bdf, bar_offset, 4U);
			pci_pdev_write_cfg(vdev->pdev.bdf, bar_offset, 4U, (uint32_t)base);
			size &= PCIM_BAR_MEM_BASE;
			size &= (~size + 1U);
			if (size == (uint32_t)~0U) {
				size = 0U;
			}
			if (size == 0U) {
				continue;
			}
		} else if (type == PCIM_BAR_MEM_64) {
			base_hi = pci_pdev_read_cfg(vdev->pdev.bdf, (bar_offset + 4U), 4U);
			base |= ((uint64_t)base_hi << 32U);
			pci_pdev_write_cfg(vdev->pdev.bdf, bar_offset, 4U, (uint32_t)~0U);
			size = pci_pdev_read_cfg(vdev->pdev.bdf, bar_offset, 4U);
			pci_pdev_write_cfg(vdev->pdev.bdf, (bar_offset + 4U), 4U, (uint32_t)~0U);
			size_hi = pci_pdev_read_cfg(vdev->pdev.bdf, (bar_offset + 4U), 4U);
			pci_pdev_write_cfg(vdev->pdev.bdf, bar_offset, 4U, (uint32_t)(base & PCIM_BAR_MEM_BASE));
			pci_pdev_write_cfg(vdev->pdev.bdf, (bar_offset + 4U), 4U, base_hi);

			size |= ((uint64_t)size_hi << 32U);
			size &= (((uint64_t)~0U << 32U) | PCIM_BAR_MEM_BASE);
			size = (~size + 1U);
			if (size > 0xffffffffU) {
				pr_err("PCI BAR size is too big to support.");
				size = 0U;
			}
			if (size == 0U) {
				idx++;
				continue;
			}
		}
		vdev->pdev.bar[idx].base = base;
		vdev->pdev.bar[idx].size = size;
		vdev->pdev.bar[idx].type = (type == PCIM_BAR_MEM_64) ? PCIBAR_MEM64 : PCIBAR_MEM32;
		vdev->bar[idx].size = (size < 0x1000U) ? 0x1000U : size;
		vdev->bar[idx].type = PCIBAR_MEM32;
		pr_dbg("%x:%x.%x bar[%d].pbase = %llx, psize = %llx, vsize = %llx, ptype = %d \n",
			vdev->pdev.bdf.bits.b, vdev->pdev.bdf.bits.d, vdev->pdev.bdf.bits.f,
			idx, vdev->pdev.bar[idx].base, vdev->pdev.bar[idx].size,
			vdev->bar[idx].size, vdev->pdev.bar[idx].type);

		if (type == PCIM_BAR_MEM_64) {
			idx++;
		}
	}
}

static int32_t partition_mode_vpci_init(const struct acrn_vm *vm)
{
	struct acrn_vm_pci_ptdev_config *ptdev_configs;
	const struct acrn_vpci *vpci = &vm->vpci;
	struct pci_vdev *vdev;
	int32_t i;
	uint8_t hdr_type;

	for (i = 0; i < vm->vm_config->pci_ptdev_num; i++) {
		/* init pci_ptdevs[] in acrn_vm struct */
		vdev = (struct pci_vdev *)vm->pci_ptdevs + i;
		ptdev_configs = vm->vm_config->pci_ptdevs + i;
		vdev->vpci = vpci;
		vdev->vbdf.value = ptdev_configs->vbdf.value;
		vdev->pdev.bdf.value = ptdev_configs->pbdf.value;

		hdr_type = (uint8_t)pci_pdev_read_cfg(vdev->pdev.bdf, PCIR_HDRTYPE, 1U);
		if ((hdr_type & PCIM_HDRTYPE) == PCIM_HDRTYPE_NORMAL) {
			partition_mode_bar_init(vdev);
			vdev->ops = &pci_ops_vdev_pt;
		} else if ((hdr_type & PCIM_HDRTYPE) == PCIM_HDRTYPE_BRIDGE) {
			vdev->ops = &pci_ops_vdev_hostbridge;
		}

		if ((vdev->ops != NULL) && (vdev->ops->init != NULL)) {
			if (vdev->ops->init(vdev) != 0) {
				pr_err("%s() failed at PCI device (bdf %x)!", __func__,
					vdev->vbdf);
			}
		}
	}

	return 0;
}

static void partition_mode_vpci_deinit(const struct acrn_vm *vm)
{
	struct pci_vdev *vdev;
	int32_t i;

	for (i = 0; i < vm->vm_config->pci_ptdev_num; i++) {
		vdev = (struct pci_vdev *)vm->pci_ptdevs + i;
		if ((vdev->ops != NULL) && (vdev->ops->deinit != NULL)) {
			if (vdev->ops->deinit(vdev) != 0) {
				pr_err("vdev->ops->deinit failed!");
			}
		}
	}
}

static void partition_mode_cfgread(struct acrn_vpci *vpci, union pci_bdf vbdf,
	uint32_t offset, uint32_t bytes, uint32_t *val)
{
	struct pci_vdev *vdev = partition_mode_find_vdev(vpci, vbdf);
	if ((vdev != NULL) && (vdev->ops != NULL)
			&& (vdev->ops->cfgread != NULL)) {
		(void)vdev->ops->cfgread(vdev, offset, bytes, val);
	}
}

static void partition_mode_cfgwrite(struct acrn_vpci *vpci, union pci_bdf vbdf,
	uint32_t offset, uint32_t bytes, uint32_t val)
{
	struct pci_vdev *vdev = partition_mode_find_vdev(vpci, vbdf);
	if ((vdev != NULL) && (vdev->ops != NULL)
			&& (vdev->ops->cfgwrite != NULL)) {
		(void)vdev->ops->cfgwrite(vdev, offset, bytes, val);
	}
}

const struct vpci_ops partition_mode_vpci_ops = {
	.init = partition_mode_vpci_init,
	.deinit = partition_mode_vpci_deinit,
	.cfgread = partition_mode_cfgread,
	.cfgwrite = partition_mode_cfgwrite,
};
