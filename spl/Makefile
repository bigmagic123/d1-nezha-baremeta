# SPDX-License-Identifier: GPL-2.0+

#
# Makefile for sunxi bootloader
# wangwei@allwinnertech.com
#

TOPDIR=$(CURDIR)
SRCTREE=$(TOPDIR)

#Q: quit for compile
Q = @
ifeq ("$(origin V)", "command line")
VERBOSE=$(V)
endif
ifndef VERBOSE
VERBOSE=0
endif

ifeq ($(VERBOSE),1)
Q=
CMD_ECHO_SILENT := true
else
CMD_ECHO_SILENT := echo
endif

#P: platform: sun50iw3p1/sun8iw15p1 etc.
PLATFORM=NULL
ifeq ("$(origin p)", "command line")
PLATFORM=$(p)
endif


#M: compile module: nand mmc nor etc.
MODULE=NULL
ifeq ("$(origin m)", "command line")
MODULE=$(m)
endif

#ddr: compile module: dd3/lpddr3/ddr4/lpddr4
MODULE=NULL
ifeq ("$(origin ddr)", "command line")
DRAM_TYPE=$(ddr)
DRAM_TYPE_NAME=_$(DRAM_TYPE)
endif

#'@':no echo, '-':error ignore, '\#':comment execution command
CP=@-\#
ifeq ("$(origin C)", "command line")
CP=@-cp -v
endif

ifeq ("$(origin b)", "command line")
BOARD=$(b)
endif

#get arch
buildconfig = ../../../.buildconfig
ifeq ($(buildconfig), $(wildcard $(buildconfig)))
	LICHEE_BUSSINESS=$(shell cat $(buildconfig) | grep -w "LICHEE_BUSSINESS" | awk -F= '{printf $$2}')
	LICHEE_CHIP_CONFIG_DIR=$(shell cat $(buildconfig) | grep -w "LICHEE_CHIP_CONFIG_DIR" | awk -F= '{printf $$2}')
	LICHEE_IC=$(shell cat $(buildconfig) | grep -w "LICHEE_IC" | awk -F= '{printf $$2}')
	LICHEE_PLAT_OUT=$(shell cat $(buildconfig) | grep -w "LICHEE_PLAT_OUT" | awk -F= '{printf $$2}')
	LICHEE_BOARD=$(shell cat $(buildconfig) | grep -w "LICHEE_BOARD" | awk -F= '{printf $$2}')
	export LICHEE_BUSSINESS LICHEE_CHIP_CONFIG_DIR LICHEE_IC LICHEE_PLAT_OUT LICHEE_BOARD

endif

CPU ?= armv7

#check input param
MK_FILE_CHANGE := $(shell if [ x$(p) != x ]; then echo yes; else echo no; fi;)
#$(info MK_FILE_CHANGE=$(MK_FILE_CHANGE))
ifeq (x$(MK_FILE_CHANGE),xyes)
	FILE_EXIST=$(shell if [ -f $(TOPDIR)/board/$(p)/common.mk ]; then echo yes; else echo no; fi;)
ifeq (x$(FILE_EXIST),xno)
$(info ***);
$(info ***configure plat or module not exist, Please run some configurator);
$(info ***e.g. make p=sun50iw3p1; make boot0);
$(info ***e.g. make p=sun50iw3p1; make sboot);
$(info ***e.g. make p=sun50iw3p1; make fes);
$(info ***e.g. copy spl file to spi-pub run then following command);
$(info ***e.g. make p=sun50iw3p1 C=1; make fes C=1)
$(info ***e.g. make p=sun8iw15p1 ddr=ddr3|lpddr3|ddr4|lpddr4; make boot0);
$(info ***e.g. make p=sun8iw15p1 ddr=ddr3|lpddr3|ddr4|lpddr4; make sboot);
$(info ***e.g. make p=sun8iw15p1 ddr=ddr3|lpddr3|ddr4|lpddr4; make fes);
$(info ***);
$(error exit);
endif
$(shell cp -f $(TOPDIR)/board/$(p)/common.mk $(TOPDIR)/.module.common.mk)
BOARD_LIST=$(subst -,_,$(subst .,_,$(shell ls $(LICHEE_CHIP_CONFIG_DIR)/configs)))
TMP=$(subst -,_,$(subst .,_,$(LICHEE_BOARD)))
$(shell CNT=0; for TEMP_BOARD in ${BOARD_LIST} ;do CNT=`expr $${CNT} + 1`;echo CFG_$${TEMP_BOARD}=$${CNT} >> $(TOPDIR)/.module.common.mk;done)
$(shell sed -i '$$a\CFG_LICHEE_BOARD=CFG_$(TMP)' $(TOPDIR)/.module.common.mk)
$(shell sed -i '$$a\DRAM_TYPE_NAME=$(DRAM_TYPE_NAME)' $(TOPDIR)/.module.common.mk)
endif

#include config file
include $(TOPDIR)/mk/config.mk
include $(TOPDIR)/mk/checkconf.mk
sinclude $(TOPDIR)/.module.common.mk


ifeq ($(shell echo ${LICHEE_BOARD} |grep -o fpga ),fpga)
	CFG_FPGA_PLATFORM=y
	export CFG_FPGA_PLATFORM
endif


#CP_BOARD=$(filter $(BOARD), $(SUPPORT_BOARD))
#ifeq (x$(CP_BOARD), x)
#ifneq (x$(LICHEE_IC), x)
#CP_BOARD=$(LICHEE_IC)
#endif
#ifeq (x$(CP_BOARD), x)
#CP_BOARD=$(PLATFORM)
#endif
#endif

ifeq (x$(CPU), xriscv64)
#riscv
TOOLCHAIN_PATH:=riscv64-linux-x86_64-20200528
else
#arm
TOOLCHAIN_PATH:=gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi
endif

export Q SOC TOPDIR SRCTREE CMD_ECHO_SILENT MODULE PLATFORM DRAM_TYPE_NAME CP CPU CP_BOARD

#$(info platform=$(PLATFORM) module=$(MODULE))

define build-boot0-module-if-exist
	@if [ x$(NAND_EXIST) = xyes ]; then  \
	$(MAKE)  -C  $(SRCTREE)/nboot nand; fi
	@if [ x$(MMC_EXIST) = xyes ]; then  \
	$(MAKE)  -C  $(SRCTREE)/nboot mmc; fi
	@if [ x$(SPINOR_EXIST) = xyes ]; then  \
	$(MAKE)  -C  $(SRCTREE)/nboot spinor; fi
endef

define update-commit-info
	$(SRCTREE)/tools/generate_hash_header_file.sh > \
		$(SRCTREE)/include/commit_info.h.tmp
	$(call mv-if-changed,$(SRCTREE)/include/commit_info.h.tmp,$(SRCTREE)/include/commit_info.h)
	$(CP) $(TOPDIR)/include/commit_info.h $(TOPDIR)/../spl-pub/board/$(CP_BOARD)
endef

MAKEFLAGS +=  --no-print-directory

NAND_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/nand.mk ]; then echo yes; else echo no; fi;)
MMC_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/mmc.mk ]; then echo yes; else echo no; fi;)
SPINOR_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/spinor.mk ]; then echo yes; else echo no; fi;)
SBOOT_EXIST:=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/sboot.mk ]; then echo yes; else echo no; fi;)
ifeq (x$(MK_FILE_CHANGE),xyes)
#makefile is called to config platform, do nothing
all:
	@echo "platform set to $(PLATFORM)"
	@mkdir -p $(TOPDIR)/../spl-pub/board/$(CP_BOARD)
	$(CP) $(TOPDIR)/board/$(PLATFORM)/*.mk $(TOPDIR)/../spl-pub/board/$(CP_BOARD)
	$(CP) -rf $(TOPDIR)/include/*	$(TOPDIR)/../spl-pub/include/
	$(CP) -rf $(TOPDIR)/mk/*	$(TOPDIR)/../spl-pub/mk/
	$(CP) -rf $(TOPDIR)/arch/$(ARCH)/cpu/$(CPU)/*.lds $(TOPDIR)/arch/$(ARCH)/cpu/$(CPU)/*_entry.S  $(TOPDIR)/../spl-pub/arch/$(ARCH)/cpu/$(CPU)/
	$(CP) -rf $(SRCTREE)/fes/main/fes1_*.c $(TOPDIR)/../spl-pub/fes/main/
	$(CP) -rf $(SRCTREE)/nboot/main/boot0_*.c $(TOPDIR)/../spl-pub/nboot/main/
	$(CP) -rf $(SRCTREE)/sboot/main/sboot_main.c $(SRCTREE)/sboot/main/sboot_head.c $(TOPDIR)/../spl-pub/sboot/main/
else
all: mkdepend
	$(MAKE) -C $(SRCTREE)/fes fes
	$(call build-boot0-module-if-exist)
ifeq (x$(SBOOT_EXIST),xyes)
	$(MAKE) -C $(SRCTREE)/sboot sboot
endif #SBOOT_EXIST
endif

ifeq (x$(NAND_EXIST),xyes)
nand: mkdepend
	$(MAKE)  -C  $(SRCTREE)/nboot nand
endif
ifeq (x$(MMC_EXIST),xyes)
mmc: mkdepend
	$(MAKE)  -C  $(SRCTREE)/nboot mmc
endif
ifeq (x$(SPINOR_EXIST),xyes)
spinor: mkdepend
	$(MAKE)  -C  $(SRCTREE)/nboot spinor
endif



fes: mkdepend
	$(MAKE) -C $(SRCTREE)/fes fes
boot0: mkdepend
	$(call build-boot0-module-if-exist)
ifeq (x$(SBOOT_EXIST),xyes)
sboot: mkdepend
	$(MAKE) -C $(SRCTREE)/sboot sboot
endif

offline_secure_spinor: mkdepend
	$(MAKE) -C $(SRCTREE)/nboot $@
offline_secure_nand: mkdepend
	$(MAKE) -C $(SRCTREE)/nboot $@
offline_secure_mmc: mkdepend
	$(MAKE) -C $(SRCTREE)/nboot $@

simulate: mkdepend
	$(MAKE) -C $(SRCTREE)/nboot $@

clean:
	@find $(TOPDIR) -type f \
		\( -name 'core' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o'	-o -name '*.exe' -o -name '*.axf' \
		-o -name '*.elf' \
		-o -name '*.depend' \
		-o -name '*.bin' \
		-o -name '*.log' \
		-o -name '*.map' \) -print \
		| xargs rm -f

	@rm -f $(TOPDIR)/nboot/boot0.lds
	@rm -f $(TOPDIR)/fes/fes1.lds
	@rm -f $(TOPDIR)/sboot/sboot.lds
	@rm -f $(TOPDIR)/autoconf.mk

distclean: clean
	@rm -f $(TOPDIR)/include/config.h
	@rm -f $(TOPDIR)/.module.common.mk
	@rm -f $(cleanfiles)

mkdepend :
	@if [ ! -f $(TOPDIR)/.module.common.mk ]; then  \
	rm -rf autoconf.mk; \
	echo "***"; \
	echo "*** Configuration file \".module.common.mk\" not found!"; \
	echo "***"; \
	echo "*** Please run some configurator (e.g. make p=sun50iw3p1)"; \
	echo "***"; \
	exit 1; \
	fi;
	$(call update-commit-info)
depend:

PHONY +=FORCE
FORCE:
.PHONY:$(PHONY)



