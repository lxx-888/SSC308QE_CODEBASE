/*
* sstar_android_image.h - Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: yi.huang <yi.huang@sigmastar.com.tw>
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

#ifndef _SSTAR_ANDROID_IMAGE_H_
#define _SSTAR_ANDROID_IMAGE_H_
struct boot_img_hdr_v2 {
	// Must be BOOT_MAGIC.
	uint8_t magic[ANDR_BOOT_MAGIC_SIZE];

	uint32_t kernel_size; /* size in bytes */
	uint32_t kernel_addr; /* physical load addr */

	uint32_t ramdisk_size; /* size in bytes */
	uint32_t ramdisk_addr; /* physical load addr */

	uint32_t second_size; /* size in bytes */
	uint32_t second_addr; /* physical load addr */

	uint32_t tags_addr; /* physical addr for kernel tags (if required) */
	uint32_t page_size; /* flash page size we assume */

	// Version of the boot image header.
	uint32_t header_version;

	// Operating system version and security patch level.
	// For version "A.B.C" and patch level "Y-M-D":
	//   (7 bits for each of A, B, C; 7 bits for (Y-2000), 4 bits for M)
	//   os_version = A[31:25] B[24:18] C[17:11] (Y-2000)[10:4] M[3:0]
	uint32_t os_version;

	uint8_t name[ANDR_BOOT_NAME_SIZE]; /* asciiz product name */

	uint8_t cmdline[ANDR_BOOT_ARGS_SIZE]; /* asciiz kernel commandline */

	uint32_t id[8]; /* timestamp / checksum / sha1 / etc */

	// Supplemental command line data; kept here to maintain
	// binary compatibility with older versions of mkbootimg.
	// Asciiz.
	uint8_t extra_cmdline[ANDR_BOOT_EXTRA_ARGS_SIZE];

	// Add by v1
	uint32_t recovery_dtbo_size;   /* size in bytes for recovery DTBO/ACPIO image */
	uint64_t recovery_dtbo_offset; /* offset to recovery dtbo/acpio in boot image */
	uint32_t header_size;

	// Add by v2
	uint32_t dtb_size; /* size in bytes for DTB image */
	uint64_t dtb_addr; /* physical load address for DTB image */
} __attribute__((packed));

typedef struct boot_img_hdr_v3 boot_img_hdr;

struct boot_img_hdr_v3 {
	// Must be BOOT_MAGIC.
	uint8_t magic[ANDR_BOOT_MAGIC_SIZE];

	uint32_t kernel_size; /* size in bytes */
	uint32_t ramdisk_size; /* size in bytes */

	// Operating system version and security patch level.
	// For version "A.B.C" and patch level "Y-M-D":
	//   (7 bits for each of A, B, C; 7 bits for (Y-2000), 4 bits for M)
	//   os_version = A[31:25] B[24:18] C[17:11] (Y-2000)[10:4] M[3:0]
	uint32_t os_version;

	uint32_t header_size;

	uint32_t reserved[4];

	// Version of the boot image header.
	uint32_t header_version;

	// Asciiz kernel commandline.
	uint8_t cmdline[ANDR_BOOT_ARGS_SIZE + ANDR_BOOT_EXTRA_ARGS_SIZE];
} __attribute__((packed));


struct vendor_boot_img_hdr_v3 {
	// Must be VENDOR_BOOT_MAGIC.
	uint8_t magic[VENDOR_BOOT_MAGIC_SIZE];

	// Version of the vendor boot image header.
	uint32_t header_version;

	uint32_t page_size; /* flash page size we assume */

	uint32_t kernel_addr; /* physical load addr */
	uint32_t ramdisk_addr; /* physical load addr */

	uint32_t vendor_ramdisk_size; /* size in bytes */

	uint8_t cmdline[VENDOR_BOOT_ARGS_SIZE]; /* asciiz kernel commandline */

	uint32_t tags_addr; /* physical addr for kernel tags (if required) */
	uint8_t name[VENDOR_BOOT_NAME_SIZE]; /* asciiz product name */

	uint32_t header_size;

	uint32_t dtb_size; /* size in bytes for DTB image */
	uint64_t dtb_addr; /* physical load address for DTB image */
} __attribute__((packed));

typedef struct vendor_boot_img_hdr_v3 vendor_boot_img_hdr;
#endif /* _SSTAR_ANDROID_IMAGE_H_ */

