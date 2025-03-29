.PHONY: verify interface misc driver osdk

ifeq ($(DUAL_OS),on)
    ifneq ($(CONFIG_ENABLE_POWER_SAVE_AOV),y)
        DUAL_OS_RELEASE:=1
        ifeq ("$(interface_fb)","enable")
            DUAL_OS_KERNEL_MODULE_LIST:=fb
        endif
    else
        DUAL_OS_RELEASE:=0 # build purelinux
    endif # CONFIG_ENABLE_POWER_SAVE_AOV
else
DUAL_OS_RELEASE:=0 # build purelinux
endif # DUAL_OS

ifeq ($(DUAL_OS_RELEASE), 1)
ifeq ($(findstring purertos,$(PRODUCT)), )
SOURCE_RELEASE:=rtos_release
SOURCE_CLEAN:=clean_rtos
else
SOURCE_RELEASE:=
SOURCE_CLEAN:=
endif
else # ifeq ($(DUAL_OS_RELEASE), 1)
ifneq ($(wildcard ../sdk/osdk),)
OSDK_RELEASE:= osdk_install
endif
ifeq ($(findstring purertos,$(PRODUCT)), )
BASE_RELEASE:=
SOURCE_RELEASE:= interface_install driver_install
MISC_RELEASE:= misc_install
DEMO_RELEASE:= verify_install
else
SOURCE_RELEASE:= purertos_release
OSDK_RELEASE:=
endif
BASE_CLEAN:=
SOURCE_CLEAN:=clean_all
DEMO_CLEAN:=
endif # DUAL_OS_RELEASE

ifneq ($(filter rtos,$(IPC_ROUTER)),)
source-release=$(MAKE) isw -f $(PROJ_ROOT)/rbuild/isw.mk PROJ_ROOT=$(PROJ_ROOT)
else
source-release=echo "release all done"
endif

sdk:
	mkdir -p $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/modules/$(KERNEL_VERSION)
ifneq ($(BASE_RELEASE),)
	@echo ------------------------------[$@:$(MAKE) $(BASE_RELEASE)]----------------------------------
	$(MAKE) $(BASE_RELEASE)
endif
ifneq ($(SOURCE_RELEASE),)
	@echo ------------------------------[$@:$(MAKE) $(SOURCE_RELEASE)]--------------------------------
	$(MAKE) $(SOURCE_RELEASE)
endif
ifneq ($(CODESIZE_STATISTICS),)
	@echo ------------------------------[$@:$(MAKE) CODESIZE_LIB]----------------------------------
	$(MAKE) CODESIZE_LIB
endif
ifneq ($(MISC_RELEASE),)
	@echo ------------------------------[$@:$(MAKE) $(MISC_RELEASE)]--------------------------------
	$(MAKE) $(MISC_RELEASE)
endif
ifneq ($(DEMO_RELEASE),)
	@echo ------------------------------[$@:$(MAKE) $(DEMO_RELEASE)]----------------------------------
	$(MAKE) $(DEMO_RELEASE)
endif
ifneq ($(OSDK_RELEASE),)
	@echo ------------------------------[$@:$(MAKE) $(OSDK_RELEASE)]----------------------------------
	$(MAKE) $(OSDK_RELEASE)
endif
	@echo ------------------------------[$@: finished!]-----------------------------------------------

$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config:
	$(MAKE) -C $(PROJ_ROOT) kbuild/$(KERNEL_VERSION)/.config

ifneq ($(wildcard ../sdk/verify),)
verify:
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify all

verify_install: verify
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify install

verify_with_depend: interface_install_with_depend
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify all
else
verify_install:
endif

ifneq ($(wildcard ../sdk/interface),)
interface_install: interface
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface install

interface_install_with_depend: interface_with_depend
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface install_with_depend

interface_with_depend:
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface all_with_depend

interface:
	@mkdir -p $(PROJ_ROOT)/image/codesize/$(CODESIZE_DEFCONFIG)/CODESIZE_TOTAL
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface all

clean_interface:$(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface distclean

libs:
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface libs
ifeq ($(DUAL_OS), on) #compile dualos module
	$(foreach m,$(DUAL_OS_KERNEL_MODULE_LIST),$(MAKE) -C $(PROJ_ROOT)/../sdk/interface $(m)_module)
endif

libs_install: libs
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface libs_install
ifeq ($(DUAL_OS), on) #install dualos module.
	$(foreach m,$(DUAL_OS_KERNEL_MODULE_LIST),$(MAKE) -C $(PROJ_ROOT)/../sdk/interface $(m)_module_install)
endif

else
interface_install:
clean_interface:
libs_install:
endif # $(wildcard ../sdk/interface)

ifneq ($(wildcard ../sdk/misc),)
misc: $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config
	$(MAKE) -C $(PROJ_ROOT)/../sdk/misc all

misc_install: misc
	$(MAKE) -C $(PROJ_ROOT)/../sdk/misc install
else
misc_install:
endif

ifneq ($(wildcard ../sdk/osdk),)
osdk: $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config
	$(MAKE) -C $(PROJ_ROOT)/../sdk/osdk all

osdk_install: osdk
	$(MAKE) -C $(PROJ_ROOT)/../sdk/osdk install
endif

driver: $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/.config
ifneq (${PRODUCT}, nvr)
	$(MAKE) -C $(PROJ_ROOT)/../sdk/driver all
endif

driver_install: driver
ifneq (${PRODUCT}, nvr)
	$(MAKE) -C $(PROJ_ROOT)/../sdk/driver install
endif

clean_sdk:$(SOURCE_CLEAN)

clean_rtos: clean_interface
ifneq ($(wildcard ../sdk/misc),)
	$(MAKE) -C $(PROJ_ROOT)/../sdk/misc clean
endif
ifneq ($(wildcard ../sdk/verify),)
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify clean
endif

clean_all: clean_interface
ifneq ($(wildcard ../sdk/misc),)
	$(MAKE) -C $(PROJ_ROOT)/../sdk/misc clean
endif
ifneq (${PRODUCT}, nvr)
	$(MAKE) -C $(PROJ_ROOT)/../sdk/driver clean
endif
ifneq ($(wildcard ../sdk/verify),)
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify clean
endif
ifneq ($(wildcard ../sdk/osdk),)
	$(MAKE) -C $(PROJ_ROOT)/../sdk/osdk clean
endif

rtos_release: libs_install
ifneq ($(wildcard ../sdk/verify),)
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify all
	$(MAKE) -C $(PROJ_ROOT)/../sdk/verify install
endif

purertos_release:
ifneq ($(wildcard ../sdk/interface),)
	$(MAKE) -C $(PROJ_ROOT)/../sdk/interface headers_install
endif

CODESIZE_LIB:
	@mkdir -p $(PROJ_ROOT)/image/codesize/$(CODESIZE_DEFCONFIG)/CODESIZE_LIB
	-cp $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/release/mi_libs/dynamic/*.so $(PROJ_ROOT)/image/codesize/$(CODESIZE_DEFCONFIG)/CODESIZE_LIB
	@echo $@ done!
