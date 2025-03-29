#
# Copyright (c) [2019~2020] SigmaStar Technology.
#
#
# This software is licensed under the terms of the GNU General Public
# License version 2, as published by the Free Software Foundation, and
# may be copied, distributed, and modified under those terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License version 2 for more details.
#

ccflags-y += -I$(srctree)/drivers/sstar/rpmb
obj-y += sstar_rpmb.o

###################################################
# Build rpmb_key.o form CONFIG_RPMB_KEY_FILE
# Notice:
# This method refers to the generation of avb_pubkey.o in common/Makefile.
obj-y += rpmb_key.o

ld_for_rpmbkey := $(LD)
ifneq ($(findstring arm-,$(LD)),)
    ifeq ($(shell $(LD) -v | grep "GNU gold" 2> /dev/null),)
        ld_for_rpmbkey := arm-linux-androideabi-ld.gold
    endif
endif
ifneq ($(findstring ld.lld,$(LD)),)
    ifdef CONFIG_ARM64
        ld_for_rpmbkey += -maarch64elf
    else ifdef CONFIG_ARM
        ld_for_rpmbkey += -marmelf
    endif
endif

$(obj)/rpmb_key.o: PRIVATE_LD := $(ld_for_rpmbkey)
ld_for_rpmbkey :=
$(obj)/rpmb_key.o: $(srctree)/$(subst $(quote),,$(CONFIG_RPMB_KEY_FILE))
	cp $< $(obj)/rpmb_key
	$(PRIVATE_LD) $(KBUILD_LDFLAGS) -r -b binary $(obj)/rpmb_key -o $@
	rm $(obj)/rpmb_key
###################################################
