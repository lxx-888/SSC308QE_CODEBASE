/*
 * drv_pcie_epc.c - Sigmastar
 *
 * Copyright (c) [2019~2020] SigmaStar Technology.
 *
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 */
#include <linux/types.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include "asm/arch/mach/platform.h"
#include "drv_pcieif.h"
#include "hal_pcie.h"
#include "drv_pcie.h"
#include "drv_pcie_epc.h"

//=================================================================================================
// Macro & Define
//=================================================================================================
#define EPC_MEM_PAGE_SZ (0x1000) // 4KB

#define EPC_FUNC_ENTER() PCIE_VERBOSE("%s+\r\n", __FUNCTION__)
#define EPC_FUNC_EXIT()  PCIE_VERBOSE("%s-\r\n", __FUNCTION__)

static u32 bar_fixsize[] = {
    HAL_PCIE_BAR0_FIXSZ, HAL_PCIE_BAR1_FIXSZ, HAL_PCIE_BAR2_FIXSZ,
    HAL_PCIE_BAR3_FIXSZ, HAL_PCIE_BAR4_FIXSZ, HAL_PCIE_BAR5_FIXSZ,
};
//=================================================================================================
// Local Variable
//=================================================================================================

//=================================================================================================
// Private Fucntions
//=================================================================================================
static void _Drv_PCIE_EpcMemInit(PCIE_EPC *epc, u32 base, u32 size, u32 page_size)
{
    PCIE_EPC_MEM *mem = &epc->mem;

    EPC_FUNC_ENTER();

    if (page_size < EPC_MEM_PAGE_SZ)
        page_size = EPC_MEM_PAGE_SZ;

    mem->phys_base = base;
    mem->page_size = page_size;
    mem->size      = size;
    mem->offset    = 0;

    EPC_FUNC_EXIT();
}

static void *_Drv_PCIE_EpcMemAlloc(PCIE_EPC *epc, u32 size, phys_addr_t *phys_addr)
{
    PCIE_EPC_MEM *mem   = &epc->mem;
    void *        space = NULL;

    EPC_FUNC_ENTER();

    size = EPC_MEM_ALIGN(size, mem->page_size);
    if ((mem->offset + size) < mem->size)
    {
        *phys_addr = mem->phys_base + mem->offset;
        mem->offset += size;
        space = (void *)*phys_addr; // 1:1 mapping
        PCIE_INFO("epc mem alloc: phys 0x%llx, virt 0x%p, size %d\r\n", *phys_addr, space, size);
    }

    EPC_FUNC_EXIT();

    return space;
}

static void _Drv_PCIE_EpcResetBar(u8 id, enum pci_barno bar, int flags)
{
    u32 reg;

    EPC_FUNC_ENTER();

    reg = PCI_BASE_ADDRESS_0 + (4 * bar);
    _pcie_dbi_ro_wr_en(id);
    _pcie_writel_dbi2(id, reg, 0x0);
    sstar_pcieif_writel_dbi(id, reg, 0x0);
    if (flags & PCI_BASE_ADDRESS_MEM_TYPE_64)
    {
        _pcie_writel_dbi2(id, reg + 4, 0x0);
        sstar_pcieif_writel_dbi(id, reg + 4, 0x0);
    }
    _pcie_dbi_ro_wr_dis(id);
    PCIE_DBG("epc reset bar%d to x%x\r\n", bar, sstar_pcieif_readl_dbi(id, reg));

    EPC_FUNC_EXIT();
}

static u8 _Drv_PCIE_EpcFindNextCap(u8 id, u8 cap_ptr, u8 cap)
{
    u8  cap_id, next_cap_ptr;
    u16 reg;

    if (!cap_ptr)
        return 0;

    reg    = sstar_pcieif_readw_dbi(id, cap_ptr);
    cap_id = (reg & 0x00ff);

    if (cap_id > PCI_CAP_ID_MAX)
        return 0;

    if (cap_id == cap)
        return cap_ptr;

    next_cap_ptr = (reg & 0xff00) >> 8;
    return _Drv_PCIE_EpcFindNextCap(id, next_cap_ptr, cap);
}

static u8 _Drv_PCIE_EpcFindCapability(u8 id, u8 cap)
{
    u8  next_cap_ptr;
    u16 reg;

    reg          = sstar_pcieif_readw_dbi(id, PCI_CAPABILITY_LIST);
    next_cap_ptr = (reg & 0x00ff);

    return _Drv_PCIE_EpcFindNextCap(id, next_cap_ptr, cap);
}

static void _Drv_PCIE_EpcSetup(PCIE_EP *ep)
{
    u32 ctl = 0, width = 0;
    u8  id = ep->port_id;

    EPC_FUNC_ENTER();

    sstar_pcieif_set_mode(id, PCIE_IF_MODE_EP);

    /* Set the number of lanes */
    switch (ep->num_lanes)
    {
        case 1:
            ctl   = SNPS_LINK_MODE_1_LANES;
            width = SNPS_LINK_WIDTH_1_LANES;
            break;
        case 2:
            ctl   = SNPS_LINK_MODE_2_LANES;
            width = SNPS_LINK_WIDTH_2_LANES;
            break;
        case 4:
            ctl   = SNPS_LINK_MODE_4_LANES;
            width = SNPS_LINK_WIDTH_4_LANES;
            break;
        case 8:
            ctl   = SNPS_LINK_MODE_8_LANES;
            width = SNPS_LINK_WIDTH_8_LANES;
            break;
        default:
            PCIE_ERR("invalid number of lanes\r\n");
            return;
    }
    sstar_pcieif_writel_dbi(id, SNPS_LINK_CONTROL,
                            ctl | (sstar_pcieif_readl_dbi(id, SNPS_LINK_CONTROL) & ~SNPS_LINK_MODE_MASK));

    sstar_pcieif_writel_dbi(id, SNPS_LINK_WIDTH_SPEED_CONTROL,
                            width | SNPS_SPEED_CHANGE | SNPS_AUTO_LANE_FLIP | SNPS_FAST_TRAIN_SEQ(0x2C));
    sstar_pcieif_writel_dbi(id, SNPS_TRGT_MAP_CTRL, sstar_pcieif_readl_dbi(id, SNPS_TRGT_MAP_CTRL) | 0x3F);

    EPC_FUNC_EXIT();
}

static void _Drv_PCIE_EpcDisableAtu(u8 id, int index, u8 is_outbound)
{
    u32 offset;

    EPC_FUNC_ENTER();

    offset = is_outbound ? EPC_ATU_OB_UNR_REG_OFFSET(index) : EPC_ATU_IB_UNR_REG_OFFSET(index);
    _pcie_writel_atu(id, PCIE_ATU_UNR_REGION_CTRL2 + offset, 0);

    EPC_FUNC_EXIT();
}

static int _Drv_PCIE_EpcProgInboundAtu(u8 id, int index, int bar, u64 cpu_addr, int type)
{
    u32 retries, val;
    u32 offset = EPC_ATU_IB_UNR_REG_OFFSET(index);
    int ret    = -EBUSY;

    EPC_FUNC_ENTER();

    _pcie_writel_atu(id, offset + PCIE_ATU_UNR_LOWER_TARGET, lower_32_bits(cpu_addr));
    _pcie_writel_atu(id, offset + PCIE_ATU_UNR_UPPER_TARGET, upper_32_bits(cpu_addr));
    _pcie_writel_atu(id, offset + PCIE_ATU_UNR_REGION_CTRL1, type);
    _pcie_writel_atu(id, offset + PCIE_ATU_UNR_REGION_CTRL2, PCIE_ATU_ENABLE | PCIE_ATU_BAR_MODE_ENABLE | (bar << 8));

    /*
     * Make sure ATU enable takes effect before any subsequent config
     * and I/O accesses.
     */
    for (retries = 0; retries < WAIT_IATU_RETRIES; retries++)
    {
        val = _pcie_readl_atu(id, offset + PCIE_ATU_UNR_REGION_CTRL2);
        if (val & PCIE_ATU_ENABLE)
        {
            ret = 0;
            break;
        }

        mdelay(WAIT_IATU_DELAY);
    }
    if (ret)
    {
        PCIE_ERR("Inbound iATU can't be enabled\n");
    }

    EPC_FUNC_EXIT();

    return ret;
}

static int _Drv_PCIE_EpcInboundAtu(PCIE_EP *ep, enum pci_barno bar, phys_addr_t addr, int type)
{
    u32 i, bitmap = 0;
    u8  free_win;
    int ret;

    EPC_FUNC_ENTER();

    for (i = 0; i < HAL_PCIE_IB_WIN_NUM; i++)
        bitmap |= 1 << i;

    for (i = 0; i < 6; i++)
    {
        if (ep->bar_to_atu[i] != 0xFF)
        {
            bitmap &= ~(1 << ep->bar_to_atu[i]);
        }
    }
    if (bitmap == 0)
    {
        PCIE_ERR("No inbound iATU available\n");
        return -ENOSR;
    }

    free_win = 0;
    while (bitmap)
    {
        if (bitmap & 0x1)
        {
            break;
        }
        bitmap = bitmap >> 1;
        free_win++;
    }

    PCIE_INFO("epc ib atu[%d]: bar %d addr 0x%llx type %d\r\n", free_win, bar, addr, type);
    ret = _Drv_PCIE_EpcProgInboundAtu(ep->port_id, free_win, bar, addr, type);
    if (ret)
        return ret;

    ep->bar_to_atu[bar] = free_win;

    EPC_FUNC_EXIT();

    return 0;
}

static int _Drv_PCIE_EpcProgOutboundAtu(u8 id, int index, int type, u64 cpu_addr, u64 pci_addr, u32 size)
{
    u32 retries, val;
    u32 offset = EPC_ATU_OB_UNR_REG_OFFSET(index);
    int ret    = -EBUSY;

    EPC_FUNC_ENTER();

    _pcie_writel_atu(id, offset + PCIE_ATU_UNR_LOWER_BASE, lower_32_bits(cpu_addr));
    _pcie_writel_atu(id, offset + PCIE_ATU_UNR_UPPER_BASE, upper_32_bits(cpu_addr));
    _pcie_writel_atu(id, offset + PCIE_ATU_UNR_LIMIT, lower_32_bits(cpu_addr + size - 1));
    _pcie_writel_atu(id, offset + PCIE_ATU_UNR_LOWER_TARGET, lower_32_bits(pci_addr));
    _pcie_writel_atu(id, offset + PCIE_ATU_UNR_UPPER_TARGET, upper_32_bits(pci_addr));
    _pcie_writel_atu(id, offset + PCIE_ATU_UNR_REGION_CTRL1, type);
    _pcie_writel_atu(id, offset + PCIE_ATU_UNR_REGION_CTRL2, PCIE_ATU_ENABLE);

    /*
     * Make sure ATU enable takes effect before any subsequent config
     * and I/O accesses.
     */
    for (retries = 0; retries < WAIT_IATU_RETRIES; retries++)
    {
        val = _pcie_readl_atu(id, offset + PCIE_ATU_UNR_REGION_CTRL2);
        if (val & PCIE_ATU_ENABLE)
        {
            ret = 0;
            break;
        }

        mdelay(WAIT_IATU_DELAY);
    }
    if (ret)
    {
        PCIE_ERR("Outbound iATU can't be enabled\n");
    }

    EPC_FUNC_EXIT();

    return ret;
}

static int _Drv_PCIE_EpcOutboundAtu(PCIE_EP *ep, phys_addr_t phys_addr, u64 pci_addr, u32 size)
{
    u32         i;
    int         ret;
    phys_addr_t cpu_addr;

    EPC_FUNC_ENTER();

    for (i = 0; i < HAL_PCIE_OB_WIN_NUM; i++)
    {
        if (ep->outbound_phys[i] == (~0U))
            break;
    }

    if (i == HAL_PCIE_OB_WIN_NUM)
    {
        PCIE_ERR("No outbound iATU available\n");
        return -ENOSR;
    }

    /* cpu addr fix-up */
    cpu_addr = phys_addr - ep->epc.mem.phys_base;

    PCIE_INFO("epc ob atu[%d]: cpu 0x%llx pci 0x%llx size %d\r\n", i, cpu_addr, pci_addr, size);
    ret = _Drv_PCIE_EpcProgOutboundAtu(ep->port_id, i, PCIE_ATU_TYPE_MEM, cpu_addr, pci_addr, size);
    if (ret)
        return ret;

    ep->outbound_phys[i] = phys_addr;

    EPC_FUNC_EXIT();

    return 0;
}

static void _Drv_PCIE_EpcUnmapAddr(PCIE_EP *ep, phys_addr_t addr)
{
    u32 i;

    EPC_FUNC_ENTER();

    for (i = 0; i < HAL_PCIE_OB_WIN_NUM; i++)
    {
        if (ep->outbound_phys[i] == addr)
            break;
    }

    if (i == HAL_PCIE_OB_WIN_NUM)
    {
        PCIE_ERR("No outbound iATU found\n");
        return;
    }

    _Drv_PCIE_EpcDisableAtu(ep->port_id, i, TRUE);
    ep->outbound_phys[i] = (~0U);

    EPC_FUNC_EXIT();
}

static int _Drv_PCIE_EpcMapAddr(PCIE_EP *ep, phys_addr_t addr, u64 pci_addr, u32 size)
{
    int ret;

    EPC_FUNC_ENTER();

    PCIE_DBG("epc map cpu 0x%llx pci 0x%llx size %d\r\n", addr, pci_addr, size);
    ret = _Drv_PCIE_EpcOutboundAtu(ep, addr, pci_addr, size);
    if (ret)
    {
        PCIE_ERR("Failed to enable address\n");
        return ret;
    }

    EPC_FUNC_EXIT();

    return 0;
}

//=================================================================================================
// Public Fucntions
//=================================================================================================
int Drv_PCIE_EpcInit(PCIE_EP *ep)
{
    u8        i;
    u8        id  = ep->port_id;
    int       ret = 0;
    PCIE_EPC *epc = &ep->epc;

    EPC_FUNC_ENTER();

    /* Initial fields of EP */
    for (i = 0; i < 6; i++)
    {
        ep->bar_to_atu[i] = 0xFF;
    }
    for (i = 0; i < HAL_PCIE_OB_WIN_NUM; i++)
    {
        ep->outbound_phys[i] = (~0U);
    }
    for (i = 0; i < EP_CB_TYPE_MAX; i++)
    {
        ep->callback[i] = NULL;
        ep->cb_data[i]  = NULL;
    }

    _Drv_PCIE_EpcSetup(ep);

    _Drv_PCIE_EpcMemInit(epc, HAL_PCIE_OB_ADDR(id), HAL_PCIE_OB_SPACE_SZ, HAL_PCIE_EP_PAGE_SZ);
    ep->msi_mem = _Drv_PCIE_EpcMemAlloc(epc, HAL_PCIE_EP_PAGE_SZ, &ep->msi_mem_phys);
    if (ep->msi_mem == NULL)
    {
        PCIE_ERR("EPC msi mem alloc failed\r\n");
        ret = -ENOMEM;
        goto _exit_init;
    }

    /* Get the offset of MSI, MSI-X capability */
    ep->msi_cap  = _Drv_PCIE_EpcFindCapability(id, PCI_CAP_ID_MSI);
    ep->msix_cap = _Drv_PCIE_EpcFindCapability(id, PCI_CAP_ID_MSIX);

    PCIE_INFO("epc init: msi 0x%llx 0x%p, msi_cap 0x%x, msix_cap 0x%x\r\n", ep->msi_mem_phys, ep->msi_mem, ep->msi_cap,
              ep->msix_cap);

_exit_init:

    EPC_FUNC_EXIT();

    return ret;
}

void Drv_PCIE_EpcResetBar(PCIE_EP *ep, enum pci_barno bar)
{
    EPC_FUNC_ENTER();

    _Drv_PCIE_EpcResetBar(ep->port_id, bar, PCI_BASE_ADDRESS_MEM_TYPE_32);

    EPC_FUNC_EXIT();
}

int Drv_PCIE_EpcSetBar(PCIE_EPC *epc, struct pci_bar *epf_bar)
{
    int            ret   = 0;
    PCIE_EP *      ep    = to_ep_from_epc(epc);
    enum pci_barno bar   = epf_bar->barno;
    size_t         size  = epf_bar->size;
    int            flags = epf_bar->flags;
    int            type;
    u32            reg = PCI_BASE_ADDRESS_0 + (4 * bar);
    u8             id  = ep->port_id;

    EPC_FUNC_ENTER();

    if (bar_fixsize[bar] && (size != bar_fixsize[bar]))
    {
        PCIE_ERR("epc bar%d: size 0x%x is unaccepted, must be x%x\r\n", bar, (u32)size, bar_fixsize[bar]);
        return -EINVAL;
    }

    if (flags & PCI_BASE_ADDRESS_SPACE_IO)
        type = PCIE_ATU_TYPE_IO;
    else
        type = PCIE_ATU_TYPE_MEM;

    ret = _Drv_PCIE_EpcInboundAtu(ep, bar, Drv_PCIE_Phys2Miu(epf_bar->phys_addr), type);
    if (ret)
        goto _exit_set_bar;

    _pcie_dbi_ro_wr_en(id);
    _pcie_writel_dbi2(id, reg, lower_32_bits(size - 1));
    sstar_pcieif_writel_dbi(id, reg, flags);

    if (flags & PCI_BASE_ADDRESS_MEM_TYPE_64)
    {
        _pcie_writel_dbi2(id, reg + 4, upper_32_bits(size - 1));
        sstar_pcieif_writel_dbi(id, reg + 4, 0);
    }

    _pcie_dbi_ro_wr_dis(id);

    PCIE_INFO("epc set bar%d: size 0x%x, flags x%x\r\n", bar, (u32)size, flags);

_exit_set_bar:

    EPC_FUNC_EXIT();

    return ret;
}

void Drv_PCIE_EpcWriteHeader(PCIE_EPC *epc, struct pci_ep_header *hdr)
{
    PCIE_EP *ep = to_ep_from_epc(epc);
    u8       id = ep->port_id;

    EPC_FUNC_ENTER();

    _pcie_dbi_ro_wr_en(id);
    sstar_pcieif_writew_dbi(id, PCI_VENDOR_ID, hdr->vendorid);
    sstar_pcieif_writew_dbi(id, PCI_DEVICE_ID, hdr->deviceid);
    sstar_pcieif_writeb_dbi(id, PCI_REVISION_ID, hdr->revid);
    sstar_pcieif_writeb_dbi(id, PCI_CLASS_PROG, hdr->progif_code);
    sstar_pcieif_writew_dbi(id, PCI_CLASS_SUB_CODE, hdr->subclass_code | hdr->baseclass_code << 8);
    sstar_pcieif_writeb_dbi(id, PCI_CACHE_LINE_SIZE, hdr->cache_line_size);
    sstar_pcieif_writew_dbi(id, PCI_SUBSYSTEM_VENDOR_ID, hdr->subsys_vendor_id);
    sstar_pcieif_writew_dbi(id, PCI_SUBSYSTEM_ID, hdr->subsys_id);
    sstar_pcieif_writeb_dbi(id, PCI_INTERRUPT_PIN, hdr->interrupt_pin);
    _pcie_dbi_ro_wr_dis(id);

    EPC_FUNC_EXIT();
}

void *Drv_PCIE_EpcMemAlloc(PCIE_EPC *epc, u32 size, phys_addr_t *phys_addr)
{
    void *addr;

    EPC_FUNC_ENTER();

    addr = _Drv_PCIE_EpcMemAlloc(epc, size, phys_addr);

    EPC_FUNC_EXIT();

    return addr;
}

int Drv_PCIE_EpcMapAddr(PCIE_EPC *epc, phys_addr_t addr, u64 pci_addr, u32 size)
{
    PCIE_EP *ep = to_ep_from_epc(epc);

    return _Drv_PCIE_EpcMapAddr(ep, addr, pci_addr, size);
}

void Drv_PCIE_EpcUnmapAddr(PCIE_EPC *epc, phys_addr_t addr)
{
    PCIE_EP *ep = to_ep_from_epc(epc);

    return _Drv_PCIE_EpcUnmapAddr(ep, addr);
}

int Drv_PCIE_EpcSetMsi(PCIE_EPC *epc, u8 interrupts)
{
    PCIE_EP *ep = to_ep_from_epc(epc);
    u8       id = ep->port_id;
    u32      val, reg;
    u8       ints, encode = 0;

    EPC_FUNC_ENTER();

    if (!ep->msi_cap)
        return -EINVAL;

    ints = interrupts >> 1;
    while (ints)
    {
        encode++;
        ints = ints >> 1;
    }
    PCIE_DBG("epc set msi irq num %d, log2 %d\r\n", interrupts, encode);

    reg = ep->msi_cap + PCI_MSI_FLAGS;
    val = sstar_pcieif_readw_dbi(id, reg);
    val &= ~PCI_MSI_FLAGS_QMASK;
    val |= (encode << 1) & PCI_MSI_FLAGS_QMASK;
    _pcie_dbi_ro_wr_en(id);
    sstar_pcieif_writew_dbi(id, reg, val);
    _pcie_dbi_ro_wr_dis(id);

    EPC_FUNC_EXIT();

    return 0;
}

int Drv_PCIE_EpcSetMsiX(PCIE_EPC *epc, u8 interrupts, enum pci_barno bir, u32 offset)
{
    PCIE_EP *ep = to_ep_from_epc(epc);
    u8       id = ep->port_id;
    u32      val, reg;

    EPC_FUNC_ENTER();

    if (!ep->msix_cap || (interrupts < 1))
        return -EINVAL;

    PCIE_DBG("epc set msix irq num %d, bir %d, offset %d\r\n", interrupts, bir, offset);

    _pcie_dbi_ro_wr_en(id);

    reg = ep->msix_cap + PCI_MSIX_CAP_FLAGS;
    val = sstar_pcieif_readw_dbi(id, reg);
    val &= ~PCI_MSIX_TABLE_SIZE;
    val |= interrupts - 1;
    sstar_pcieif_writew_dbi(id, reg, val);

    reg = ep->msix_cap + PCI_MSIX_TABLE;
    val = offset | bir;
    sstar_pcieif_writel_dbi(id, reg, val);

    reg = ep->msix_cap + PCI_MSIX_PBA;
    val = (offset + (interrupts * PCI_MSIX_ENTRY_SIZE)) | bir;
    sstar_pcieif_writel_dbi(id, reg, val);

    _pcie_dbi_ro_wr_dis(id);

    EPC_FUNC_EXIT();

    return 0;
}

void Drv_PCIE_EpcIrqHandler(PCIE_EP *ep)
{
    u8 i;

    for (i = 0; i < EP_CB_TYPE_MAX; i++)
    {
        if (ep->callback[i])
            ep->callback[i](ep->cb_data[i]);
    }
}

int Drv_PCIE_EpcRegisterIrqHandler(PCIE_EP *ep, PCIE_EP_CB_TYPE type, EP_CALLBACK cb, void *data)
{
    if (type >= EP_CB_TYPE_MAX)
        return -EINVAL;

    ep->callback[type] = cb;
    ep->cb_data[type]  = data;

    if (type == EP_CB_WRADDRMATCH)
        sstar_pcieif_writew_mac(ep->port_id, REG_PCIE_IRQ2_MASK, ~(IRQ2_INTR_ADDR_MATCH)); // unmask wr_addr_match_int

    return 0;
}

int Drv_PCIE_EpcRaiseMsiIrq(PCIE_EPC *epc, u8 interrupt_num)
{
    PCIE_EP *    ep = to_ep_from_epc(epc);
    u8           id = ep->port_id;
    unsigned int aligned_offset;
    u16          msg_ctrl, msg_data;
    u32          msg_addr_lower, msg_addr_upper, reg;
    u64          msg_addr;
    bool         has_upper;
    int          ret = 0;

    EPC_FUNC_ENTER();

    if (!ep->msi_cap)
        return -EINVAL;

    /* Raise MSI per the PCI Local Bus Specification Revision 3.0, 6.8.1. */
    reg            = ep->msi_cap + PCI_MSI_FLAGS;
    msg_ctrl       = sstar_pcieif_readw_dbi(id, reg);
    has_upper      = !!(msg_ctrl & PCI_MSI_FLAGS_64BIT);
    reg            = ep->msi_cap + PCI_MSI_ADDRESS_LO;
    msg_addr_lower = sstar_pcieif_readl_dbi(id, reg);
    if (has_upper)
    {
        reg            = ep->msi_cap + PCI_MSI_ADDRESS_HI;
        msg_addr_upper = sstar_pcieif_readl_dbi(id, reg);
        reg            = ep->msi_cap + PCI_MSI_DATA_64;
        msg_data       = sstar_pcieif_readw_dbi(id, reg);
    }
    else
    {
        msg_addr_upper = 0;
        reg            = ep->msi_cap + PCI_MSI_DATA_32;
        msg_data       = sstar_pcieif_readw_dbi(id, reg);
    }
    aligned_offset = msg_addr_lower & (epc->mem.page_size - 1);
    msg_addr       = ((u64)msg_addr_upper) << 32 | (msg_addr_lower & ~aligned_offset);
    PCIE_DBG("epc raise msi: msg_addr_lower 0x%x, msg_addr_upper 0x%x, align_ofst 0x%x, page_size 0x%x\r\n",
             msg_addr_lower, msg_addr_upper, aligned_offset, epc->mem.page_size);
    ret = _Drv_PCIE_EpcMapAddr(ep, ep->msi_mem_phys, msg_addr, epc->mem.page_size);
    if (ret)
        goto _exit_msi;

    PCIE_DBG("epc raise msi: msg addr 0x%p, data 0x%x\r\n", ep->msi_mem + aligned_offset,
             msg_data | (interrupt_num - 1));

    writel(msg_data | (interrupt_num - 1), ep->msi_mem + aligned_offset);

    _Drv_PCIE_EpcUnmapAddr(ep, ep->msi_mem_phys);

    EPC_FUNC_EXIT();

_exit_msi:
    return ret;
}

int Drv_PCIE_EpcRaiseMsiXIrq(PCIE_EPC *epc, u8 interrupt_num)
{
    PCIE_EP *        ep  = to_ep_from_epc(epc);
    u8               id  = ep->port_id;
    PCIE_EPF *       epf = epc->epf;
    PCIE_MSIX_TABLE *table;
    unsigned int     aligned_offset;
    u32              vec_ctrl, msg_data, table_offset, reg;
    u64              msg_addr;
    u8               bir;
    int              ret = 0;

    EPC_FUNC_ENTER();

    if (!ep->msix_cap)
        return -EINVAL;

    reg          = ep->msix_cap + PCI_MSIX_TABLE;
    table_offset = sstar_pcieif_readl_dbi(id, reg);
    bir          = (table_offset & PCI_MSIX_BIR);
    table_offset &= PCI_MSIX_TABLE_OFFSET;
    table = (PCIE_MSIX_TABLE *)((char *)phys_to_virt(epf->bar[bir].phys_addr) + table_offset);

    msg_addr = table[(interrupt_num - 1)].msg_addr;
    msg_data = table[(interrupt_num - 1)].msg_data;
    vec_ctrl = table[(interrupt_num - 1)].vector_ctrl;
    if (vec_ctrl & PCI_MSIX_ENTRY_VECTOR_MASK)
        return -EFAULT;

    aligned_offset = msg_addr & (epc->mem.page_size - 1);
    msg_addr       = msg_addr & ~aligned_offset;
    ret            = _Drv_PCIE_EpcMapAddr(ep, ep->msi_mem_phys, msg_addr, epc->mem.page_size);
    if (ret)
        goto _exit_msix;

    PCIE_DBG("epc raise msi-x: msg addr 0x%p 0x%llx, data 0x%x\r\n", ep->msi_mem + aligned_offset, msg_addr, msg_data);
    writel(msg_data, ep->msi_mem + aligned_offset);

    _Drv_PCIE_EpcUnmapAddr(ep, ep->msi_mem_phys);

    EPC_FUNC_EXIT();

_exit_msix:
    return ret;
}

void Drv_PCIE_EpcSetWriteMatchAddr(PCIE_EPC *epc, void *addr)
{
    PCIE_EP *ep = to_ep_from_epc(epc);
    u8       id = ep->port_id;

    EPC_FUNC_ENTER();

    sstar_pcieif_writew_mac(id, REG_WR_MATCH_ADDR0, (ulong)addr & 0xFFFF);
    sstar_pcieif_writew_mac(id, REG_WR_MATCH_ADDR1, ((ulong)addr >> 16) & 0xFFFF);
    sstar_pcieif_writew_mac(id, REG_WR_MATCH_ADDR2, upper_32_bits((ulong)addr) & 0x000F);

    PCIE_INFO("epc wr addr match 0x%p\r\n", addr);

    EPC_FUNC_EXIT();
}

int Drv_PCIE_EpcHandleWriteAddrMatchIrq(PCIE_EP *ep)
{
    u16 status, mask;
    u8  id   = ep->port_id;
    int intr = 0;

    status = sstar_pcieif_readw_mac(id, REG_PCIE_IRQ2_STAT);
    mask   = sstar_pcieif_readw_mac(id, REG_PCIE_IRQ2_MASK);
    status &= ~(mask);

    if (status & IRQ2_INTR_ADDR_MATCH)
    {
        sstar_pcieif_writew_mac(id, REG_PCIE_IRQ2_CLR, IRQ2_INTR_ADDR_MATCH);
        intr = 1;
        sstar_pcieif_writew_mac(id, REG_PCIE_IRQ2_CLR, 0);
    }

    return intr;
}

void Drv_PCIE_EpcStart(PCIE_EPC *epc)
{
    PCIE_EP *ep = to_ep_from_epc(epc);

    EPC_FUNC_ENTER();

    PCIE_DBG("start linking...\r\n");
    sstar_pcieif_start_link(ep->port_id);
    PCIE_DBG("link setup done\r\n");

    EPC_FUNC_EXIT();
}
