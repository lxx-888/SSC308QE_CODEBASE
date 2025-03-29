/*
* sram.S- Sigmastar
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
/*------------------------------------------------------------------------------
    Function Code
-------------------------------------------------------------------------------*/
#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <asm/hardware/cache-l2x0.h>
#include "mailbox_str.h"


#define SUSPEND_KEEP_INFO 0xC0000000
// offsetof(struct suspend_keep_info, dram_crc)
#define DRAM_CRC_OFFSET 0x1C
// offsetof(struct suspend_keep_info, dram_crc_start)
#define DRAM_CRC_START_OFFSET_L 0x20
#define DRAM_CRC_START_OFFSET_H 0x24
// offsetof(struct suspend_keep_info, dram_crc_size)
#define DRAM_CRC_SIZE_OFFSET_L 0x28
#define DRAM_CRC_SIZE_OFFSET_H 0x2C


#define ARCH_us(x) (6*x)
    .align 3
.globl sram_suspend_imi
.globl v7_cpu_resume

/* A macro about using arch timer to delay
    inputs :
    t : us to delay
    l : For different branch naming
*/
.macro arch_usdelay t, l
    ldr   r8, =ARCH_us(\t)      /* delay tms */
    mrrc  p15, 0, r9, r10, cr14
    adds  r9, r9, r8            /* Target tick */
    adc   r10, r10, #0x00       /* Add with carry instruction */
delay_retry\l:
    isb sy
    mrrc  p15, 0, r11, r12, cr14
    cmp   r12, r10              /* compare MSB part*/
    blt   delay_retry\l         /* jump to delay_retry in case r10 is bigger (N==1) */
    cmp   r11, r9               /* compare LSB part*/
    blt   delay_retry\l         /* jump to delay_retry in case r9 is bigger (N==1) */
.endm

////RREG_B
.macro  RREG_B  ret, reg
    ldr   r7, =\reg
    ldrb  \ret, [r7]
.endm

////RREG_W
.macro  RREG_W  ret, reg
    ldr   r7, =\reg
    ldr   \ret, [r7]
.endm

////WREG_B
.macro  WREG_B  reg, mask, val
    ldr   r7, =\reg
    ldr   r8, =\mask
    ldr   r9, =\val
    ldrb  r6, [r7]
    bic   r6, r8
    orr   r6, r9
    strb  r6, [r7]
.endm

////WREG_W
.macro  WREG_W  reg, val
    ldr   r6, =\reg
    ldr   r7, =\val
    str   r7, [r6]
.endm

////MBOX
.macro  MBOX  val
    WREG_W  0xfd200800, \val
.endm

/*
 * dcache_line_size - get the minimum D-cache line size from the CTR register
 * on ARMv7.
 */
    .macro  dcache_line_size, reg, tmp
#ifdef CONFIG_CPU_V7M
    movw    \tmp, #:lower16:BASEADDR_V7M_SCB + V7M_SCB_CTR
    movt    \tmp, #:upper16:BASEADDR_V7M_SCB + V7M_SCB_CTR
    ldr     \tmp, [\tmp]
#else
    mrc p15, 0, \tmp, c0, c0, 1     @ read ctr
#endif
    lsr \tmp, \tmp, #16
    and \tmp, \tmp, #0xf        @ cache line size encoding
    mov \reg, #4            @ bytes per word
    mov \reg, \reg, lsl \tmp        @ actual cache line size
    .endm

ENTRY(sram_suspend_imi)
    stmfd  sp!, {r0-r10, lr}
start_dram_crc:
	ldr   r1, =0xfd000000
	ldr   r3, =0x100200
	add   r2, r1, r3, lsl #1

	// skip do dram crc if crc size is zero
	ldr   r4, =SUSPEND_KEEP_INFO
	ldr   r0, [r4, #DRAM_CRC_SIZE_OFFSET_L]
	cmp   r0, #0x0000
	beq   skip_dram_crc

	//wriu -w  0x100201, 0x001C
	ldr   r0, =0x001C
	strh  r0, [r2, #0x01 << 2]
	// wriu -w  0x100202, 0x0340
	ldr   r0, =0x0340
	strh  r0, [r2, #0x02 << 2]
	// wriu -w  0x100203, 0x0010
	ldr   r0, =0x0000
	strh  r0, [r2, #0x03 << 2]

	// set src address
	ldr   r3, =0xFFFF
	ldr   r4, =SUSPEND_KEEP_INFO
	ldr   r0, [r4, #DRAM_CRC_START_OFFSET_L]
	and   r0, r3
	strh  r0, [r2, #0x04 << 2]

	ldr   r0, [r4, #DRAM_CRC_START_OFFSET_L]
	lsr   r0, r0, #16
	and   r0, r3
	strh  r0, [r2, #0x05 << 2]

	ldr   r0, [r4, #DRAM_CRC_START_OFFSET_H]
	and   r0, r3
	strh  r0, [r2, #0x1A << 2]

	// set dst address
	ldr   r0, =0x0000
	str   r0, [r2, #0x06 << 2]
	str   r0, [r2, #0x07 << 2]
	str   r0, [r2, #0x1B << 2]

	// set calc size
	ldr   r4, =SUSPEND_KEEP_INFO
	ldr   r0, [r4, #DRAM_CRC_SIZE_OFFSET_L]
	and   r0, r3
	strh  r0, [r2, #0x08 << 2]

	ldr   r0, [r4, #DRAM_CRC_SIZE_OFFSET_L]
	lsr   r0, r0, #16
	and   r0, r3
	strh  r0, [r2, #0x09 << 2]

	//wriu -w  0x10020A, 0x1DB7
	ldr   r0, =0x1DB7
	str   r0, [r2, #0x0A << 2]
	//wriu -w  0x10020B, 0x04C1
	ldr   r0, =0x04C1
	str   r0, [r2, #0x0B << 2]
	//wriu -w  0x10020C, 0xFFFF
	ldr   r0, =0xFFFF
	str   r0, [r2, #0x0C << 2]
	//wriu -w  0x10020D, 0xFFFF
	ldr   r0, =0xFFFF
	str   r0, [r2, #0x0D << 2]

	// trigger bdma
	ldr   r0, =0x0001
	str   r0, [r2]

poll:
	ldrh  r0, [r2, #0x01 << 2]
	and   r0, r0, #0x0008
	cmp   r0, #0x0008
	bne   poll

	// pass crc result by suspend_keep_info.suspend_crc
	ldr   r4, =SUSPEND_KEEP_INFO
	ldrh  r0, [r2, #0x0D << 2]
	lsl   r1, r0, #16
	ldrh  r0, [r2, #0x0C << 2]
	orr   r1, r1, r0
	str   r1, [r4, #DRAM_CRC_OFFSET]

    // flush cache
    ldr   r0, =SUSPEND_KEEP_INFO
    ldr   r1, =DRAM_CRC_OFFSET

    dcache_line_size r2, r3
    add r1, r0, r1
    sub r3, r2, #1
    bic r0, r0, r3
next:
    mcr p15, 0, r0, c7, c14, 1      @ clean & invalidate D line / unified line
    add r0, r0, r2
    cmp r0, r1
    blo next
    dsb st
skip_dram_crc:

    // 0x1647 0x5a [8] -> 0: not lp, 1: lp
    ldr   r4, =0xFD2C8E00
    ldr   r0, [r4, #0x168]
    and   r0, r0, #0x100
    cmp   r0, #0x100
    beq   I6dw_LPDDR4_SR_Enter

disable_port:
    MBOX    0x5001
    // 04.I6dw_DDR3_SR_Enter.txt
    // disable port
    WREG_W  0xfd2c20f4, 0x0001 // wriu -w 0x16107a 0x0001 // pa   mcg riu clock on
    WREG_W  0xfd2ca1ac, 0x0300 //wriu -w 0x1650d6 0x0300 // garb mcg riu clock on
    WREG_W  0xfd2ca3ac, 0x0300 //wriu -w 0x1651d6 0x0300 // garb mcg riu clock on
    WREG_W  0xfd2ca5ac, 0x0300 //wriu -w 0x1652d6 0x0300 // garb mcg riu clock on

    WREG_W  0xfd2c2000, 0x2b20 //wriu -w 0x161000 0x2b20 // NW  mask
    WREG_W  0xfd2c2004, 0x2b20 //wriu -w 0x161002 0x2b20 // DLA mask
    WREG_W  0xfd2c2008, 0x2b20 //wriu -w 0x161004 0x2b20 // CPU mask

    WREG_W  0xfd2c2080, 0x2b20 //wriu -w 0x161040 0x2b20 // NW  mask
    WREG_W  0xfd2c2084, 0x2b20 //wriu -w 0x161042 0x2b20 // DLA mask
    WREG_W  0xfd2c2088, 0x2b20 //wriu -w 0x161044 0x2b20 // CPU mask
    WREG_W  0xfd2c2108, 0x2b20 //wriu -w 0x161084 0x2b20 // BWLA mask

    WREG_W  0xfd2c207c, 0x0001 // wriu -w 0x16103e 0x0001 // pa toggle
    WREG_W  0xfd2c20fc, 0x0001 //wriu -w 0x16107e 0x0001 // pa toggle

    WREG_W  0xfd2ca050, 0xffff //wriu -w 0x165028 0xffff // garb mask
    WREG_W  0xfd2ca250, 0xffff //wriu -w 0x165128 0xffff // garb mask
    WREG_W  0xfd2ca450, 0xffff //wriu -w 0x165228 0xffff // garb mask

    WREG_W  0xfd2ca110, 0xffff //wriu -w 0x165088 0xffff // garb mask
    WREG_W  0xfd2ca310, 0xffff //wriu -w 0x165188 0xffff // garb mask
    WREG_W  0xfd2ca510, 0xffff //wriu -w 0x165288 0xffff // garb mask

    WREG_W  0xfd2ca04c, 0x0017 // wriu -w 0x165026 0x0017 // garb toggle
    WREG_W  0xfd2ca24c, 0x0017 // wriu -w 0x165126 0x0017 // garb toggle
    WREG_W  0xfd2ca44c, 0x0017 // wriu -w 0x165226 0x0017 // garb toggle
    WREG_W  0xfd2ca10c, 0x0017 //wriu -w 0x165086 0x0017 // garb toggle
    WREG_W  0xfd2ca30c, 0x0017 //wriu -w 0x165186 0x0017 // garb toggle
    WREG_W  0xfd2ca50c, 0x0017 //wriu -w 0x165286 0x0017 // garb toggle

    WREG_W  0xfd2c20f4, 0x0000 // wriu -w 0x16107a 0x0000 // pa   mcg riu clock off
    WREG_W  0xfd2ca1ac, 0x0100 //wriu -w 0x1650d6 0x0100 // garb mcg riu clock off
    WREG_W  0xfd2ca3ac, 0x0100 //wriu -w 0x1651d6 0x0100 // garb mcg riu clock off
    WREG_W  0xfd2ca5ac, 0x0100 //wriu -w 0x1652d6 0x0100 // garb mcg riu clock off
    // wait 20
    arch_usdelay 100,__LINE__

dram_precharge_all:
    MBOX    0x5002
    //---------------------------
    // Disable Pre-Charge PD @I6dw-DDR3/4
    //---------------------------
    WREG_W  0xfd20240c,  0x1000    //wriu -w   0x101206  0x1000    //[12]   : reg_cke_always_on = 1

    //---------------------------
    //Pre-charge all then one refresh
    //---------------------------
    WREG_B  0xfd2c2641,  0x80, 0x00 //wriu -b   0x161321  0x80 0x00 //[15]   : switch single_cmd to old = 0

    WREG_W  0xfd202430,  0x0400    //wriu -w   0x101218  0x0400    //reg_mrx = 0400 (precharege all)
    WREG_B  0xfd202400,  0x30, 0x20 //wriu -b   0x101200  0x30 0x20 //[05]   : reg_auto_ref_off  = 1
                                  //[04]   : reg_odt           = 0
    WREG_B  0xfd202401,  0x1e, 0x04 //wriu -b   0x101201  0x1e 0x04 //[12:9] : reg_single_cmd    = 2 (precharege)
    WREG_B  0xfd202401,  0x01, 0x00 //wriu -b   0x101201  0x01 0x00 //[8]    : reg_single_cmd_en = 0
    WREG_B  0xfd202401,  0x01, 0x01 //wriu -b   0x101201  0x01 0x01 //[8]    : reg_single_cmd_en = 1
    WREG_B  0xfd202401,  0x01, 0x00 //wriu -b   0x101201  0x01 0x00 //[8]    : reg_single_cmd_en = 0
    WREG_B  0xfd202401,  0x1e, 0x02 //wriu -b   0x101201  0x1e 0x02 //[12:9] : reg_single_cmd    = 1 (refresh)
    WREG_B  0xfd202401,  0x01, 0x00 //wriu -b   0x101201  0x01 0x00 //[8]    : reg_single_cmd_en = 0
    WREG_B  0xfd202401,  0x01, 0x01 //wriu -b   0x101201  0x01 0x01 //[8]    : reg_single_cmd_en = 1
    WREG_B  0xfd202401,  0x01, 0x00 //wriu -b   0x101201  0x01 0x00 //[8]    : reg_single_cmd_en = 0s
    // delay, DO NOT REMOVE THIS DELAY!!!!
    arch_usdelay 100,__LINE__

dram_enter_self_refresh:
    MBOX    0x5003
    WREG_W  0xfd202400,  0x202e //wriu -w   0x101200  0x202e //Bit[13] reg_self_refresh  = 1

    // wait 10
    arch_usdelay 10,__LINE__

ana_pwr_down:
    MBOX    0x5004
    // 05.I6d_AN_PowerDown.txt
    WREG_B  0xfd2c8f68, 0x3f, 0x3f  //wriu -b 0x1647b4 0x3f 0x3f  // digital oenz

    WREG_B  0xfd2c8e00, 0x08, 0x08  //wriu -b 0x164700 0x08 0x08    //gpio_oenz
    WREG_B  0xfd2c8e32, 0x20, 0x20  //wriu -b 0x164719 0x20 0x20    //gpio_oenz_dft
    WREG_B  0xfd2c8ff8, 0x3f, 0x3f  //wriu -b 0x1647fc 0x3f 0x3f    //gpio_oenz_dq/cmd/cke/mask/clk/cs

    WREG_B  0xfd2c8e00, 0x10, 0x10  //wriu -b 0x164700 0x10 0x10
    WREG_B  0xfd2c8e02, 0x60, 0x60  //wriu -b 0x164701 0x60 0x60
    WREG_B  0xfd2c8e32, 0x40, 0x40  //wriu -b 0x164719 0x40 0x40    //sel_gpio_dft

    WREG_B  0xfd2c8ea8, 0x70, 0x70  //wriu -b 0x164754 0x70 0x70
    WREG_B  0xfd2c8ea8, 0x70, 0x70  //wriu -b 0x164708 0x3f 0x00
    b chip_pwr_down

I6dw_LPDDR4_SR_Enter:
    MBOX    0x5011
    // 04.I6dw_LPDDR4_SR_Enter.txt
    // disable port
    WREG_W  0xfd2c20f4, 0x0001 // wriu -w 0x16107a 0x0001 // pa   mcg riu clock on
    WREG_W  0xfd2ca1ac, 0x0300 //wriu -w 0x1650d6 0x0300 // garb mcg riu clock on
    WREG_W  0xfd2ca3ac, 0x0300 //wriu -w 0x1651d6 0x0300 // garb mcg riu clock on
    WREG_W  0xfd2ca5ac, 0x0300 //wriu -w 0x1652d6 0x0300 // garb mcg riu clock on

    WREG_W  0xfd2c2000, 0x2b20 //wriu -w 0x161000 0x2b20 // NW  mask
    WREG_W  0xfd2c2004, 0x2b20 //wriu -w 0x161002 0x2b20 // DLA mask
    WREG_W  0xfd2c2008, 0x2b20 //wriu -w 0x161004 0x2b20 // CPU mask

    WREG_W  0xfd2c2080, 0x2b20 //wriu -w 0x161040 0x2b20 // NW  mask
    WREG_W  0xfd2c2084, 0x2b20 //wriu -w 0x161042 0x2b20 // DLA mask
    WREG_W  0xfd2c2088, 0x2b20 //wriu -w 0x161044 0x2b20 // CPU mask
    WREG_W  0xfd2c2108, 0x2b20 //wriu -w 0x161084 0x2b20 // BWLA mask

    WREG_W  0xfd2c207c, 0x0001 // wriu -w 0x16103e 0x0001 // pa toggle
    WREG_W  0xfd2c20fc, 0x0001 //wriu -w 0x16107e 0x0001 // pa toggle

    WREG_W  0xfd2ca050, 0xffff //wriu -w 0x165028 0xffff // garb mask
    WREG_W  0xfd2ca250, 0xffff //wriu -w 0x165128 0xffff // garb mask
    WREG_W  0xfd2ca450, 0xffff //wriu -w 0x165228 0xffff // garb mask

    WREG_W  0xfd2ca110, 0xffff //wriu -w 0x165088 0xffff // garb mask
    WREG_W  0xfd2ca310, 0xffff //wriu -w 0x165188 0xffff // garb mask
    WREG_W  0xfd2ca510, 0xffff //wriu -w 0x165288 0xffff // garb mask

    WREG_W  0xfd2ca04c, 0x0017 // wriu -w 0x165026 0x0017 // garb toggle
    WREG_W  0xfd2ca24c, 0x0017 // wriu -w 0x165126 0x0017 // garb toggle
    WREG_W  0xfd2ca44c, 0x0017 // wriu -w 0x165226 0x0017 // garb toggle
    WREG_W  0xfd2ca10c, 0x0017 //wriu -w 0x165086 0x0017 // garb toggle
    WREG_W  0xfd2ca30c, 0x0017 //wriu -w 0x165186 0x0017 // garb toggle
    WREG_W  0xfd2ca50c, 0x0017 //wriu -w 0x165286 0x0017 // garb toggle

    WREG_W  0xfd2c20f4, 0x0000 // wriu -w 0x16107a 0x0000 // pa   mcg riu clock off
    WREG_W  0xfd2ca1ac, 0x0100 //wriu -w 0x1650d6 0x0100 // garb mcg riu clock off
    WREG_W  0xfd2ca3ac, 0x0100 //wriu -w 0x1651d6 0x0100 // garb mcg riu clock off
    WREG_W  0xfd2ca5ac, 0x0100 //wriu -w 0x1652d6 0x0100 // garb mcg riu clock off
    // wait 20
    arch_usdelay 100,__LINE__

    MBOX    0x5002
    //---------------------------
    // Disable Pre-Charge PD @I6dw-LPDDR4
    //---------------------------
    WREG_W  0xfd20240c,  0x5000    //wriu -w   0x101206  0x1000    //[12]   : reg_cke_always_on = 1

    //---------------------------
    //Pre-charge all then one refresh
    //---------------------------
    WREG_B  0xfd2c2641,  0x80, 0x00 //wriu -b   0x161321  0x80 0x00 //[15]   : switch single_cmd to old = 0

    WREG_W  0xfd202430,  0x0400    //wriu -w   0x101218  0x0400    //reg_mrx = 0400 (precharege all)
    WREG_B  0xfd202400,  0x30, 0x20 //wriu -b   0x101200  0x30 0x20 //[05]   : reg_auto_ref_off  = 1
                                  //[04]   : reg_odt           = 0
    WREG_B  0xfd202401,  0x1e, 0x04 //wriu -b   0x101201  0x1e 0x04 //[12:9] : reg_single_cmd    = 2 (precharege)
    WREG_B  0xfd202401,  0x01, 0x00 //wriu -b   0x101201  0x01 0x00 //[8]    : reg_single_cmd_en = 0
    WREG_B  0xfd202401,  0x01, 0x01 //wriu -b   0x101201  0x01 0x01 //[8]    : reg_single_cmd_en = 1
    WREG_B  0xfd202401,  0x01, 0x00 //wriu -b   0x101201  0x01 0x00 //[8]    : reg_single_cmd_en = 0
    WREG_B  0xfd202401,  0x1e, 0x02 //wriu -b   0x101201  0x1e 0x02 //[12:9] : reg_single_cmd    = 1 (refresh)
    WREG_B  0xfd202401,  0x01, 0x00 //wriu -b   0x101201  0x01 0x00 //[8]    : reg_single_cmd_en = 0
    WREG_B  0xfd202401,  0x01, 0x01 //wriu -b   0x101201  0x01 0x01 //[8]    : reg_single_cmd_en = 1
    WREG_B  0xfd202401,  0x01, 0x00 //wriu -b   0x101201  0x01 0x00 //[8]    : reg_single_cmd_en = 0s
    // delay, DO NOT REMOVE THIS DELAY!!!!
    arch_usdelay 100,__LINE__

    MBOX    0x5003
    WREG_W  0xfd202400,  0x202e //wriu -w   0x101200  0x202e //Bit[13] reg_self_refresh  = 1
	// CKE low into power-down
    WREG_B  0xfd202400,  0x02, 0x00 //Bit[01]   : reg_cke = 0
    // wait 10
    arch_usdelay 10,__LINE__

    MBOX    0x5004
    // 05.I6d_AN_PowerDown.txt
    WREG_B  0xfd2c8ea8, 0x70, 0x70  //wriu -b 0x164754 0x70 0x70
    WREG_B  0xfd2c8ea8, 0x70, 0x70  //wriu -b 0x164708 0x3f 0x00

chip_pwr_down:
    MBOX    0x5005
#ifdef CONFIG_PM_MCU_POWER_OFF
#if defined(CONFIG_PM_MCU_USE_UART1)
    ldr   r7, =0xfd221200
#elif defined(CONFIG_PM_MCU_USE_FUART)
    ldr   r7, =0xfd220400
#endif
    // Prot.header
    ldr   r8, =0x5a
    strb  r8, [r7]
    strb  r8, [r7]
    strb  r8, [r7]
    strb  r8, [r7]
    // Prot.eCmdType
    ldr   r8, =0x00
    strb  r8, [r7]
    // Prot.eTaskCmd
    ldr   r8, =0x07
    strb  r8, [r7]
    // Prot.isReject
    ldr   r8, =0x00
    strb  r8, [r7]
    // Prot.eSocState
    ldr   r8, =0x06
    strb  r8, [r7]
    // Prot.u16UserDefine
    ldr   r8, =0x00
    strb  r8, [r7]
    strb  r8, [r7]
    // Prot.tail
    ldr   r8, =0xa5
    strb  r8, [r7]
    strb  r8, [r7]
    strb  r8, [r7]
    strb  r8, [r7]
#else
    //06. rtcpoc power down
    //set bank34, offset f, bit[0] to 0
    WREG_B  0xfd00683c, 0x01, 0x00
    //set bank34, offset 49, bit[12] to 0 //cut IO4
    WREG_B  0xfd006925, 0x10, 0x00

    //iso flow
    WREG_W  0xfd0068c8, 0x0001  //wriu -w   0x003464  0x0001
    WREG_W  0xfd0068c0, 0x0001  //wriu -w   0x003460  0x0001
#endif
ENDPROC(sram_suspend_imi)
.ltorg
