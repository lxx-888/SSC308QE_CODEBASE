INC += $(MODULE_PATH)/../nlohmann
INC += $(MODULE_PATH)/../ss_console
INC += $(MODULE_PATH)/../ss_util
ifeq ($(interface_pcie), enable)
CODEDEFINE += -DPCIE_RPMSG_ROUTE
endif
