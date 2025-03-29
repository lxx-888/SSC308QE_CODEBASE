/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifdef CONFIG_SSTAR_SELF_WORKQUEUE
#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/gfp.h>
#include <asm/unaligned.h>
#include <net/Sstar_mac80211.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <net/Sstar_mac80211.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/freezer.h>
#include "Sstar_workqueue.h"
#include "ieee80211_Sstar_mem.h"
enum {
	WQ_THREAD_WAKEUP,
	WQ_THREAD_SHOULD_SUSPEND,
	WQ_THREAD_SUSPENED,
	WQ_THREAD_SHOULD_STOP,
};
struct work_wait {
	struct Sstar_work_struct work;
};

#define WQ_THREAD_PR 0

static struct Sstar_workqueue_struct *common_wq = NULL;
/*
*test wk bit
*/
#define wk_is_pending(wk)                                                      \
	test_bit(SSTAR_WORK_STRUCT_PENDING_BIT, Sstar_work_data_bits(wk))
#define wk_is_delayed(wk)                                                      \
	test_bit(SSTAR_WORK_STRUCT_DELAYED_BIT, Sstar_work_data_bits(wk))
#define wk_is_running(wk)                                                      \
	test_bit(SSTAR_WORK_STRUCT_RUNNING_BIT, Sstar_work_data_bits(wk))
#define wk_is_waiting(wk)                                                      \
	test_bit(SSTAR_WORK_STRUCT_WAIT_BIT, Sstar_work_data_bits(wk))
#define wk_is_barrier(wk)                                                      \
	test_bit(SSTAR_WORK_STRUCT_BARRIER_BIT, Sstar_work_data_bits(wk))

/*
*clear wk bit
*/
#define wk_clear_pending(wk)                                                   \
	clear_bit(SSTAR_WORK_STRUCT_PENDING_BIT, Sstar_work_data_bits(wk))
#define wk_clear_delayed(wk)                                                   \
	clear_bit(SSTAR_WORK_STRUCT_DELAYED_BIT, Sstar_work_data_bits(wk))
#define wk_clear_running(wk)                                                   \
	clear_bit(SSTAR_WORK_STRUCT_RUNNING_BIT, Sstar_work_data_bits(wk))
#define wk_clear_waiting(wk)                                                   \
	clear_bit(SSTAR_WORK_STRUCT_WAIT_BIT, Sstar_work_data_bits(wk))
#define wk_clear_barrier(wk)                                                   \
	clear_bit(SSTAR_WORK_STRUCT_BARRIER_BIT, Sstar_work_data_bits(wk))

/*
*set wk bit
*/
#define wk_set_pending(wk)                                                     \
	set_bit(SSTAR_WORK_STRUCT_PENDING_BIT, Sstar_work_data_bits(wk))
#define wk_set_delayed(wk)                                                     \
	set_bit(SSTAR_WORK_STRUCT_DELAYED_BIT, Sstar_work_data_bits(wk))
#define wk_set_running(wk)                                                     \
	set_bit(SSTAR_WORK_STRUCT_RUNNING_BIT, Sstar_work_data_bits(wk))
#define wk_set_waiting(wk)                                                     \
	set_bit(SSTAR_WORK_STRUCT_WAIT_BIT, Sstar_work_data_bits(wk))
#define wk_set_barrier(wk)                                                     \
	set_bit(SSTAR_WORK_STRUCT_BARRIER_BIT, Sstar_work_data_bits(wk))

#define wk_is_busy(_wk)                                                        \
	(wk_is_pending(_wk) || wk_is_delayed(_wk) || wk_is_running(_wk))

#ifdef CONFIG_SSTAR_WK_TEST
static struct task_struct *test_thread = NULL;
#define WQ_THREAD_TEST_PR WQ_THREAD_PR + 1

#define wk_is_testpending(wk)                                                  \
	test_bit(SSTAR_WORK_STRUCT_TEST_PENDING_BIT, Sstar_work_data_bits(wk))
#define wk_is_testdelayed(wk)                                                  \
	test_bit(SSTAR_WORK_STRUCT_TEST_DELAYED_BIT, Sstar_work_data_bits(wk))
#define wk_is_testrunning(wk)                                                  \
	test_bit(SSTAR_WORK_STRUCT_TEST_RUNNING_BIT, Sstar_work_data_bits(wk))
#define wk_is_testwaiting(wk)                                                  \
	test_bit(SSTAR_WORK_STRUCT_TEST_WAIT_BIT, Sstar_work_data_bits(wk))

#define wk_clear_testpending(wk)                                               \
	clear_bit(SSTAR_WORK_STRUCT_TEST_PENDING_BIT, Sstar_work_data_bits(wk))
#define wk_clear_testdelayed(wk)                                               \
	clear_bit(SSTAR_WORK_STRUCT_TEST_DELAYED_BIT, Sstar_work_data_bits(wk))
#define wk_clear_testrunning(wk)                                               \
	clear_bit(SSTAR_WORK_STRUCT_TEST_RUNNING_BIT, Sstar_work_data_bits(wk))
#define wk_clear_testwaiting(wk)                                               \
	clear_bit(SSTAR_WORK_STRUCT_TEST_WAIT_BIT, Sstar_work_data_bits(wk))

#define wk_set_testpending(wk)                                                 \
	set_bit(SSTAR_WORK_STRUCT_TEST_PENDING_BIT, Sstar_work_data_bits(wk))
#define wk_set_testdelayed(wk)                                                 \
	set_bit(SSTAR_WORK_STRUCT_TEST_DELAYED_BIT, Sstar_work_data_bits(wk))
#define wk_set_testrunning(wk)                                                 \
	set_bit(SSTAR_WORK_STRUCT_TEST_RUNNING_BIT, Sstar_work_data_bits(wk))
#define wk_set_testwaiting(wk)                                                 \
	set_bit(SSTAR_WORK_STRUCT_TEST_WAIT_BIT, Sstar_work_data_bits(wk))

static void Sstar_workqueue_test_work(struct Sstar_work_struct *work)
{
}
static void Sstar_workqueue_test_delay_work(struct Sstar_work_struct *work)
{
}
static void Sstar_wk_test_init(struct Sstar_work_struct *work)
{
	init_waitqueue_head(&work->running);
	init_waitqueue_head(&work->pending);
	init_waitqueue_head(&work->delayed);
}
static int Sstar_workqueue_test_thread(void *priv)
{
	struct sched_param param = { .sched_priority = WQ_THREAD_TEST_PR };
	struct Sstar_work_struct work;
	struct Sstar_delayed_work delay_work;
	struct Sstar_workqueue_struct *wq = NULL;
	bool ret = false;

	ieee80211_Sstar_init_work(&work, Sstar_workqueue_test_work,
				  "test_work");
	ieee80211_Sstar_init_delay_work(&delay_work,
					Sstar_workqueue_test_delay_work,
					"test_delay_work");
	/*
	*the policy of the sheduler is same with the sdio irq thread
	*/
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 9, 0))
	sched_set_fifo(current);
#else
	sched_setscheduler(current, SCHED_FIFO, &param);
#endif
	set_current_state(TASK_INTERRUPTIBLE);
	while (!kthread_should_stop()) {
		set_current_state(TASK_INTERRUPTIBLE);
		if (!kthread_should_stop())
			schedule_timeout(2 * HZ);
		__set_current_state(TASK_RUNNING);

		wq = ieee80211_Sstar_alloc_workqueue("wq_test", 0);

		if (wq == NULL) {
			Sstar_printk_always("%s err\n", __func__);
			break;
		}
		/*
		*[TEST] flush pending work function
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST WK] flush pending work\n");
		ieee80211_Sstar_queue_work(wq, &work);
		ret = ieee80211_Sstar_flush_work(&work);
		Sstar_printk_always("[TEST WK] flush pending work finish(%d)\n",
				    ret);
		/*
		*[TEST] flush running work function
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST WK] flush running work\n");
		wk_set_testrunning(&work);
		ieee80211_Sstar_queue_work(wq, &work);
		wait_event_interruptible(work.running,
					 wk_is_testrunning(&work) == 0);
		ret = ieee80211_Sstar_flush_work(&work);
		Sstar_printk_always("[TEST WK] flush running work finish(%d)\n",
				    ret);
		/*
		*[TEST] concle work pending
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST WK] concle work pending\n");
		ieee80211_Sstar_queue_work(wq, &work);
		ret = ieee80211_Sstar_cancel_work_sync(&work);
		Sstar_printk_always(
			"[TEST WK] concle work pending finish(%d)\n", ret);
		/*
		*[TEST] concle work running
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST WK] concle work running\n");
		wk_set_testrunning(&work);
		ieee80211_Sstar_queue_work(wq, &work);
		wait_event_interruptible(work.running,
					 wk_is_testrunning(&work) == 0);
		ret = ieee80211_Sstar_cancel_work_sync(&work);
		Sstar_printk_always("[TEST WK] concle work running(%d)\n", ret);
		/*
		*[TEST] concle dup work
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST WK] concle dup work\n");
		wk_set_testrunning(&work);
		ieee80211_Sstar_queue_work(wq, &work);
		wait_event_interruptible(work.running,
					 wk_is_testrunning(&work) == 0);
		ieee80211_Sstar_queue_work(wq, &work);
		ret = ieee80211_Sstar_cancel_work_sync(&work);
		Sstar_printk_always("[TEST WK] concle dup work (%d)\n", ret);
		/*
		*[TEST] queue dup work
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST WK] que dup work\n");
		ieee80211_Sstar_queue_work(wq, &work);
		ret = ieee80211_Sstar_queue_work(wq, &work);
		Sstar_printk_always("[TEST WK] que dup work (%d)\n", ret);
		////////////////////////////////////////////////////////////
		/*
		*[TEST] flush delayed work
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST DWK] flush delayed work\n");
		ieee80211_Sstar_queue_delayed_work(wq, &delay_work, HZ / 10);
		ret = ieee80211_Sstar_flush_delayed_work(&delay_work);
		Sstar_printk_always("[TEST DWK] flush delayed work (%d)\n",
				    ret);
		/*
		*[TEST] flush delayed work already in pending
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST DWK] flush pending work\n");
		wk_set_testpending(&delay_work.work);
		ieee80211_Sstar_queue_delayed_work(wq, &delay_work, HZ / 10);
		wait_event_interruptible(delay_work.work.pending,
					 wk_is_testpending(&delay_work.work) ==
						 0);
		ret = ieee80211_Sstar_flush_delayed_work(&delay_work);
		Sstar_printk_always("[TEST DWK] flush pending work (%d)\n",
				    ret);
		/*
		*[TEST] flush delayed work already in running
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST DWK] flush running work\n");
		wk_set_testrunning(&delay_work.work);
		ieee80211_Sstar_queue_delayed_work(wq, &delay_work, HZ / 10);
		wait_event_interruptible(delay_work.work.running,
					 wk_is_testrunning(&delay_work.work) ==
						 0);
		ret = ieee80211_Sstar_flush_delayed_work(&delay_work);
		Sstar_printk_always("[TEST DWK] flush running work(%d)\n", ret);

		///////////////////////////////////////////////////////////////
		/*
		*[TEST] concle delayed work
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST DWK] concle delayed work\n");
		ieee80211_Sstar_queue_delayed_work(wq, &delay_work, HZ / 10);
		ret = ieee80211_Sstar_cancel_delayed_work(&delay_work);
		Sstar_printk_always("[TEST DWK] concle delayed work(%d)\n",
				    ret);
		/*
		*[TEST] concle pending work
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST DWK] concle pending work\n");
		wk_set_testpending(&delay_work.work);
		ieee80211_Sstar_queue_delayed_work(wq, &delay_work, HZ / 10);
		wait_event_interruptible(delay_work.work.pending,
					 wk_is_testpending(&delay_work.work) ==
						 0);
		ret = ieee80211_Sstar_cancel_delayed_work(&delay_work);
		Sstar_printk_always("[TEST DWK] concle pending work(%d)\n",
				    ret);
		/*
		*[TEST] concle running work
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST DWK] concle running work\n");
		wk_set_testrunning(&delay_work.work);
		ieee80211_Sstar_queue_delayed_work(wq, &delay_work, HZ / 10);
		wait_event_interruptible(delay_work.work.running,
					 wk_is_testrunning(&delay_work.work) ==
						 0);
		ret = ieee80211_Sstar_cancel_delayed_work(&delay_work);
		Sstar_printk_always("[TEST DWK] concle running work(%d)\n",
				    ret);

		//////////////////////////////////////////////////////////////
		/*
		*[TEST] concle delayed work sync
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST DWK] concle delayed work sync\n");
		ieee80211_Sstar_queue_delayed_work(wq, &delay_work, HZ / 10);
		ret = ieee80211_Sstar_cancel_delayed_work_sync(&delay_work);
		Sstar_printk_always("[TEST DWK] concle delayed work sync(%d)\n",
				    ret);
		/*
		*[TEST] concle pending work sync
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST DWK] concle pending work sync\n");
		wk_set_testpending(&delay_work.work);
		ieee80211_Sstar_queue_delayed_work(wq, &delay_work, HZ / 10);
		wait_event_interruptible(delay_work.work.pending,
					 wk_is_testpending(&delay_work.work) ==
						 0);
		ret = ieee80211_Sstar_cancel_delayed_work_sync(&delay_work);
		Sstar_printk_always("[TEST DWK] concle pending work sync(%d)\n",
				    ret);
		/*
		*[TEST] concle running work sync
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST DWK] concle running work sync\n");
		wk_set_testrunning(&delay_work.work);
		ieee80211_Sstar_queue_delayed_work(wq, &delay_work, HZ / 10);
		wait_event_interruptible(delay_work.work.running,
					 wk_is_testrunning(&delay_work.work) ==
						 0);
		ret = ieee80211_Sstar_cancel_delayed_work_sync(&delay_work);
		Sstar_printk_always("[TEST DWK] concle running work sync(%d)\n",
				    ret);
		/*
		*[TEST] concle delayed work sync
		*/
		/*
		*destroy wq
		*/
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always("[TEST WQ] destroy wq\n");
		ieee80211_Sstar_queue_work(wq, &work);
		ieee80211_Sstar_queue_delayed_work(wq, &delay_work, HZ / 10);
		ieee80211_Sstar_destroy_workqueue(wq);
		wq = NULL;
		Sstar_printk_always(
			"############################################################");
		Sstar_printk_always(
			"############################################################");
	}
	__set_current_state(TASK_RUNNING);
	if (wq) {
		ieee80211_Sstar_flush_work(&work);
		ieee80211_Sstar_flush_delayed_work(&delay_work);
		ieee80211_Sstar_destroy_workqueue(wq);
	}

	return 0;
}
static void Sstar_workqueue_test_thread_int(void)
{
	test_thread = kthread_create(Sstar_workqueue_test_thread, NULL,
				     "test_thread");

	if (IS_ERR(test_thread)) {
		Sstar_printk_always("workqueue test thread err\n");
	} else {
		wake_up_process(test_thread);
	}
}

static void Sstar_workqueue_test_thread_exit(void)
{
	if (test_thread == NULL)
		return;

	kthread_stop(test_thread);
}
#endif
static void Sstar_wk_go_running(struct Sstar_work_struct *work)
{
	wk_set_running(work);
#ifdef CONFIG_SSTAR_WK_TEST
	if (wk_is_testrunning(work)) {
		wk_clear_testrunning(work);
		wake_up(&work->running);
	}
#endif
}

static void Sstar_wk_out_running(struct Sstar_work_struct *work)
{
	wk_clear_running(work);
}

static void Sstar_wk_go_pending(struct Sstar_work_struct *work)
{
	wk_set_pending(work);
#ifdef CONFIG_SSTAR_WK_TEST
	if (wk_is_testpending(work)) {
		wk_clear_testpending(work);
		wake_up(&work->pending);
	}
#endif
}

static void Sstar_wk_out_pending(struct Sstar_work_struct *work)
{
	wk_clear_pending(work);
}

static void Sstar_wk_go_delayed(struct Sstar_work_struct *work)
{
	wk_set_delayed(work);
}

static void Sstar_wk_out_delayed(struct Sstar_work_struct *work)
{
	wk_clear_delayed(work);
}
static void Sstar_wk_go_waiting(struct Sstar_work_struct *work)
{
	wk_set_waiting(work);
}

static void Sstar_wk_out_waiting(struct Sstar_work_struct *work)
{
	wk_clear_waiting(work);
}

static void Sstar_wk_go_barrier(struct Sstar_work_struct *work)
{
	wk_set_barrier(work);
}

static void Sstar_wk_out_barrier(struct Sstar_work_struct *work)
{
	if (wk_is_barrier(work)) {
		wk_clear_barrier(work);
		wake_up(&work->barrier);
	}
}

static int Sstar_wk_in_runing(struct Sstar_work_struct *work)
{
	return !!wk_is_running(work);
}

static int Sstar_wk_in_delayed(struct Sstar_work_struct *work)
{
	return !!wk_is_delayed(work);
}

static int Sstar_wk_in_pending(struct Sstar_work_struct *work)
{
	return !!wk_is_pending(work);
}

static int Sstar_wk_in_waiting(struct Sstar_work_struct *work)
{
	return !!wk_is_waiting(work);
}
static int Sstar_wk_in_barrier(struct Sstar_work_struct *work)
{
	return !!wk_is_barrier(work);
}
static int Sstar_workqueue_wait_action(struct Sstar_workqueue_struct *wq)
{
	set_current_state(TASK_INTERRUPTIBLE);
	while (!kthread_should_stop()) {
		if (test_and_clear_bit(WQ_THREAD_WAKEUP, &wq->flags)) {
			__set_current_state(TASK_RUNNING);
			return 0;
		}
		set_current_state(TASK_INTERRUPTIBLE);
		if (!kthread_should_stop())
			schedule_timeout(20 * HZ);
		set_current_state(TASK_INTERRUPTIBLE);
	}
	__set_current_state(TASK_RUNNING);
	return -1;
}

static void Sstar_workqueue_process_works(struct Sstar_workqueue_struct *wq)
{
	unsigned long flags;
	struct Sstar_work_struct *work = NULL;
	Sstar_printk_debug("%s in\n", wq->name);
	spin_lock_irqsave(&wq->lock, flags);
	while (!list_empty(&wq->works)) {
		work = list_first_entry(&wq->works, struct Sstar_work_struct,
					entry);
		WARN_ON(work == NULL);
		list_del(&work->entry);
		Sstar_wk_out_pending(work);
		Sstar_wk_go_running(work);
		spin_unlock_irqrestore(&wq->lock, flags);
		Sstar_printk_debug("%s in\n", work->name);
		BUG_ON(work->func == NULL);
		work->func(work);
		Sstar_printk_debug("%s out\n", work->name);
		spin_lock_irqsave(&wq->lock, flags);
		Sstar_wk_out_running(work);
		/*
		*be carefully add any code which will use work after Sstar_wk_out_barrier.
		*/
		Sstar_wk_out_barrier(work);
	}
	spin_unlock_irqrestore(&wq->lock, flags);
	Sstar_printk_debug("%s out\n", wq->name);
}
static int Sstar_workqueue_thread(void *priv)
{
	struct Sstar_workqueue_struct *wq =
		(struct Sstar_workqueue_struct *)priv;
	struct sched_param param = { .sched_priority = WQ_THREAD_PR };

	Sstar_printk_init("%s\n", wq->name);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 9, 0))
	sched_set_fifo(current);
#else
	sched_setscheduler(current, SCHED_FIFO, &param);
#endif

	while (!Sstar_workqueue_wait_action(wq)) {
		Sstar_workqueue_process_works(wq);
	};
	Sstar_printk_init("%s:exit\n", wq->name);
	Sstar_workqueue_process_works(wq);
	return 0;
}
static void ieee80211_Sstar_wait_work(struct Sstar_work_struct *work)
{
	struct work_wait *wait = container_of(work, struct work_wait, work);

	Sstar_printk_debug("%s:wait done\n", work->name);
}
static void Sstar_workqueue_wakeup(struct Sstar_workqueue_struct *wq,
				   struct task_struct *work_thread)
{
	if (test_and_set_bit(WQ_THREAD_WAKEUP, &wq->flags) == 0) {
		wake_up_process(work_thread);
	}
}
static bool _ieee80211_Sstar_queue_work(struct Sstar_workqueue_struct *wq,
					struct Sstar_work_struct *work)
{
	unsigned long flags = 0;
	bool ret = false;
	struct task_struct *thread = NULL;

	work->wq = wq;

	spin_lock_irqsave(&wq->lock, flags);
	thread = wq->work_thread;
	if (thread) {
		list_add_tail(&work->entry, &wq->works);
		Sstar_printk_debug("[queue work] [%s]->[%s]\n", work->name,
				   wq->name);
		Sstar_workqueue_wakeup(wq, thread);
		ret = true;
	} else {
		Sstar_wk_out_pending(work);
	}
	spin_unlock_irqrestore(&wq->lock, flags);

	return ret;
}
static void
_ieee80211_Sstar_clear_pending_work(struct Sstar_workqueue_struct *wq,
				    struct Sstar_work_struct *work)
{
	BUG_ON(wq == NULL);
	list_del(&work->entry);
	Sstar_wk_out_pending(work);
}
static void _ieee80211_Sstar_clear_delay_work(struct Sstar_workqueue_struct *wq,
					      struct Sstar_work_struct *work)
{
	unsigned long flags = 0;
	spin_lock_irqsave(&wq->lock, flags);
	list_del(&work->entry);
	Sstar_wk_out_delayed(work);
	if (Sstar_wk_in_waiting(work)) {
		Sstar_wk_out_waiting(work);
		wake_up(&work->timeout);
	}
	Sstar_printk_debug("[clear delay work] [%s]->[%s]\n", work->name,
			   wq->name);
	spin_unlock_irqrestore(&wq->lock, flags);
}
static void Sstar_delayed_work_timer_fn(unsigned long __data)
{
	struct Sstar_delayed_work *dwork = (struct Sstar_delayed_work *)__data;
	unsigned long flags = 0;
	/* should have been called from irqsafe timer with irq already off */
	Sstar_printk_debug("[%s] time out\n", dwork->work.name);
	spin_lock_irqsave(&dwork->work.lock, flags);
	dwork->n_timeout++;
	if (Sstar_wk_in_delayed(&dwork->work)) {
		_ieee80211_Sstar_clear_delay_work(dwork->wq, &dwork->work);
		if (!Sstar_wk_in_pending(&dwork->work)) {
			Sstar_wk_go_pending(&dwork->work);
			_ieee80211_Sstar_queue_work(dwork->wq, &dwork->work);
		} else {
			WARN_ON(1);
		}
	} else {
		Sstar_printk_debug("[%s] cancled\n", dwork->work.name);
	}
	spin_unlock_irqrestore(&dwork->work.lock, flags);
}
static void Sstar_waitwk_init(struct work_wait *wait, const char *name)
{
	ieee80211_Sstar_init_work(&wait->work, ieee80211_Sstar_wait_work, name);
}
static void Sstar_waitwk_work_for_done(struct work_wait *wait)
{
	wait_event_interruptible(wait->work.barrier,
				 Sstar_wk_in_barrier(&wait->work) == 0);
}
static void Sstar_waitwk_insert(struct work_wait *wait, struct list_head *head,
				bool tail)
{
	Sstar_wk_go_barrier(&wait->work);
	Sstar_wk_go_pending(&wait->work);

	if (tail == true) {
		list_add_tail(&wait->work.entry, head);
	} else {
		list_add(&wait->work.entry, head);
	}
}
void ieee80211_Sstar_init_work(struct Sstar_work_struct *work,
			       Sstar_work_func_t func, const char *name)
{
	INIT_LIST_HEAD(&work->entry);
	work->data = 0;
	work->func = func;
	work->name = name;
	spin_lock_init(&work->lock);
	init_waitqueue_head(&work->timeout);
	init_waitqueue_head(&work->barrier);
#ifdef CONFIG_SSTAR_WK_TEST
	Sstar_wk_test_init(work);
#endif
}
void ieee80211_Sstar_init_delay_work(struct Sstar_delayed_work *work,
				     Sstar_work_func_t func, const char *name)
{
	ieee80211_Sstar_init_work(&work->work, func, name);
	work->n_started = 0;
	work->n_timeout = 0;
	Sstar_setup_timer(&work->timer, Sstar_delayed_work_timer_fn,
			  (unsigned long)(work));
}

struct Sstar_workqueue_struct *ieee80211_Sstar_alloc_workqueue(const char *name,
							       long flags)
{
	struct Sstar_workqueue_struct *wq;
	void *bh;
	Sstar_printk_always("%s:[%s] want alloc wq\n", __func__, name);
	wq = Sstar_kzalloc(sizeof(struct Sstar_workqueue_struct), GFP_KERNEL);

	if (wq == NULL)
		goto err;

	INIT_LIST_HEAD(&wq->works);
	INIT_LIST_HEAD(&wq->flush_works);
	INIT_LIST_HEAD(&wq->delay_works);
	spin_lock_init(&wq->lock);
	init_waitqueue_head(&wq->wait_flushed);
	mutex_init(&wq->mutex);
	wq->name = name;
	wq->flush_empty = true;
	bh = kthread_create(Sstar_workqueue_thread, wq, name);
	if (IS_ERR(bh)) {
		Sstar_printk_err("workqueue %s err\n", name);
		goto err;
	} else {
		spin_lock_bh(&wq->lock);
		rcu_assign_pointer(wq->work_thread, bh);
		spin_unlock_bh(&wq->lock);
		wake_up_process(wq->work_thread);
	}
	Sstar_printk_always("%s:alloc[%s]\n", __func__, wq->name);
	return wq;
err:
	if (wq)
		Sstar_kfree(wq);
	return NULL;
}
void ieee80211_Sstar_workqueue_init(void)
{
	common_wq = ieee80211_Sstar_alloc_workqueue("Sstar_cowq", 0);
	WARN_ON(common_wq == NULL);
#ifdef CONFIG_SSTAR_WK_TEST
	Sstar_workqueue_test_thread_int();
#endif
}
void ieee80211_Sstar_workqueue_exit(void)
{
	if (common_wq)
		ieee80211_Sstar_destroy_workqueue(common_wq);
#ifdef CONFIG_SSTAR_WK_TEST
	Sstar_workqueue_test_thread_exit();
#endif
}
bool ieee80211_Sstar_queue_work(struct Sstar_workqueue_struct *wq,
				struct Sstar_work_struct *work)
{
	unsigned long flags = 0;
	bool ret = false;

	spin_lock_irqsave(&work->lock, flags);
	if (!Sstar_wk_in_pending(work)) {
		Sstar_wk_go_pending(work);
		ret = _ieee80211_Sstar_queue_work(wq, work);
	}
	spin_unlock_irqrestore(&work->lock, flags);

	return ret;
}
static bool
_ieee80211_Sstar_queue_delayed_work(struct Sstar_workqueue_struct *wq,
				    struct Sstar_delayed_work *dwork,
				    unsigned long delay)
{
	unsigned long flags = 0;
	bool ret = false;
	struct task_struct *thread = NULL;
	struct Sstar_timer_list *timer = &dwork->timer;

	spin_lock_irqsave(&wq->lock, flags);
	thread = wq->work_thread;
	if (thread) {
		list_add_tail(&dwork->work.entry, &wq->delay_works);
		Sstar_wk_go_delayed(&dwork->work);
		dwork->wq = wq;
		timer->expires = jiffies + delay;
		dwork->n_started++;
		Sstar_add_timer(timer);
		Sstar_printk_debug("[queue delay work]->[%s]\n",
				   dwork->work.name);
		ret = true;
	} else {
		Sstar_wk_out_pending(&dwork->work);
	}
	spin_unlock_irqrestore(&wq->lock, flags);

	return ret;
}
bool ieee80211_Sstar_queue_delayed_work(struct Sstar_workqueue_struct *wq,
					struct Sstar_delayed_work *dwork,
					unsigned long delay)
{
	unsigned long flags = 0;
	bool ret = false;

	spin_lock_irqsave(&dwork->work.lock, flags);
	if (!Sstar_wk_in_pending(&dwork->work)) {
		if (!delay) {
			Sstar_wk_go_pending(&dwork->work);
			ret = _ieee80211_Sstar_queue_work(wq, &dwork->work);
			spin_unlock_irqrestore(&dwork->work.lock, flags);
			return ret;
		}
		if (Sstar_wk_in_delayed(&dwork->work)) {
			WARN_ON_ONCE(Sstar_wk_in_delayed(&dwork->work));
			if (Sstar_del_timer(&dwork->timer)) {
				dwork->n_timeout++;
				_ieee80211_Sstar_clear_delay_work(wq,
								  &dwork->work);
			} else {
				spin_unlock_irqrestore(&dwork->work.lock,
						       flags);
				Sstar_printk_always(
					"%s,delay work is runing,check code\n",
					dwork->work.name);
				return false;
			}
		}
		ret = _ieee80211_Sstar_queue_delayed_work(wq, dwork, delay);
	} else {
		Sstar_printk_debug("[%s] is pending\n", dwork->work.name);
	}
	spin_unlock_irqrestore(&dwork->work.lock, flags);

	return ret;
}

void ieee80211_Sstar_flush_workqueue(struct Sstar_workqueue_struct *wq)
{
	struct work_wait wait;
	unsigned long flags;

	spin_lock_irqsave(&wq->lock, flags);
	Sstar_printk_debug("[flush wq]->[%s] in\n", wq->name);
	Sstar_waitwk_init(&wait, "flush");

	if (wq->work_thread == NULL) {
		spin_unlock_irqrestore(&wq->lock, flags);
		return;
	}
	Sstar_waitwk_insert(&wait, &wq->works, true);
	Sstar_workqueue_wakeup(wq, wq->work_thread);
	spin_unlock_irqrestore(&wq->lock, flags);
	Sstar_waitwk_work_for_done(&wait);

	Sstar_printk_debug("[flush wq]->[%s] out\n", wq->name);
}

void ieee80211_Sstar_destroy_workqueue(struct Sstar_workqueue_struct *wq)
{
	unsigned long flags = 0;
	struct work_wait wait;
	struct task_struct *work_thread = NULL;

	Sstar_waitwk_init(&wait, "destroy");

	if (wq == NULL)
		return;

	Sstar_printk_debug("[destroy wq]->[%s] in\n", wq->name);
	spin_lock_irqsave(&wq->lock, flags);
	do {
		work_thread = wq->work_thread;
		wq->work_thread = NULL;

		if (work_thread == NULL)
			break;
		while (!list_empty(&wq->delay_works)) {
			struct Sstar_work_struct *work =
				list_first_entry(&wq->delay_works,
						 struct Sstar_work_struct,
						 entry);
			struct Sstar_delayed_work *dwork =
				Sstar_to_delayed_work(work);
			Sstar_printk_always("%s:flush start\n", work->name);
			spin_unlock_irqrestore(&wq->lock, flags);
			if (Sstar_del_timer_sync(&dwork->timer)) {
				Sstar_delayed_work_timer_fn(
					(unsigned long)dwork);
			}
			spin_lock_irqsave(&wq->lock, flags);
			Sstar_printk_always("%s:flush end\n", work->name);
		}
		Sstar_waitwk_insert(&wait, &wq->works, true);
		Sstar_workqueue_wakeup(wq, work_thread);
		spin_unlock_irqrestore(&wq->lock, flags);

		Sstar_waitwk_work_for_done(&wait);

		spin_lock_irqsave(&wq->lock, flags);

	} while (0);
	spin_unlock_irqrestore(&wq->lock, flags);

	if (work_thread) {
		kthread_stop(work_thread);
	}

	if (wq) {
		Sstar_kfree(wq);
	}
	Sstar_printk_debug("[destroy wq]->[%s] out\n", wq->name);
	return;
}

bool ieee80211_Sstar_flush_delayed_work(struct Sstar_delayed_work *dwork)
{
	struct Sstar_work_struct *work = &dwork->work;
	unsigned long flags1 = 0;
	unsigned long flags2 = 0;
	struct Sstar_workqueue_struct *wq;
	bool ret = false;
	struct work_wait wait;
	struct list_head *head;
	bool tail = false;

	Sstar_waitwk_init(&wait, dwork->work.name);

	spin_lock_irqsave(&work->lock, flags1);

	if (likely(Sstar_del_timer(&dwork->timer))) {
		BUG_ON(dwork->wq == NULL);
		dwork->n_timeout++;
		_ieee80211_Sstar_clear_delay_work(dwork->wq, &dwork->work);
		if (!Sstar_wk_in_pending(work)) {
			Sstar_wk_go_pending(work);
			ret = _ieee80211_Sstar_queue_work(dwork->wq, work);
			if (ret == false) {
				goto w_lock;
			}
		}
	} else if (Sstar_wk_in_delayed(work)) {
		/*
		*timer is time out ,wait runing timer;
		*/
		Sstar_wk_go_waiting(work);
		Sstar_printk_debug("[flush_delayed_work]->[%s] wait timeout\n",
				   work->name);
		spin_unlock_irqrestore(&work->lock, flags1);
		wait_event_interruptible(work->timeout,
					 Sstar_wk_in_waiting(work) == 0);
		spin_lock_irqsave(&work->lock, flags1);
	}

	if ((wq = dwork->wq) == NULL) {
		ret = false;
		goto w_lock;
	}

	spin_lock_irqsave(&wq->lock, flags2);

	if (!wk_is_busy(work)) {
		ret = false;
		goto wq_lock;
	}

	if (Sstar_wk_in_pending(work)) {
		head = work->entry.next;
		tail = true;
	} else if (Sstar_wk_in_runing(work)) {
		head = &wq->works;
		tail = false;
	} else
		BUG_ON(1);

	if (wq->work_thread) {
		Sstar_waitwk_insert(&wait, head, tail);
		Sstar_workqueue_wakeup(wq, wq->work_thread);
	} else {
		goto wq_lock;
	}
	ret = true;
	spin_unlock_irqrestore(&wq->lock, flags2);
	spin_unlock_irqrestore(&work->lock, flags1);

	Sstar_waitwk_work_for_done(&wait);
	return ret;
wq_lock:
	spin_unlock_irqrestore(&wq->lock, flags2);
w_lock:
	spin_unlock_irqrestore(&work->lock, flags1);
	return ret;
}
int ieee80211_Sstar_schedule_work(struct Sstar_work_struct *work)
{
	if (!common_wq)
		return 0;
	return ieee80211_Sstar_queue_work(common_wq, work);
}

bool ieee80211_Sstar_cancel_delayed_work_sync(struct Sstar_delayed_work *dwork)
{
	unsigned long flags1 = 0;
	unsigned long flags2 = 0;
	struct Sstar_work_struct *work = &dwork->work;
	struct Sstar_workqueue_struct *wq;
	bool ret = false;
	struct work_wait wait;
	struct list_head *head = NULL;

	Sstar_waitwk_init(&wait, work->name);

	Sstar_printk_debug("[cancel_delayed sync]->[%s] in\n", work->name);

	spin_lock_irqsave(&work->lock, flags1);

	if (likely(Sstar_del_timer(&dwork->timer))) {
		Sstar_printk_debug("[cancel_delayed sync]->[%s] success\n",
				   work->name);
		ret = true;
		dwork->n_timeout++;
		BUG_ON(dwork->wq == NULL);
		_ieee80211_Sstar_clear_delay_work(dwork->wq, &dwork->work);
	} else if (Sstar_wk_in_delayed(work)) {
		/*
		*timer is time out ,wait runing timer;
		*/
		Sstar_wk_go_waiting(work);
		Sstar_printk_always(
			"[cancel_delayed sync]->[%s] wait timeout\n",
			work->name);
		spin_unlock_irqrestore(&work->lock, flags1);
		wait_event_interruptible(work->timeout,
					 Sstar_wk_in_waiting(work) == 0);
		spin_lock_irqsave(&work->lock, flags1);
		ret = true;
	}

	if ((wq = dwork->wq) == NULL) {
		goto w_lock;
	}

	spin_lock_irqsave(&wq->lock, flags2);

	if (Sstar_wk_in_pending(work)) {
		_ieee80211_Sstar_clear_pending_work(wq, work);
		Sstar_printk_debug("[cancel_delayed sync]->[%s] pendding\n",
				   work->name);
		ret = true;
	}
	if (!wk_is_busy(work)) {
		goto wq_lock;
	}
	if (!Sstar_wk_in_runing(work)) {
		BUG_ON(1);
	}
	if (wq->work_thread) {
		Sstar_waitwk_insert(&wait, &wq->works, false);
		Sstar_workqueue_wakeup(wq, wq->work_thread);
	} else {
		goto wq_lock;
	}
	ret = false;
	spin_unlock_irqrestore(&wq->lock, flags2);
	spin_unlock_irqrestore(&work->lock, flags1);

	Sstar_waitwk_work_for_done(&wait);
	return ret;
wq_lock:
	spin_unlock_irqrestore(&wq->lock, flags2);
w_lock:
	spin_unlock_irqrestore(&work->lock, flags1);
	return ret;
}

bool ieee80211_Sstar_cancel_delayed_work(struct Sstar_delayed_work *dwork)
{
	unsigned long flags1 = 0;
	unsigned long flags2 = 0;
	struct Sstar_workqueue_struct *wq;
	struct Sstar_work_struct *work = &dwork->work;
	bool ret = false;

	spin_lock_irqsave(&work->lock, flags1);

	Sstar_printk_debug("[cancel_delayed]->[%s] in\n", work->name);

	if (likely(Sstar_del_timer(&dwork->timer))) {
		BUG_ON(dwork->wq == NULL);
		dwork->n_timeout++;
		_ieee80211_Sstar_clear_delay_work(dwork->wq, work);
		Sstar_printk_debug("[cancel_delayed]->[%s] sucess\n",
				   work->name);
		ret = true;
	}

	if ((wq = dwork->wq) == NULL) {
		Sstar_printk_debug("[cancel_delayed]->[%s] wq err\n",
				   work->name);
		goto exit;
	}

	spin_lock_irqsave(&wq->lock, flags2);
	if (Sstar_wk_in_pending(work)) {
		_ieee80211_Sstar_clear_pending_work(wq, work);
		Sstar_printk_debug("[cancel_delayed]->[%s] pendding\n",
				   work->name);
		ret = true;
	}

	if (wk_is_busy(work)) {
		ret = false;
	}
	spin_unlock_irqrestore(&wq->lock, flags2);
exit:
	spin_unlock_irqrestore(&work->lock, flags1);
	return ret;
}

bool ieee80211_Sstar_cancel_work_sync(struct Sstar_work_struct *work)
{
	unsigned long flags1 = 0;
	unsigned long flags2 = 0;
	struct Sstar_workqueue_struct *wq;
	bool ret = false;
	struct work_wait wait;

	Sstar_waitwk_init(&wait, work->name);
	Sstar_printk_debug("[cancel_work]->[%s] in\n", work->name);

	spin_lock_irqsave(&work->lock, flags1);

	if ((wq = work->wq) == NULL) {
		Sstar_printk_debug("[cancel_work]->[%s] out 1\n", work->name);
		ret = false;
		goto w_lock;
	}

	spin_lock_irqsave(&wq->lock, flags2);

	if (Sstar_wk_in_pending(work)) {
		_ieee80211_Sstar_clear_pending_work(wq, work);
		Sstar_printk_debug("[cancel_work]->[%s] pending\n", work->name);
		ret = true;
	}

	if (!wk_is_busy(work)) {
		Sstar_printk_debug("[cancel_work]->[%s] out 1\n", work->name);
		goto wq_lock;
	}

	if (!Sstar_wk_in_runing(work)) {
		BUG_ON(1);
	}

	if (wq->work_thread) {
		Sstar_waitwk_insert(&wait, &wq->works, false);
		Sstar_workqueue_wakeup(wq, wq->work_thread);
	} else
		goto wq_lock;

	ret = false;
	spin_unlock_irqrestore(&wq->lock, flags2);
	spin_unlock_irqrestore(&work->lock, flags1);

	Sstar_waitwk_work_for_done(&wait);

	Sstar_printk_debug("[cancel_work]->[%s] finished\n", work->name);
	return ret;
wq_lock:
	spin_unlock_irqrestore(&wq->lock, flags2);
w_lock:
	spin_unlock_irqrestore(&work->lock, flags1);
	return ret;
}
bool ieee80211_Sstar_flush_work(struct Sstar_work_struct *work)
{
	unsigned long flags1 = 0;
	unsigned long flags2 = 0;
	struct Sstar_workqueue_struct *wq;
	bool ret = false;
	struct work_wait wait;
	struct list_head *head = NULL;
	bool tail = false;

	Sstar_waitwk_init(&wait, work->name);

	Sstar_printk_debug("[flush_work]->[%s] in\n", work->name);
	spin_lock_irqsave(&work->lock, flags1);

	if ((wq = work->wq) == NULL) {
		ret = false;
		Sstar_printk_debug("[flush_work]->[%s] out 1\n", work->name);
		goto w_lock;
	}

	spin_lock_irqsave(&wq->lock, flags2);

	if (!wk_is_busy(work)) {
		Sstar_printk_debug("[cancel_work]->[%s] out 1\n", work->name);
		goto wq_lock;
	}

	if (Sstar_wk_in_runing(work)) {
		head = &wq->works;
		tail = false;
	} else if (Sstar_wk_in_pending(work)) {
		head = work->entry.next;
		tail = true;
	} else {
		BUG_ON(1);
	}

	if (wq->work_thread) {
		Sstar_waitwk_insert(&wait, head, true);
		Sstar_workqueue_wakeup(wq, wq->work_thread);
	} else {
		goto wq_lock;
	}

	spin_unlock_irqrestore(&wq->lock, flags2);
	spin_unlock_irqrestore(&work->lock, flags1);
	ret = true;
	Sstar_waitwk_work_for_done(&wait);
	Sstar_printk_debug("[flush_work]->[%s] finished\n", work->name);
	return ret;
wq_lock:
	spin_unlock_irqrestore(&wq->lock, flags2);
w_lock:
	spin_unlock_irqrestore(&work->lock, flags1);
	return ret;
}
#endif
