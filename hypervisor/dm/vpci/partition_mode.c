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
	struct vpci_vdev_array *vdev_array;
	struct pci_vdev *vdev;
	int32_t i;

	vdev_array = vpci->vm->vm_desc->vpci_vdev_array;
	for (i = 0; i < vdev_array->num_pci_vdev; i++) {
		vdev = &vdev_array->vpci_vdev_list[i];
		if (vdev->vbdf.value == vbdf.value) {
			return vdev;
		}
	}

	return NULL;
}

/* get pdev bar address and load bar size and type to vdev */
void partition_mode_bar_init(struct pci_vdev *vdev)
{
	uint64_t size;
	uint16_t orig_cmd;
	uint8_t	type, idx;

	orig_cmd = pci_pdev_read_cfg(vdev->pdev.bdf, PCIR_COMMAND, 2U);
	if ((orig_cmd & PCIM_CMD_MEMORY) != 0U ){
		/* Disable memory decode before sizing BAR */
		pci_pdev_write_cfg(vdev->pdev.bdf, PCIR_COMMAND, 2U, orig_cmd & ~PCIM_CMD_MEMORY);
	}

	for (idx = 0; idx < PCI_BAR_COUNT; idx++) {
		vpci_cfgbar_decode(vdev, idx);

		type = vdev->pdev.bar[idx].type;
		size = vdev->pdev.bar[idx].size;
		if (size > 0xffffffffU) {
			pr_err("PCI BAR size is too big to support.");
			size = 0U;
		}
		if ((size == 0U) || (type == PCIBAR_IO_SPACE) || (type == PCIBAR_NONE)) {
			if (type == PCIBAR_MEM64) {
				idx++;
			}
			continue;
		}

		vdev->bar[idx].size = (size < 0x1000U) ? 0x1000U : size;
		vdev->bar[idx].type = PCIBAR_MEM32;
		pr_dbg("%x:%x.%x bar[%d].pbase = %llx, psize = %llx, vsize = %llx, ptype = %d \n",
			vdev->pdev.bdf.bits.b, vdev->pdev.bdf.bits.d, vdev->pdev.bdf.bits.f,
			idx, vdev->pdev.bar[idx].base, vdev->pdev.bar[idx].size,
			vdev->bar[idx].size, vdev->pdev.bar[idx].type);

		if (type == PCIBAR_MEM64) {
			idx++;
		}
	}
	/* Restore pci cmd */
	pci_pdev_write_cfg(vdev->pdev.bdf, PCIR_COMMAND, 2U, orig_cmd);
}

static int32_t partition_mode_vpci_init(const struct acrn_vm *vm)
{
	struct vpci_vdev_array *vdev_array;
	const struct acrn_vpci *vpci = &vm->vpci;
	struct pci_vdev *vdev;
	int32_t i;

	vdev_array = vm->vm_desc->vpci_vdev_array;

	for (i = 0; i < vdev_array->num_pci_vdev; i++) {
		vdev = &vdev_array->vpci_vdev_list[i];
		vdev->vpci = vpci;

		if (vdev->vbdf.value != 0U) {
			partition_mode_bar_init(vdev);
			vdev->ops = &pci_ops_vdev_pt;
		} else {
			vdev->ops = &pci_ops_vdev_hostbridge;
		}

		if (vdev->ops->init != NULL) {
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
	struct vpci_vdev_array *vdev_array;
	struct pci_vdev *vdev;
	int32_t i;

	vdev_array = vm->vm_desc->vpci_vdev_array;

	for (i = 0; i < vdev_array->num_pci_vdev; i++) {
		vdev = &vdev_array->vpci_vdev_list[i];
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
