/*
 * sdio_test.c- Sigmastar
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
/***************************************************************************************************************
 *
 * FileName sdio_test.c
 *     @author jeremy.wang (2012/01/10)
 * Desc:
 *     This layer between Linux SD Driver layer and IP Hal layer.
 *     (1) The goal is we don't need to change any Linux SD Driver code, but we can handle here.
 *     (2) You could define Function/Ver option for using, but don't add Project option here.
 *     (3) You could use function option by Project option, but please add to ms_sdmmc.h
 *
 ***************************************************************************************************************/

#if 1 //
#include "sdio.h"
#include "sd.h"
#endif

int Sdio_test_irq_callback(U8_T u8Slot)
{
#if 1 // test
    static U32_T nC = 0;

    nC++;

    // if ( ( nC % 100 )==0 )
    {
        CamOsPrintf("---------------CB INT !!! nC = %d\r\n", nC);
    }
#endif

    //
    sdio_irq_enable(u8Slot, TRUE);

    return 0;
}

#if 1 //

void SdioTest(U8_T u8Slot)
{
#if 1
    // unsigned int test_loop = 0, test_temp = 0, i = 0;
    unsigned int test_loop = 0, i = 0;

    unsigned short ret;
    U8_T *         r_buf, *w_buf;
    U8_T           func;
    U32_T          addr;
    U32_T          count;
    U8_T           op_code;
    // u32 blk_size;
#endif
    CamOsPrintf("****** SDIO Test ******\r\n");

    //    sdio_init();

    sdio_set_irq_callback(u8Slot, Sdio_test_irq_callback);

    sdio_irq_enable(u8Slot, TRUE);

    sdio_card_init(u8Slot);

#if 0
    // CMD52
    CamOsMsSleep(2);
    addr = 6;
    ret = sdio_read_byte(func, addr, r_buf);
	if (ret)
    {
        CamOsPrintf("- sdio_write_byte ERROR\r\n");
	}

    //
    sdio_card_reset();
#endif

    CamOsPrintf("--- SDIO Init OK ---\r\n");

#if 1
    //
    r_buf = CamOsMemAlloc(16);
    w_buf = CamOsMemAlloc(16);

    while (1)
    {
        CamOsPrintf("--- SDIO Test Loop %d ---\r\n", test_loop);

        //
        CamOsMsSleep(500);

#if 1
        // OK
        CamOsMsSleep(2);

        func = 0;
        addr = 0x13;
        ret  = sdio_read_byte(u8Slot, func, addr, r_buf);

        if (ret)
        {
            CamOsPrintf("- sdio_read_byte ERROR\r\n");
        }
        else
        {
            CamOsPrintf("- sdio_read_byte [0] = 0x%x\r\n", r_buf[0]);
        }

        // OK
        CamOsMsSleep(2);
        w_buf[0] = r_buf[0];

        func = 0;
        addr = 0x13;
        ret  = sdio_write_byte(u8Slot, func, addr, w_buf[0]);
        if (ret)
        {
            CamOsPrintf("- sdio_write_byte ERROR\r\n");
        }
#endif

#if 1
        // OK
        CamOsMsSleep(2);
        func    = 1;
        addr    = 40; // 16
        op_code = 0;
        count   = 4;

        for (i = 0; i < count; i++)
        {
            w_buf[i] = test_loop + i;
        }

        ret = sdio_write_byte_multi(u8Slot, func, op_code, addr, count, w_buf);
        if (ret)
        {
            CamOsPrintf("- sdio_write_byte_multi ERROR\r\n");
        }
#endif

#if 0
        // NG
        CamOsMsSleep(2);
        func = 1;
        addr = (1<<15);
        op_code = 1;
        count = 4;

        ret = sdio_read_byte_multi(func, op_code, addr, count, r_buf);
    	if (ret)
        {
            CamOsPrintf("- sdio_read_byte_multi ERROR\r\n");
    	}
        else
        {
            CamOsPrintf("- sdio_read_byte_multi\r\n");
            for (i = 0; i < count; i++)
            {
                CamOsPrintf("- [%d] = 0x%x\r\n", i, r_buf[i]);
            }
        }
#endif

#if 0
        // EV_STS_WD_CERR
        CamOsMsSleep(2);
        func = 1;
        addr = (1<<15); //1;
        op_code = 1;
        count = 1;//12; // 12 2;
        blk_size = 512;

        for (i = 0; i < (count * blk_size); i++)
        {
            w_buf[i] = test_loop + i;
        }

        ret = sdio_write_block_multi(func, op_code, addr, count, blk_size, w_buf);
        {
            CamOsPrintf("- sdio_write_block_multi ERROR\r\n");
    	}
#endif

#if 0
        // EV_STS_MIE_TOUT
        CamOsMsSleep(2);
        func = 1;
        addr = 4+16+32; //1;
        op_code = 0;
        count = 1;
        blk_size = 512;

        ret = sdio_read_block_multi(func, op_code, addr, count, blk_size, r_buf);
    	if (ret)
        {
            CamOsPrintf("- sdio_read_block_multi ERROR\r\n");
    	}
        else
        {
            CamOsPrintf("- sdio_read_block_multi\r\n");
            for (i = 0; i < (count * blk_size); i++)
            {
                CamOsPrintf("- [%d] = 0x%x\r\n", i, r_buf[i]);
            }
        }
#endif

        //
        test_loop++;
    };

    //
    CamOsMemRelease(r_buf);
    CamOsMemRelease(w_buf);
#endif
    return;
}
#endif

void sdtest(U8_T u8Slot)
{
    U64_T sd_capt;
    U32_T sd_addr, err, blkcnt, loop   = 0, i;
    U8_T *r_buf, *w_buf, flag, compare = 0;

    //    sd_init();

    if (MS_SD_Card_Detect(u8Slot))
    {
        CamOsPrintf("card alive !\n");
        //        sd_card_init(u8_slot);
        sd_capt = sd_get_capacity(u8Slot);
        CamOsPrintf(" !\n");
        sd_capt = sd_capt;
        CamOsPrintf("SD card capacity: %lld %s -- %d %s \n", (unsigned long long)sd_capt, "B",
                    (int)(sd_capt / 1024 / 1024 / 1024), "GB");
    }

    while (loop < 10)
    {
        CamOsPrintf("rw test start! loop...%d\n", loop++);

        if (!MS_SD_Card_Detect(u8Slot))
        {
            CamOsScanf("%d", &flag);
            if (flag == 0)
                break;

            continue;
        }

        sd_addr = 0x233;
        r_buf   = CamOsMemAlloc(512);
        w_buf   = CamOsMemAlloc(512);

        for (i = 0; i < 512; i++)
        {
            r_buf[i] = i;
            w_buf[i] = ~i;
        }

        err = sd_write_block(u8Slot, sd_addr, w_buf);
        if (err)
            CamOsPrintf("write block error\n");

        err = sd_read_block(u8Slot, sd_addr, r_buf);
        if (err)
            CamOsPrintf("read block error\n");

        for (i = 0; i < 512; i++)
        {
            if (r_buf[i] != w_buf[i])
            {
                compare = 1;
                break;
            }
        }
        if (compare)
        {
            compare = 0;
            CamOsPrintf("signal block r/w test fail \n");
        }
        else
            CamOsPrintf("signal block r/w test pass \n");

        CamOsMemRelease(r_buf);
        CamOsMemRelease(w_buf);

        sd_addr = 0x122;
        blkcnt  = 2;

        r_buf = CamOsMemAlloc(512 * blkcnt);
        w_buf = CamOsMemAlloc(512 * blkcnt);

        for (i = 0; i < 512 * blkcnt; i++)
        {
            r_buf[i] = i;
            w_buf[i] = ~i;
        }

        err = sd_write_block_multi(u8Slot, sd_addr, blkcnt, w_buf);
        if (err)
            CamOsPrintf("write block error\n");

        err = sd_read_block_multi(u8Slot, sd_addr, blkcnt, r_buf);
        if (err)
            CamOsPrintf("read block error\n");

        for (i = 0; i < 512 * blkcnt; i++)
        {
            if (r_buf[i] != w_buf[i])
            {
                compare = 1;
                break;
            }
        }
        if (compare)
        {
            compare = 0;
            CamOsPrintf("multi block r/w test fail \n");
        }
        else
            CamOsPrintf("multi block r/w test pass \n");

        CamOsMemRelease(r_buf);
        CamOsMemRelease(w_buf);

        CamOsMsDelay(1000);
    }
}

#if 1
#include "initcall.h"
#include "sys_sys_isw_cli.h"
#include "cam_os_wrapper.h"

s32 sdmmc_test(CLI_t *pCli, char *p)
{
    s32   nParamCnt;
    U8_T  u8Slot;
    char *pEnd;

    nParamCnt = CliTokenCount(pCli);

    if (nParamCnt != 1)
    {
        CamOsPrintf(KERN_EMERG "Usage: uarttest [uart ID]\n");
        return eCLI_PARSE_INPUT_ERROR;
    }

    pCli->tokenLvl++;
    p      = CliTokenPop(pCli);
    u8Slot = strtoul(p, &pEnd, 10);

    sdtest(u8Slot);

    return eCLI_PARSE_OK;
}
SS_RTOS_CLI_CMD(sdtest, "Input sdmmc slot to test", "", sdmmc_test);
#endif
