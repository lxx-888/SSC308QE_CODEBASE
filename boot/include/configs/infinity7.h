/*
 * infinity7.h - Sigmastar
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

#ifndef __INFINITY7_H
#define __INFINITY7_H

/*------------------------------------------------------------------------------
    Constant
-------------------------------------------------------------------------------*/
/* This is columbus2 hardware */
#define CONFIG_ARCH_INFINITY7	1
#define CONFIG_SYS_L2CACHE_OFF		/* No L2 cache */
/*#define CONFIG_SYS_ARCH_TIMER   1*/
#define CONFIG_MS_PIU_TIMER   1
#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_DELAY_STR "\x0d" /* press ENTER to interrupt BOOT */

#define CONFIG_DISPLAY_BOARDINFO    1

#define CONFIG_BOARD_LATE_INIT

#if CONFIG_SSTAR_VERSION_FPGA
#define CONFIG_SYS_HZ_CLOCK 24000000
#define CONFIG_UART_CLOCK   24000000
#define CONFIG_BAUDRATE	    115200
#define CONFIG_PIUTIMER_CLOCK 12000000
#else
#define CONFIG_SYS_HZ_CLOCK 400000000
#define CONFIG_UART_CLOCK   172800000
#define CONFIG_BAUDRATE	    115200
#define CONFIG_PIUTIMER_CLOCK 12000000
#endif

#define CONFIG_WDT_CLOCK    CONFIG_PIUTIMER_CLOCK

/* OTP Base Address */
#define CONFIG_OTPCTRL_FUN_ADDRESS              0xA0000020UL
#define CONFIG_OTPCTRL_LOG_ADDRESS              0xA0010000UL

/* define baud rate */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*------------------------------------------------------------------------------
    Macro
-------------------------------------------------------------------------------*/

/* boot delay time */
#ifndef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY	0
#endif //CONFIG_BOOTDELAY
#define CONFIG_ZERO_BOOTDELAY_CHECK

/*
#define CONFIG_MS_ANDROID_RECOVERY  1
#define CONFIG_MS_DISPLAY   1
#define CONFIG_MS_SHOW_LOGO 1
#define CONFIG_MS_ISP       1
#define CONFIG_MS_PIUTIMER 1
*/

#define CONFIG_SKIP_LOWLEVEL_INIT
/*#define CONFIG_DISPLAY_BOARDINFO *//*for checkboard*/

/*
 * Size of malloc() pool
 */
#ifndef CONFIG_SYS_MALLOC_LEN
#ifdef CONFIG_SSTAR_ANDROID_BOOTLOADER
/* at less the total partition size of boot and vendor_boot need to be reserved */
#define CONFIG_SYS_MALLOC_LEN	    (CONFIG_ENV_SIZE + 4*1024*1024 + 32*1024*1024 + 32*1024*1024)
#else
#define CONFIG_SYS_MALLOC_LEN	    (CONFIG_ENV_SIZE + 4*1024*1024)
#endif
#endif

/*
 * Miscellaneous configurable options
 */
#ifndef CONFIG_SYS_LONGHELP
#define CONFIG_SYS_LONGHELP                     /* undef to save memory     */
#endif //CONFIG_SYS_LONGHELP

/* wherher Prompt is defined, use "SigmaStar #" */
#ifdef CONFIG_SYS_PROMPT
#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT       "SigmaStar # "  /* Monitor Command Prompt   */
#endif /* CONFIG_SYS_PROMPT */
#define CONFIG_SYS_CBSIZE       512             /* Console I/O Buffer Size  */

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE	        (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS	        64		    /* max number of command args   */
#define CONFIG_SYS_BARGSIZE	    CONFIG_SYS_CBSIZE	/* Boot Argument Buffer Size    */

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	    (4*1024*1024)  /* regular stack */

/*
 * Physical Memory Map
 */
#ifndef CONFIG_NR_DRAM_BANKS
#define CONFIG_NR_DRAM_BANKS	1   /* we have 1 bank of DRAM */
#endif //CONFIG_NR_DRAM_BANKS
#define PHYS_SDRAM_1		0x20000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	CONFIG_SSTAR_RAM_SIZE	/* 16/64 MB */

/*Enable watchdog*/
/*#define CONFIG_HW_WATCHDOG 1*/
#ifdef CONFIG_HW_WATCHDOG
#define CONFIG_HW_WATCHDOG_TIMEOUT_S	60
#endif


#define CONFIG_SYS_MEMTEST_START	0x20000000	/* memtest works on	*/
#define CONFIG_SYS_MEMTEST_END		0x22000000	/* 0 ... 32 MB in DRAM	*/

#define CONFIG_SYS_SDRAM_BASE	PHYS_SDRAM_1
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE  - GENERATED_GBL_DATA_SIZE)

#define CONFIG_UBOOT_RAM_SIZE   CONFIG_SSTAR_RAM_SIZE // let us to use only 64MB for uboot



/* RAM base address */
#define RAM_START_ADDR          0x20000000
#define IMI_START_ADDR          0xA0000000

/* RAM size */
#define RAM_SIZE		        PHYS_SDRAM_1_SIZE
#define IMI_SIZE                0x74000 // (464k)
/* The address used to save tag list used when kernel is booting */
#define BOOT_PARAMS 	        (RAM_START_ADDR)
#define BOOT_PARAMS_LEN         0x2000

/* CFG load address */
#define CONFIG_SYS_LOAD_ADDR	        (BOOT_PARAMS+BOOT_PARAMS_LEN+0x4000)

#define CONFIG_CMDLINE_TAG       1    /* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG        1

/* kernel starting address */
#define KERNEL_RAM_BASE	        CFG_LOAD_ADDR

/* Which block used to save IPL, u-boot and kernel images */
#define IPL_NAND_BLOCK      0
#define UBOOT_NAND_BLOCK    1
#define KERNEL_NAND_BLOCK   2


#define CONFIG_CMDLINE_EDITING 1

#ifndef CONFIG_AUTO_COMPLETE
#define CONFIG_AUTO_COMPLETE
#endif//CONFIG_AUTO_COMPLETE

/* boot time analysis*/
#define CONFIG_BOOT_TIME_ANALYSIS			0
#define CONFIG_BOOT_TIME_ANALYSIS_USE_RTC	0

#define CONFIG_SYS_NO_FLASH 			   1

#define ENV_PART_NAME                   "ENV"
#define ENV_REDUND_PART_NAME            "ENV1"

/*
 * FLASH driver setup
 */

#if defined(CONFIG_MS_SDMMC) || defined(CONFIG_MS_EMMC) || defined(CONFIG_MS_USB)
#define CONFIG_CMD_FAT
#endif

#ifdef CONFIG_MS_SDMMC
#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_DOS_PARTITION
#define CONFIG_MS_SDMMC_MAX_READ_BLOCKS 1024
#define CONFIG_MMC_FDISK
#ifdef CONFIG_MS_SAVE_ENV_IN_SDMMC
#define CONFIG_ENV_IS_IN_MMC       1
#define CONFIG_SYS_MMC_ENV_DEV     0
#define CONFIG_ENV_SIZE         0x00020000
#define CONFIG_ENV_OFFSET          0x4F000
#endif
#endif

#ifdef CONFIG_MS_EMMC
#ifndef CONFIG_MMC
#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#endif
#endif

/*
 * NAND flash
 */
#ifdef CONFIG_SSTAR_NAND

#define MTDIDS_DEFAULT			"nand0=nand0"    /* "nor0=physmap-flash.0,nand0=nand" */
/*	must be different from real partition to test NAND partition function */
#define MTDPARTS_DEFAULT		"mtdparts=nand0:0xC0000@0x140000(NPT),-(UBI)"
/*	#define MTDPARTS_DEFAULT    "mtdparts=nand0:0x60000@0x140000(IPL0),0x60000(IPL1),0x60000(IPL_CUST0),0x60000(IPL_CUST1),0xC0000(UBOOT0),0xC0000(UBOOT1),0x60000(ENV),0x340000(KERNEL),0x340000(RECOVERY),-(UBI)"*/

#define CONFIG_EXTRA_ENV_SETTINGS                              \
       "mtdids=" MTDIDS_DEFAULT "\0"                           \
       "mtdparts=" MTDPARTS_DEFAULT "\0"                       \
       "partition=nand0,0\0"                                   \
       ""

#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_NAND_MAX_CHIPS	1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0 /* not actually used */
#endif /*CONFIG_CMD_NAND*/

#ifdef CONFIG_ENV_IS_IN_NAND
#ifdef CONFIG_ENV_SIZE
#undef CONFIG_ENV_SIZE
#endif
#ifdef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_OFFSET
#endif
#define CONFIG_ENV_SIZE            0x1000
#define CONFIG_ENV_OFFSET          ss_nand_env_offset
#define CONFIG_ENV_RANGE           ss_nand_env_part_size
#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
#ifdef CONFIG_ENV_OFFSET_REDUND
#undef CONFIG_ENV_OFFSET_REDUND
#endif
#define CONFIG_ENV_OFFSET_REDUND   ss_nand_env_redund_offset
#endif
#endif
#endif /* CONFIG_SSTAR_NAND */

#ifdef CONFIG_SSTAR_NOR
#define CONFIG_SF_DEFAULT_BUS 0
#define CONFIG_SF_DEFAULT_CS 0
#define CONFIG_SF_DEFAULT_SPEED 0
#define CONFIG_SF_DEFAULT_MODE 0
#define CONFIG_SYS_MAX_FLASH_BANKS 0
#define CONFIG_CMD_SF
#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
#ifdef CONFIG_ENV_SIZE
#undef CONFIG_ENV_SIZE
#endif
#ifdef CONFIG_ENV_SECT_SIZE
#undef CONFIG_ENV_SECT_SIZE
#endif
#ifdef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_OFFSET
#endif
#define CONFIG_ENV_SIZE            0x1000
#define CONFIG_ENV_SECT_SIZE       0x1000
#define CONFIG_ENV_OFFSET          ss_sf_env_offset
#ifdef CONFIG_SYS_REDUNDAND_ENVIRONMENT
#ifdef CONFIG_ENV_OFFSET_REDUND
#undef CONFIG_ENV_OFFSET_REDUND
#endif
#define CONFIG_ENV_OFFSET_REDUND   ss_sf_env_redund_offset
#endif
#endif
#endif /* CONFIG_SSTAR_NOR */


#ifdef CONFIG_MS_EMMC
    #ifdef CONFIG_MS_SAVE_ENV_IN_EMMC
        #define CONFIG_ENV_IS_IN_MMC       1
        #define CONFIG_SYS_MMC_ENV_DEV     0
        #define CONFIG_MS_EMMC_DEV_INDEX   1
        #define CONFIG_EMMC_PARTITION
        #define CONFIG_UNLZO_DST_ADDR      0x24000000
        #define CONFIG_ENV_SIZE            0x1000	/* Total Size of Environment Sector */
        /* bottom 4KB of available space in Uboot */
        /* 0x40000 reserved for UBoot, 0x40000 maximum storage size of uboot */
        #define CONFIG_ENV_OFFSET          0x4F000
    #else
        #define CONFIG_MS_EMMC_DEV_INDEX   0
        #define CONFIG_EMMC_PARTITION
    #endif
#endif /* CONFIG_MS_EMMC */

/*
 * USB configuration
 */
#ifdef CONFIG_MS_USB
#if !defined(CONFIG_USB)
    #define     CONFIG_USB
#endif
#define     CONFIG_CMD_USB
#if !defined(CONFIG_USB_STORAGE)
    #define     CONFIG_USB_STORAGE
#endif

#ifdef CONFIG_MS_ENABLE_USB_LAN_MODULE
#define ENABLE_USB_LAN_MODULE
#define CONFIG_CMD_NET
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#define CONFIG_BOOTP_GATEWAY
#endif

#endif



/*following config should be considered to be removed after confirmed*/

/* #define CONFIG_SKIP_RELOCATE_UBOOT dropped after uboot 201012 */
/*
#undef CONFIG_USE_IRQ	// we don't need IRQ/FIQ stuff
*/
/*#define KERNEL_IMAGE_SIZE       0x1000000	// 10M  kernel image size */

/*  move to cedric_defconfig
 *
 * 	#define CONFIG_MS_SDMMC     1
 *	#define CONFIG_MS_EMMC      1
 *  #define CONFIG_MS_NAND      1
 *
 */

 /* Ethernet configuration */
#define CONFIG_MINIUBOOT
#ifdef CONFIG_MS_EMAC
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_TFTP_PORT
#endif

#define CONFIG_SYS_BOOTM_LEN    0x2000000

#define CONFIG_XZ

#define XZ_DICT_ADDR            0x22000000  /*used for XZ decompress*/
#define XZ_DICT_LENGTH          0x01000000

#define PHY_ANEG_TIMEOUT	    8000

/*
#define ENABLE_DOUBLE_SYSTEM_CHECK  1
*/

/* SENSOR */
#define CONFIG_MS_SRCFG

#if defined(CONFIG_USB_GADGET)
    #if defined(CONFIG_USBDOWNLOAD_GADGET)
        #define CONFIG_G_DNL_VENDOR_NUM         0x1D6B
        #define CONFIG_G_DNL_PRODUCT_NUM        0x0101
        #define CONFIG_G_DNL_MANUFACTURER       "SigmaStar"
        #ifndef CONFIG_USB_GADGET_VBUS_DRAW
        #define CONFIG_USB_GADGET_VBUS_DRAW     2
        #endif

        #if defined(CONFIG_CMD_FASTBOOT)
            #define CONFIG_USB_FASTBOOT_BUF_ADDR    (BOOT_PARAMS + 0x1000000)
            #define CONFIG_USB_FASTBOOT_BUF_SIZE    0x500000
        #endif
    #endif
    #if defined(CONFIG_CMD_SSTAR_UFU)
        #define CONFIG_MD5
    #endif
    #if !defined(CONFIG_USB_GADGET_DUALSPEED)
        #define CONFIG_USB_GADGET_DUALSPEED
    #endif
#endif

#ifndef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE         0x00020000	/* Total Size of Environment Sector */
#endif

#define RSA_SIG_LEN     256
#define SECURITYBOOT_INC_HEADER

#endif	/* __INFINITY7_H */
