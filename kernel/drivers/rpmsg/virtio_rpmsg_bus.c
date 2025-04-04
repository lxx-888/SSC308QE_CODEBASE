// SPDX-License-Identifier: GPL-2.0
/*
 * Virtio-based remote processor messaging bus
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 * Copyright (C) 2011 Google, Inc.
 *
 * Ohad Ben-Cohen <ohad@wizery.com>
 * Brian Swetland <swetland@google.com>
 */

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/dma-mapping.h>
#include <linux/idr.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/rpmsg.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/virtio.h>
#include <linux/virtio_byteorder.h>
#include <linux/virtio_ids.h>
#include <linux/virtio_config.h>
#include <linux/wait.h>
#include <linux/rpmsg.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <rpmsg_dualos.h>

#include "rpmsg_internal.h"

/**
 * struct virtproc_info - virtual remote processor state
 * @vdev:	the virtio device
 * @rvq:	rx virtqueue
 * @svq:	tx virtqueue
 * @rbufs:	kernel address of rx buffers
 * @sbufs:	kernel address of tx buffers
 * @num_bufs:	total number of buffers for rx and tx
 * @buf_size:   size of one rx or tx buffer
 * @last_sbuf:	index of last tx buffer used
 * @bufs_dma:	dma base addr of the buffers
 * @tx_lock:	protects svq, sbufs and sleepers, to allow concurrent senders.
 *		sending a message might require waking up a dozing remote
 *		processor, which involves sleeping, hence the mutex.
 * @endpoints:	idr of local endpoints, allows fast retrieval
 * @endpoints_lock: lock of the endpoints set
 * @sendq:	wait queue of sending contexts waiting for a tx buffers
 * @sleepers:	number of senders that are waiting for a tx buffer
 * @ns_ept:	the bus's name service endpoint
 *
 * This structure stores the rpmsg state of a given virtio remote processor
 * device (there might be several virtio proc devices for each physical
 * remote processor).
 */
struct virtproc_info {
	struct virtio_device *vdev;
	struct virtqueue *rvq, *svq;
	void *rbufs, *sbufs;
	unsigned int num_bufs;
	unsigned int buf_size;
	int last_sbuf;
	dma_addr_t bufs_dma;
	struct mutex tx_lock;
	struct idr endpoints;
	struct mutex endpoints_lock;
	wait_queue_head_t sendq;
	atomic_t sleepers;
	struct rpmsg_endpoint *ns_ept;
#if defined(CONFIG_ARCH_SSTAR)
	rpmsg_device_type_t dev_type;
	int dev_id;
#endif
};

/**
 * struct virtio_rpmsg_channel - rpmsg channel descriptor
 * @rpdev: the rpmsg channel device
 * @vrp: the virtio remote processor device this channel belongs to
 *
 * This structure stores the channel that links the rpmsg device to the virtio
 * remote processor device.
 */
struct virtio_rpmsg_channel {
	struct rpmsg_device rpdev;

	struct virtproc_info *vrp;
};

#define to_virtio_rpmsg_channel(_rpdev) \
	container_of(_rpdev, struct virtio_rpmsg_channel, rpdev)

static void virtio_rpmsg_destroy_ept(struct rpmsg_endpoint *ept);
static int virtio_rpmsg_send(struct rpmsg_endpoint *ept, void *data, int len);
static int virtio_rpmsg_sendto(struct rpmsg_endpoint *ept, void *data, int len,
			       u32 dst);
static int virtio_rpmsg_send_offchannel(struct rpmsg_endpoint *ept, u32 src,
					u32 dst, void *data, int len);
static int virtio_rpmsg_trysend(struct rpmsg_endpoint *ept, void *data, int len);
static int virtio_rpmsg_trysendto(struct rpmsg_endpoint *ept, void *data,
				  int len, u32 dst);
static int virtio_rpmsg_trysend_offchannel(struct rpmsg_endpoint *ept, u32 src,
					   u32 dst, void *data, int len);

static const struct rpmsg_endpoint_ops virtio_endpoint_ops = {
	.destroy_ept = virtio_rpmsg_destroy_ept,
	.send = virtio_rpmsg_send,
	.sendto = virtio_rpmsg_sendto,
	.send_offchannel = virtio_rpmsg_send_offchannel,
	.trysend = virtio_rpmsg_trysend,
	.trysendto = virtio_rpmsg_trysendto,
	.trysend_offchannel = virtio_rpmsg_trysend_offchannel,
};

/**
 * rpmsg_sg_init - initialize scatterlist according to cpu address location
 * @sg: scatterlist to fill
 * @cpu_addr: virtual address of the buffer
 * @len: buffer length
 *
 * An internal function filling scatterlist according to virtual address
 * location (in vmalloc or in kernel).
 */
static void
rpmsg_sg_init(struct scatterlist *sg, void *cpu_addr, unsigned int len)
{
	if (is_vmalloc_addr(cpu_addr)) {
		sg_init_table(sg, 1);
		sg_set_page(sg, vmalloc_to_page(cpu_addr), len,
			    offset_in_page(cpu_addr));
	} else {
		WARN_ON(!virt_addr_valid(cpu_addr));
		sg_init_one(sg, cpu_addr, len);
	}
}

/**
 * __ept_release() - deallocate an rpmsg endpoint
 * @kref: the ept's reference count
 *
 * This function deallocates an ept, and is invoked when its @kref refcount
 * drops to zero.
 *
 * Never invoke this function directly!
 */
static void __ept_release(struct kref *kref)
{
	struct rpmsg_endpoint *ept = container_of(kref, struct rpmsg_endpoint,
						  refcount);
	/*
	 * At this point no one holds a reference to ept anymore,
	 * so we can directly free it
	 */
	kfree(ept);
}

/* for more info, see below documentation of rpmsg_create_ept() */
static struct rpmsg_endpoint *__rpmsg_create_ept(struct virtproc_info *vrp,
						 struct rpmsg_device *rpdev,
						 rpmsg_rx_cb_t cb,
						 void *priv, u32 addr)
{
	int id_min, id_max, id;
	struct rpmsg_endpoint *ept;
	struct device *dev = rpdev ? &rpdev->dev : &vrp->vdev->dev;

	ept = kzalloc(sizeof(*ept), GFP_KERNEL);
	if (!ept)
		return NULL;

	kref_init(&ept->refcount);
	mutex_init(&ept->cb_lock);

	ept->rpdev = rpdev;
	ept->cb = cb;
	ept->priv = priv;
	ept->ops = &virtio_endpoint_ops;

	/* do we need to allocate a local address ? */
	if (addr == RPMSG_ADDR_ANY) {
		id_min = RPMSG_RESERVED_ADDRESSES;
		id_max = 0;
	} else {
		id_min = addr;
		id_max = addr + 1;
	}

	mutex_lock(&vrp->endpoints_lock);

	/* bind the endpoint to an rpmsg address (and allocate one if needed) */
	id = idr_alloc(&vrp->endpoints, ept, id_min, id_max, GFP_KERNEL);
	if (id < 0) {
		dev_err(dev, "idr_alloc failed: %d\n", id);
		goto free_ept;
	}
	ept->addr = id;

	mutex_unlock(&vrp->endpoints_lock);

	return ept;

free_ept:
	mutex_unlock(&vrp->endpoints_lock);
	kref_put(&ept->refcount, __ept_release);
	return NULL;
}

static struct rpmsg_endpoint *virtio_rpmsg_create_ept(struct rpmsg_device *rpdev,
						      rpmsg_rx_cb_t cb,
						      void *priv,
						      struct rpmsg_channel_info chinfo)
{
	struct virtio_rpmsg_channel *vch = to_virtio_rpmsg_channel(rpdev);

	return __rpmsg_create_ept(vch->vrp, rpdev, cb, priv, chinfo.src);
}

/**
 * __rpmsg_destroy_ept() - destroy an existing rpmsg endpoint
 * @vrp: virtproc which owns this ept
 * @ept: endpoing to destroy
 *
 * An internal function which destroy an ept without assuming it is
 * bound to an rpmsg channel. This is needed for handling the internal
 * name service endpoint, which isn't bound to an rpmsg channel.
 * See also __rpmsg_create_ept().
 */
static void
__rpmsg_destroy_ept(struct virtproc_info *vrp, struct rpmsg_endpoint *ept)
{
	/* make sure new inbound messages can't find this ept anymore */
	mutex_lock(&vrp->endpoints_lock);
	idr_remove(&vrp->endpoints, ept->addr);
	mutex_unlock(&vrp->endpoints_lock);

	/* make sure in-flight inbound messages won't invoke cb anymore */
	mutex_lock(&ept->cb_lock);
	ept->cb = NULL;
	mutex_unlock(&ept->cb_lock);

	kref_put(&ept->refcount, __ept_release);
}

static void virtio_rpmsg_destroy_ept(struct rpmsg_endpoint *ept)
{
	struct virtio_rpmsg_channel *vch = to_virtio_rpmsg_channel(ept->rpdev);

	__rpmsg_destroy_ept(vch->vrp, ept);
}

static int virtio_rpmsg_announce_create(struct rpmsg_device *rpdev)
{
	struct virtio_rpmsg_channel *vch = to_virtio_rpmsg_channel(rpdev);
	struct virtproc_info *vrp = vch->vrp;
	struct device *dev = &rpdev->dev;
	int err = 0;

	if (!rpdev->ept || !rpdev->announce)
		return err;
	/* need to tell remote processor's name service about this channel ? */
	if (virtio_has_feature(vrp->vdev, VIRTIO_RPMSG_F_NS)) {
		struct rpmsg_ns_msg nsm;

		strncpy(nsm.name, rpdev->id.name, RPMSG_NAME_SIZE);
		nsm.addr = cpu_to_virtio32(vrp->vdev, rpdev->ept->addr);
		nsm.flags = cpu_to_virtio32(vrp->vdev, RPMSG_NS_CREATE);

		err = rpmsg_sendto(rpdev->ept, &nsm, sizeof(nsm), RPMSG_NS_ADDR);
		if (err)
			dev_err(dev, "failed to announce service %d\n", err);
	}

	/*
	 * need to tell remote processor's address service about the address allocated
	 * to this channel
	 */
	if (virtio_has_feature(vrp->vdev, VIRTIO_RPMSG_F_AS)) {
		struct rpmsg_as_msg asmsg;

		strncpy(asmsg.name, rpdev->id.name, RPMSG_NAME_SIZE);
		asmsg.dst = rpdev->src;
		asmsg.src = rpdev->dst;
		asmsg.flags = RPMSG_AS_ASSIGN;

		err = rpmsg_sendto(rpdev->ept, &asmsg, sizeof(asmsg), RPMSG_AS_ADDR);
		if (err)
			dev_err(dev, "failed to announce service %d\n", err);
	}

	return err;
}

static int virtio_rpmsg_announce_destroy(struct rpmsg_device *rpdev)
{
	struct virtio_rpmsg_channel *vch = to_virtio_rpmsg_channel(rpdev);
	struct virtproc_info *vrp = vch->vrp;
	struct device *dev = &rpdev->dev;
	int err = 0;

	if (!rpdev->ept || !rpdev->announce)
		return err;

	/*
	 * need to tell remote processor's address service that we're freeing
	 * the address allocated to this channel
	 */
	if (virtio_has_feature(vrp->vdev, VIRTIO_RPMSG_F_AS)) {
		struct rpmsg_as_msg asmsg;

		strncpy(asmsg.name, rpdev->id.name, RPMSG_NAME_SIZE);
		asmsg.dst = rpdev->src;
		asmsg.src = rpdev->dst;
		asmsg.flags = RPMSG_AS_FREE;

		err = rpmsg_sendto(rpdev->ept, &asmsg, sizeof(asmsg), RPMSG_AS_ADDR);
		if (err)
			dev_err(dev, "failed to announce service %d\n", err);
	}
	/* tell remote processor's name service we're removing this channel */
	if (virtio_has_feature(vrp->vdev, VIRTIO_RPMSG_F_NS)) {
		struct rpmsg_ns_msg nsm;

		strncpy(nsm.name, rpdev->id.name, RPMSG_NAME_SIZE);
		nsm.addr = cpu_to_virtio32(vrp->vdev, rpdev->ept->addr);
		nsm.flags = cpu_to_virtio32(vrp->vdev, RPMSG_NS_DESTROY);

		err = rpmsg_sendto(rpdev->ept, &nsm, sizeof(nsm), RPMSG_NS_ADDR);
		if (err)
			dev_err(dev, "failed to announce service %d\n", err);
	}

	return err;
}

static const struct rpmsg_device_ops virtio_rpmsg_ops = {
	.create_ept = virtio_rpmsg_create_ept,
	.announce_create = virtio_rpmsg_announce_create,
	.announce_destroy = virtio_rpmsg_announce_destroy,
};

static void virtio_rpmsg_release_device(struct device *dev)
{
	struct rpmsg_device *rpdev = to_rpmsg_device(dev);
	struct virtio_rpmsg_channel *vch = to_virtio_rpmsg_channel(rpdev);

	kfree(vch);
}

/*
 * create an rpmsg channel using its name and address info.
 * this function will be used to create both static and dynamic
 * channels.
 */
static struct rpmsg_device *rpmsg_create_channel(struct virtproc_info *vrp,
						 struct rpmsg_channel_info *chinfo,
						 bool announce)
{
	struct virtio_rpmsg_channel *vch;
	struct rpmsg_device *rpdev;
	struct device *tmp, *dev = &vrp->vdev->dev;
	int ret;

	/* make sure a similar channel doesn't already exist */
	tmp = rpmsg_find_device(dev, chinfo);
	if (tmp) {
		/* decrement the matched device's refcount back */
		put_device(tmp);
		dev_err(dev, "channel %s:%x:%x already exist\n",
				chinfo->name, chinfo->src, chinfo->dst);
		return NULL;
	}

	vch = kzalloc(sizeof(*vch), GFP_KERNEL);
	if (!vch)
		return NULL;

	/* Link the channel to our vrp */
	vch->vrp = vrp;

	/* Assign public information to the rpmsg_device */
	rpdev = &vch->rpdev;
	rpdev->src = chinfo->src;
	rpdev->dst = chinfo->dst;
	rpdev->ops = &virtio_rpmsg_ops;

	/*
	 * rpmsg server channels has predefined local address (for now),
	 * and their existence needs to be announced remotely
	 */
	if (rpdev->src != RPMSG_ADDR_ANY || announce)
		rpdev->announce = true;

	strncpy(rpdev->id.name, chinfo->name, RPMSG_NAME_SIZE);

	rpdev->dev.parent = &vrp->vdev->dev;
	rpdev->dev.release = virtio_rpmsg_release_device;
	rpdev->max_payload_length = vrp->buf_size - sizeof(struct rpmsg_hdr);
#if defined(CONFIG_ARCH_SSTAR)
	rpdev->type = vrp->dev_type;
	rpdev->dev_id = vrp->dev_id;
#endif
	ret = rpmsg_register_device(rpdev);
	if (ret)
		return NULL;

	return rpdev;
}

/* super simple buffer "allocator" that is just enough for now */
static void *get_a_tx_buf(struct virtproc_info *vrp)
{
	unsigned int len;
	void *ret;

	/* support multiple concurrent senders */
	mutex_lock(&vrp->tx_lock);

	/*
	 * either pick the next unused tx buffer
	 * (half of our buffers are used for sending messages)
	 */
	if (vrp->last_sbuf < vrp->num_bufs / 2)
		ret = vrp->sbufs + vrp->buf_size * vrp->last_sbuf++;
	/* or recycle a used one */
	else
		ret = virtqueue_get_buf(vrp->svq, &len);

	mutex_unlock(&vrp->tx_lock);

	return ret;
}

/**
 * rpmsg_upref_sleepers() - enable "tx-complete" interrupts, if needed
 * @vrp: virtual remote processor state
 *
 * This function is called before a sender is blocked, waiting for
 * a tx buffer to become available.
 *
 * If we already have blocking senders, this function merely increases
 * the "sleepers" reference count, and exits.
 *
 * Otherwise, if this is the first sender to block, we also enable
 * virtio's tx callbacks, so we'd be immediately notified when a tx
 * buffer is consumed (we rely on virtio's tx callback in order
 * to wake up sleeping senders as soon as a tx buffer is used by the
 * remote processor).
 */
static void rpmsg_upref_sleepers(struct virtproc_info *vrp)
{
	/* support multiple concurrent senders */
	mutex_lock(&vrp->tx_lock);

	/* are we the first sleeping context waiting for tx buffers ? */
	if (atomic_inc_return(&vrp->sleepers) == 1)
		/* enable "tx-complete" interrupts before dozing off */
		virtqueue_enable_cb(vrp->svq);

	mutex_unlock(&vrp->tx_lock);
}

/**
 * rpmsg_downref_sleepers() - disable "tx-complete" interrupts, if needed
 * @vrp: virtual remote processor state
 *
 * This function is called after a sender, that waited for a tx buffer
 * to become available, is unblocked.
 *
 * If we still have blocking senders, this function merely decreases
 * the "sleepers" reference count, and exits.
 *
 * Otherwise, if there are no more blocking senders, we also disable
 * virtio's tx callbacks, to avoid the overhead incurred with handling
 * those (now redundant) interrupts.
 */
static void rpmsg_downref_sleepers(struct virtproc_info *vrp)
{
	/* support multiple concurrent senders */
	mutex_lock(&vrp->tx_lock);

	/* are we the last sleeping context waiting for tx buffers ? */
	if (atomic_dec_and_test(&vrp->sleepers))
		/* disable "tx-complete" interrupts */
		virtqueue_disable_cb(vrp->svq);

	mutex_unlock(&vrp->tx_lock);
}

/**
 * rpmsg_send_offchannel_raw() - send a message across to the remote processor
 * @rpdev: the rpmsg channel
 * @src: source address
 * @dst: destination address
 * @data: payload of message
 * @len: length of payload
 * @wait: indicates whether caller should block in case no TX buffers available
 *
 * This function is the base implementation for all of the rpmsg sending API.
 *
 * It will send @data of length @len to @dst, and say it's from @src. The
 * message will be sent to the remote processor which the @rpdev channel
 * belongs to.
 *
 * The message is sent using one of the TX buffers that are available for
 * communication with this remote processor.
 *
 * If @wait is true, the caller will be blocked until either a TX buffer is
 * available, or 15 seconds elapses (we don't want callers to
 * sleep indefinitely due to misbehaving remote processors), and in that
 * case -ERESTARTSYS is returned. The number '15' itself was picked
 * arbitrarily; there's little point in asking drivers to provide a timeout
 * value themselves.
 *
 * Otherwise, if @wait is false, and there are no TX buffers available,
 * the function will immediately fail, and -ENOMEM will be returned.
 *
 * Normally drivers shouldn't use this function directly; instead, drivers
 * should use the appropriate rpmsg_{try}send{to, _offchannel} API
 * (see include/linux/rpmsg.h).
 *
 * Returns 0 on success and an appropriate error value on failure.
 */
static int rpmsg_send_offchannel_raw(struct rpmsg_device *rpdev,
				     u32 src, u32 dst,
				     void *data, int len, bool wait)
{
	struct virtio_rpmsg_channel *vch = to_virtio_rpmsg_channel(rpdev);
	struct virtproc_info *vrp = vch->vrp;
	struct device *dev = &rpdev->dev;
	struct scatterlist sg;
	struct rpmsg_hdr *msg;
	int err;

	/* bcasting isn't allowed */
	if (src == RPMSG_ADDR_ANY || dst == RPMSG_ADDR_ANY) {
		dev_err(dev, "invalid addr (src 0x%x, dst 0x%x)\n", src, dst);
		return -EINVAL;
	}

	/*
	 * We currently use fixed-sized buffers, and therefore the payload
	 * length is limited.
	 *
	 * One of the possible improvements here is either to support
	 * user-provided buffers (and then we can also support zero-copy
	 * messaging), or to improve the buffer allocator, to support
	 * variable-length buffer sizes.
	 */
	if (len > vrp->buf_size - sizeof(struct rpmsg_hdr)) {
		dev_err(dev, "message is too big (%d)\n", len);
		return -EMSGSIZE;
	}

	/* grab a buffer */
	msg = get_a_tx_buf(vrp);
	if (!msg && !wait)
		return -ENOMEM;

	/* no free buffer ? wait for one (but bail after 15 seconds) */
	while (!msg) {
		/* enable "tx-complete" interrupts, if not already enabled */
		rpmsg_upref_sleepers(vrp);

		/*
		 * sleep until a free buffer is available or 15 secs elapse.
		 * the timeout period is not configurable because there's
		 * little point in asking drivers to specify that.
		 * if later this happens to be required, it'd be easy to add.
		 */
		err = wait_event_interruptible_timeout(vrp->sendq,
					(msg = get_a_tx_buf(vrp)),
					msecs_to_jiffies(15000));

		/* disable "tx-complete" interrupts if we're the last sleeper */
		rpmsg_downref_sleepers(vrp);

		/* timeout ? */
		if (!err) {
			dev_err(dev, "timeout waiting for a tx buffer\n");
			return -ERESTARTSYS;
		}
	}

	msg->len = cpu_to_virtio16(vrp->vdev, len);
	msg->flags = 0;
	msg->src = cpu_to_virtio32(vrp->vdev, src);
	msg->dst = cpu_to_virtio32(vrp->vdev, dst);
	msg->reserved = 0;
	memcpy_toio(msg->data, data, len);

	dev_dbg(dev, "TX From 0x%x, To 0x%x, Len %d, Flags %d, Reserved %d\n",
		src, dst, len, msg->flags, msg->reserved);
#if defined(CONFIG_DYNAMIC_DEBUG)
	dynamic_hex_dump("rpmsg_virtio TX: ", DUMP_PREFIX_NONE, 16, 1,
			 msg, sizeof(*msg) + len, true);
#endif

	rpmsg_sg_init(&sg, msg, sizeof(*msg) + len);

	mutex_lock(&vrp->tx_lock);

	/* add message to the remote processor's virtqueue */
	err = virtqueue_add_outbuf(vrp->svq, &sg, 1, msg, GFP_KERNEL);
	if (err) {
		/*
		 * need to reclaim the buffer here, otherwise it's lost
		 * (memory won't leak, but rpmsg won't use it again for TX).
		 * this will wait for a buffer management overhaul.
		 */
		dev_err(dev, "virtqueue_add_outbuf failed: %d\n", err);
		goto out;
	}

	/* tell the remote processor it has a pending message to read */
	virtqueue_kick(vrp->svq);
out:
	mutex_unlock(&vrp->tx_lock);
	return err;
}

static int virtio_rpmsg_send(struct rpmsg_endpoint *ept, void *data, int len)
{
	struct rpmsg_device *rpdev = ept->rpdev;
	u32 src = ept->addr, dst = rpdev->dst;

	return rpmsg_send_offchannel_raw(rpdev, src, dst, data, len, true);
}

static int virtio_rpmsg_sendto(struct rpmsg_endpoint *ept, void *data, int len,
			       u32 dst)
{
	struct rpmsg_device *rpdev = ept->rpdev;
	u32 src = ept->addr;

	return rpmsg_send_offchannel_raw(rpdev, src, dst, data, len, true);
}

static int virtio_rpmsg_send_offchannel(struct rpmsg_endpoint *ept, u32 src,
					u32 dst, void *data, int len)
{
	struct rpmsg_device *rpdev = ept->rpdev;

	return rpmsg_send_offchannel_raw(rpdev, src, dst, data, len, true);
}

static int virtio_rpmsg_trysend(struct rpmsg_endpoint *ept, void *data, int len)
{
	struct rpmsg_device *rpdev = ept->rpdev;
	u32 src = ept->addr, dst = rpdev->dst;

	return rpmsg_send_offchannel_raw(rpdev, src, dst, data, len, false);
}

static int virtio_rpmsg_trysendto(struct rpmsg_endpoint *ept, void *data,
				  int len, u32 dst)
{
	struct rpmsg_device *rpdev = ept->rpdev;
	u32 src = ept->addr;

	return rpmsg_send_offchannel_raw(rpdev, src, dst, data, len, false);
}

static int virtio_rpmsg_trysend_offchannel(struct rpmsg_endpoint *ept, u32 src,
					   u32 dst, void *data, int len)
{
	struct rpmsg_device *rpdev = ept->rpdev;

	return rpmsg_send_offchannel_raw(rpdev, src, dst, data, len, false);
}

static int rpmsg_recv_single(struct virtproc_info *vrp, struct device *dev,
			     struct rpmsg_hdr *msg, unsigned int len)
{
	struct rpmsg_endpoint *ept;
	struct scatterlist sg;
	unsigned int msg_len = virtio16_to_cpu(vrp->vdev, msg->len);
	int err;

	dev_dbg(dev, "From: 0x%x, To: 0x%x, Len: %d, Flags: %d, Reserved: %d\n",
		virtio32_to_cpu(vrp->vdev, msg->src),
		virtio32_to_cpu(vrp->vdev, msg->dst), msg_len,
		virtio16_to_cpu(vrp->vdev, msg->flags),
		virtio32_to_cpu(vrp->vdev, msg->reserved));
#if defined(CONFIG_DYNAMIC_DEBUG)
	dynamic_hex_dump("rpmsg_virtio RX: ", DUMP_PREFIX_NONE, 16, 1,
			 msg, sizeof(*msg) + msg_len, true);
#endif

	/*
	 * We currently use fixed-sized buffers, so trivially sanitize
	 * the reported payload length.
	 */
	if (len > vrp->buf_size ||
	    msg_len > (len - sizeof(struct rpmsg_hdr))) {
		dev_warn(dev, "inbound msg too big: (%d, %d)\n", len, msg_len);
		return -EINVAL;
	}

	/* use the dst addr to fetch the callback of the appropriate user */
	mutex_lock(&vrp->endpoints_lock);

	ept = idr_find(&vrp->endpoints, virtio32_to_cpu(vrp->vdev, msg->dst));

	/* let's make sure no one deallocates ept while we use it */
	if (ept)
		kref_get(&ept->refcount);

	mutex_unlock(&vrp->endpoints_lock);

	if (ept) {
		/* make sure ept->cb doesn't go away while we use it */
		mutex_lock(&ept->cb_lock);

		if (ept->cb)
			ept->cb(ept->rpdev, msg->data, msg_len, ept->priv,
				virtio32_to_cpu(vrp->vdev, msg->src));

		mutex_unlock(&ept->cb_lock);

		/* farewell, ept, we don't need you anymore */
		kref_put(&ept->refcount, __ept_release);
	} else
		dev_warn(dev, "msg received with no recipient\n");

	/* publish the real size of the buffer */
	rpmsg_sg_init(&sg, msg, vrp->buf_size);

	/* add the buffer back to the remote processor's virtqueue */
	err = virtqueue_add_inbuf(vrp->rvq, &sg, 1, msg, GFP_KERNEL);
	if (err < 0) {
		dev_err(dev, "failed to add a virtqueue buffer: %d\n", err);
		return err;
	}

	return 0;
}

/* called when an rx buffer is used, and it's time to digest a message */
static void rpmsg_recv_done(struct virtqueue *rvq)
{
	struct virtproc_info *vrp = rvq->vdev->priv;
	struct device *dev = &rvq->vdev->dev;
	struct rpmsg_hdr *msg;
	unsigned int len, msgs_received = 0;
	int err;

	msg = virtqueue_get_buf(rvq, &len);
	if (!msg) {
		dev_err(dev, "uhm, incoming signal, but no used buffer ?\n");
		return;
	}

	virtqueue_disable_cb(rvq);
	while (msg) {
		err = rpmsg_recv_single(vrp, dev, msg, len);
		if (err)
			break;

		msgs_received++;

		msg = virtqueue_get_buf(rvq, &len);
	}
	virtqueue_enable_cb(rvq);

	/*
	 * Try to read message one more time in case a new message is submitted
	 * after virtqueue_get_buf() inside the while loop but before enabling
	 * callbacks
	 */
	msg = virtqueue_get_buf(rvq, &len);
	if (msg) {
		err = rpmsg_recv_single(vrp, dev, msg, len);
		if (!err)
			msgs_received++;
	}

	dev_dbg(dev, "Received %u messages\n", msgs_received);

	/* tell the remote processor we added another available rx buffer */
	if (msgs_received)
		virtqueue_kick(vrp->rvq);
}

/*
 * This is invoked whenever the remote processor completed processing
 * a TX msg we just sent it, and the buffer is put back to the used ring.
 *
 * Normally, though, we suppress this "tx complete" interrupt in order to
 * avoid the incurred overhead.
 */
static void rpmsg_xmit_done(struct virtqueue *svq)
{
	struct virtproc_info *vrp = svq->vdev->priv;

	dev_dbg(&svq->vdev->dev, "%s\n", __func__);

	/* wake up potential senders that are waiting for a tx buffer */
	wake_up_interruptible(&vrp->sendq);
}

/* invoked when a name service announcement arrives */
static int rpmsg_ns_cb(struct rpmsg_device *rpdev, void *data, int len,
		       void *priv, u32 src)
{
	struct rpmsg_ns_msg *msg = data;
	struct rpmsg_device *newch;
	struct rpmsg_channel_info chinfo;
	struct virtproc_info *vrp = priv;
	struct device *dev = &vrp->vdev->dev;
	int ret;

#if defined(CONFIG_DYNAMIC_DEBUG)
	dynamic_hex_dump("NS announcement: ", DUMP_PREFIX_NONE, 16, 1,
			 data, len, true);
#endif

	if (len != sizeof(*msg)) {
		dev_err(dev, "malformed ns msg (%d)\n", len);
		return -EINVAL;
	}

	/*
	 * the name service ept does _not_ belong to a real rpmsg channel,
	 * and is handled by the rpmsg bus itself.
	 * for sanity reasons, make sure a valid rpdev has _not_ sneaked
	 * in somehow.
	 */
	if (rpdev) {
		dev_err(dev, "anomaly: ns ept has an rpdev handle\n");
		return -EINVAL;
	}

	/* don't trust the remote processor for null terminating the name */
	msg->name[RPMSG_NAME_SIZE - 1] = '\0';

	strncpy(chinfo.name, msg->name, sizeof(chinfo.name));
	chinfo.src = RPMSG_ADDR_ANY;
	chinfo.dst = virtio32_to_cpu(vrp->vdev, msg->addr);

	dev_info(dev, "%sing channel %s addr 0x%x\n",
		 virtio32_to_cpu(vrp->vdev, msg->flags) & RPMSG_NS_DESTROY ?
		 "destroy" : "creat", msg->name, chinfo.dst);

#if defined(CONFIG_SSTAR_DUALOS_ADAPTOR_RC)
	if (strcmp(chinfo.name, "dualos_adaptor") == 0 &&
	    vrp->dev_id != 0xffffffff) {
		dualos_rpmsg_remote_adaptor_online(vrp->dev_type, vrp->dev_id);
		return 0;
	}
#endif
	if (virtio32_to_cpu(vrp->vdev, msg->flags) & RPMSG_NS_DESTROY) {
		ret = rpmsg_unregister_device(&vrp->vdev->dev, &chinfo);
		if (ret)
			dev_err(dev, "rpmsg_destroy_channel failed: %d\n", ret);
	} else {
		newch = rpmsg_create_channel(vrp, &chinfo, msg->flags & RPMSG_AS_ANNOUNCE);
		if (!newch)
			dev_err(dev, "rpmsg_create_channel failed\n");
	}

	return 0;
}

static int rpmsg_probe(struct virtio_device *vdev)
{
	vq_callback_t *vq_cbs[] = { rpmsg_recv_done, rpmsg_xmit_done };
	static const char * const names[] = { "input", "output" };
	struct virtqueue *vqs[2];
	struct virtproc_info *vrp;
	void *bufs_va;
	int err = 0, i;
	size_t total_buf_space;
	bool notify;

	vrp = kzalloc(sizeof(*vrp), GFP_KERNEL);
	if (!vrp)
		return -ENOMEM;

	vrp->vdev = vdev;
#if defined(CONFIG_ARCH_SSTAR)
	vrp->dev_type = vdev->id.vendor >> 16;
	vrp->dev_id = vdev->id.vendor & 0x0000ffff;
#endif

	idr_init(&vrp->endpoints);
	mutex_init(&vrp->endpoints_lock);
	mutex_init(&vrp->tx_lock);
	init_waitqueue_head(&vrp->sendq);

	/* We expect two virtqueues, rx and tx (and in this order) */
	err = virtio_find_vqs(vdev, 2, vqs, vq_cbs, names, NULL);
	if (err)
		goto free_vrp;

	vrp->rvq = vqs[0];
	vrp->svq = vqs[1];

	/* we expect symmetric tx/rx vrings */
	WARN_ON(virtqueue_get_vring_size(vrp->rvq) !=
		virtqueue_get_vring_size(vrp->svq));

	/* we need less buffers if vrings are small */
	if (virtqueue_get_vring_size(vrp->rvq) < MAX_RPMSG_NUM_BUFS / 2)
		vrp->num_bufs = virtqueue_get_vring_size(vrp->rvq) * 2;
	else
		vrp->num_bufs = MAX_RPMSG_NUM_BUFS;

	vrp->buf_size = MAX_RPMSG_BUF_SIZE;

	total_buf_space = vrp->num_bufs * vrp->buf_size;

	bufs_va = virtio_alloc_buffer(vdev, total_buf_space);
	if (!bufs_va) {
	/* allocate coherent memory for the buffers */
		if (!vdev->mem_virt) {
			bufs_va = dma_alloc_coherent(vdev->dev.parent->parent,
				     total_buf_space, &vrp->bufs_dma,
				     GFP_KERNEL);
		} else {
			bufs_va = vdev->mem_virt + 0x8000;
			vrp->bufs_dma = vdev->mem_phys + 0x8000;
		}

		if (!bufs_va) {
			err = -ENOMEM;
			goto vqs_del;
		}
	}

	dev_dbg(&vdev->dev, "buffers: va %pK, dma %pad\n",
		bufs_va, &vrp->bufs_dma);

	/* half of the buffers is dedicated for RX */
	vrp->rbufs = bufs_va;

	/* and half is dedicated for TX */
	vrp->sbufs = bufs_va + total_buf_space / 2;

	/* set up the receive buffers */
	for (i = 0; i < vrp->num_bufs / 2; i++) {
		struct scatterlist sg;
		void *cpu_addr = vrp->rbufs + i * vrp->buf_size;

		rpmsg_sg_init(&sg, cpu_addr, vrp->buf_size);

		err = virtqueue_add_inbuf(vrp->rvq, &sg, 1, cpu_addr,
					  GFP_KERNEL);
		WARN_ON(err); /* sanity check; this can't really happen */
	}

	/* suppress "tx-complete" interrupts */
	virtqueue_disable_cb(vrp->svq);

	vdev->priv = vrp;

	/* if supported by the remote processor, enable the name service */
	if (virtio_has_feature(vdev, VIRTIO_RPMSG_F_NS)) {
		/* a dedicated endpoint handles the name service msgs */
		vrp->ns_ept = __rpmsg_create_ept(vrp, NULL, rpmsg_ns_cb,
						vrp, RPMSG_NS_ADDR);
		if (!vrp->ns_ept) {
			dev_err(&vdev->dev, "failed to create the ns ept\n");
			err = -ENOMEM;
			goto free_coherent;
		}
	}

	/*
	 * Prepare to kick but don't notify yet - we can't do this before
	 * device is ready.
	 */
	notify = virtqueue_kick_prepare(vrp->rvq);

	/* From this point on, we can notify and get callbacks. */
	virtio_device_ready(vdev);

	/* tell the remote processor it can start sending messages */
	/*
	 * this might be concurrent with callbacks, but we are only
	 * doing notify, not a full kick here, so that's ok.
	 */
	if (notify)
		virtqueue_notify(vrp->rvq);

	dev_info(&vdev->dev, "rpmsg host is online\n");

	return 0;

free_coherent:
	if (!vrp->bufs_dma) {
		virtio_free_buffer(vdev, bufs_va, total_buf_space);
	} else if (!vdev->mem_virt) {
		dma_free_coherent(vdev->dev.parent->parent, total_buf_space,
			  bufs_va, vrp->bufs_dma);
	}
vqs_del:
	vdev->config->del_vqs(vrp->vdev);
free_vrp:
	kfree(vrp);
	return err;
}

static int rpmsg_remove_device(struct device *dev, void *data)
{
	device_unregister(dev);

	return 0;
}

static void rpmsg_remove(struct virtio_device *vdev)
{
	struct virtproc_info *vrp = vdev->priv;
	size_t total_buf_space = vrp->num_bufs * vrp->buf_size;
	int ret;

	vdev->config->reset(vdev);

	ret = device_for_each_child(&vdev->dev, NULL, rpmsg_remove_device);
	if (ret)
		dev_warn(&vdev->dev, "can't remove rpmsg device: %d\n", ret);

	if (vrp->ns_ept)
		__rpmsg_destroy_ept(vrp, vrp->ns_ept);

	idr_destroy(&vrp->endpoints);

	vdev->config->del_vqs(vrp->vdev);

	if (!vrp->bufs_dma) {
		virtio_free_buffer(vdev, vrp->rbufs, total_buf_space);
	} else {
		dma_free_coherent(vdev->dev.parent->parent, total_buf_space,
			  vrp->rbufs, vrp->bufs_dma);
	}

	kfree(vrp);
}

static struct virtio_device_id id_table[] = {
	{ VIRTIO_ID_RPMSG, VIRTIO_DEV_ANY_ID },
	{ 0 },
};

static unsigned int features[] = {
	VIRTIO_RPMSG_F_NS,
	VIRTIO_RPMSG_F_AS,
};

static struct virtio_driver virtio_ipc_driver = {
	.feature_table	= features,
	.feature_table_size = ARRAY_SIZE(features),
	.driver.name	= KBUILD_MODNAME,
	.driver.owner	= THIS_MODULE,
	.id_table	= id_table,
	.probe		= rpmsg_probe,
	.remove		= rpmsg_remove,
};

static int __init rpmsg_init(void)
{
	int ret;

	ret = register_virtio_driver(&virtio_ipc_driver);
	if (ret)
		pr_err("failed to register virtio driver: %d\n", ret);

	return ret;
}
subsys_initcall(rpmsg_init);

static void __exit rpmsg_fini(void)
{
	unregister_virtio_driver(&virtio_ipc_driver);
}
module_exit(rpmsg_fini);

MODULE_DEVICE_TABLE(virtio, id_table);
MODULE_DESCRIPTION("Virtio-based remote processor messaging bus");
MODULE_LICENSE("GPL v2");
