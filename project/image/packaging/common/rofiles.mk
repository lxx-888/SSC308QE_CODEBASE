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
rofiles_$(PRODUCT):
rofiles: rofiles_$(PRODUCT)

rofiles_$(PRODUCT):
	@echo [================= $@ =================]
	mkdir -p $(rofiles$(RESOURCE));

	if [ "$(DLA_FIRMWARE_LIST)" != "" ]; then \
		cp -rf $(foreach n,$(DLA_FIRMWARE_LIST),$(PROJ_ROOT)/board/$(CHIP)/dla_file/$(n)) $(rofiles$(RESOURCE))/; \
	fi;

	if [ "$(IQ_API_LIST)" != "" ]; then \
		for f in $(IQ_API_LIST); do \
			$(call copy_if_exists,$(PROJ_ROOT)/board/$(CHIP)/iqfile/$$f,$(rofiles$(RESOURCE))) \
		done; \
	fi;

	cp -rf $(PROJ_ROOT)/board/rtos/ascii_8x16 $(rofiles$(RESOURCE))/ ;
	cp -rf $(PROJ_ROOT)/board/rtos/hanzi_16x16 $(rofiles$(RESOURCE))/ ;
	cp -rf $(PROJ_ROOT)/board/rtos/200X131.argb1555 $(rofiles$(RESOURCE))/200X131.argb ;
	cp -rf $(PROJ_ROOT)/board/rtos/200X133.bmp $(rofiles$(RESOURCE))/ ;

	$(call pack_apifiles,$(SENSOR_TYPE),0,$(rofiles$(RESOURCE)))
	$(call pack_apifiles,$(SENSOR_TYPE1),1,$(rofiles$(RESOURCE)))
	$(call pack_apifiles,$(SENSOR_TYPE2),2,$(rofiles$(RESOURCE)))
