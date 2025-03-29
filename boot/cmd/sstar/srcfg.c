/*
 * srcfg.c - Sigmastar
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

#include <common.h>
#include <command.h>
#include <malloc.h>
#include "drv_sensor_if.h"

int do_sensor_cfg(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    if (argc < 9)
    {
        printf("missing parameters\n");
        return CMD_RET_USAGE;
    }

    u32 SnrIdx     = simple_strtol(argv[1], 0, 10);
    u32 eBusType   = simple_strtol(argv[2], 0, 10);
    u32 SnrPadSel  = simple_strtol(argv[3], 0, 10);
    u32 MclkPadSel = simple_strtol(argv[4], 0, 10);
    u32 RstPadSel  = simple_strtol(argv[5], 0, 10);
    u32 PwnPadSel  = simple_strtol(argv[6], 0, 10);
    u32 MclkIdx    = simple_strtol(argv[7], 0, 10);
    u32 Pdwn       = simple_strtol(argv[8], 0, 10);
    u32 Rst        = simple_strtol(argv[9], 0, 10);

    DrvSensorRegInit();
    _DRV_SENSOR_IF_SetVifSetIoPad(SnrIdx, eBusType, (u8)SnrPadSel, (u8)MclkPadSel, (u8)RstPadSel, (u8)PwnPadSel);
    _DRV_SENSOR_IF_SetSensorMCLK(SnrIdx, MclkIdx);
    _DRV_SENSOR_IF_SetSensorPdwn(SnrIdx, Pdwn);
    _DRV_SENSOR_IF_SetSensorReset(SnrIdx, Rst);

    printf(
        "\n Sensor configuration, ID=%u, Bus=%u, MCLK=%u, "
        "PWD_PIN=%u, RST_PIN=%u \n\n",
        SnrIdx, eBusType, MclkIdx, Pdwn, Rst);
    return 0;
}

U_BOOT_CMD(srcfg, CONFIG_SYS_MAXARGS, 1, do_sensor_cfg, "sensor pin and mclk configuration.",
           "SensorId BusType LaneNum Mclk PwdPin RstPin\n"
           " -SensorPadId:      Select sensor ID 0~2\n"
           " -BusType:          Select sensor bus type 0:Parallel, 1:MIPI, 2:BT601, 3:BT656, 4:BT1120\n"
           " -Sensor Pad Sel:   Select sensor pad mode\n"
           " -Mclk Pad Sel:     Select mclk pad mode\n"
           " -Reset Pad Sel:    Select Reset pad mode\n"
           " -PowerDown Pad Sel:Select mclk pad mode\n"
           " -Mclk Idx:         0: 27m, 1: 72m, 2: 61.7m, 3: 54m, 4: 48m, 5: 43.2m, 6: 36m, 7: 24m, 8: 21.6m, 9: 12m, "
           "10: 5.4m\n"
           " -PwdPin:           sensor power down pin 0:low 1:high\n"
           " -RstPin:           sensor reset pin 0:low 1:high\n");
