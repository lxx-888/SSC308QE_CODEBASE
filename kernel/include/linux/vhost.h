/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _VHOST_H
#define _VHOST_H

#include <linux/configfs.h>
#include <linux/eventfd.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/poll.h>
#include <linux/file.h>
#include <linux/uio.h>
#include <linux/virtio_config.h>
#include <linux/virtio_ring.h>
#include <linux/vringh.h>
#include <linux/atomic.h>
#include <linux/vhost_iotlb.h>
#include <linux/irqbypass.h>
#include <uapi/linux/vhost.h>

struct vhost_work;
typedef void (*vhost_work_fn_t)(struct vhost_work *work);

#define VHOST_WORK_QUEUED 1
struct vhost_work {
	struct llist_node	  node;
	vhost_work_fn_t		  fn;
	unsigned long		  flags;
};

/* Poll a file (eventfd or socket) */
/* Note: there's nothing vhost specific about this structure. */
struct vhost_poll {
	poll_table                table;
	wait_queue_head_t        *wqh;
	wait_queue_entry_t              wait;
	struct vhost_work	  work;
	__poll_t		  mask;
	struct vhost_dev	 *dev;
};

void vhost_work_init(struct vhost_work *work, vhost_work_fn_t fn);
void vhost_work_queue(struct vhost_dev *dev, struct vhost_work *work);
bool vhost_has_work(struct vhost_dev *dev);

void vhost_poll_init(struct vhost_poll *poll, vhost_work_fn_t fn,
		     __poll_t mask, struct vhost_dev *dev);
int vhost_poll_start(struct vhost_poll *poll, struct file *file);
void vhost_poll_stop(struct vhost_poll *poll);
void vhost_poll_flush(struct vhost_poll *poll);
void vhost_poll_queue(struct vhost_poll *poll);
void vhost_work_flush(struct vhost_dev *dev, struct vhost_work *work);
long vhost_vring_ioctl(struct vhost_dev *d, unsigned int ioctl, void __user *argp);

struct vhost_log {
	u64 addr;
	u64 len;
};

enum vhost_uaddr_type {
	VHOST_ADDR_DESC = 0,
	VHOST_ADDR_AVAIL = 1,
	VHOST_ADDR_USED = 2,
	VHOST_NUM_ADDRS = 3,
};

struct vhost_vring_call {
	struct eventfd_ctx *ctx;
	struct irq_bypass_producer producer;
};

enum vhost_type {
	VHOST_TYPE_UNKNOWN,
	VHOST_TYPE_USER,
	VHOST_TYPE_KERN,
	VHOST_TYPE_MMIO,
};

/* The virtqueue structure describes a queue attached to a device. */
struct vhost_virtqueue {
	struct vhost_dev *dev;
	enum vhost_type type;
	struct vringh vringh;
	int index;
	void (*callback)(struct vhost_virtqueue *vq);
	void (*notify)(struct vhost_virtqueue *vq);

	/* The actual ring of buffers. */
	struct mutex mutex;
	unsigned int num;
	vring_desc_t __user *desc;
	vring_avail_t __user *avail;
	vring_used_t __user *used;
	const struct vhost_iotlb_map *meta_iotlb[VHOST_NUM_ADDRS];
	struct file *kick;
	struct vhost_vring_call call_ctx;
	struct eventfd_ctx *error_ctx;
	struct eventfd_ctx *log_ctx;

	struct vhost_poll poll;

	/* The routine to call when the Guest pings us, or timeout. */
	vhost_work_fn_t handle_kick;

	/* Last available index we saw. */
	u16 last_avail_idx;

	/* Caches available index value from user. */
	u16 avail_idx;

	/* Last index we used. */
	u16 last_used_idx;

	/* Used flags */
	u16 used_flags;

	/* Last used index value we have signalled on */
	u16 signalled_used;

	/* Last used index value we have signalled on */
	bool signalled_used_valid;

	/* Log writes to used structure. */
	bool log_used;
	u64 log_addr;

	struct vringh_mmiov wiov;
	struct vringh_mmiov riov;
	struct iovec iov[UIO_MAXIOV];
	struct iovec iotlb_iov[64];
	struct iovec *indirect;
	struct vring_used_elem *heads;
	/* Protected by virtqueue mutex. */
	struct vhost_iotlb *umem;
	struct vhost_iotlb *iotlb;
	void *private_data;
	u64 acked_backend_features;
	/* Log write descriptors */
	void __user *log_base;
	struct vhost_log *log;
	struct iovec log_iov[64];

	/* Ring endianness. Defaults to legacy native endianness.
	 * Set to true when starting a modern virtio device. */
	bool is_le;
#ifdef CONFIG_VHOST_CROSS_ENDIAN_LEGACY
	/* Ring endianness requested by userspace for cross-endian support. */
	bool user_be;
#endif
	u32 busyloop_timeout;
};

struct vhost_msg_node {
  union {
	  struct vhost_msg msg;
	  struct vhost_msg_v2 msg_v2;
  };
  struct vhost_virtqueue *vq;
  struct list_head node;
};

struct vhost_driver_item {
	struct config_group group;
	u32 vendor;
	u32 device;
};

enum vhost_notify_event {
	NOTIFY_SET_STATUS,
	NOTIFY_FINALIZE_FEATURES,
	NOTIFY_RESET,
};

typedef void vhost_vq_callback_t(struct vhost_virtqueue *);
/**
 * struct vhost_config_ops - set of function pointers for performing vhost
 *   device specific operation
 * @create_vqs: ops to create vhost virtqueue
 * @del_vqs: ops to delete vhost virtqueue
 * @write: ops to write data to buffer provided by remote virtio driver
 * @read: ops to read data from buffer provided by remote virtio driver
 * @set_features: ops to set vhost device features
 * @set_status: ops to set vhost device status
 * @get_status: ops to get vhost device status
 */
struct vhost_config_ops {
	int (*create_vqs)(struct vhost_dev *vdev, unsigned int nvqs,
			  unsigned int num_bufs, struct vhost_virtqueue *vqs[],
			  vhost_vq_callback_t *callbacks[],
			  const char * const names[]);
	void (*del_vqs)(struct vhost_dev *vdev);
	int (*write)(struct vhost_dev *vdev, u64 vhost_dst, void *src, int len);
	int (*read)(struct vhost_dev *vdev, void *dst, u64 vhost_src, int len);
	int (*set_features)(struct vhost_dev *vdev, u64 device_features);
	int (*set_status)(struct vhost_dev *vdev, u8 status);
	u8 (*get_status)(struct vhost_dev *vdev);
};

struct vhost_driver {
	struct device_driver driver;
	struct vhost_device_id *id_table;
	struct config_group *group;
	int (*probe)(struct vhost_dev *dev);
	int (*remove)(struct vhost_dev *dev);
};

#define to_vhost_driver(drv) (container_of((drv), struct vhost_driver, driver))

struct vhost_dev {
	struct device dev;
	struct vhost_driver *driver;
	struct vhost_device_id id;
	int index;
	const struct vhost_config_ops *ops;
	struct blocking_notifier_head notifier;
	struct mm_struct *mm;
	struct mutex mutex;
	struct vhost_virtqueue **vqs;
	u64 features;
	int nvqs;
	struct eventfd_ctx *log_ctx;
	struct llist_head work_list;
	struct task_struct *worker;
	struct vhost_iotlb *umem;
	struct vhost_iotlb *iotlb;
	spinlock_t iotlb_lock;
	struct list_head read_list;
	struct list_head pending_list;
	wait_queue_head_t wait;
	int iov_limit;
	int weight;
	int byte_weight;
	u64 kcov_handle;
	bool use_worker;
	int (*msg_handler)(struct vhost_dev *dev,
			   struct vhost_iotlb_msg *msg);
};

#define to_vhost_dev(d) container_of((d), struct vhost_dev, dev)

static inline void vhost_set_drvdata(struct vhost_dev *vdev, void *data)
{
	dev_set_drvdata(&vdev->dev, data);
}

static inline void *vhost_get_drvdata(struct vhost_dev *vdev)
{
	return dev_get_drvdata(&vdev->dev);
}

static inline
struct vhost_driver_item *to_vhost_driver_item(struct config_item *item)
{
	return container_of(to_config_group(item), struct vhost_driver_item,
			    group);
}

int vhost_register_driver(struct vhost_driver *driver);
void vhost_unregister_driver(struct vhost_driver *driver);
int vhost_register_device(struct vhost_dev *vdev);
void vhost_unregister_device(struct vhost_dev *vdev);

struct config_group *vhost_cfs_add_driver_group(const char *name);
void vhost_cfs_remove_driver_group(struct config_group *group);
struct config_group *
vhost_cfs_add_driver_item(struct config_group *driver_group, u32 vendor,
			  u32 device);
void vhost_cfs_remove_driver_item(struct config_group *group);
struct config_group *vhost_cfs_add_device_item(struct vhost_dev *vdev);
void vhost_cfs_remove_device_item(struct config_group *group);

int vhost_create_vqs(struct vhost_dev *vdev, unsigned int nvqs,
		     unsigned int num_bufs, struct vhost_virtqueue *vqs[],
		     vhost_vq_callback_t *callbacks[],
		     const char * const names[]);
void vhost_del_vqs(struct vhost_dev *vdev);
int vhost_write(struct vhost_dev *vdev, u64 vhost_dst, void *src, int len);
int vhost_read(struct vhost_dev *vdev, void *dst, u64 vhost_src, int len);
int vhost_set_features(struct vhost_dev *vdev, u64 device_features);
u64 vhost_get_features(struct vhost_dev *vdev);
int vhost_set_status(struct vhost_dev *vdev, u8 status);
u8 vhost_get_status(struct vhost_dev *vdev);

int vhost_register_notifier(struct vhost_dev *vdev, struct notifier_block *nb);

u64 vhost_virtqueue_get_outbuf(struct vhost_virtqueue *vq, u16 *head, int *len);
u64 vhost_virtqueue_get_inbuf(struct vhost_virtqueue *vq, u16 *head, int *len);
void vhost_virtqueue_put_buf(struct vhost_virtqueue *vq, u16 head, int len);

void vhost_virtqueue_disable_cb(struct vhost_virtqueue *vq);
bool vhost_virtqueue_enable_cb(struct vhost_virtqueue *vq);
void vhost_virtqueue_notify(struct vhost_virtqueue *vq);
void vhost_virtqueue_kick(struct vhost_virtqueue *vq);
void vhost_virtqueue_callback(struct vhost_virtqueue *vq);

bool vhost_exceeds_weight(struct vhost_virtqueue *vq, int pkts, int total_len);
void vhost_dev_init(struct vhost_dev *, struct vhost_virtqueue **vqs,
		    int nvqs, int iov_limit, int weight, int byte_weight,
		    bool use_worker,
		    int (*msg_handler)(struct vhost_dev *dev,
				       struct vhost_iotlb_msg *msg));
long vhost_dev_set_owner(struct vhost_dev *dev);
bool vhost_dev_has_owner(struct vhost_dev *dev);
long vhost_dev_check_owner(struct vhost_dev *);
struct vhost_iotlb *vhost_dev_reset_owner_prepare(void);
void vhost_dev_reset_owner(struct vhost_dev *dev, struct vhost_iotlb *iotlb);
void vhost_dev_cleanup(struct vhost_dev *);
void vhost_dev_stop(struct vhost_dev *);
long vhost_dev_ioctl(struct vhost_dev *, unsigned int ioctl, void __user *argp);
long vhost_vring_ioctl(struct vhost_dev *d, unsigned int ioctl, void __user *argp);
bool vhost_vq_access_ok(struct vhost_virtqueue *vq);
bool vhost_log_access_ok(struct vhost_dev *);

int vhost_get_vq_desc(struct vhost_virtqueue *,
		      struct iovec iov[], unsigned int iov_count,
		      unsigned int *out_num, unsigned int *in_num,
		      struct vhost_log *log, unsigned int *log_num);
void vhost_discard_vq_desc(struct vhost_virtqueue *, int n);

bool vhost_vq_is_setup(struct vhost_virtqueue *vq);
int vhost_vq_init_access(struct vhost_virtqueue *);
int vhost_add_used(struct vhost_virtqueue *, unsigned int head, int len);
int vhost_add_used_n(struct vhost_virtqueue *, struct vring_used_elem *heads,
		     unsigned count);
void vhost_add_used_and_signal(struct vhost_dev *, struct vhost_virtqueue *,
			       unsigned int id, int len);
void vhost_add_used_and_signal_n(struct vhost_dev *, struct vhost_virtqueue *,
			       struct vring_used_elem *heads, unsigned count);
void vhost_signal(struct vhost_dev *, struct vhost_virtqueue *);
void vhost_disable_notify(struct vhost_dev *, struct vhost_virtqueue *);
bool vhost_vq_avail_empty(struct vhost_dev *, struct vhost_virtqueue *);
bool vhost_enable_notify(struct vhost_dev *, struct vhost_virtqueue *);

int vhost_log_write(struct vhost_virtqueue *vq, struct vhost_log *log,
		    unsigned int log_num, u64 len,
		    struct iovec *iov, int count);
int vq_meta_prefetch(struct vhost_virtqueue *vq);

struct vhost_msg_node *vhost_new_msg(struct vhost_virtqueue *vq, int type);
void vhost_enqueue_msg(struct vhost_dev *dev,
		       struct list_head *head,
		       struct vhost_msg_node *node);
struct vhost_msg_node *vhost_dequeue_msg(struct vhost_dev *dev,
					 struct list_head *head);
void vhost_set_backend_features(struct vhost_dev *dev, u64 features);

__poll_t vhost_chr_poll(struct file *file, struct vhost_dev *dev,
			    poll_table *wait);
ssize_t vhost_chr_read_iter(struct vhost_dev *dev, struct iov_iter *to,
			    int noblock);
ssize_t vhost_chr_write_iter(struct vhost_dev *dev,
			     struct iov_iter *from);
int vhost_init_device_iotlb(struct vhost_dev *d, bool enabled);

void vhost_iotlb_map_free(struct vhost_iotlb *iotlb,
			  struct vhost_iotlb_map *map);

#define vq_err(vq, fmt, ...) do {                                  \
		pr_debug(pr_fmt(fmt), ##__VA_ARGS__);       \
		if ((vq)->error_ctx)                               \
				eventfd_signal((vq)->error_ctx, 1);\
	} while (0)

enum {
	VHOST_FEATURES = (1ULL << VIRTIO_F_NOTIFY_ON_EMPTY) |
			 (1ULL << VIRTIO_RING_F_INDIRECT_DESC) |
			 (1ULL << VIRTIO_RING_F_EVENT_IDX) |
			 (1ULL << VHOST_F_LOG_ALL) |
			 (1ULL << VIRTIO_F_ANY_LAYOUT) |
			 (1ULL << VIRTIO_F_VERSION_1)
};

/**
 * vhost_vq_set_backend - Set backend.
 *
 * @vq            Virtqueue.
 * @private_data  The private data.
 *
 * Context: Need to call with vq->mutex acquired.
 */
static inline void vhost_vq_set_backend(struct vhost_virtqueue *vq,
					void *private_data)
{
	vq->private_data = private_data;
}

/**
 * vhost_vq_get_backend - Get backend.
 *
 * @vq            Virtqueue.
 *
 * Context: Need to call with vq->mutex acquired.
 * Return: Private data previously set with vhost_vq_set_backend.
 */
static inline void *vhost_vq_get_backend(struct vhost_virtqueue *vq)
{
	return vq->private_data;
}

static inline bool vhost_has_feature(struct vhost_dev *vdev, int bit)
{
	return vdev->features & (1ULL << bit);
}

static inline bool vhost_backend_has_feature(struct vhost_virtqueue *vq, int bit)
{
	return vq->acked_backend_features & (1ULL << bit);
}

#ifdef CONFIG_VHOST_CROSS_ENDIAN_LEGACY
static inline bool vhost_is_little_endian(struct vhost_virtqueue *vq)
{
	return vq->is_le;
}
#else
static inline bool vhost_is_little_endian(struct vhost_virtqueue *vq)
{
	return virtio_legacy_is_little_endian() || vq->is_le;
}
#endif

/* Memory accessors */
static inline u16 vhost16_to_cpu(struct vhost_virtqueue *vq, __virtio16 val)
{
	return __virtio16_to_cpu(vhost_is_little_endian(vq), val);
}

static inline __virtio16 cpu_to_vhost16(struct vhost_virtqueue *vq, u16 val)
{
	return __cpu_to_virtio16(vhost_is_little_endian(vq), val);
}

static inline u32 vhost32_to_cpu(struct vhost_virtqueue *vq, __virtio32 val)
{
	return __virtio32_to_cpu(vhost_is_little_endian(vq), val);
}

static inline __virtio32 cpu_to_vhost32(struct vhost_virtqueue *vq, u32 val)
{
	return __cpu_to_virtio32(vhost_is_little_endian(vq), val);
}

static inline u64 vhost64_to_cpu(struct vhost_virtqueue *vq, __virtio64 val)
{
	return __virtio64_to_cpu(vhost_is_little_endian(vq), val);
}

static inline __virtio64 cpu_to_vhost64(struct vhost_virtqueue *vq, u64 val)
{
	return __cpu_to_virtio64(vhost_is_little_endian(vq), val);
}
#endif
