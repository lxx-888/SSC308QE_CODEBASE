/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#include "mi_sys.h"
#include "mi_pcie.h"
#include "ss_thread.h"
#include "ss_cmd_base.h"
#include "ss_pcie_ring_connector.h"
#include "amigos_module_init.h"
#include "amigos_module_pcie.h"

class pcie_ring_mi_connector : public pcie_ring_connector
{
    public:
        explicit pcie_ring_mi_connector(unsigned int size)
        {
            long long phy_addr = 0;
            char *virt_addr = NULL;
            int ret = 0;

            EXPECT_OK(MI_SYS_MMA_Alloc(0, NULL, size, (MI_PHY *)&phy_addr), ret, MI_SUCCESS);
            if (ret != MI_SUCCESS)
            {
                cout << "MMA ALloc error!\n" << endl;
                throw -1;
            }
            EXPECT_OK(MI_SYS_Mmap(phy_addr, size, (void **)&virt_addr, TRUE), ret, MI_SUCCESS);
            if (ret != MI_SUCCESS)
            {
                MI_SYS_MMA_Free(0, phy_addr);
                throw -1;
            }
            memset(virt_addr, 0, size);
            EXPECT_OK(MI_SYS_FlushInvCache(virt_addr, size), ret, MI_SUCCESS);
            ring_size       = size;
            local_phy_addr  = phy_addr;
            local_virt_addr = virt_addr;
            init_pool(size, virt_addr);
            cout << "PHY: " << "0x" << hex << phy_addr << " VIRT: " << hex << (long)virt_addr << endl;
        }
        virtual ~pcie_ring_mi_connector()
        {
            int ret = 0;
            if (local_virt_addr)
            {
                EXPECT_OK(MI_SYS_FlushInvCache(local_virt_addr, ring_size), ret, MI_SUCCESS);
                EXPECT_OK(MI_SYS_Munmap(local_virt_addr, ring_size), ret, MI_SUCCESS);
                local_virt_addr = NULL;
            }
            if (local_phy_addr)
            {
                EXPECT_OK(MI_SYS_MMA_Free(0, local_phy_addr), ret, MI_SUCCESS);
                local_phy_addr = -1;
            }
        }
        static pcie_ring_mi_connector *create(const string &handle, int size)
        {
            pcie_ring_mi_connector *connector = NULL;
            connector = new pcie_ring_mi_connector(size);
            assert(connector);
            if (!ss_handle::install(handle, connector))
            {
                //MMap Buffer delete and unmap in the clase.
                delete connector;
                return NULL;
            }
            return connector;
        }
    private:
        virtual int flush_cache(ring_addr start, ring_addr end)
        {
            unsigned int size[2];
            int ret = 0;
            if (local_phy_addr == -1)
            {
                cout << "local phy addr not ready." << endl;
                return -1;
            }
            get_range_size(start, end, size);
            if (!size[1])
            {
                ret = MI_SYS_FlushInvCache(local_virt_addr + start, size[0]);
                return ret;
            }
            ret |= MI_SYS_FlushInvCache(local_virt_addr + start, size[0]);
            ret |= MI_SYS_FlushInvCache(local_virt_addr, size[1]);
            return ret;
        }
        virtual int dma_task(ring_addr start, ring_addr end)
        {
            MI_PCIE_DmaTask_t stDmaTask;
            MI_PCIE_DEV devId = (MI_PCIE_DEV)dma_dev_id;
            unsigned int size[2];

            if (work_mode == 0 || start == end)
            {
                //it is reasonable that return success.
                return 0;
            }
            if (remote_phy_addr == -1)
            {
                cout << "remote phy addr not sync." << endl;
                return -1;
            }
            if (dma_dev_id == (unsigned int)-1)
            {
                cout << "dev id not set!" << endl;
                return -1;
            }
            if (work_mode == -1)
            {
                cout << "work_mode not set!" << endl;
                return -1;
            }
            if (remote_chip_id == (unsigned int)-1)
            {
                cout << "remote chip id not set!" << endl;
                return -1;
            }
            get_range_size(start, end, size);
            memset(&stDmaTask, 0, sizeof(MI_PCIE_DmaTask_t));
            stDmaTask.bReadFromRmt = (dma_dev_id % 2) ? FALSE : TRUE;
            if (size[0])
            {
                stDmaTask.u32BlockCount = 1;
                stDmaTask.bReadFromRmt = (dma_dev_id % 2) ? FALSE : TRUE;
                stDmaTask.stDmaBlocks[0].phyLocalAddr = local_phy_addr + start;
                stDmaTask.stDmaBlocks[0].phyRemoteAddr = remote_phy_addr + start;
                stDmaTask.stDmaBlocks[0].u32BlkSize = size[0];
            }
            if (size[1])
            {
                stDmaTask.u32BlockCount = 2;
                stDmaTask.stDmaBlocks[1].phyLocalAddr = local_phy_addr;
                stDmaTask.stDmaBlocks[1].phyRemoteAddr = remote_phy_addr;
                stDmaTask.stDmaBlocks[1].u32BlkSize = size[1];
            }
            return MI_PCIE_DmaTask(devId, &stDmaTask);
        }
};

std::map<unsigned int, unsigned int> AmigosModulePcie::mapPcieCreateDev;
AmigosModulePcie::AmigosModulePcie(const std::string &strInSection)
    : AmigosSurfacePcie(strInSection), AmigosModuleMiBase(this)
{
}
AmigosModulePcie::~AmigosModulePcie()
{
}
unsigned int AmigosModulePcie::GetModId() const
{
    return E_MI_MODULE_ID_PCIE;
}
unsigned int AmigosModulePcie::GetInputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
unsigned int AmigosModulePcie::GetOutputType(unsigned int port) const
{
    return E_MOD_PORT_TYPE_KERNEL | E_MOD_PORT_TYPE_USER;
}
bool AmigosModulePcie::_IsSyncMode(unsigned int inPortId) const
{
    return false;
}
void AmigosModulePcie::_Init()
{
    BlockBindCheck();
    if (!stPcieInfo.uintEsRingBuf || stPcieInfo.uintDevType == E_MI_PCIE_WORKMODE_EP)
    {
        auto it = mapPcieCreateDev.find(stModInfo.devId);
        if (it == mapPcieCreateDev.end())
        {
            MI_PCIE_DevAttr_t  stPcieDevAttr;
            memset(&stPcieDevAttr, 0, sizeof(MI_PCIE_DevAttr_t));
            stPcieDevAttr.enMode = (MI_PCIE_WorkMode_e )stPcieInfo.uintDevType;
            MI_PCIE_CreateDevice(stModInfo.devId, &stPcieDevAttr);
        }
        mapPcieCreateDev[stModInfo.devId]++;
    }
    if (!stPcieInfo.uintEsRingBuf)
    {
        MI_PCIE_ChnAttr_t stPcieChnAttr;
        memset(&stPcieChnAttr, 0, sizeof(MI_PCIE_ChnAttr_t));
        stPcieChnAttr.u16TargetChipId = (MI_U16)stPcieInfo.uintTgtId;
        stPcieChnAttr.u32TargetChnId = (MI_U32)stPcieInfo.uintTgtChnId;
        MI_PCIE_CreateChannel(stModInfo.devId, stModInfo.chnId, &stPcieChnAttr);
        MI_PCIE_StartChannel(stModInfo.devId, stModInfo.chnId);
    }
}
void AmigosModulePcie::BlockBindCheck()
{
    if (mapModInputInfo.size() == 1
        && mapModOutputInfo.size() == 0
        && (stModInfo.devId % 2))
    {
        AMIGOS_INFO("PCIE Running TX mode. Type is %s\n", stPcieInfo.uintDevType ? "EP" : "RC");
        return;
    }
    if (mapModOutputInfo.size() == 1
        && mapModInputInfo.size() == 0
        && !(stModInfo.devId % 2))
    {
        AMIGOS_INFO("PCIE Running RX mode. Type is %s\n", stPcieInfo.uintDevType ? "EP" : "RC");
        return;
    }
    AMIGOS_ERR("Pcie Pipeline bind error, input size %lu, output size %lu, devType %d\n", mapModInputInfo.size(), mapModOutputInfo.size(), stPcieInfo.uintDevType);
    ASSERT(0);
}
ss_connector_base *AmigosModulePcie::CreateSsRingConnector()
{
    std::stringstream ssLocalConnectorHandle;
    unsigned int uintChipId = (stPcieInfo.uintDevType == E_MI_PCIE_WORKMODE_EP ? 0x10000 : 0) | (stPcieInfo.uintTgtId & 0xFF);
    std::stringstream ssRemoteConnectorHandle;

    ssLocalConnectorHandle << std::dec << ((stModInfo.devId << 8) | stModInfo.chnId) << std::endl;
    pcie_ring_connector *connector = pcie_ring_mi_connector::create(ssLocalConnectorHandle.str(), 0x100000);
    ASSERT(connector);

    ssRemoteConnectorHandle << std::dec << ((stPcieInfo.uintTgtDevId << 8) | stPcieInfo.uintTgtChnId) << std::endl;
    connector->set_pcie_info(stModInfo.devId, stPcieInfo.uintDevType, uintChipId, ssRemoteConnectorHandle.str());
    //Only Tx mode set tx wq.
    if (stModInfo.devId % 2)
    {
        connector->create_tx_ring_workqueue();
    }
    return connector;
}
void AmigosModulePcie::DestroySsRingConnector(ss_connector_base *connector)
{
    std::stringstream ssLocalConnectorHandle;

    pcie_ring_connector *pPcieRingConnector = dynamic_cast <pcie_ring_connector *>(connector);
    ASSERT(pPcieRingConnector);
    ssLocalConnectorHandle << std::dec << ((stModInfo.devId << 8) | stModInfo.chnId) << std::endl;
    if (stModInfo.devId % 2)
    {
        pPcieRingConnector->destroy_tx_ring_workqueue();
    }
    delete pPcieRingConnector;
}
void AmigosModulePcie::_Deinit()
{
    if (!stPcieInfo.uintEsRingBuf)
    {
        MI_PCIE_StopChannel(stModInfo.devId, stModInfo.chnId);
        MI_PCIE_DestroyChannel(stModInfo.devId, stModInfo.chnId);
    }
    if (!stPcieInfo.uintEsRingBuf || stPcieInfo.uintDevType == E_MI_PCIE_WORKMODE_EP)
    {
        auto it = mapPcieCreateDev.find(stModInfo.devId);
        if (it != mapPcieCreateDev.end())
        {
            it->second--;
            if (!it->second)
            {
                MI_PCIE_DestroyDevice(stModInfo.devId);
                mapPcieCreateDev.erase(it);
            }
        }
    }
}
ss_connector_base *AmigosModulePcie::_CreateConnector(unsigned int inPortId, bool &bFast)
{
    if (stPcieInfo.uintEsRingBuf)
    {
        bFast = false;
        return CreateSsRingConnector();
    }
    return AmigosModuleMiBase::_CreateConnector(inPortId, bFast);
}
void AmigosModulePcie::_DestroyConnector(ss_connector_base *connector)
{
    if (stPcieInfo.uintEsRingBuf)
    {
        DestroySsRingConnector(connector);
        return;
    }
    AmigosModuleMiBase::_DestroyConnector(connector);
}
void * AmigosModulePcie::_PcieReader(struct ss_thread_buffer *pstBuf)
{
    PcieReaderDesc *pPcieDesc = (PcieReaderDesc *)pstBuf->buf;
    ASSERT(pPcieDesc);
    ASSERT(pPcieDesc->pOutConnector);
    auto packet = pPcieDesc->pPcieRingConnector->grab();
    if (!packet)
    {
        //Time out!
        return NULL;
    }
    pPcieDesc->pOutConnector->come(packet);
    pPcieDesc->pPcieRingConnector->back(packet);
    return NULL;
}
void AmigosModulePcie::_StartOut(unsigned int outPortId)
{
    struct ss_thread_attr stSsThreadAttr;
    PcieReaderDesc pcieDesc;

    if (!stPcieInfo.uintEsRingBuf)
    {
        AmigosModuleMiBase::_StartOut(outPortId);
        return;
    }
    auto iterPortOut = this->mapPortOut.find(outPortId);
    if (iterPortOut == this->mapPortOut.end())
    {
        AMIGOS_ERR("Port %d has not base sturcture.\n", outPortId);
        return;
    }
    pcieDesc.pOutConnector = &iterPortOut->second.connector;
    pcieDesc.pPcieRingConnector = CreateSsRingConnector();
    memset(&stSsThreadAttr, 0, sizeof(struct ss_thread_attr));
    stSsThreadAttr.do_signal = NULL;
    stSsThreadAttr.do_monitor = _PcieReader;
    stSsThreadAttr.monitor_cycle_sec = 0;
    stSsThreadAttr.monitor_cycle_nsec = 0;
    stSsThreadAttr.in_buf.buf = (void *)&this->mapPcieOutDesc[outPortId];
    stSsThreadAttr.in_buf.size = 0;
    snprintf(stSsThreadAttr.thread_name, 128, "%s", GetOutPortIdStr(outPortId).c_str());
    pcieDesc.threadHandle = ss_thread_open(&stSsThreadAttr);
    if (!pcieDesc.threadHandle)
    {
        this->mapPcieOutDesc.erase(outPortId);
        AMIGOS_ERR("Monitor return error!\n");
        return;
    }
    this->mapPcieOutDesc[outPortId] = pcieDesc;
}
void AmigosModulePcie::_StopOut(unsigned int outPortId)
{
    if (!stPcieInfo.uintEsRingBuf)
    {
        AmigosModuleMiBase::_StopOut(outPortId);
        return;
    }
    auto iter = this->mapPcieOutDesc.find(outPortId);
    if (iter == this->mapPcieOutDesc.end())
    {
        return;
    }
    ss_thread_close(iter->second.threadHandle);
    DestroySsRingConnector(iter->second.pPcieRingConnector);
    this->mapPcieOutDesc.erase(iter);
}
int AmigosModulePcie::_Connected(unsigned int outPortId, unsigned int ref)
{
    if (!stPcieInfo.uintEsRingBuf)
    {
        AmigosModuleMiBase::_Connected(outPortId, ref);
        return 0;
    }
    auto iter = this->mapPcieOutDesc.find(outPortId);
    if (iter == this->mapPcieOutDesc.end())
    {
        return -1;
    }
    return ss_thread_start_monitor(iter->second.threadHandle);
}
int AmigosModulePcie::_Disconnected(unsigned int outPortId, unsigned int ref)
{
    if (!stPcieInfo.uintEsRingBuf)
    {
        AmigosModuleMiBase::_Disconnected(outPortId, ref);
        return 0;
    }
    auto iter = this->mapPcieOutDesc.find(outPortId);
    if (iter == this->mapPcieOutDesc.end())
    {
        return -1;
    }
    return ss_thread_stop(iter->second.threadHandle);
}
AMIGOS_MODULE_INIT("PCIE", AmigosModulePcie);
