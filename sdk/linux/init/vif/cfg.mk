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
DEP_MODULE:=common sys

# SIGMASTAR_INC_DIR+=$(PROJ_ROOT)/../sdk/impl/common/hal_common/
# SIGMASTAR_INC_DIR+=$(PROJ_ROOT)/../sdk/impl/vif/drv/inc/
# SIGMASTAR_INC_DIR+=$(PROJ_ROOT)/../sdk/impl/vif/drv/pub/
# SIGMASTAR_INC_DIR+=$(PROJ_ROOT)/../sdk/impl/vif/mi/
# SIGMASTAR_INC_DIR+=$(PROJ_ROOT)/../sdk/impl/vif/hal/pub/
# SIGMASTAR_INC_DIR+=$(PROJ_ROOT)/../sdk/impl/vif/hal/chip/$(CHIP)/

INIT_FILES+=vif_init.c
INIT_FILES+= vif_export.c
API_EXPORT_FILE+=vif_api_export.c
