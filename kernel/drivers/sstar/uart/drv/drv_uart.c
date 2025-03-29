/*
 * drv_uart.c- Sigmastar
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
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/nmi.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/reboot.h>
#include <cam_os_wrapper.h>
#include <drv_uart_ioctl.h>
#include <drv_padmux.h>
#include <drv_puse.h>
#include <hal_uart.h>
#define UART_DEBUG 0

#if UART_DEBUG
#define uart_dbg(fmt, arg...) printk(KERN_INFO fmt, ##arg)
#else
#define uart_dbg(fmt, arg...)
#endif
#define uart_err(fmt, arg...) printk(KERN_ERR fmt, ##arg)

struct sstar_uart
{
    struct uart_port port;
    struct uart_hal  hal;
    struct clk *     clk;

    CamOsTimer_t       timer; /* "no irq" timer */
    CamOsTsem_t        task_stop_sem;
    char               rs485_task_name[32];
    CamOsThread        rs485_task;
    CamOsThreadAttrb_t rs485_attrb;
    CamOsTsem_t        rs485_sem;
    volatile bool      rs485_stop;
    char               urdma_task_name[32];
    CamOsThread        urdma_task;
    CamOsThreadAttrb_t urdma_attrb;
    CamOsTsem_t        urdma_sem;
    volatile bool      urdma_stop;

    char uart_irq_name[32];
    char urdma_irq_name[32];
    char tx_empty_irq_name[32];
    u32  uart_irq;
    u32  urdma_irq;
    u32  tx_empty_irq;
    u32  clk_mcu_rate;

    u32 sctp_en;
    u32 urdma_en;

    u32 rx_guard;
    u32 rx_silent;
    u32 rx_disable;
    u32 tx_silent;
    u32 tx_disable;
    u32 pad_disable;
    u32 bug_thre;
    u8  is_suspend;
    u8  no_suspend;
};

static struct console sstar_uart_cons;
static int            console_disable = 0;

static struct uart_driver sstar_uart_driver = {
    .owner       = THIS_MODULE,
    .driver_name = "ss_uart",
    .dev_name    = "ttyS",
    .nr          = 3,
    .cons        = &sstar_uart_cons,
};

static struct sstar_uart *sstar_uart_console = NULL;

static int sstar_uart_rs485_gpio(struct serial_rs485 *rs485, bool send)
{
    if ((rs485->flags & SER_RS485_ENABLED) && send)
    {
        if (rs485->delay_rts_before_send)
            udelay(rs485->delay_rts_before_send);
        if (rs485->flags & SER_RS485_RTS_ON_SEND)
        {
            gpio_set_value(rs485->padding[0], 1);
            uart_dbg("start send data, rs485 gpio set high\n");
        }
        else
        {
            gpio_set_value(rs485->padding[0], 0);
            uart_dbg("start send data, rs485 gpio low\n");
        }
    }
    else if ((rs485->flags & SER_RS485_ENABLED) && !send)
    {
        if (rs485->delay_rts_after_send)
            udelay(rs485->delay_rts_after_send);
        if (rs485->flags & SER_RS485_RTS_AFTER_SEND)
        {
            gpio_set_value(rs485->padding[0], 1);
            uart_dbg("end data send, rs485 gpio set high\n");
        }
        else
        {
            gpio_set_value(rs485->padding[0], 0);
            uart_dbg("end data send, rs485 gpio set low\n");
        }
    }
    return 0;
}

static int sstar_uart_rs485_config(struct uart_port *port, struct serial_rs485 *rs485)
{
    if (rs485->flags & SER_RS485_ENABLED)
    {
        if (gpio_request(rs485->padding[0], "gpio_irq_test") < 0)
        {
            uart_dbg("request gpio[%d] failed...\n", rs485->padding[0]);
            return -EFAULT;
        }
        if (rs485->flags & SER_RS485_RTS_ON_SEND)
        {
            if (gpio_direction_output(rs485->padding[0], 0) < 0)
            {
                uart_dbg("gpio_direction_output low[%d] failed...\n", rs485->padding[0]);
                return -EFAULT;
            }
        }
        else
        {
            if (gpio_direction_output(rs485->padding[0], 1) < 0)
            {
                uart_dbg("gpio_direction_output high[%d] failed...\n", rs485->padding[0]);
                return -EFAULT;
            }
        }
        uart_dbg("rs485->flags=0x%x\n", rs485->flags);
    }
    else
    {
        gpio_free(rs485->padding[0]);
    }

    memcpy(&port->rs485, rs485, sizeof(*rs485));
    return 0;
}

static void *sstar_uart_rs485_task(void *data)
{
    struct uart_port * p          = (struct uart_port *)data;
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    while (1)
    {
        if (CamOsTsemTimedDown(&sstar_uart->rs485_sem, 5000) != CAM_OS_OK)
        {
            continue;
        }

        if (sstar_uart->rs485_stop)
        {
            break;
        }

        if (CAM_OS_OK == CamOsThreadShouldStop())
        {
            break;
        }

        while (!(HAL_UART_FIFO_TX_SHIFT_EMPTY & hal_uart_get_status(&sstar_uart->hal)))
            ;

        sstar_uart_rs485_gpio(&p->rs485, false);
    }

    CamOsTsemUp(&sstar_uart->task_stop_sem);

    return NULL;
}
#if HAL_UART_TX_EMPTY_INTERRUPT
static irqreturn_t sstar_uart_tx_empty_interrupt(int irq, void *dev_id)
{
    struct uart_port * p          = dev_id;
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;
    struct circ_buf *  xmit       = &p->state->xmit;
    u32                irq_type   = 0;
    unsigned long      flags;

    spin_lock_irqsave(&p->lock, flags);

    irq_type = hal_uart_get_irq_type(&sstar_uart->hal, HAL_UART_IRQ_TX_EMPTY);

    if (irq_type == HAL_UART_IRQ_TX_EMPTY_INT)
    {
        if (uart_circ_empty(xmit) || uart_tx_stopped(p))
        {
            sstar_uart_rs485_gpio(&p->rs485, false);
        }
    }

    spin_unlock_irqrestore(&p->lock, flags);

    return IRQ_HANDLED;
}
#endif
static void sstar_uart_check_rx_disable(struct uart_port *p, u8 *buf, int count)
{
    int                i;
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    if (!uart_console(p))
        return;

    for (i = 0; i < count; i++)
    {
        if ('1' == buf[i])
        {
            sstar_uart->rx_disable++;
        }
        else
        {
            sstar_uart->rx_disable = 0;
        }

        if ('2' == buf[i])
        {
            sstar_uart->tx_disable++;
        }
        else
        {
            sstar_uart->tx_disable = 0;
        }

        if (5 == sstar_uart->rx_disable)
        {
            if (1 - sstar_uart->rx_silent)
            {
                hal_uart_write(&sstar_uart->hal, "disable uart\n", 13);
            }

            sstar_uart->rx_silent = 1 - sstar_uart->rx_silent;

            if (0 == sstar_uart->rx_silent)
            {
                hal_uart_write(&sstar_uart->hal, "enable uart\n", 12);
            }

            sstar_uart->rx_disable = 0;
        }

        if (5 == sstar_uart->tx_disable)
        {
            if (1 - sstar_uart->tx_silent)
            {
                hal_uart_write(&sstar_uart->hal, "disable uart tx\n", 16);
            }

            sstar_uart->tx_silent = 1 - sstar_uart->tx_silent;

            if (0 == sstar_uart->tx_silent)
            {
                hal_uart_write(&sstar_uart->hal, "enable uart tx\n", 15);
            }

            sstar_uart->tx_disable = 0;
        }

        if ('3' == buf[i])
        {
            sstar_uart->pad_disable++;
        }
        else
        {
            sstar_uart->pad_disable = 0;
        }

        if (5 == sstar_uart->pad_disable)
        {
            hal_uart_deinit(&sstar_uart->hal);
        }
    }
}

static u16 sstar_uart_set_clk(struct uart_port *p, u32 request_baud)
{
    unsigned int       num_parents;
    unsigned int       tolerance = 3, rate, divisor = 0, real_baud;
    unsigned int       tolerance_calc = ~(unsigned int)0x00;
    unsigned int       tolerance_actu = ~(unsigned int)0x00;
    unsigned int       min_time       = 0;
    unsigned int       cycle_time     = 0;
    struct sstar_uart *sstar_uart     = NULL;
    int                i, parent_index = -1;
    struct clk **      clk_parents;

    if (!p->dev)
    {
        sstar_uart = (struct sstar_uart *)p->private_data;
    }
    else
    {
        sstar_uart = (struct sstar_uart *)p->dev->driver_data;

        if (of_property_read_u32(p->dev->of_node, "tolerance", &tolerance))
        {
            tolerance = 3;
        }
    }

    if (IS_ERR(sstar_uart->clk))
    {
        p->uartclk = 172800000;
        return 0;
    }

    num_parents = clk_hw_get_num_parents(__clk_get_hw(sstar_uart->clk));

    if (!num_parents)
    {
        rate      = clk_get_rate(sstar_uart->clk);
        divisor   = (rate + (8 * request_baud)) / (16 * request_baud);
        real_baud = rate / (16 * divisor);

        uart_dbg("ttyS%d divisor=0x%02x baud=%d clk=%d\n", p->line, divisor, real_baud, rate);
        if ((abs(real_baud - request_baud) * 100 / request_baud) < tolerance)
        {
            p->uartclk = rate;
        }
    }
    else
    {
        clk_parents = kzalloc((sizeof(*clk_parents) * num_parents), GFP_KERNEL);
        if (!clk_parents)
        {
            kfree(clk_parents);
            p->uartclk = clk_get_rate(sstar_uart->clk);
            return 0;
        }

        for (i = 0; i < num_parents; i++)
        {
            clk_parents[i] = clk_hw_get_parent_by_index(__clk_get_hw(sstar_uart->clk), i)->clk;
            rate           = clk_get_rate(clk_parents[i]);
            divisor        = (rate + (8 * request_baud)) / (16 * request_baud);
            real_baud      = rate / (16 * divisor);
            if (p->line != 0)
            {
                uart_dbg("ttyS%d divisor=0x%02x baud=%d clk=%d\n", p->line, divisor, real_baud, rate);
            }
            tolerance_calc = (abs(real_baud - request_baud) * 100 / request_baud);
            if (tolerance_calc < tolerance)
            {
                if (tolerance_calc < tolerance_actu)
                {
                    tolerance_actu = tolerance_calc;
                    parent_index   = i;
                }
            }
        }

        if (parent_index >= num_parents)
        {
            divisor = 0;
        }
        else
        {
            clk_set_parent(sstar_uart->clk, clk_parents[parent_index]);
            p->uartclk = clk_get_rate(clk_parents[parent_index]);
            divisor    = (p->uartclk + (8 * request_baud)) / (16 * request_baud);
        }

        kfree(clk_parents);
    }

    /* Setting urdma rx_timeout_value dynamically */
    if (sstar_uart->clk_mcu_rate && sstar_uart->urdma_en)
    {
        min_time   = 1000000000 / request_baud * 10;
        cycle_time = 1000000000 / sstar_uart->clk_mcu_rate;

        for (i = 0; i < HAL_URDMA_RX_MAX_TIMEOUT; i++)
        {
            if ((cycle_time * (1 << i)) > min_time)
            {
                sstar_uart->hal.rx_timeout = i;
                break;
            }
        }
    }

    return divisor;
}

static void sstar_uart_getchar(struct uart_port *p)
{
    u32                fifo_status;
    u32                flag       = 0;
    u32                ch         = 0; /* Character read from UART port */
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    fifo_status = hal_uart_get_status(&sstar_uart->hal);

    /* check if Receiver Data Ready */
    if ((fifo_status & HAL_UART_FIFO_RX_READY) != HAL_UART_FIFO_RX_READY)
    {
        sstar_uart->rx_guard++;
        if (unlikely(sstar_uart->rx_guard > 2000))
        {
            hal_uart_deinit(&sstar_uart->hal);
            hal_uart_init(&sstar_uart->hal);
            sstar_uart->rx_guard = 0;
        }
        return;
    }
    sstar_uart->rx_guard = 0;

    /* while data ready, start to read data from UART FIFO */
    do
    {
        flag = TTY_NORMAL;
        /* read data from UART IP */
        hal_uart_read(&sstar_uart->hal, (u8 *)&ch, 1);
        p->icount.rx++;

        if (unlikely(fifo_status
                     & (HAL_UART_FIFO_BI | HAL_UART_FIFO_RX_PE | HAL_UART_FIFO_RX_FE | HAL_UART_FIFO_RX_OE)))
        {
            if (fifo_status & HAL_UART_FIFO_BI)
            {
                fifo_status &= ~(HAL_UART_FIFO_RX_FE | HAL_UART_FIFO_RX_PE);
                p->icount.brk++;
                /*
                 * We do the SysRQ and SAK checking
                 * here because otherwise the break
                 * may get masked by ignore_status_mask
                 * or read_status_mask.
                 */
                if (uart_handle_break(p))
                    goto IGNORE_CHAR;
            }
            else if (fifo_status & HAL_UART_FIFO_RX_PE)
            {
                p->icount.parity++;
            }
            else if (fifo_status & HAL_UART_FIFO_RX_FE)
            {
                p->icount.frame++;
            }
            else if (fifo_status & HAL_UART_FIFO_RX_OE)
            {
                p->icount.overrun++;
            }

            /*
             * Mask off conditions which should be ingored.
             */
            fifo_status &= p->read_status_mask;

            if (fifo_status & HAL_UART_FIFO_BI)
            {
                flag = TTY_BREAK;
            }
            else if (fifo_status & HAL_UART_FIFO_RX_PE)
            {
                flag = TTY_PARITY;
            }
            else if (fifo_status & HAL_UART_FIFO_RX_FE)
            {
                flag = TTY_FRAME;
            }
        }
        if (sstar_uart->rx_silent == 0)
        {
#ifdef CONFIG_MAGIC_SYSRQ_SERIAL
            if (uart_handle_sysrq_char(p, ch))
            {
                goto IGNORE_CHAR;
            }
#endif
            uart_insert_char(p, fifo_status, HAL_UART_FIFO_RX_OE, ch, flag);
        }

        sstar_uart_check_rx_disable(p, (u8 *)(&ch), 1);

        fifo_status = hal_uart_get_status(&sstar_uart->hal);
    } while (fifo_status & HAL_UART_FIFO_RX_READY);

IGNORE_CHAR:
    tty_flip_buffer_push(&p->state->port);
}

static void sstar_uart_putchar(struct uart_port *p)
{
    int                count;
    u32                fifo_status;
    struct circ_buf *  xmit;
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    /* Parameter out-of-bound check */
    if (!p)
    {
        return;
    }

    xmit = &p->state->xmit;

    if (p->x_char)
    {
        while (!(hal_uart_get_status(&sstar_uart->hal) & HAL_UART_FIFO_TX_SHIFT_EMPTY))
        {
            // nothing to do
        }

        hal_uart_write(&sstar_uart->hal, &(p->x_char), 1);
        p->icount.tx++;
        p->x_char = 0;
        return;
    }

    if (uart_circ_empty(xmit) || uart_tx_stopped(p))
    {
        if ((p->rs485.flags & SER_RS485_ENABLED) && !(sstar_uart->tx_empty_irq))
        {
            CamOsTsemUp(&sstar_uart->rs485_sem);
        }

        hal_uart_irq_enable(&sstar_uart->hal, (sstar_uart->urdma_en) ? HAL_UART_IRQ_URDMA_TX : HAL_UART_IRQ_TX, 0);
        return;
    }

    fifo_status = hal_uart_get_status(&sstar_uart->hal);

    if (HAL_UART_FIFO_TX_EMPTY == (fifo_status & (HAL_UART_FIFO_TX_EMPTY))) // Tx FIFO Empty
    {
        count = p->fifosize;
    }
    else if (HAL_UART_FIFO_TX_NOT_FULL == (fifo_status & (HAL_UART_FIFO_TX_NOT_FULL))) // not empty, but not full
    {
        count = 1;
    }
    else
    {
        count = 1;
    }

    do
    {
        if (sstar_uart->tx_silent == 0)
            hal_uart_write(&sstar_uart->hal, &(xmit->buf[xmit->tail]), 1);

        xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
        p->icount.tx++;

        if (uart_circ_empty(xmit))
        {
            break;
        }
    } while (--count > 0);

    if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
        uart_write_wakeup(p);
}

static void sstar_urdma_putchar(struct uart_port *p)
{
    struct sstar_uart *sstar_uart        = (struct sstar_uart *)p->dev->driver_data;
    struct circ_buf *  xmit              = &p->state->xmit;
    u16                circ_buf_out_size = CIRC_CNT_TO_END(xmit->head, xmit->tail, UART_XMIT_SIZE);
    int                count             = 0;

    if (circ_buf_out_size)
    {
        if (sstar_uart->tx_silent == 0)
            count = hal_uart_write(&sstar_uart->hal, &xmit->buf[xmit->tail], circ_buf_out_size);
        else
            count = circ_buf_out_size;
        p->icount.tx += count;
        xmit->tail = (xmit->tail + count) & (UART_XMIT_SIZE - 1);
    }

    return;
}

static void sstar_urdma_getchar(struct uart_port *p)
{
    u32                cnt        = 0;
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    hal_uart_read(&sstar_uart->hal, NULL, 0);

    while (p)
    {
        cnt = CIRC_CNT_TO_END(sstar_uart->hal.rx_buf_wptr, sstar_uart->hal.rx_buf_rptr, sstar_uart->hal.rx_urdma_size);

        if (cnt <= 0)
            break;

        if (sstar_uart->rx_silent == 0)
        {
            tty_insert_flip_string(&p->state->port, &sstar_uart->hal.rx_buf[sstar_uart->hal.rx_buf_rptr], cnt);
        }

        sstar_uart_check_rx_disable(p, &sstar_uart->hal.rx_buf[sstar_uart->hal.rx_buf_rptr], cnt);

        sstar_uart->hal.rx_buf_rptr = (sstar_uart->hal.rx_buf_rptr + cnt) & (sstar_uart->hal.rx_urdma_size - 1);
        p->icount.rx += cnt;
    }

    tty_flip_buffer_push(&p->state->port);

    return;
}

static irqreturn_t sstar_uart_interrupt(int irq, void *dev_id)
{
    struct uart_port * p          = dev_id;
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;
    u32                irq_type;
    unsigned long      flags;

    local_fiq_disable();

    spin_lock_irqsave(&p->lock, flags);

    irq_type = hal_uart_get_irq_type(&sstar_uart->hal, HAL_UART_IRQ);

    if (irq_type == HAL_UART_IRQ_RX) /* Receive Data Available or Character timeout or Receiver line status */
    {
        sstar_uart_getchar(p);
    }
    else if (irq_type == HAL_UART_IRQ_TX) /* Transmitter Holding Register Empty */
    {
        if (sstar_uart->tx_silent == 0)
            sstar_uart_putchar(p);
    }

    spin_unlock_irqrestore(&p->lock, flags);

    local_fiq_enable();
    return IRQ_HANDLED;
}

static void *sstar_urdma_task(void *user_data)
{
    struct uart_port * p          = (struct uart_port *)user_data;
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;
    struct circ_buf *  xmit;
    unsigned long      flags;

    while (1)
    {
        if (CamOsTsemTimedDown(&sstar_uart->urdma_sem, 5000) != CAM_OS_OK)
        {
            continue;
        }

        if (sstar_uart->urdma_stop)
        {
            break;
        }

        spin_lock_irqsave(&p->lock, flags);
        xmit = &p->state->xmit;
        if (uart_circ_empty(xmit) || uart_tx_stopped(p))
        {
            if ((p->rs485.flags & SER_RS485_ENABLED) && !(sstar_uart->tx_empty_irq))
            {
                CamOsTsemUp(&sstar_uart->rs485_sem);
            }

            hal_uart_irq_enable(&sstar_uart->hal, (sstar_uart->urdma_en) ? HAL_UART_IRQ_URDMA_TX : HAL_UART_IRQ_TX, 0);
        }
        else
        {
            hal_uart_irq_enable(&sstar_uart->hal, HAL_UART_IRQ_URDMA_TX, 1);
        }

        if (uart_circ_chars_pending(xmit))
        {
            sstar_urdma_putchar(p);
        }

        if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
            uart_write_wakeup(p);

        spin_unlock_irqrestore(&p->lock, flags);
    }

    CamOsTsemUp(&sstar_uart->task_stop_sem);

    return NULL;
}

static irqreturn_t sstar_urdma_interrupt(int irq, void *dev_id)
{
    struct uart_port * p          = dev_id;
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;
    u32                irq_type;
    unsigned long      flags;

    local_fiq_disable();

    spin_lock_irqsave(&sstar_uart->port.lock, flags);

    irq_type = hal_uart_get_irq_type(&sstar_uart->hal, HAL_UART_IRQ_URDMA);

    if (irq_type & HAL_UART_IRQ_URDMA_RX)
    {
        sstar_urdma_getchar(p);
    }
    else if (irq_type & HAL_UART_IRQ_URDMA_TX)
    {
        CamOsTsemUp(&sstar_uart->urdma_sem);
    }

    spin_unlock_irqrestore(&sstar_uart->port.lock, flags);

    local_fiq_enable();
    return IRQ_HANDLED;
}

static void sstar_uart_serial8250_backup_timeout(void *data)
{
    struct sstar_uart *sstar_uart = (struct sstar_uart *)data;
    unsigned long      flags;
    struct circ_buf *  xmit;
    u32                reg_lsr_status = 0x00;
    u32                reg_usr_status = 0x00;

    xmit = &sstar_uart->port.state->xmit;

    spin_lock_irqsave(&sstar_uart->port.lock, flags);

    if (!uart_circ_empty(xmit) && !uart_tx_stopped(&sstar_uart->port))
    {
        reg_lsr_status = hal_uart_get_status(&sstar_uart->hal) & (HAL_UART_FIFO_TX_SHIFT_EMPTY);
        reg_usr_status = hal_uart_get_usr(&sstar_uart->hal) & HAL_UART_FIFO_TX_EMPTY;
        /*
         * This should be a safe test for anyone who doesn't trust the
         * IIR bits on their UART, but it's specifically designed for
         * the "Diva" UART used on the management processor on many HP
         * ia64 and parisc boxes.
         */
        if ((sstar_uart->hal.tx_int_en) && (reg_lsr_status == (HAL_UART_FIFO_TX_SHIFT_EMPTY)) && reg_usr_status)
        {
            hal_uart_irq_enable(&sstar_uart->hal, HAL_UART_IRQ_TX, 0);
            hal_uart_irq_enable(&sstar_uart->hal, HAL_UART_IRQ_TX, 1);
        }
    }

    spin_unlock_irqrestore(&sstar_uart->port.lock, flags);

    /* Standard timer interval plus 0.2s to keep the port running */
    CamOsTimerModify(&sstar_uart->timer, CamOsJiffiesToMs(uart_poll_timeout(&sstar_uart->port) + HZ / 5));
}

static void sstar_uart_set_mctrl(struct uart_port *p, u32 mctrl) {}
static void sstar_uart_enable_ms(struct uart_port *p) {}
static void sstar_uart_break_ctl(struct uart_port *p, int ctl)
{
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    hal_uart_break(&sstar_uart->hal, ctl ? 1 : 0);
}

static void sstar_uart_release_port(struct uart_port *p) {}
static void sstar_uart_config_port(struct uart_port *p, s32 flags) {}
static u32  sstar_uart_get_mctrl(struct uart_port *p)
{
    return (TIOCM_CAR | TIOCM_CTS | TIOCM_DSR);
}
static s32 sstar_uart_request_port(struct uart_port *p)
{
    return 0;
}
static s32 sstar_uart_verify_port(struct uart_port *p, struct serial_struct *ser)
{
    return 0;
}
static const char *sstar_uart_type(struct uart_port *p)
{
    return NULL;
}

static u32 sstar_uart_tx_empty(struct uart_port *p)
{
    u32                status;
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;
    unsigned long      flags;
    u32                ret = 0;

    spin_lock_irqsave(&p->lock, flags);
    status = hal_uart_get_status(&sstar_uart->hal);

    if (sstar_uart->hal.urdma_en)
    {
        ret = (status & HAL_UART_URDMA_TX_EMPTY) ? TIOCSER_TEMT : 0;
    }
    else
    {
        ret = ((status & HAL_UART_FIFO_TX_EMPTY) && (status & HAL_UART_FIFO_TX_SHIFT_EMPTY)) ? TIOCSER_TEMT : 0;
    }
    spin_unlock_irqrestore(&p->lock, flags);

    return ret;
}

static void sstar_uart_stop_rx(struct uart_port *p)
{
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    hal_uart_irq_enable(&sstar_uart->hal, (sstar_uart->urdma_en) ? HAL_UART_IRQ_URDMA_RX : HAL_UART_IRQ_RX, 0);
}

static void sstar_uart_stop_tx(struct uart_port *p)
{
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    if ((p->rs485.flags & SER_RS485_ENABLED) && !(sstar_uart->tx_empty_irq))
    {
        CamOsTsemUp(&sstar_uart->rs485_sem);
    }

    hal_uart_irq_enable(&sstar_uart->hal, (sstar_uart->urdma_en) ? HAL_UART_IRQ_URDMA_TX : HAL_UART_IRQ_TX, 0);
}

static void sstar_uart_start_tx(struct uart_port *p)
{
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    if (p->rs485.flags & SER_RS485_ENABLED)
    {
        sstar_uart_rs485_gpio(&p->rs485, true);
    }

    if (sstar_uart->urdma_en)
    {
        CamOsTsemUp(&sstar_uart->urdma_sem);
    }
    else
    {
        hal_uart_irq_enable(&sstar_uart->hal, HAL_UART_IRQ_TX, 1);
    }
}

static s32 sstar_uart_startup(struct uart_port *p)
{
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;
    struct circ_buf *  xmit;
    unsigned long      flags;

    clk_prepare_enable(sstar_uart->clk);
    sstar_uart->port.uartclk = clk_get_rate(sstar_uart->clk);

    /* we do not support CTS now
     * FIXME: p->flags &= ~ASYNC_CTS_FLOW;
     * ASYNC_CTS_FLOW is uapi definition, counld't use in kernel
     */
    p->flags &= ~(1U << 26);

    spin_lock_irqsave(&p->lock, flags);

    sstar_uart->hal.urdma_en = sstar_uart->urdma_en;

    xmit = &p->state->xmit;
    if (uart_circ_chars_pending(xmit))
        uart_circ_clear(xmit);

    spin_unlock_irqrestore(&p->lock, flags);

    hal_uart_irq_enable(&sstar_uart->hal, (sstar_uart->urdma_en) ? HAL_UART_IRQ_URDMA_RX : HAL_UART_IRQ_RX, 0);
    hal_uart_irq_enable(&sstar_uart->hal, (sstar_uart->urdma_en) ? HAL_UART_IRQ_URDMA_TX : HAL_UART_IRQ_TX, 0);

    if (sstar_uart->urdma_en)
    {
        memset((void *)(sstar_uart->urdma_task_name), 0, sizeof(sstar_uart->urdma_task_name));
        sprintf(sstar_uart->urdma_task_name, "sstar_urdma_task%d", p->line);
        sstar_uart->urdma_attrb.szName     = sstar_uart->urdma_task_name;
        sstar_uart->urdma_attrb.nPriority  = 71;
        sstar_uart->urdma_attrb.nStackSize = 0;
        CamOsTsemInit(&sstar_uart->urdma_sem, 0);

        if (CAM_OS_OK
            != CamOsThreadCreate(&sstar_uart->urdma_task, &sstar_uart->urdma_attrb, (void *)sstar_urdma_task,
                                 (void *)p))
        {
            uart_err("failed to create the %s thread\n", sstar_uart->urdma_task_name);
            return -EAGAIN;
        }

        memset((void *)(sstar_uart->urdma_irq_name), 0, sizeof(sstar_uart->urdma_irq_name));
        sprintf(sstar_uart->urdma_irq_name, "sstar_urdma%d", p->line);

        if (0
            != request_irq(sstar_uart->urdma_irq, sstar_urdma_interrupt, IRQF_SHARED, sstar_uart->urdma_irq_name,
                           (void *)p))
        {
            uart_err("failed to register the %s interrupt service program\n", sstar_uart->urdma_irq_name);
            return -EAGAIN;
        }
    }
    else
    {
        memset((void *)(sstar_uart->uart_irq_name), 0, sizeof(sstar_uart->uart_irq_name));
        sprintf(sstar_uart->uart_irq_name, "sstar_uart%d", p->line);

        if (0
            != request_irq(sstar_uart->uart_irq, sstar_uart_interrupt, IRQF_SHARED, sstar_uart->uart_irq_name,
                           (void *)p))
        {
            uart_err("failed to register the %s interrupt service program\n", sstar_uart->uart_irq_name);
            return -EAGAIN;
        }
    }

#if HAL_UART_TX_EMPTY_INTERRUPT
    if (sstar_uart->tx_empty_irq)
    {
        memset((void *)(sstar_uart->tx_empty_irq_name), 0, sizeof(sstar_uart->tx_empty_irq_name));
        sprintf(sstar_uart->tx_empty_irq_name, "sstar_tx_empty_int%d", p->line);

        if (0
            != request_irq(sstar_uart->tx_empty_irq, sstar_uart_tx_empty_interrupt, IRQF_SHARED,
                           sstar_uart->tx_empty_irq_name, (void *)p))
        {
            uart_err("failed to register the %s interrupt service program\n", sstar_uart->tx_empty_irq_name);
            return -EAGAIN;
        }

        hal_uart_irq_enable(&sstar_uart->hal, HAL_UART_IRQ_TX_EMPTY, 1);
    }
    else
#endif
    {
        memset((void *)(sstar_uart->rs485_task_name), 0, sizeof(sstar_uart->rs485_task_name));
        sprintf(sstar_uart->rs485_task_name, "sstar_rs485_task%d", p->line);
        sstar_uart->rs485_attrb.szName     = sstar_uart->rs485_task_name;
        sstar_uart->rs485_attrb.nPriority  = 71;
        sstar_uart->rs485_attrb.nStackSize = 0;
        CamOsTsemInit(&sstar_uart->rs485_sem, 0);

        if (CAM_OS_OK
            != CamOsThreadCreate(&sstar_uart->rs485_task, &sstar_uart->rs485_attrb, (void *)sstar_uart_rs485_task,
                                 (void *)p))
        {
            uart_err("failed to create the %s thread\n", sstar_uart->rs485_task_name);
            return -EAGAIN;
        }
    }

    sstar_uart->rx_guard = 0;

    if (uart_console(p))
    {
        console_lock();
        hal_uart_tx_flush(&sstar_uart_console->hal);
        if (sstar_uart_console && sstar_uart_console != sstar_uart)
        {
            sstar_uart->hal.digmux   = sstar_uart_console->hal.digmux;
            sstar_uart->tx_silent    = sstar_uart_console->tx_silent;
            sstar_uart->rx_silent    = sstar_uart_console->rx_silent;
            sstar_uart->hal.urdma_en = sstar_uart->urdma_en;
            kfree((void *)sstar_uart_console);
            sstar_uart_console = sstar_uart;
        }
    }

    hal_uart_init(&sstar_uart->hal);

    hal_uart_irq_enable(&sstar_uart->hal, (sstar_uart->urdma_en) ? HAL_UART_IRQ_URDMA_RX : HAL_UART_IRQ_RX, 1);

    if (uart_console(p))
    {
        console_unlock();
    }

    sstar_uart->bug_thre = sstar_uart->urdma_en ? 0 : HAL_UART_8250_BUG_THRE;

    /*
     * [PATCH] from drivers/tty/serial/8250/8250_core.c
     * The above check will only give an accurate result the first time
     * the port is opened so this value needs to be preserved.
     */
    if (sstar_uart->bug_thre)
    {
        CamOsTimerInit(&sstar_uart->timer);
        CamOsTimerAdd(&sstar_uart->timer, CamOsJiffiesToMs(uart_poll_timeout(&sstar_uart->port) + HZ / 5),
                      (void *)sstar_uart, sstar_uart_serial8250_backup_timeout);
    }

    return 0;
}

static void sstar_uart_shutdown(struct uart_port *p)
{
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    if (uart_console(p))
    {
        console_lock();
    }

    hal_uart_deinit(&sstar_uart->hal);

    if (uart_console(p))
    {
        sstar_uart->hal.urdma_en = 0;
        console_unlock();
    }

#if HAL_UART_TX_EMPTY_INTERRUPT
    hal_uart_irq_enable(&sstar_uart->hal, HAL_UART_IRQ_TX_EMPTY, 0);

    if (sstar_uart->tx_empty_irq)
    {
        free_irq(sstar_uart->tx_empty_irq, (void *)p);
    }
    else
#endif
    {
        sstar_uart->rs485_stop = true;
        CamOsTsemUp(&sstar_uart->rs485_sem);
        CamOsTsemDown(&sstar_uart->task_stop_sem);
        CamOsThreadStop(sstar_uart->rs485_task);
        sstar_uart->rs485_stop = false;
        CamOsTsemDeinit(&sstar_uart->rs485_sem);
    }

    hal_uart_irq_enable(&sstar_uart->hal, (sstar_uart->urdma_en) ? HAL_UART_IRQ_URDMA_RX : HAL_UART_IRQ_RX, 0);

    if (sstar_uart->urdma_en)
    {
        hal_uart_clr_urdma_intr(&sstar_uart->hal);
        sstar_uart->urdma_stop = true;
        CamOsTsemUp(&sstar_uart->urdma_sem);
        CamOsTsemDown(&sstar_uart->task_stop_sem);
        CamOsThreadStop(sstar_uart->urdma_task);
        sstar_uart->urdma_stop = false;
        CamOsTsemDeinit(&sstar_uart->urdma_sem);
        free_irq(sstar_uart->urdma_irq, (void *)p);
    }
    else
    {
        free_irq(sstar_uart->uart_irq, (void *)p);
    }

    if (sstar_uart->bug_thre)
    {
        CamOsTimerDeleteSync(&sstar_uart->timer);
        CamOsTimerDeinit(&sstar_uart->timer);
    }

    if (!IS_ERR(sstar_uart->clk))
    {
        clk_disable_unprepare(sstar_uart->clk);
    }
}

static void sstar_uart_set_termios(struct uart_port *p, struct ktermios *termios, struct ktermios *old)
{
    /* Define Local Variables */
    struct sstar_uart *sstar_uart = NULL;
    u16                divisor    = 0;
    u32                baudrate   = 0;
    unsigned long      flags;

    if (p->dev)
    {
        sstar_uart = (struct sstar_uart *)p->dev->driver_data;
    }
    else
    {
        sstar_uart = (struct sstar_uart *)p->private_data;
    }

    switch (termios->c_cflag & CSIZE)
    {
        case CS5:
            sstar_uart->hal.char_bits = 5;
            break;
        case CS6:
            sstar_uart->hal.char_bits = 6;
            break;
        case CS7:
            sstar_uart->hal.char_bits = 7;
            break;
        case CS8:
            sstar_uart->hal.char_bits = 8;
            break;
        default:
            uart_err("uart%d unsupported bits:%d\n", p->line, termios->c_cflag & CSIZE);
            break;
    }

    if (termios->c_cflag & CSTOPB)
    {
        sstar_uart->hal.stop_bits = 2;
    }
    else
    {
        sstar_uart->hal.stop_bits = 1;
    }

    if (termios->c_cflag & PARENB)
    {
        sstar_uart->hal.parity_en = 1;

        if (termios->c_cflag & PARODD)
        {
            sstar_uart->hal.even_parity_sel = 0;
        }
        else
        {
            sstar_uart->hal.even_parity_sel = 1;
        }
    }
    else
    {
        sstar_uart->hal.parity_en = 0;
    }

    if (sstar_uart && sstar_uart->sctp_en == 1)
    {
        if (termios->c_cflag & CRTSCTS)
        {
            sstar_uart->hal.rtscts_en = 1;
        }
        else
        {
            sstar_uart->hal.rtscts_en = 0;
        }
    }

    if (uart_console(p))
    {
        console_lock();
        hal_uart_tx_flush(&sstar_uart->hal);
    }

    // NOTE: we are going to set LCR, be carefully here
    if ((sstar_uart->is_suspend) || !old || (tty_termios_baud_rate(old) != tty_termios_baud_rate(termios)))
    {
        baudrate                = uart_get_baud_rate(p, termios, old, 0, 115200 * 30);
        divisor                 = sstar_uart_set_clk(p, baudrate);
        sstar_uart->hal.divisor = divisor ? divisor : sstar_uart->hal.divisor;
    }

    spin_lock_irqsave(&p->lock, flags);
    hal_uart_config(&sstar_uart->hal);
    spin_unlock_irqrestore(&p->lock, flags);

    if (uart_console(p))
    {
        console_unlock();
    }
}

static int sstar_uart_ioctl(struct uart_port *p, unsigned int cmd, unsigned long arg)
{
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    switch (cmd)
    {
        case UART_CMD_DISABLE_RX:
            sstar_uart->rx_silent = 1;
            break;
        case UART_CMD_ENABLE_RX:
            sstar_uart->rx_silent = 0;
            break;
        case UART_CMD_DISABLE_TX:
            sstar_uart->tx_silent = 1;
            break;
        case UART_CMD_ENABLE_TX:
            sstar_uart->tx_silent = 0;
            break;
        default:
            return -ENOIOCTLCMD;
    }

    return 0;
}

/* UART Operations */
static struct uart_ops sstar_uart_ops = {
    .tx_empty     = sstar_uart_tx_empty,
    .set_mctrl    = sstar_uart_set_mctrl, /* Not supported in MSB2501 */
    .get_mctrl    = sstar_uart_get_mctrl, /* Not supported in MSB2501 */
    .stop_tx      = sstar_uart_stop_tx,
    .start_tx     = sstar_uart_start_tx,
    .stop_rx      = sstar_uart_stop_rx,
    .enable_ms    = sstar_uart_enable_ms, /* Not supported in MSB2501 */
    .break_ctl    = sstar_uart_break_ctl, /* Not supported in MSB2501 */
    .startup      = sstar_uart_startup,
    .shutdown     = sstar_uart_shutdown,
    .set_termios  = sstar_uart_set_termios,
    .type         = sstar_uart_type,         /* Not supported in MSB2501 */
    .release_port = sstar_uart_release_port, /* Not supported in MSB2501 */
    .request_port = sstar_uart_request_port, /* Not supported in MSB2501 */
    .config_port  = sstar_uart_config_port,  /* Not supported in MSB2501 */
    .verify_port  = sstar_uart_verify_port,  /* Not supported in MSB2501 */
    .ioctl        = sstar_uart_ioctl,
};

ssize_t rx_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct tty_port *  port       = dev_get_drvdata(dev);
    struct uart_state *state      = container_of(port, struct uart_state, port);
    struct uart_port * p          = state->uart_port;
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    if ((count >= 3) && !strncmp(buf, "off", 3))
    {
        sstar_uart->rx_silent = 1;
    }
    else if ((count >= 2) && !strncmp(buf, "on", 2))
    {
        sstar_uart->rx_silent = 0;
    }

    return count;
}

ssize_t tx_switch_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct tty_port *  port       = dev_get_drvdata(dev);
    struct uart_state *state      = container_of(port, struct uart_state, port);
    struct uart_port * p          = state->uart_port;
    struct sstar_uart *sstar_uart = (struct sstar_uart *)p->dev->driver_data;

    if ((count >= 3) && !strncmp(buf, "off", 3))
    {
        sstar_uart->tx_silent = 1;
    }
    else if ((count >= 2) && !strncmp(buf, "on", 2))
    {
        sstar_uart->tx_silent = 0;
    }

    return count;
}

static DEVICE_ATTR_WO(rx_switch);
static DEVICE_ATTR_WO(tx_switch);

static struct attribute *sstar_uart_dev_attrs[] = {&dev_attr_rx_switch.attr, &dev_attr_tx_switch.attr, NULL};

static struct attribute_group sstar_uart_dev_attr_group = {
    .attrs = sstar_uart_dev_attrs,
};

static s32 sstar_uart_probe(struct platform_device *pdev)
{
    int                ret        = 0;
    u32                dts_value  = 0;
    u32                urdma_en   = 0;
    dma_addr_t         dma_handle = 0;
    struct resource *  res;
    struct sstar_uart *sstar_uart;
    struct clk *       clk_mcu;

    sstar_uart = devm_kzalloc(&pdev->dev, sizeof(struct sstar_uart), GFP_KERNEL);
    if (!sstar_uart)
        return -ENOMEM;

    sstar_uart->clk = of_clk_get(pdev->dev.of_node, 0);
    if (IS_ERR(sstar_uart->clk))
    {
        uart_err("failed to get uart clk\n");
        return -EINVAL;
    }

    clk_mcu = of_clk_get(pdev->dev.of_node, 1);
    if (IS_ERR(clk_mcu))
    {
        uart_err("failed to get uart clk_mcu\n");
    }
    else
    {
        sstar_uart->clk_mcu_rate = clk_get_rate(clk_mcu);
        clk_put(clk_mcu);
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

    if (res == NULL)
    {
        uart_err("no memory resource defined\n");
        ret = -ENODEV;
        goto out;
    }

    sstar_uart->hal.uart_base = res->start;

    res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (res == NULL)
    {
        uart_err("no irq resource defined\n");
        ret = -ENODEV;
        goto out;
    }

    sstar_uart->uart_irq = res->start;

    of_property_read_u32(pdev->dev.of_node, "sctp_enable", &sstar_uart->sctp_en);

    urdma_en             = of_property_read_bool(pdev->dev.of_node, "dma-enable");
    sstar_uart->urdma_en = urdma_en;

    sstar_uart->no_suspend = of_property_read_bool(pdev->dev.of_node, "no-suspend");

    if (!of_property_read_u32(pdev->dev.of_node, "tx_fifo_level", &dts_value))
    {
        sstar_uart->hal.tx_fifo_level = (u8)(dts_value & 0xFF);
    }

    if (!of_property_read_u32(pdev->dev.of_node, "rx_fifo_level", &dts_value))
    {
        sstar_uart->hal.rx_fifo_level = (u8)(dts_value & 0xFF);
    }

    if (!of_property_read_u32(pdev->dev.of_node, "rx_pin", &dts_value))
    {
        sstar_uart->hal.rx_pin = (u8)(dts_value & 0xFF);
    }

#if defined(CONFIG_SSTAR_PADMUX) && defined(CONFIG_SSTAR_GPIO)
    if (!of_property_read_u32(pdev->dev.of_node, "puse-channel", &dts_value))
    {
        sstar_uart->hal.break_ctl_pad =
            (u8)(drv_padmux_getpad(drv_padmux_getpuse(PUSE_UART, dts_value, UART_TX)) & 0xFF);
        sstar_uart->hal.padmux = drv_padmux_getmode(drv_padmux_getpuse(PUSE_UART, dts_value, UART_TX));
    }
    else
    {
        sstar_uart->hal.break_ctl_pad = -1;
        sstar_uart->hal.padmux        = -1;
    }
#else
    sstar_uart->hal.break_ctl_pad = -1;
    sstar_uart->hal.padmux        = -1;
#endif

    if (!of_property_read_u32(pdev->dev.of_node, "digmux", &dts_value))
    {
        sstar_uart->hal.digmux = (u8)(dts_value & 0xFF);
        if (sstar_uart->hal.digmux != 0xFF)
            hal_uart_set_digmux(&sstar_uart->hal);
    }

    if (sstar_uart->urdma_en)
    {
#ifdef CONFIG_PHYS_ADDR_T_64BIT
        /* enable the uart to access DRAM bigger than 4GB when using DMA */
        if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)))
        {
            uart_err("no suitable DMA available\n");
        }
#endif

        res = platform_get_resource(pdev, IORESOURCE_MEM, 1);

        if (res == NULL)
        {
            sstar_uart->urdma_en = 0;
            uart_err("no urdma memory resource defined...\n");
            goto dma_err;
        }

        sstar_uart->hal.urdma_base = res->start;

        res = platform_get_resource(pdev, IORESOURCE_IRQ, 1);
        if (res == NULL)
        {
            sstar_uart->urdma_en = 0;
            uart_err("no urdma irq resource defined\n");
            goto dma_err;
        }

        sstar_uart->urdma_irq = res->start;

#if HAL_UART_TX_EMPTY_INTERRUPT
        res = platform_get_resource(pdev, IORESOURCE_IRQ, 2);
        if (res == NULL)
        {
            uart_err("no tx_empty_irq irq resource defined\n");
        }
        else
        {
            sstar_uart->tx_empty_irq = res->start;
        }
#endif

        sstar_uart->hal.rx_buf =
            dma_alloc_coherent(&pdev->dev, PAGE_ALIGN(HAL_URDMA_RXBUF_LENGTH), &dma_handle, GFP_KERNEL);
        sstar_uart->hal.rx_urdma_base = dma_handle;

        sstar_uart->hal.tx_buf =
            dma_alloc_coherent(&pdev->dev, PAGE_ALIGN(HAL_URDMA_TXBUF_LENGTH), &dma_handle, GFP_KERNEL);
        sstar_uart->hal.tx_urdma_base = dma_handle;

        if (!sstar_uart->hal.rx_buf || !sstar_uart->hal.tx_buf)
        {
            sstar_uart->urdma_en = 0;
            uart_err("Allocate urdma rx_buffer/tx_buffer failed, use UART mode\n");
            goto dma_err;
        }

        sstar_uart->hal.rx_urdma_base = Chip_Phys_to_MIU(sstar_uart->hal.rx_urdma_base);
        sstar_uart->hal.tx_urdma_base = Chip_Phys_to_MIU(sstar_uart->hal.tx_urdma_base);
        sstar_uart->hal.rx_urdma_size = PAGE_ALIGN(HAL_URDMA_RXBUF_LENGTH);
        sstar_uart->hal.tx_urdma_size = PAGE_ALIGN(HAL_URDMA_TXBUF_LENGTH);
    dma_err:
        sstar_uart->hal.urdma_en = sstar_uart->urdma_en;
    }

    sstar_uart->port.line         = of_alias_get_id(pdev->dev.of_node, "serial");
    sstar_uart->port.type         = PORT_8250;
    sstar_uart->port.dev          = &pdev->dev;
    sstar_uart->port.ops          = &sstar_uart_ops;
    sstar_uart->port.regshift     = 0;
    sstar_uart->port.fifosize     = 32; // HW FIFO is 32Bytes
    sstar_uart->port.timeout      = HZ;
    sstar_uart->port.iotype       = UPIO_MEM;
    sstar_uart->port.has_sysrq    = IS_ENABLED(CONFIG_MAGIC_SYSRQ_SERIAL);
    sstar_uart->port.rs485_config = sstar_uart_rs485_config;
    sstar_uart->port.private_data = (void *)sstar_uart;
    sstar_uart->port.attr_group   = &sstar_uart_dev_attr_group;
    spin_lock_init(&sstar_uart->port.lock);
    CamOsTsemInit(&sstar_uart->task_stop_sem, 0);

    uart_dbg("[%s] line=%d name=%s\n", __func__, sstar_uart->port.line, pdev->name);

    platform_set_drvdata(pdev, sstar_uart);

    ret = uart_add_one_port(&sstar_uart_driver, &sstar_uart->port);
    if (ret != 0)
        goto out;

    return 0;
out:
    uart_err("uart%d: driver probe failure\n", sstar_uart->port.line);
    clk_disable_unprepare(sstar_uart->clk);
    clk_put(sstar_uart->clk);

    return ret;
}

static s32 sstar_uart_remove(struct platform_device *pdev)
{
    struct sstar_uart *sstar_uart = platform_get_drvdata(pdev);

    uart_remove_one_port(&sstar_uart_driver, &sstar_uart->port);
    CamOsTsemDeinit(&sstar_uart->task_stop_sem);

    if (sstar_uart->urdma_en)
    {
        dma_free_coherent(&pdev->dev, PAGE_ALIGN(HAL_URDMA_RXBUF_LENGTH), &sstar_uart->hal.rx_urdma_base, GFP_KERNEL);
        dma_free_coherent(&pdev->dev, PAGE_ALIGN(HAL_URDMA_TXBUF_LENGTH), &sstar_uart->hal.tx_urdma_base, GFP_KERNEL);
    }

    if (!IS_ERR(sstar_uart->clk))
    {
        clk_disable_unprepare(sstar_uart->clk);
        clk_put(sstar_uart->clk);
    }

    return 0;
}

static s32 sstar_uart_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct sstar_uart *sstar_uart = platform_get_drvdata(pdev);
    struct uart_port * p          = &sstar_uart->port;
#ifdef CONFIG_PM_MCU_POWER_OFF
    struct uart_state *ustate = sstar_uart_driver.state + p->line;
    struct tty_port *  port   = &ustate->port;

    // not suspend the uart1/fuart port
#if defined(CONFIG_PM_MCU_USE_UART1) || defined(CONFIG_PM_MCU_USE_FUART)
    if ((sstar_uart->no_suspend) && tty_port_initialized(port))
#else
    if (0)
#endif
    {
        if (sstar_uart->urdma_en)
        {
            hal_uart_change_mode_to_fifo(&sstar_uart->hal);
        }
        return 0;
    }
#endif // CONFIG_PM_MCU_POWER_OFF

    uart_suspend_port(&sstar_uart_driver, p);
    sstar_uart->is_suspend = 1;
    if ((!console_suspend_enabled) && uart_console(p))
    {
        console_lock();
#if HAL_UART_TX_EMPTY_INTERRUPT
        hal_uart_irq_enable(&sstar_uart->hal, HAL_UART_IRQ_TX_EMPTY, 0);
#endif
        hal_uart_irq_enable(&sstar_uart->hal, (sstar_uart->urdma_en) ? HAL_UART_IRQ_URDMA_RX : HAL_UART_IRQ_RX, 0);
        hal_uart_deinit(&sstar_uart->hal);
        sstar_uart->hal.urdma_en = 0;
        console_unlock();

        return 0;
    }

    return 0;
}

static s32 sstar_uart_resume(struct platform_device *pdev)
{
    struct sstar_uart *sstar_uart = platform_get_drvdata(pdev);
    struct uart_port * p          = &sstar_uart->port;
#ifdef CONFIG_PM_MCU_POWER_OFF
    struct uart_state *ustate = sstar_uart_driver.state + p->line;
    struct tty_port *  port   = &ustate->port;

#if defined(CONFIG_PM_MCU_USE_UART1) || defined(CONFIG_PM_MCU_USE_FUART)
    if ((sstar_uart->no_suspend) && tty_port_initialized(port))
#else
    if (0)
#endif
    {
        uart_suspend_port(&sstar_uart_driver, p);
    }
#endif // CONFIG_PM_MCU_POWER_OFF

    uart_resume_port(&sstar_uart_driver, &sstar_uart->port);

    if ((!console_suspend_enabled) && uart_console(p))
    {
        console_lock();
        sstar_uart->hal.urdma_en = sstar_uart->urdma_en;
        hal_uart_init(&sstar_uart->hal);
        hal_uart_config(&sstar_uart->hal);
        hal_uart_irq_enable(&sstar_uart->hal, (sstar_uart->urdma_en) ? HAL_UART_IRQ_URDMA_RX : HAL_UART_IRQ_RX, 1);
#if HAL_UART_TX_EMPTY_INTERRUPT
        hal_uart_irq_enable(&sstar_uart->hal, HAL_UART_IRQ_TX_EMPTY, 1);
#endif
        console_unlock();
    }
    sstar_uart->is_suspend = 0;
    return 0;
}

static const struct of_device_id sstar_uart_of_match_table[] = {{.compatible = "sstar,uart"}, {}};
MODULE_DEVICE_TABLE(of, sstar_uart_of_match_table);

static struct platform_driver sstar_uart_platform_driver = {
    .probe   = sstar_uart_probe,
    .remove  = sstar_uart_remove,
    .suspend = sstar_uart_suspend,
    .resume  = sstar_uart_resume,
    .driver =
        {
            .name           = "ss_uart",
            .owner          = THIS_MODULE,
            .of_match_table = of_match_ptr(sstar_uart_of_match_table),
        },
};

static s32 __init sstar_uart_module_init(void)
{
    int ret;

    sstar_uart_driver.nr = of_alias_get_highest_id("serial") + 1;

    ret = uart_register_driver(&sstar_uart_driver);
    if (ret != 0)
        return ret;
    ret = platform_driver_register(&sstar_uart_platform_driver);
    if (ret != 0)
    {
        uart_err("platform_driver_register failed!!\n");
        uart_unregister_driver(&sstar_uart_driver);
    }
    return ret;
}

static void __exit sstar_uart_module_exit(void)
{
    platform_driver_unregister(&sstar_uart_platform_driver);
    uart_unregister_driver(&sstar_uart_driver);
}

module_init(sstar_uart_module_init);
module_exit(sstar_uart_module_exit);

static int sstar_uart_notify_sys(struct notifier_block *this, unsigned long code, void *unused)
{
    unsigned long     flags;
    struct uart_port *p = &sstar_uart_console->port;

    spin_lock_irqsave(&p->lock, flags);
    hal_uart_deinit(&sstar_uart_console->hal);
    sstar_uart_console->hal.urdma_en = 0;
    spin_unlock_irqrestore(&p->lock, flags);

    return NOTIFY_DONE;
}

static struct notifier_block sstar_uart_reboot_notifier = {
    .notifier_call = sstar_uart_notify_sys,
};

static struct notifier_block sstar_uart_restart_notifier = {
    .notifier_call = sstar_uart_notify_sys,
    .priority      = 256,
};

static s32 sstar_uart_console_prepare(struct console *co)
{
    struct device_node *console_np = NULL;
    struct resource     res;
    u32                 dts_value        = 0;
    char                console_name[16] = {0};
    int                 idx              = co->index;
    struct uart_driver *drv              = (struct uart_driver *)co->data;
    struct uart_state * state;

    for (state = drv->state; state; state++)
    {
        if (state->uart_port->line == idx)
        {
            sstar_uart_console = (struct sstar_uart *)state->uart_port->private_data;
            return 0;
        }
    }

    sstar_uart_console = kzalloc(sizeof(struct sstar_uart), GFP_KERNEL);

    if (!sstar_uart_console)
        return -ENOMEM;

    if ((0 > idx) || (HAL_UART_NR_PORTS <= idx))
        return -ENODEV;

    sprintf(console_name, "serial%d", idx);

    console_np = of_find_node_by_path(console_name);

    if (!console_np)
    {
        console_np = of_find_node_by_path("console");
        idx        = 0;
    }

    if (!console_np)
        return -ENODEV;

    sstar_uart_console->clk = of_clk_get(console_np, 0);
    if (IS_ERR(sstar_uart_console->clk))
    {
        return -EINVAL;
    }

    // enable clk in probe, because if UART no clk, it can not be accessed.
    clk_prepare_enable(sstar_uart_console->clk);
    sstar_uart_console->port.uartclk = clk_get_rate(sstar_uart_console->clk);

    if (!of_property_read_u32(console_np, "digmux", &dts_value))
    {
        sstar_uart_console->hal.digmux = (u8)(dts_value & 0xFF);
    }
    else
    {
        sstar_uart_console->hal.digmux = 0x0;
    }

    BUG_ON(of_address_to_resource(console_np, 0, &res));

    sstar_uart_console->hal.uart_base     = (unsigned long)res.start;
    sstar_uart_console->urdma_en          = 0;
    sstar_uart_console->port.type         = PORT_8250;
    sstar_uart_console->port.ops          = &sstar_uart_ops;
    sstar_uart_console->port.regshift     = 0;
    sstar_uart_console->port.fifosize     = 32; // HW FIFO is 32Bytes
    sstar_uart_console->port.cons         = NULL;
    sstar_uart_console->port.line         = idx;
    sstar_uart_console->port.private_data = (void *)sstar_uart_console;
    sstar_uart_console->tx_silent         = console_disable ? 1 : 0;
    sstar_uart_console->rx_silent         = console_disable ? 1 : 0;

    sstar_uart_cons.index = sstar_uart_console->port.line;

    hal_uart_set_digmux(&sstar_uart_console->hal);

    if (0 != register_reboot_notifier(&sstar_uart_reboot_notifier))
    {
        return -EINVAL;
    }

    if (0 != register_restart_handler(&sstar_uart_restart_notifier))
    {
        return -EINVAL;
    }

    return 0;
}

static void sstar_uart_console_putchar(struct uart_port *p, s32 ch)
{
    unsigned char      c          = (unsigned char)(ch & 0xFF);
    struct sstar_uart *sstar_uart = (struct sstar_uart *)(p->private_data);

    if (sstar_uart_console->tx_silent)
        return;

    while (1 != hal_uart_write(&sstar_uart->hal, &c, 1))
        ;
}

static void sstar_uart_console_write(struct console *co, const char *str, u32 count)
{
    struct uart_port *p;
    unsigned long     flags;
    int               locked = 1;
    unsigned int      step_count;

    if ((!str) || co->index >= HAL_UART_NR_PORTS || co->index < 0)
    {
        return;
    }

    p = &sstar_uart_console->port;
    while (count) // shorten the time of interrupt closed by console log need verify
    {
        if (count < 16)
        {
            step_count = count;
            count      = 0;
        }
        else
        {
            step_count = 16;
            count -= 16;
        }

        if (p->sysrq || oops_in_progress)
            locked = spin_trylock_irqsave(&p->lock, flags);
        else
            spin_lock_irqsave(&p->lock, flags);

        uart_console_write(p, str, step_count, sstar_uart_console_putchar);

        if (locked)
            spin_unlock_irqrestore(&p->lock, flags);
        str += step_count;
    }

    return;
}

static s32 sstar_uart_console_setup(struct console *co, char *options)
{
    /* Define Local Variables */
    s32 baud   = 115200;
    s32 bits   = 8;
    s32 parity = 'n';
    s32 flow   = 'n';

    if (!options)
    {
        options = "115200n8r"; /* Set default baudrate for console*/
    }

    /* parsing the command line arguments */
    uart_parse_options(options, &baud, &parity, &bits, &flow);
    sstar_uart_console_prepare(co);

    return uart_set_options(&sstar_uart_console->port, co, baud, parity, bits, flow);
}

static int sstar_uart_console_match(struct console *co, char *name, int idx, char *options)
{
    co->index = idx;
    return -ENODEV;
}

/* Serial Console Structure Definition */
static struct console sstar_uart_cons = {
    .name   = "ttyS",
    .write  = sstar_uart_console_write,
    .setup  = sstar_uart_console_setup,
    .flags  = CON_PRINTBUFFER,
    .device = uart_console_device,
    .data   = &sstar_uart_driver,
    .index  = -1,
    .match  = sstar_uart_console_match,
};

static int __init sstar_uart_setup(char *s)
{
    if (!strncmp(s, "off", 3))
        console_disable = 1;

    return 1;
}

__setup("uart=", sstar_uart_setup);

// Only compiling as build-in ko register early console
#ifndef MODULE
static int __init sstar_early_console_init(void)
{
    register_console(&sstar_uart_cons);
    return 0;
}

console_initcall(sstar_early_console_init);
#endif

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sstar uart driver");
MODULE_AUTHOR("SSTAR");
MODULE_ALIAS("platform:sstar_kdrv_uart");
