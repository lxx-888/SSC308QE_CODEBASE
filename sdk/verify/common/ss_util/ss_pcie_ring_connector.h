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
#ifndef __SS_PCIE_RING_CONNECTOR__
#define __SS_PCIE_RING_CONNECTOR__
#include "ss_ring_connector.h"

class pcie_ring_connector : public ss_ring_connector
{
    public:
        struct packet_ring_range
        {
            ring_addr start;
            ring_addr end;
        };
        pcie_ring_connector()
        {
            remote_phy_addr = -1;
            dma_dev_id = -1;
            work_mode = -1;
            remote_chip_id = -1;
            b_create_tx_wq = false;
            ring_read_dma = 0;
            ring_write_dma = 0;
        }
        virtual ~pcie_ring_connector()
        {
            if (b_create_tx_wq)
            {
                ss_thread_close(thread_handle);
                b_create_tx_wq = false;
            }
        }
        int create_tx_ring_workqueue(void)
        {
            struct ss_thread_attr ss_attr;

            if (b_create_tx_wq)
            {
                ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Handle: %s TX workqueue had created!\n", get_handle().c_str());
                return -1;
            }
            memset(&ss_attr, 0, sizeof(struct ss_thread_attr));
            ss_attr.do_signal = do_ring_tx;
            ss_attr.in_buf.buf = this;
            snprintf(ss_attr.thread_name, 32, "perh%s", get_handle().c_str()); //pcie ring handle
            thread_handle = ss_thread_open(&ss_attr);
            if (!thread_handle)
            {
                return -1;
            }
            access(); /* During PCIe transmission, only RC or EP can be read or write. */
            b_create_tx_wq = true;
            return 0;
        }
        int destroy_tx_ring_workqueue(void)
        {
            if (b_create_tx_wq == false)
            {
                ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Handle: %s TX workqueue had not been created!\n", get_handle().c_str());
                return -1;
            }
            if (ss_thread_close(thread_handle) != -1)
            {
                b_create_tx_wq = false;
                return 0;
            }
            ring_read_dma = 0;
            ring_write_dma = 0;
            return -1;
        }
        virtual int come(stream_packet_obj &packet) override
        {
            ring_addr start = 0, end = 0;
            ring_stream_header *packet_ring = NULL;

            {
                ss_auto_lock lock(connector_mutex);

                //Prepare free ring buffer to fill packet header data.
                start = prepare_buf(sizeof(ring_stream_header));
                if (start == (ring_addr)-1)
                    return -1;

                //Translate ring addr to system addr.
                from_ring_buf((char *&)packet_ring, start);

                //Copy the header  to ring.
                *packet_ring = packet;

                //Prepare es stream data by header.
                end = prepare_ring_packet(*packet_ring);
                if (end == (ring_addr)-1)
                    return -1;

                //Copy es stream data by header to ring.
                copy_to_ring_packet(*packet_ring, *packet);

                //Active the write pointer.
                active_buf();
            }

            return schedule_coming_things(start, end);
        }
        virtual void update(long virt)
        {
            ring_addr start = 0, end = 0;
            {
                ss_auto_lock lock(connector_mutex);

                //Get the read/write pointer.
                use_all_ring(start, end);

                //prepare_buf(0) is the last prepare pointer.
                if (virt != prepare_buf(0))
                {
                    //Reset the ring buffer.
                    flip_all();
                    cout << "Update order error!" << endl;
                    return;
                }

                //Active the write pointer.
                active_buf();
                end = (ring_addr)virt;
            }
            schedule_coming_things(start, end);
        }
        int do_ring_rx(ring_addr start, ring_addr end)
        {
            ss_auto_lock lock(connector_mutex);
            if (end != start)
            {
                if (prepare_buf_to_end(end) == (ring_addr)-1)
                {
                    cout << "pcie rx/tx ring not match!" << endl;
                    flip_by_set(start, end);
                }
                //I don't care wether it is ep or rc, if rc, this function do nothing and ep will do it before in ep's tx ring's do_ring_pcie_tx function.
                dma_task(start, end);
                //Wether ep do dma on local or remote, it must flush local addr cache.
                flush_cache(start, end);
                active_buf();
            }
            if (can_send())
            {
                ring_stream_header packet_ring;
                int go_ret = 0;
                ring_addr ring_data_end;
                if (copy_from_ring((char *)&packet_ring, sizeof(ring_stream_header)) == -1)
                    return -1;
                ring_data_end = from_ring_packet(packet_ring);
                if (ring_data_end == (ring_addr)-1)
                    return -1;
                //cout << "END: 0x" << (long)ring_data_end << endl;
                auto new_pack = stream_packet_base::make<ring_stream_packet>(packet_ring);
                assert(new_pack);
                go_ret = go(new_pack);
                if (go_ret != -1)
                {
                    flip_buf(ring_data_end);
                }
                return go_ret;
            }
            //Wake up 'grab'.
            wake_up_cond();

            return 0;
        }
        void set_remote_phyaddr(unsigned long long phy)
        {
            remote_phy_addr = phy;
            cout << "MODE: " << work_mode << " GET Remote phyaddr: 0x" << hex << phy << endl;
        }
        int set_pcie_info(unsigned int dma_dev, int mode, unsigned int rmt_chip_id, const string &rmt_ring_handle)
        {
            dma_dev_id         = dma_dev;
            work_mode          = mode;
            remote_chip_id     = rmt_chip_id;
            remote_ring_handle = rmt_ring_handle;
            //local's phy addr is the remote's remote phy addr.
            if (work_mode == 0)
            {
                //Only current is rc mode, then send rc's address to ep, because it is only ep to do dma.
                return run_cmd(remote_chip_id, "internal_pcie_set_remote_phyaddr %s 0x%llx", remote_ring_handle.c_str(), local_phy_addr);
            }
            return 0;
        }
    protected:
        unsigned int ring_size;
        long long local_phy_addr;
        long long remote_phy_addr;
        char * local_virt_addr;
        unsigned int dma_dev_id;
        int work_mode; //0 rc, 1 ep.
        unsigned int remote_chip_id;
    private:
        //Implement in child.
        virtual int flush_cache(ring_addr start, ring_addr end) = 0;
        virtual int dma_task(ring_addr start, ring_addr end) = 0;

        void notify_remote(ring_addr start, ring_addr end)
        {
            run_cmd_nolock(remote_chip_id, "internal_pcie_do_ring_rx %s 0x%x 0x%x", remote_ring_handle.c_str(), start, end);
        }
        int schedule_coming_things(ring_addr ring_read, ring_addr ring_write)
        {
            struct ss_thread_user_data work_data;
            struct packet_ring_range coming_range;

            if (!b_create_tx_wq)
            {
                return -1;
            }
            memset(&work_data, 0, sizeof(struct ss_thread_user_data));
            coming_range.start = ring_read;
            coming_range.end = ring_write;
            work_data.data = &coming_range;
            work_data.size = sizeof(struct packet_ring_range);
            return ss_thread_send(thread_handle, &work_data);
        }
        static void *do_ring_tx(struct ss_thread_buffer *thread_buf, struct ss_thread_user_data *work_data)
        {
            //Tx behavior only, Stream comming...
            pcie_ring_connector *connector = NULL;
            ring_addr ring_read = 0, ring_write = 0;
            struct packet_ring_range *coming = (struct packet_ring_range *)work_data->data;

            assert(thread_buf);
            connector = (pcie_ring_connector *)thread_buf->buf;
            if (!coming)
                return NULL;
            if (sizeof(struct packet_ring_range) != work_data->size)
                return NULL;
            if (!connector)
                return NULL;

            ss_auto_lock lock(connector->connector_mutex);
            if (!connector->in_range(coming->start, coming->end, connector->ring_read_dma, connector->ring_write_dma))
            {
                //Get tx ring start/end offset.
                connector->use_all_ring(connector->ring_read_dma, connector->ring_write_dma);
                connector->flush_cache(connector->ring_read_dma, connector->ring_write_dma);
                //It is the first one of pending schedule ring tx worequeue.
                //If local is ep it will work, if not, dma will work on ring_pcie_rx.
                connector->dma_task(connector->ring_read_dma, connector->ring_write_dma);
                ring_read = connector->ring_read_dma;
                ring_write = connector->ring_write_dma;
            }
            //Every incoming send message to rx to do grab function.
            connector->notify_remote(ring_read, ring_write);
            if (coming->end == connector->ring_write_dma)
            {
                //It is the last one of schedules list.
                connector->flip_buf(connector->ring_write_dma);
            }
            return NULL;
        }
        bool b_create_tx_wq;
        void *thread_handle;
        string remote_ring_handle;
        ring_addr ring_read_dma;
        ring_addr ring_write_dma;
};
static inline int pcie_set_remote_phyaddr(vector<string> &in_strs)
{
    const string &ring_handle = in_strs[1];
    unsigned long long phyaddr = ss_cmd_atoi(in_strs[2].c_str());

    pcie_ring_connector *connector = dynamic_cast <pcie_ring_connector *>(pcie_ring_connector::get(ring_handle));
    if (!connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle %s error!\n", ring_handle.c_str());
        return -1;
    }
    connector->set_remote_phyaddr(phyaddr);

    return 0;
}
static inline int pcie_do_ring_rx(vector<string> &in_strs)
{
    const string &ring_handle = in_strs[1];
    ring_addr start = ss_cmd_atoi(in_strs[2].c_str());
    ring_addr end = ss_cmd_atoi(in_strs[3].c_str());
    pcie_ring_connector *connector = dynamic_cast <pcie_ring_connector *>(pcie_ring_connector::get(ring_handle));
    if (!connector)
    {
        ss_print(PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT, "Connector handle error!\n");
        return -1;
    }
    return connector->do_ring_rx(start, end);
}
MOD_CMDS(pcie_ring_internal)
{
    //*****************************INTERNAL USE COMMANDS********************************
    //Althrough user can execute it by console/shell, but this is for internal use.
    ADD_CMD("internal_pcie_set_remote_phyaddr", pcie_set_remote_phyaddr, 2);
    ADD_CMD("internal_pcie_do_ring_rx", pcie_do_ring_rx, 3);
    //*********************************************************************************
}
#endif
