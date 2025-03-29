/*
 * ddr_ott.h - Sigmastar
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

#ifndef _DDR_OTT_H_
#define _DDR_OTT_H_

/*
 * custimized options
 */
#define DDRTRAIN_INFO_EMMC_BLKOFST 0x1E00

/*
 * mailbox definition
 */
#define REG_ADDR_DDR_INIT_MODE (REG_ADDR_BASE_MAILBOX + (0xB << 2))
#define SET_DDR_INIT_MODE(x)   (OUTREG16(REG_ADDR_DDR_INIT_MODE, (x)))
#define GET_DDR_INIT_MODE()    (INREG16(REG_ADDR_DDR_INIT_MODE))

#define FLAG_DDR_USE_DEFULT        0
#define FLAG_DDR_USE_TRAIN_DATA    1
#define FLAG_DDR_RUN_AUTOK         2
#define FLAG_DDR_INIT_MODE_UNKNOWN 0xFFFF

/*
 * flag definition
 */
#define FLAG_FORCE_AUTOK_ON  0x656e3d31 // 'en=1'
#define FLAG_FORCE_AUTOK_OFF 0x656e3d30 // 'en=0'

/*
 * training data definition
 */
#define DATA_HDR_MAGIC  0x4441545F // 'DAT_'
#define DATA_TAIL_MAGIC 0x5F544144 // '_TAD'

#define K_CODE_NUM         1                 // kcode
#define TX_KCODE_NUM       1                 // txkcode
#define KCODE_OFFSET_NUM   (0x97 - 0x94 + 1) // kcode offset
#define W_DQ_PHASE_NUM     (0x23 - 0x00 + 1) // w_dq
#define W_DQ_SKEW_NUM      4                 // dq skew
#define W_DQ_OEN_SKEW_NUM  4                 // dq oen skew
#define R_DQ_PHASE_NUM     (0x57 - 0x34 + 1) // r_dq
#define R_DQ_SW_PHASE_NUM  4                 // r_dq SW phase
#define DELAY_1T_NUM       36                //(0x05-0x00+1) delay_1t
#define TRIGGER_LVL_NUM    2                 // trigger lvl
#define VREF_DQ_NUM        1                 // vref_dq
#define R_DQSM_NUM         4                 // dqsm
#define W_DQS_SKEW_NUM     4
#define W_DQS_OEN_SKEW_NUM 4
#define W_DQS_PHASE_NUM    4
#define W_CMD_PHASE_NUM    28
#define DRV_CA_NUM         3
#define DRV_DQ_NUM         4

typedef struct train_flag
{
    uint32_t enable;
    uint32_t version;
    uint32_t reserved[125];
} __attribute__((packed)) train_flag_t;

typedef struct ott_flag
{
    uint32_t     chksum;
    train_flag_t flag;
} __attribute__((packed)) ott_flag_t;

typedef struct train_data
{
    uint32_t header;
    // common train data for ddr4/lpddr4
    uint16_t kcode;
    uint16_t tx_kcode;
    uint16_t ref_mr_osc;
    uint8_t  kcode_offset[KCODE_OFFSET_NUM];
    uint8_t  w_dq_phase[W_DQ_PHASE_NUM];
    uint8_t  w_dq_skew[W_DQ_SKEW_NUM];
    uint8_t  w_dq_oen_skew[W_DQ_OEN_SKEW_NUM];
    uint8_t  r_dq_phase[R_DQ_PHASE_NUM];
    uint8_t  delay_1t[DELAY_1T_NUM];
    uint8_t  trig_lvl[TRIGGER_LVL_NUM];
    uint8_t  vref_dq[VREF_DQ_NUM];
    uint8_t  wr_dq_win[W_DQ_PHASE_NUM * 2];
    uint8_t  verify_delay_1t[DELAY_1T_NUM * 2];
    uint8_t  verify_skew[W_DQ_SKEW_NUM * 2];
    uint8_t  rd_dq_win[R_DQ_SW_PHASE_NUM * 2];

    // soc_zq, LPDDR4X
    uint16_t odt_msb;
    uint16_t odt_lsb;
    uint16_t drvcap[DRV_CA_NUM];
    uint16_t drvcan[DRV_CA_NUM];
    uint16_t drv_dqp[DRV_DQ_NUM];
    uint16_t drv_dqn[DRV_DQ_NUM];

    // cbt, LPDDR4(X)
    uint8_t mr_vrefca;

    // w_ca, LPDDR4(X)
    uint8_t w_cs_skew;
    uint8_t w_cs1_skew;
    uint8_t w_clk_phase;
    uint8_t w_clk1_phase;
    uint8_t w_cs_phase;
    uint8_t w_cs1_phase;
    uint8_t w_cmd_skew;
    uint8_t w_cmd1_skew;
    uint8_t w_cmd_phase[W_CMD_PHASE_NUM];

    // w_lvl, LPDDR4(X)
    uint8_t w_dqs_skew[W_DQS_SKEW_NUM];
    uint8_t w_dqs_oen_skew[W_DQS_OEN_SKEW_NUM];
    uint8_t w_dqs_phase[W_DQS_PHASE_NUM];

    // dqsm, LPDDR4(X), LPDDR3
    uint8_t r_dqsm_delay[R_DQSM_NUM];
    uint8_t r_dqsm_skew[R_DQSM_NUM];
    uint8_t r_dqsm_phase[R_DQSM_NUM];

    uint8_t  reserved[118];
    uint32_t tail;
} __attribute__((packed)) train_data_t;

typedef struct ott_data
{
    uint32_t     chksum;
    train_data_t data;
} __attribute__((packed)) ott_data_t;

/*
 * public data struction
 */
#define MIU_CH_COUNT 1
typedef struct ddrtrain_info
{
    /* 512-byte flags */
    ott_flag_t flag;
    /* training data for two channels */
    ott_data_t data[MIU_CH_COUNT];
} __attribute__((packed)) ddrtrain_info_t;

#if !defined(__UBOOT__) && defined(ENABLE_DDROTT)
uint8_t ddrtrain_is_flag_valid(void);
uint8_t ddrtrain_is_data_valid(uint8_t ch);
uint8_t ddrtrain_is_forced_to_run(void);
void    ddrtrain_store_data(uint8_t is_ddr4, uint8_t miu);
void    ddrtrain_apply_data(uint8_t is_ddr4, uint8_t miu);
#define ddrtrain_set_ddr_init_mode(x) SET_DDR_INIT_MODE(x)
void ddrtrain_init(void);
#endif

#if defined(__UBOOT__)
ddrtrain_info_t *read_ddrtrain_info(void);
int              store_ddrtrain_info(ddrtrain_info_t *train_info);
#endif

#endif /* _DDR_OTT_H_ */
