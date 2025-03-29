/*
* ipl.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#ifndef _IPL_H_
#define _IPL_H_

#define IPL_HEADER_CHAR     (0x4C5049)
#define IPL__HEADER_CHAR    (0x5F4C5049)
#define IPLC_HEADER_CHAR    (0x434C5049)
#define IPLK_HEADER_CHAR    (0x4E4C5049)

#define IPL_IMAGE_MAGIC_OFFSET                  4
#define IPL_IMAGE_SIZE_OFFSET                   8
#define IPL_IMAGE_AUTH_OFFSET                   10
#define IPL_IMAGE_CID_OFFSET                    11
#define IPL_IMAGE_CHECKSUM_OFFSET               12
#define IPL_IMAGE_HEADER_VER_OFFSET             14
#define IPL_IMAGE_DATA_OFFSET                   16
#define IPL_IMAGE_HEADER_VER_OFFSET             14
#define IPL_CUST_IMAGE_KEYN_OFFSET              16
#define IPL_CUST_IMAGE_KEYAES_OFFSET            18

#define image_get_ipl_data(a)                   (U32) (a + IPL_IMAGE_DATA_OFFSET)
#define image_get_ipl_magic(a)                  (*((volatile U32*) (a + IPL_IMAGE_MAGIC_OFFSET)))
//#define image_get_ipl_size(a)                   (*((volatile U16*) (a + IPL_IMAGE_SIZE_OFFSET)))
#define image_get_ipl_auth(a)                   (*((volatile U8*) (a + IPL_IMAGE_AUTH_OFFSET)))
#define image_get_ipl_cid(a)                    (*((volatile U8*) (a + IPL_IMAGE_CID_OFFSET)))
#define image_get_ipl_checksum(a)               (*((volatile U32*) (a + IPL_IMAGE_CHECKSUM_OFFSET)))
#define image_get_ipl_header_ver(a)             (*((volatile U8*) (a + IPL_IMAGE_HEADER_VER_OFFSET)))

#define image_get_ipl_cust_keyn(a)              (*((volatile U16*) (a + IPL_CUST_IMAGE_KEYN_OFFSET)))
#define image_get_ipl_cust_keyaes(a)            (*((volatile U16*) (a + IPL_CUST_IMAGE_KEYAES_OFFSET)))
#define image_get_ipl_cust_aes(a)               ((image_get_ipl_cust_keyaes(a)==0x1)?(0):(image_get_ipl_cust_keyaes(a)))
#define image_get_ipl_cust_keyn_data(a)         (U32) (a + image_get_ipl_size(a) - image_get_ipl_cust_aes(a) - image_get_ipl_cust_keyn(a))
#define image_get_ipl_cust_keyaes_data(a)       (U32) (image_get_ipl_cust_keyn_data(a) + image_get_ipl_cust_keyn(a))

#define image_get_uboot_dataCRC(a)              (*((volatile U32*) (a + 0x18)))

static inline U32 image_get_ipl_size(U32 a)
{
    if (image_get_ipl_header_ver(a) >= 0x05)
    {
        return ((*((volatile U16*)(a + IPL_IMAGE_SIZE_OFFSET)))<<4);
    }
    else
    {
        return (*((volatile U16*)(a + IPL_IMAGE_SIZE_OFFSET)));
    }
}

#endif /* _IPL_H_ */

