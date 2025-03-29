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

SSTAR_CHIP_FILE := '.sstar_chip.txt'

# workaround: make -c [folder] clean
# additional $(MAKEFLAGS) with 'w' passed
# use $(filter-out w,$(MAKEFLAGS)) instead of $(MAKEFLAGS)

%: force
	@$(MAKE) -f Makefile $(filter-out w,$(MAKEFLAGS)) $@

%_defconfig: force
	@printf "Check Kconfigs for no newline at the end...\n"
	@if [ "$(shell which git)" != "" ];then for f in $$(git status --short 2>/dev/null | grep '^\(A .*\)\|^\( M.*\)' | cut -d' ' -f3 | grep 'Kconfig');do if [ ! -z $$(tail -c 1 $$f) ];then printf "  \033[31;40mERROR:\033[0m: No new line at end of [$$f]\n";fi;done;fi
	@echo "Extract CHIP NAME ($(firstword $(subst _, ,$@))) to $(SSTAR_CHIP_FILE)"
	@echo $(strip $(firstword $(subst _, ,$@))) > $(SSTAR_CHIP_FILE)
	@echo $@ >> $(SSTAR_CHIP_FILE)
	@$(MAKE) -f Makefile $(filter-out w,$(MAKEFLAGS)) $@

GNUmakefile: force
	@$(MAKE) -f Makefile $(filter-out w,$(MAKEFLAGS)) $(filter-out GNUmakefile,$@)

check_kconfig:
	@printf "Check Kconfigs for no newline at the end...\n"
	@for f in $$(find -name 'Kconfig');do if [ ! -z $$(tail -c 1 $$f) ];then printf "  \033[31;40mERROR:\033[0m No new line at end of [$$f]\n";fi;done

makefile:;

force:;
