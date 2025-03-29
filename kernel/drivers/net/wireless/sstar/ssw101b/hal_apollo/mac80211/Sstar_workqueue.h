/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef SSTAR_WORKQUEUE_H
#define SSTAR_WORKQUEUE_H
#ifdef CONFIG_SSTAR_SELF_WORKQUEUE
//#define CONFIG_SSTAR_WK_TEST
#define SSTAR_WORK_STRUCT_PENDING_BIT 0 /* work item is pending execution */
#define SSTAR_WORK_STRUCT_DELAYED_BIT 1 /* work item is delayed */
#define SSTAR_WORK_STRUCT_WAIT_BIT 2
#define SSTAR_WORK_STRUCT_RUNNING_BIT 3
#define SSTAR_WORK_STRUCT_BARRIER_BIT 4

#ifdef CONFIG_SSTAR_WK_TEST
#define SSTAR_WORK_STRUCT_TEST_BIT 5
#define SSTAR_WORK_STRUCT_TEST_PENDING_BIT                                     \
	(SSTAR_WORK_STRUCT_TEST_BIT + SSTAR_WORK_STRUCT_PENDING_BIT)
#define SSTAR_WORK_STRUCT_TEST_DELAYED_BIT                                     \
	(SSTAR_WORK_STRUCT_TEST_BIT + SSTAR_WORK_STRUCT_DELAYED_BIT)
#define SSTAR_WORK_STRUCT_TEST_WAIT_BIT                                        \
	(SSTAR_WORK_STRUCT_TEST_BIT + SSTAR_WORK_STRUCT_WAIT_BIT)
#define SSTAR_WORK_STRUCT_TEST_RUNNING_BIT                                     \
	(SSTAR_WORK_STRUCT_TEST_BIT + SSTAR_WORK_STRUCT_RUNNING_BIT)
#endif
#define Sstar_work_data_bits(work) ((unsigned long *)(&(work)->data))

struct Sstar_workqueue_struct;
struct Sstar_work_struct;
struct Sstar_workqueue_struct {
	struct list_head works;
	struct list_head flush_works;
	struct list_head delay_works;
	struct task_struct __rcu *work_thread;
	unsigned long flags;
	spinlock_t lock;
	bool flush_empty;
	wait_queue_head_t wait_flushed;
	const char *name;
	struct mutex mutex;
};
typedef void (*Sstar_work_func_t)(struct Sstar_work_struct *work);

struct Sstar_work_struct {
	unsigned long data;
	struct list_head entry;
	Sstar_work_func_t func;
	spinlock_t lock;
	wait_queue_head_t timeout;
	wait_queue_head_t barrier;
	struct Sstar_workqueue_struct *wq;
	const char *name;
#ifdef CONFIG_SSTAR_WK_TEST
	wait_queue_head_t running;
	wait_queue_head_t pending;
	wait_queue_head_t delayed;
#endif
};
struct Sstar_delayed_work {
	struct Sstar_work_struct work;
	struct Sstar_timer_list timer;
	struct Sstar_workqueue_struct __rcu *wq;
	int n_started;
	int n_timeout;
};
void ieee80211_Sstar_init_work(struct Sstar_work_struct *work,
			       Sstar_work_func_t func, const char *name);
void ieee80211_Sstar_init_delay_work(struct Sstar_delayed_work *work,
				     Sstar_work_func_t func, const char *name);
struct Sstar_workqueue_struct *ieee80211_Sstar_alloc_workqueue(const char *name,
							       long flags);
bool ieee80211_Sstar_queue_work(struct Sstar_workqueue_struct *wq,
				struct Sstar_work_struct *work);
bool ieee80211_Sstar_queue_delayed_work(struct Sstar_workqueue_struct *wq,
					struct Sstar_delayed_work *dwork,
					unsigned long delay);
void ieee80211_Sstar_flush_workqueue(struct Sstar_workqueue_struct *wq);
void ieee80211_Sstar_destroy_workqueue(struct Sstar_workqueue_struct *wq);
bool ieee80211_Sstar_flush_delayed_work(struct Sstar_delayed_work *dwork);
int ieee80211_Sstar_schedule_work(struct Sstar_work_struct *work);
bool ieee80211_Sstar_cancel_delayed_work_sync(struct Sstar_delayed_work *dwork);
bool ieee80211_Sstar_cancel_delayed_work(struct Sstar_delayed_work *dwork);
bool ieee80211_Sstar_cancel_work_sync(struct Sstar_work_struct *work);
bool ieee80211_Sstar_flush_work(struct Sstar_work_struct *work);
void ieee80211_Sstar_workqueue_init(void);
void ieee80211_Sstar_workqueue_exit(void);

#define Sstar_cancel_delayed_work ieee80211_Sstar_cancel_delayed_work
#define Sstar_cancel_delayed_work_sync ieee80211_Sstar_cancel_delayed_work_sync
#define Sstar_cancel_work_sync ieee80211_Sstar_cancel_work_sync
#define Sstar_queue_work ieee80211_Sstar_queue_work
#define Sstar_queue_delayed_work ieee80211_Sstar_queue_delayed_work
#define Sstar_flush_workqueue ieee80211_Sstar_flush_workqueue
#define SSTAR_INIT_WORK(_wk, _f) ieee80211_Sstar_init_work(_wk, _f, #_f);
#define Sstar_create_singlethread_workqueue(_name)                             \
	ieee80211_Sstar_alloc_workqueue(_name, 0);
#define Sstar_destroy_workqueue ieee80211_Sstar_destroy_workqueue
#define SSTAR_INIT_DELAYED_WORK(_dwk, _f)                                      \
	ieee80211_Sstar_init_delay_work(_dwk, _f, #_f)
#define Sstar_flush_delayed_work ieee80211_Sstar_flush_delayed_work
#define Sstar_schedule_work ieee80211_Sstar_schedule_work
#define Sstar_alloc_ordered_workqueue ieee80211_Sstar_alloc_workqueue
#define Sstar_work_pending(wk)                                                 \
	test_bit(SSTAR_WORK_STRUCT_PENDING_BIT, Sstar_work_data_bits(wk))
#define Sstar_flush_work ieee80211_Sstar_flush_work
#define Sstar_wq_init() ieee80211_Sstar_workqueue_init()
#define Sstar_wq_exit() ieee80211_Sstar_workqueue_exit()

static inline struct Sstar_delayed_work *
Sstar_to_delayed_work(struct Sstar_work_struct *work)
{
	return container_of(work, struct Sstar_delayed_work, work);
}

#else
#define Sstar_cancel_delayed_work cancel_delayed_work
#define Sstar_cancel_delayed_work_sync cancel_delayed_work_sync
#define Sstar_cancel_work_sync cancel_work_sync
#define Sstar_queue_work queue_work
#define Sstar_queue_delayed_work queue_delayed_work
#define Sstar_flush_workqueue flush_workqueue
#define SSTAR_INIT_WORK INIT_WORK
#define Sstar_create_singlethread_workqueue create_singlethread_workqueue
#define Sstar_destroy_workqueue destroy_workqueue
#define SSTAR_INIT_DELAYED_WORK INIT_DELAYED_WORK
#define Sstar_flush_delayed_work flush_delayed_work
#define Sstar_schedule_work schedule_work
#define Sstar_alloc_ordered_workqueue alloc_ordered_workqueue
#define Sstar_work_pending work_pending
#define Sstar_to_delayed_work to_delayed_work
#define Sstar_flush_work flush_work
#define Sstar_wq_init()
#define Sstar_wq_exit()
#endif
#endif /* SSTAR_WORKQUEUE_H */
