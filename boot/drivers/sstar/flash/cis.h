/*
 * cis.h- Sigmastar
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
#ifndef _CIS_H_
#define _CIS_H_

#define CIS_PAGE_SIZE      0x800
#define CIS_PAGE_SIZE_MASK 0x7FF
#define CIS_MAPPING_CNT    6

#define CIS_HEADER_SIZE     0x4
#define CIS_ROM_HEADER_SIZE 0x04
// SPINAND DEFINE
#define FLASH_PAGES_PER_BLOCK_DEFAULT (64)
#define FLASH_BLOCK_MASK_DEFAULT      (FLASH_PAGES_PER_BLOCK_DEFAULT - 1)
#define FLASH_PAGE_SIZE_MASK_DEFAULT  (0x7FF)
#define FLASH_SNI_BLOCKS_COUNT        10
#define CIS_EXT_SNI_HEADER_SIZE       (0x10)
#define CIS_EXT_SNI_RESERVED_SIZE     (0x1BC)
// SPINOR DEFINE
#define CIS_IPL_OFFSET          (0x00)
#define CIS_SEARCH_END          (0x40000)
#define CIS_NRI_SIZE            (0x300)
#define CIS_SNI_SIZE            (0x300)
#define CIS_ROM_SIZE            (0x100)
#define CIS_AVL_OFFSET_POSITION 0x2000
// SPINAND SNI

typedef signed char        s8;
typedef unsigned char      u8;
typedef signed short       s16;
typedef unsigned short     u16;
typedef signed int         s32;
typedef unsigned int       u32;
typedef signed long long   s64;
typedef unsigned long long u64;

enum cis_storage_type
{
    CIS_NAND,
    CIS_NOR,
    CIS_DRAM
};

typedef struct
{
    u8  command;
    u32 address;
    u8  address_bytes;
    u8  dummy;
    u16 data_bytes;
    u16 value;
} flash_cmd_set_t;

typedef enum
{
    SPINAND_OTP_AVAIL                   = 0x0001,
    SPINAND_ALL_LOCK                    = 0x0002,
    SPINAND_EXT_ECC                     = 0x0004,
    SPINAND_ECC_RESERVED_NONE_CORRECTED = 0x0008,
    SPINAND_ECC_RESERVED_CORRECTED      = 0x0010,
    SPINAND_NEED_QE                     = 0x0020,
    SPINAND_MULTI_DIES                  = 0x0040,
    SPINAND_VOLATILE                    = 0x0080,
    SPINAND_PLANE_SELECT                = 0x0100,
    SPINAND_CONFIG_READY                = 0x8000
} spinand_func_flag_t;

typedef enum
{
    SPINAND_NO_CR = 0,
    SPINAND_NONE_BUF_MODE,
    SPINAND_BUF_MODE,
} spinand_cr_type_t;

typedef enum
{
    SPINAND_CR_NONE            = 0x00,
    SPINAND_CR_NEXT_STATUS     = 0x01,
    SPINAND_CR_BUSY_AFTER_READ = 0x02,
    SPINAND_CR_BUSY_AFTER_NEXT = 0x04,
    SPINAND_CR_END_WITH_REST   = 0x08,
    SPINAND_CR_BLOCK_WITH_LAST = 0x10,
} spinand_cr_check_flag_t;

typedef enum
{
    SPINAND_THRESHOLD               = 0x01,
    SPINAND_BITFLIP                 = 0x02,
    SPINAND_SOCECC                  = 0x04,
    SPINAND_ENABLE_UBI_BBM          = 0x40,
    SPINAND_RESERVED_NONE_CORRECTED = 0x80,
} spinand_ecc_type_t;

typedef enum
{
    SPINAND_FLAG_ACTIVE     = 0x00000001,
    SPINAND_FLAG_QE         = 0x00000002,
    SPINAND_FLAG_PADDRIVING = 0x00000004,
    SPINAND_FLAG_PHASE      = 0x00000008,
} spinand_flag_t;

enum SPINOR_FLAG
{
    SPINOR_FLAG_ACTIVE     = 0x00000001,
    SPINOR_FLAG_QE         = 0x00000002,
    SPINOR_FLAG_PADDRIVING = 0x00000004,
    SPINOR_FLAG_PHASE      = 0x00000008,
};

typedef enum
{
    SPINAND_READ = 0x0,
    SPINAND_PROGRAM,
    SPINAND_RANDOM,
} spinand_access_t;

typedef struct
{
    flash_cmd_set_t srp0;
    flash_cmd_set_t srp1;
} spinand_srp_t;

typedef struct
{
    flash_cmd_set_t complement;
    flash_cmd_set_t topBottom;
    flash_cmd_set_t blocks;
} spinand_memory_protect_t;

typedef struct
{
    spinand_memory_protect_t blockStatus;
    spinand_srp_t            srp;
} spinand_protect_t;

typedef struct
{
    flash_cmd_set_t load;
    flash_cmd_set_t none_buf_mode_code;
} spinand_none_buf_t;

typedef struct
{
    flash_cmd_set_t next_page;
    flash_cmd_set_t last_page;
    flash_cmd_set_t check_busy;
    u8              check_flag; // reference spinand_cr_check_flag_t
} spinand_buf_mode_t;

typedef struct
{
    u8 cr_type; // reference spinand_cr_type_t
    union
    {
        spinand_none_buf_t none_buf_mode;
        spinand_buf_mode_t buf_mode;
    } cr_profile;
} spinand_cr_mode_t;

typedef struct
{
    flash_cmd_set_t   qe_status;
    flash_cmd_set_t   read;
    flash_cmd_set_t   program;
    flash_cmd_set_t   random;
    spinand_cr_mode_t cr_mode;
} spinand_access_config_t;

typedef struct
{
    u32             die_size;
    flash_cmd_set_t die_code;
} spinand_die_t;

typedef struct
{
    flash_cmd_set_t otp_lock;
    flash_cmd_set_t otp_enabled;
} spinand_otp_t;

typedef struct
{
    flash_cmd_set_t   ecc_enabled;
    spinand_otp_t     otp;
    spinand_die_t     die_config;
    spinand_protect_t protect_status;
} spinand_ext_config_t;

typedef struct
{
    u32 start;
    u32 length;
} spinand_otp_info;

typedef struct
{
    u8               otp_en;
    u8               reserved[27];
    spinand_otp_info factory;
    spinand_otp_info user;
    flash_cmd_set_t  otpread;
    flash_cmd_set_t  otpprogram;
} spinand_otp_config_t;

typedef struct
{
    u8              ecc_en;
    u8              ecc_type; // bit0 : threshold  bit1 : bitflip_table  bit2 : soc_ecc  bit7: reserved_none_corrected
    u8              ecc_status_mask;
    u8              ecc_not_correct_status;
    u8              ecc_reserved;
    u8              bitflip_threshold;
    u8              ecc_mode; // when soc ecc
    u8              reserved[31];
    flash_cmd_set_t ecc_status;
    flash_cmd_set_t threshold;
} spinand_ecc_config_t;

typedef struct
{
    u8 wcommand;
    u8 wdummy;
    u8 address1;
    u8 value1;
    u8 address2;
    u8 value2;
} flash_reg_init_set_t;

typedef struct
{
    u16                     flags; // reference spinand_func_flag_t
    u32                     max_wait_time;
    spinand_ext_config_t    ext_config;
    spinand_access_config_t access;
    spinand_ecc_config_t    ecc_config;
    flash_cmd_set_t         readid;
    flash_reg_init_set_t    init_set;
    u8                      reserved[2];
} spinand_ext_profile_t;

typedef struct
{
    spinand_ext_profile_t profile;
    u8                    reserved[24];
} spinand_ext_info_t;

typedef struct
{
    u8  id_byte_cnt;
    u8  id[15];
    u16 spare_byte_cnt;
    u16 page_byte_cnt;
    u16 blk_page_cnt;
    u16 blk_cnt;
    u16 sector_byte_cnt;
    u8  plane_cnt;
    u8  wrap_config;
    u8  riu_read;
    u8  max_clk;
    u8  ubootpba;
    u8  bl0pba;
    u8  bl1pba;
    u8  blpinb;
    u8  bakcnt;
    u8  bakofs;
    u8  hash_pba[3]; // NO USED
    u8  boot_id_loc;
    u8  have_phase;
    u8  phase;
    u8  reserved[22]; // just for aligning to 64bytes + magic[16] = 80bytes
} spinand_info_t;

typedef struct
{
    /*-------unused------------*/
    spinand_otp_config_t otp_config;
    /*-------------------------*/
} spinand_ext_configuration_t;

typedef struct
{
    u8                          magic[4];
    u32                         crc32;
    spinand_info_t              spinand_info;
    spinand_ext_info_t          spinand_ext_info;
    u8                          vender_name[16];
    u8                          partnumber[16];
    spinand_ext_configuration_t spinand_ext_configuration;
} spinand_sni_t;

/*
|-----------------------|
| spinand_basic_info_t  |
|-----------------------|
| spinand_info_t        |
|-----------------------|
| spinand_ext_info_t    |
|-----------------------|
*/

typedef struct
{
    u8  command;
    u8  address_bytes;
    u16 address;
    u8  dummy;
    u8  reserved[3];
    /* bit[31:24] 0:str mode 1:dtr mode
     * bit[23:0] 1-1-1 1-1-4 1-4-4 .....
     * bit[23:16] = command line - 1
     * bit[15:8] = address line - 1
     * bit[7:0] = data line - 1
     */
    u32 mode;
} spinand_read_set_t;

typedef struct
{
    u8 command;
    u8 r_command;
    u8 address;
    u8 value;
} spinand_reg_set_t;

typedef struct
{
    u8 clk;
    u8 dma;
    u8 phase;
    u8 polority;
    u8 cycles_delay;
    /* pad_driving[0] CZ
     * pad_driving[1] CK
     * pad_driving[2] DI
     * pad_driving[3] DO
     * pad_driving[4] WPZ
     * pad_driving[5] HLD
     */
    u8 pad_driving[6];
    u8 reserved[21];
} spinand_ctrl_cfg_t;

typedef struct
{
    u8                 id_bytes;
    u8                 id[7];
    u16                spare_bytes;
    u16                page_bytes;
    u16                sector_bytes;
    u16                block_page_cnt;
    u16                block_cnt;
    u8                 plane_cnt;
    u8                 u8_BL0PBA;
    u8                 u8_BL1PBA;
    u8                 u8_BLPINB;
    u8                 u8_BAKCNT;
    u8                 u8_BAKOFS;
    u8                 reserved[44];
    spinand_read_set_t read_set;
    spinand_reg_set_t  qe;
} spinand_basic_cfg_t;

typedef struct
{
    u8                  magic[4];
    u32                 crc32;
    u32                 flag;
    spinand_ctrl_cfg_t  ctrl_cfg;
    spinand_basic_cfg_t basic_cfg;
} spinand_basic_info_t;

// SPINOR SNI
typedef struct
{
    u8              need_qe;
    flash_cmd_set_t r_quad_enabled;
    flash_cmd_set_t w_quad_enabled;
} spinor_quad_cfg_t;

typedef struct
{
    flash_cmd_set_t srp0;
    flash_cmd_set_t srp1;
} spinor_srp_t;

typedef struct
{
    flash_cmd_set_t complement;
    flash_cmd_set_t topBottom;
    flash_cmd_set_t blocks;
} spinor_memory_protect_t;

typedef struct
{
    spinor_memory_protect_t block_status;
    spinor_srp_t            srp;
} spinor_protect_status_t;

typedef struct
{
    u8                      id_byte_cnt;
    u8                      id[15];
    u8                      max_clk;
    u16                     page_byte_cnt;
    u16                     sector_byte_cnt;
    u8                      have_phase;
    u8                      phase;
    u32                     blk_bytes_cnt;
    u32                     capacity;
    u32                     max_wait_time;
    u8                      reserved[4];
    u32                     extsni;
    spinor_protect_status_t r_protect_status;
    spinor_protect_status_t w_protect_status;
    spinor_quad_cfg_t       qe;
    flash_cmd_set_t         read_data;
    flash_cmd_set_t         program;
} spinor_info_t;

typedef struct
{
    /*-------unused------------*/
    u8            reserved[4];
    spinor_info_t spinor_info;
    /*-------------------------*/
} spinor_ext_profile_t;

typedef struct
{
    u8                   magic[4];
    u32                  crc32;
    spinor_info_t        spinor_info;
    u8                   vender_name[12];
    u8                   partnumber[16];
    spinor_ext_profile_t spinor_ext_profile;
} spinor_nri_t;

typedef struct
{
    u8  command;
    u8  data_bytes;
    u8  r_command[4];
    u8  reserved[2];
    u32 value;
} spinor_reg_set_t;

typedef struct
{
    u8 command;
    u8 address_bytes;
    u8 dummy;
    u8 reserved[1];
    /* bit[31:24] 0:str mode 1:dtr mode
     * bit[23:0] 1-1-1 1-1-4 1-4-4 .....
     * bit[23:16] = command line - 1
     * bit[15:8] = address line - 1
     * bit[7:0] = data line - 1
     */
    u32 mode;
} spinor_read_set_t;

typedef struct
{
    u8 clk;
    u8 dma;
    u8 phase;
    u8 polority;
    u8 cycles_delay;
    /* pad_driving[0] CZ
     * pad_driving[1] CK
     * pad_driving[2] DI
     * pad_driving[3] DO
     * pad_driving[4] WPZ
     * pad_driving[5] HLD
     */
    u8 pad_driving[6];
    u8 reserved[21];
} spinor_ctrl_cfg_t;

typedef struct
{
    spinor_read_set_t read_set;
    spinor_reg_set_t  qe;
    u8                reserved[32];
} spinor_basic_cfg_t;

typedef struct
{
    u8                 magic[4];
    u32                crc32;
    u32                flag;
    spinor_ctrl_cfg_t  ctrl_cfg;
    spinor_basic_cfg_t basic_cfg;
} spinor_basic_info_t;

struct parts_info
{
    u8 part_name[16]; // The partition's name
    u8 trunk;         // A distinguishing number for partitions with the same name,e.g. IPL0,IPL1,IPL2
    u8 backup_trunk;  // The backup_partition's index
    u8 active;        // Mark the partition is active or not
    u8 group;         // Reserved function, e.g. put IPL0,IPL_CUST1,UBOOT0 into the same group,then there status are all
                      // active
    u32 offset;       // The partition's start address(byte)
    u32 size;         // The partition's size
    u8  engine;
    u8  cs_select;
    u16 reserved;
};

struct parts_tbl
{
    u8  magic[16]; //"SSTARSEMICIS0001"
    u32 checksum;
    u32 size;
    // parts_info_t *pst_partsInfo;
};

#if defined(CONFIG_SSTAR_NAND)
u8 sstar_cis_get_sni_from_dram(void *handle, u8 *sni_list);
u8 sstar_cis_get_sni(void *handle);
#endif

#if defined(CONFIG_SSTAR_NOR)
u8 sstar_cis_get_nri_from_dram(void *handle, u8 *nri_list);
u8 sstar_cis_get_nri(void *handle);
#endif

struct parts_tbl *sstar_cis_get_pni(void);

u8   sstar_cis_update_sni(u8 *sni_buf);
u8   sstar_cis_update_nri(u8 *nri_buf);
u8   sstar_cis_update_pni(u8 *pni_buf);
u8   sstar_cis_init(u8 *cis_buffer, u8 cnt, u8 *fcie_buf, u8 fcie_buf_path, u8 load_cis);
void sstar_cis_deinit(void);

#endif /* _CIS_H_ */
