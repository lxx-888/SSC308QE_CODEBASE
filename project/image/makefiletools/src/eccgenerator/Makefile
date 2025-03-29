# SigmaStar trade secret
# Copyright (c) [2019~2020] SigmaStar Technology.
# All rights reserved.
#
# Unless otherwise stipulated in writing, any and all information contained
# herein regardless in any format shall remain the sole proprietary of
# SigmaStar and be kept in strict confidence
# (SigmaStar Confidential Information) by the recipient.
# Any unauthorized act including without limitation unauthorized disclosure,
# copying, use, reproduction, sale, distribution, modification, disassembling,
# reverse engineering and compiling of the contents of SigmaStar Confidential
# Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
# rights to any and all damages, losses, costs and expenses resulting therefrom.
#
SRCS_DIR    = $(CURDIR)/src
APP_SRCS	= $(SRCS_DIR)/cisbl_gen.c $(SRCS_DIR)/ECC/rs_enc_fcie3_new.c $(SRCS_DIR)/ECC/ECC.c \
              $(SRCS_DIR)/CIS.c $(SRCS_DIR)/img_phy.c $(SRCS_DIR)/img_romsim.c

APP_OBJS	= $(APP_SRCS:.c=.o)

SRCS		= $(APP_SRCS)
OBJS		= $(APP_OBJS)

TOPDIR		:= $(shell cd ../; pwd)
MAKEFILE	= Makefile

#------------------------------------------------------------------------------
#   Make utilities
#------------------------------------------------------------------------------

MAKE	= make
LD		= ld
AR		= /usr/bin/ar
RANLIB	= /usr/bin/ranlib
RM		= /bin/rm

#------------------------------------------------------------------------------
#   Make options
#------------------------------------------------------------------------------

CFLAGS		= -Iinc
LFLAGS		= 
SRCS_LIB    = $(CURDIR)/lib

# lib from algorithm team, can NOT link together
LIBS		= $(SRCS_LIB)/libBCH_bench.a
#LIBS		= $(SRCS_LIB)/libBCH_bench_fcie4.a $(SRCS_LIB)/libBCH_bench_fcie3.a  

#------------------------------------------------------------------------------
#   Make rulesets
#------------------------------------------------------------------------------

#CC = arm_v6_vfp_le-gcc 
#CROSS_COMPILE=arm-v7a8-linux-gnueabi-
#CROSS_COMPILE=arm-none-linux-gnueabi-
#CROSS_COMPILE=mips-linux-gnu-
CROSS_COMPILE=
CC = $(CROSS_COMPILE)gcc -msoft-float



all: ecc_util

ecc_util: $(OBJS)
	$(CC) -o $@ $(LFLAGS) $(OBJS) $(LIBS)  -static
	

$(APP_OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $*.c -o $*.o

clean:
	-$(RM) -f $(OBJS) ecc_util
	
# DO NOT DELETE

