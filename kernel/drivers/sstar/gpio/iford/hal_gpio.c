/*
 * hal_gpio.c- Sigmastar
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

#include <gpio_os.h>
#include <hal_gpio.h>
#include <hal_gpio_common.h>
#include <gpio.h>
#include <padmux.h>
#include <hal_pinmux.h>

#define GPIO_DBG 0
#if GPIO_DBG
#define gpio_err(args...) gpio_print(args)
#else
#define gpio_err(args...)
#endif

typedef struct
{
    u32 reg;
    u16 mask;
} hal_gpio_reg_info;

u32 gChipBaseAddr    = GPIO_IO_ADDRESS(0x1F203C00);
u32 gPmSleepBaseAddr = GPIO_IO_ADDRESS(0x1F001C00);
u32 gSarBaseAddr     = GPIO_IO_ADDRESS(0x1F002800);
u32 gRIUBaseAddr     = GPIO_IO_ADDRESS(0x1F000000);

#define HAL_GPIO_CHIPTOP_REG(addr)  (*(volatile u16 *)(gChipBaseAddr + (addr << 1)))
#define HAL_GPIO_PM_SLEEP_REG(addr) (*(volatile u16 *)(gPmSleepBaseAddr + (addr << 1)))
#define HAL_GPIO_SAR_REG(addr)      (*(volatile u16 *)(gSarBaseAddr + (addr << 1)))
#define HAL_GPIO_RIU_REG(addr)      (*(volatile u16 *)(gRIUBaseAddr + (addr << 1)))

#define REG_ALL_PAD_IN 0xA1

static int _pmsleep_to_irq_table[] = {
    INT_PM_GPI_FIQ_PAD_PM_GPIO12,      INT_PM_GPI_FIQ_PAD_PM_GPIO11,       INT_PM_GPI_FIQ_PAD_PM_UART_TX,
    INT_PM_GPI_FIQ_PAD_PM_UART_RX,     INT_PM_GPI_FIQ_PAD_PM_PSPI0_INT,    INT_PM_GPI_FIQ_PAD_PM_PSPI0_DI,
    INT_PM_GPI_FIQ_PAD_PM_PSPI0_DO,    INT_PM_GPI_FIQ_PAD_PM_PSPI0_CK,     INT_PM_GPI_FIQ_PAD_PM_PSPI0_CZ,
    INT_PM_GPI_FIQ_PAD_PM_GPIO10,      INT_PM_GPI_FIQ_PAD_PM_GPIO9,        INT_PM_GPI_FIQ_PAD_PM_GPIO8,
    INT_PM_GPI_FIQ_PAD_PM_GPIO7,       INT_PM_GPI_FIQ_PAD_PM_PWM3,         INT_PM_GPI_FIQ_PAD_PM_PWM2,
    INT_PM_GPI_FIQ_PAD_PM_PWM1,        INT_PM_GPI_FIQ_PAD_PM_PWM0,         INT_PM_GPI_FIQ_PAD_PM_GPIO6,
    INT_PM_GPI_FIQ_PAD_PM_GPIO5,       INT_PM_GPI_FIQ_PAD_PM_GPIO4,        INT_PM_GPI_FIQ_PAD_PM_UART2_TX,
    INT_PM_GPI_FIQ_PAD_PM_UART2_RX,    INT_PM_GPI_FIQ_PAD_PM_I2C_CLK,      INT_PM_GPI_FIQ_PAD_PM_I2C_SDA,
    INT_PM_GPI_FIQ_PAD_PM_SDIO_INT,    INT_PM_GPI_FIQ_PAD_PM_GPIO3,        INT_PM_GPI_FIQ_PAD_PM_GPIO2,
    INT_PM_GPI_FIQ_PAD_PM_GPIO1,       INT_PM_GPI_FIQ_PAD_PM_GPIO0,        INT_PM_GPI_FIQ_PAD_PM_SDIO_D1,
    INT_PM_GPI_FIQ_PAD_PM_SDIO_D0,     INT_PM_GPI_FIQ_PAD_PM_SDIO_CLK,     INT_PM_GPI_FIQ_PAD_PM_SDIO_CMD,
    INT_PM_GPI_FIQ_PAD_PM_SDIO_D3,     INT_PM_GPI_FIQ_PAD_PM_SDIO_D2,      INT_PM_GPI_FIQ_PAD_PM_FUART_RTS,
    INT_PM_GPI_FIQ_PAD_PM_FUART_CTS,   INT_PM_GPI_FIQ_PAD_PM_FUART_RX,     INT_PM_GPI_FIQ_PAD_PM_FUART_TX,
    INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO0, INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO1,  INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO2,
    INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO3, INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO4,  INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO5,
    INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO6, INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO7,  INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO8,
    INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO9, INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO10, INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO11,
    INT_PM_GPI_FIQ_PAD_PM_HSRAM_GPIO12};

static int _gpi_to_irq_table[] = {
    INT_GPI_FIQ_PAD_I2C1_SDA,     INT_GPI_FIQ_PAD_SR_RST0,      INT_GPI_FIQ_PAD_SR_MCLK0,
    INT_GPI_FIQ_PAD_I2C1_SCL,     INT_GPI_FIQ_PAD_SR_RST1,      INT_GPI_FIQ_PAD_SR_MCLK1,
    INT_GPI_FIQ_PAD_I2C2_SCL,     INT_GPI_FIQ_PAD_I2C2_SDA,     INT_GPI_FIQ_PAD_GPIO0,
    INT_GPI_FIQ_PAD_GPIO1,        INT_GPI_FIQ_PAD_GPIO2,        INT_GPI_FIQ_PAD_GPIO3,
    INT_GPI_FIQ_PAD_GPIO4,        INT_GPI_FIQ_PAD_GPIO5,        INT_GPI_FIQ_PAD_GPIO6,
    INT_GPI_FIQ_PAD_GPIO7,        INT_GPI_FIQ_PAD_GPIO8,        INT_GPI_FIQ_PAD_GPIO9,
    INT_GPI_FIQ_PAD_GPIO10,       INT_GPI_FIQ_PAD_GPIO11,       INT_GPI_FIQ_PAD_SD0_CDZ,
    INT_GPI_FIQ_PAD_SD0_D1,       INT_GPI_FIQ_PAD_SD0_D0,       INT_GPI_FIQ_PAD_SD0_CLK,
    INT_GPI_FIQ_PAD_SD0_CMD,      INT_GPI_FIQ_PAD_SD0_D3,       INT_GPI_FIQ_PAD_SD0_D2,
    INT_GPI_FIQ_PAD_FUART_RTS,    INT_GPI_FIQ_PAD_FUART_CTS,    INT_GPI_FIQ_PAD_FUART_RX,
    INT_GPI_FIQ_PAD_FUART_TX,     INT_GPI_FIQ_PAD_MSPI_CZ,      INT_GPI_FIQ_PAD_MSPI_DO,
    INT_GPI_FIQ_PAD_MSPI_DI,      INT_GPI_FIQ_PAD_MSPI_CK,      INT_GPI_FIQ_PAD_SPI0_DO,
    INT_GPI_FIQ_PAD_SPI0_DI,      INT_GPI_FIQ_PAD_SPI0_HLD,     INT_GPI_FIQ_PAD_SPI0_WPZ,
    INT_GPI_FIQ_PAD_SPI0_CZ,      INT_GPI_FIQ_PAD_SPI0_CK,      INT_GPI_FIQ_PAD_I2C0_SDA,
    INT_GPI_FIQ_PAD_I2C0_SCL,     INT_GPI_FIQ_PAD_OUTN_RX0_CH5, INT_GPI_FIQ_PAD_OUTP_RX0_CH5,
    INT_GPI_FIQ_PAD_OUTN_RX0_CH4, INT_GPI_FIQ_PAD_OUTP_RX0_CH4, INT_GPI_FIQ_PAD_OUTN_RX0_CH3,
    INT_GPI_FIQ_PAD_OUTP_RX0_CH3, INT_GPI_FIQ_PAD_OUTN_RX0_CH2, INT_GPI_FIQ_PAD_OUTP_RX0_CH2,
    INT_GPI_FIQ_PAD_OUTN_RX0_CH1, INT_GPI_FIQ_PAD_OUTP_RX0_CH1, INT_GPI_FIQ_PAD_OUTN_RX0_CH0,
    INT_GPI_FIQ_PAD_OUTP_RX0_CH0};

//-------------------------------------------------------------------------------------------------
//  Global Functions
//-------------------------------------------------------------------------------------------------

void hal_gpio_init(void)
{
    HAL_GPIO_CHIPTOP_REG(REG_ALL_PAD_IN) &= ~(BIT0 | BIT1);
}

void hal_gpio_pad_set(u8 gpio_index)
{
    hal_gpio_pad_set_val(gpio_index, PINMUX_FOR_GPIO_MODE);
}

void hal_gpio_pad_clr(u8 gpio_index)
{
    hal_gpio_pad_clr_val(gpio_index, PINMUX_FOR_GPIO_MODE);
}

u8 hal_gpio_padgroupmode_set(u32 pad_mode)
{
    return hal_gpio_pad_set_mode(pad_mode);
}

u8 hal_gpio_pad_val_set(u8 gpio_index, u32 pad_mode)
{
    return hal_gpio_pad_set_val((u32)gpio_index, pad_mode);
}

u8 hal_gpio_pad_val_get(u8 gpio_index, u32 *pad_mode)
{
    return hal_gpio_pad_get_val((u32)gpio_index, pad_mode);
}

void hal_gpio_vol_val_set(u8 group, u32 mode)
{
    hal_gpio_set_vol((u32)group, mode);
}

u8 hal_gpio_pad_val_check(u8 gpio_index, u32 pad_mode)
{
    return hal_gpio_pad_check_val((u32)gpio_index, pad_mode);
}

u8 hal_gpio_pad_oen(u8 gpio_index)
{
    u32 pad_mode;
    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    hal_gpio_pad_get_val(gpio_index, &pad_mode);
    if (pad_mode == PINMUX_FOR_GPIO_MODE)
    {
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_oen) &= (~gpio_table[gpio_index].m_oen);
        return 0;
    }
    else
    {
        gpio_err("GPIO[%d] not in GPIO MODE\n", gpio_index);
        return 1;
    }
}

u8 hal_gpio_pad_odn(u8 gpio_index)
{
    u32 pad_mode;
    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    hal_gpio_pad_get_val(gpio_index, &pad_mode);
    if (pad_mode == PINMUX_FOR_GPIO_MODE)
    {
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_oen) |= gpio_table[gpio_index].m_oen;
        return 0;
    }
    else
    {
        gpio_err("GPIO[%d] not in GPIO MODE\n", gpio_index);
        return 1;
    }
}

u8 hal_gpio_pad_level(u8 gpio_index, u8 *pad_level)
{
    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    *pad_level = ((HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_in) & gpio_table[gpio_index].m_in) ? 1 : 0);
    return 0;
}

u8 hal_gpio_pad_in_out(u8 gpio_index, u8 *pad_in_out)
{
    u32 pad_mode;
    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    hal_gpio_pad_get_val(gpio_index, &pad_mode);
    if (pad_mode == PINMUX_FOR_GPIO_MODE)
    {
        *pad_in_out = ((HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_oen) & gpio_table[gpio_index].m_oen) ? 1 : 0);
        return 0;
    }
    else
    {
        gpio_err("GPIO[%d] not in GPIO MODE\n", gpio_index);
        return 1;
    }
}

u8 hal_gpio_pull_high(u8 gpio_index)
{
    u32 pad_mode;
    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    hal_gpio_pad_get_val(gpio_index, &pad_mode);
    if (pad_mode == PINMUX_FOR_GPIO_MODE)
    {
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_out) |= gpio_table[gpio_index].m_out;
        return 0;
    }
    else
    {
        gpio_err("GPIO[%d] not in GPIO MODE\n", gpio_index);
        return 1;
    }
}

u8 hal_gpio_pull_low(u8 gpio_index)
{
    u32 pad_mode;
    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    hal_gpio_pad_get_val(gpio_index, &pad_mode);
    if (pad_mode == PINMUX_FOR_GPIO_MODE)
    {
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_out) &= (~gpio_table[gpio_index].m_out);
        return 0;
    }
    else
    {
        gpio_err("GPIO[%d] not in GPIO MODE\n", gpio_index);
        return 1;
    }
}

u8 hal_gpio_pull_up(u8 gpio_index)
{
    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    else if (gpio_table[gpio_index].r_pe)
    {
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_pe) |= gpio_table[gpio_index].m_pe;
        if (gpio_table[gpio_index].r_ps)
        {
            HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_ps) |= gpio_table[gpio_index].m_ps;
        }
    }
    else
    {
        return 1; // no support pull up
    }
    return 0;
}

u8 hal_gpio_pull_down(u8 gpio_index)
{
    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    else if (gpio_table[gpio_index].r_pe)
    {
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_pe) |= gpio_table[gpio_index].m_pe;
        if (gpio_table[gpio_index].r_ps)
        {
            HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_ps) &= ~gpio_table[gpio_index].m_ps;
        }
    }
    else
    {
        return 1; // no support pull down
    }
    return 0;
}

u8 hal_gpio_pull_off(u8 gpio_index)
{
    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    else if (gpio_table[gpio_index].r_pe)
    {
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_pe) &= ~gpio_table[gpio_index].m_pe;
    }
    else
    {
        return 1; // no support pull enable
    }
    return 0;
}

u8 hal_gpio_pull_status(u8 gpio_index, u8 *pull_status)
{
    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    else if (gpio_table[gpio_index].r_pe)
    {
        if (HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_pe) & gpio_table[gpio_index].m_pe)
        {
            if (HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_ps) & gpio_table[gpio_index].m_ps)
                *pull_status = MHAL_PULL_UP;
            else
                *pull_status = MHAL_PULL_DOWN;
        }
        else
            *pull_status = MHAL_PULL_OFF;
    }
    else
    {
        return 1;
    }
    return 0;
}

u8 hal_gpio_set_high(u8 gpio_index)
{
    u32 pad_mode;
    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    hal_gpio_pad_get_val(gpio_index, &pad_mode);
    if (pad_mode == PINMUX_FOR_GPIO_MODE)
    {
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_oen) &= (~gpio_table[gpio_index].m_oen);
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_out) |= gpio_table[gpio_index].m_out;
        return 0;
    }
    else
    {
        gpio_err("GPIO[%d] not in GPIO MODE\n", gpio_index);
        return 1;
    }
}

u8 hal_gpio_set_low(u8 gpio_index)
{
    u32 pad_mode;
    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    hal_gpio_pad_get_val(gpio_index, &pad_mode);
    if (pad_mode == PINMUX_FOR_GPIO_MODE)
    {
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_oen) &= (~gpio_table[gpio_index].m_oen);
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_out) &= (~gpio_table[gpio_index].m_out);
        return 0;
    }
    else
    {
        gpio_err("GPIO[%d] not in GPIO MODE\n", gpio_index);
        return 1;
    }
}

u8 hal_gpio_drv_set(u8 gpio_index, u8 level)
{
    u8  lsb  = 0;
    u16 mask = 0;

    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    else if (gpio_table[gpio_index].r_drv)
    {
        mask = gpio_table[gpio_index].m_drv;

        if (!mask)
        {
            return 1;
        }

        // Calculate LSB by dichotomy
        if ((mask & 0xFF) == 0)
        {
            mask >>= 8;
            lsb += 8;
        }
        if ((mask & 0xF) == 0)
        {
            mask >>= 4;
            lsb += 4;
        }
        if ((mask & 0x3) == 0)
        {
            mask >>= 2;
            lsb += 2;
        }
        if ((mask & 0x1) == 0)
        {
            lsb += 1;
        }

        if (level > (gpio_table[gpio_index].m_drv >> lsb))
        {
            return 1;
        }
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_drv) &= ~gpio_table[gpio_index].m_drv;
        HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_drv) |= ((level << lsb) & gpio_table[gpio_index].m_drv);
    }
    else
    {
        return 1; // no support set driving
    }
    return 0;
}

u8 hal_gpio_drv_get(u8 gpio_index, u8 *level)
{
    u8  lsb  = 0;
    u16 mask = 0;

    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }
    else if (gpio_table[gpio_index].r_drv)
    {
        mask = gpio_table[gpio_index].m_drv;

        // Calculate LSB by dichotomy
        if ((mask & 0xFF) == 0)
        {
            mask >>= 8;
            lsb += 8;
        }
        if ((mask & 0xF) == 0)
        {
            mask >>= 4;
            lsb += 4;
        }
        if ((mask & 0x3) == 0)
        {
            mask >>= 2;
            lsb += 2;
        }
        if ((mask & 0x1) == 0)
        {
            lsb += 1;
        }
        *level = ((HAL_GPIO_RIU_REG(gpio_table[gpio_index].r_drv) & gpio_table[gpio_index].m_drv) >> lsb);
        return 0;
    }
    else
    {
        return 1;
    }
}

static int hal_gpio_pm_to_irq(u8 gpio_index)
{
    if ((gpio_index >= PAD_PM_GPIO12) && (gpio_index <= PAD_PM_HSRAM_GPIO12))
    {
        return _pmsleep_to_irq_table[gpio_index - PAD_PM_GPIO12];
    }
    else
    {
        return -1;
    }
}

int hal_gpio_gpi_to_irq(u8 gpio_index)
{
    if ((gpio_index >= PAD_I2C1_SDA) && gpio_index <= PAD_I2C0_SCL)
    {
        return _gpi_to_irq_table[gpio_index - PAD_I2C1_SDA];
    }
    else if ((gpio_index >= PAD_OUTN_RX0_CH5) && (gpio_index <= PAD_OUTP_RX0_CH0))
    {
        return _gpi_to_irq_table[(gpio_index - PAD_OUTN_RX0_CH5) + (PAD_I2C0_SCL - PAD_I2C1_SDA + 1)];
    }
    else
    {
        return -1;
    }
}

#if GPI_TYPE
int hal_gpio_to_irq(u8 gpio_index)
{
    struct device_node *intr_node;
    struct irq_domain * intr_domain;
    struct irq_fwspec   fwspec;
    int                 hwirq, virq = -1;

    if ((hwirq = hal_gpio_pm_to_irq(gpio_index)) >= 0)
    {
        // get virtual irq number for request_irq
        intr_node   = of_find_compatible_node(NULL, NULL, "sstar,pm-gpi-intc");
        intr_domain = irq_find_host(intr_node);
        if (!intr_domain)
            return -ENXIO;

        fwspec.param_count = 1;
        fwspec.param[0]    = hwirq;
        fwspec.fwnode      = of_node_to_fwnode(intr_node);
        virq               = irq_create_fwspec_mapping(&fwspec);
    }
    else if ((hwirq = hal_gpio_gpi_to_irq(gpio_index)) >= 0)
    {
        // get virtual irq number for request_irq
        intr_node   = of_find_compatible_node(NULL, NULL, "sstar,gpi-intc");
        intr_domain = irq_find_host(intr_node);
        if (!intr_domain)
            return -ENXIO;

        fwspec.param_count = 1;
        fwspec.param[0]    = hwirq;
        fwspec.fwnode      = of_node_to_fwnode(intr_node);
        virq               = irq_create_fwspec_mapping(&fwspec);
    }

    return virq;
}
#else
#define MHAL_GPIO_SUPPORT_INTR 0
extern struct vint_handler_t ss_pm_vintc;
extern struct vint_handler_t ss_gpi_vintc;
int                          hal_gpio_to_irq(u8 gpio_index)
{
    IntInitParam_u uInitParam = {{0}};
    int            hwirq = 0, virq = -1;

    if ((virq = hal_gpio_pm_to_irq(gpio_index)) >= 0)
    {
        uInitParam.intc.eMap      = INTC_MAP_IRQ;
        uInitParam.intc.ePriority = INTC_PRIORITY_7;
        uInitParam.intc.pfnIsr    = (PfnIntcISR)ss_pm_vintc.pvIntIsrCB;
        DrvRegisterVirtualInterrupt(&uInitParam, &ss_pm_vintc);

        hwirq = ss_pm_vintc.nHWIRQNum;
    }
    else if ((virq = hal_gpio_gpi_to_irq(gpio_index)) >= 0)
    {
        uInitParam.intc.eMap      = INTC_MAP_IRQ;
        uInitParam.intc.ePriority = INTC_PRIORITY_7;
        uInitParam.intc.pfnIsr    = (PfnIntcISR)ss_gpi_vintc.pvIntIsrCB;
        DrvRegisterVirtualInterrupt(&uInitParam, &ss_gpi_vintc);

        hwirq = ss_gpi_vintc.nHWIRQNum;
    }

    return (virq << 16 | hwirq);
}
#endif

u8 hal_gpio_get_check_count(void)
{
    return hal_gpio_pad_check_info_count();
}

void *hal_gpio_get_check_info(u8 index)
{
    return hal_gpio_pad_check_info_get(index);
}

u8 hal_gpio_name_to_num(u8 *p_name, u8 *gpio_index)
{
    u8 index;
    for (index = 0; index < gpio_table_size; index++)
    {
        if (!strcmp((const char *)gpio_table[index].p_name, (const char *)p_name))
        {
            *gpio_index = index;
            return 0;
        }
    }
    return 1;
}

u8 hal_gpio_num_to_name(u8 gpio_index, const u8 **p_name)
{
    if (gpio_index >= GPIO_NR)
    {
        p_name = NULL;
        return 1;
    }
    *p_name = gpio_table[gpio_index].p_name;
    return 0;
}

u8 hal_gpio_padmode_to_val(u8 *p_mode, u32 *mode_to_val)
{
    return hal_gpio_padmux_to_val(p_mode, mode_to_val);
}

u32 *hal_gpio_padmode_to_padindex(u32 mode)
{
    return hal_gpio_padmdoe_to_padindex(mode);
}

u8 hal_gpio_get_reg_info(u8 gpio_index, void *reg_cfg)
{
    hal_gpio_reg_info *reg_info = (hal_gpio_reg_info *)reg_cfg;

    if (gpio_index >= GPIO_NR)
    {
        return 1;
    }

    reg_info->reg  = gpio_table[gpio_index].r_out;
    reg_info->mask = gpio_table[gpio_index].m_out;

    return 0;
}
