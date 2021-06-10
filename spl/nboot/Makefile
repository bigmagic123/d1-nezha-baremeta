#
# (C) Copyright 2000-2011
# SPDX-License-Identifier: GPL-2.0+
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2011
# Daniel Schwierzeck, daniel.schwierzeck@googlemail.com.
#
# (C) Copyright 2011
# Texas Instruments Incorporated - http://www.ti.com/
# Aneesh V <aneesh@ti.com>
#
# See the file COPYING in the root directory of the source tree for details.
#
# Based on top-level Makefile.
#

SKIP_AUTO_CONF:=yes
NORMAL_GOALS:=mmc spinor nand
include $(TOPDIR)/mk/config.mk
include $(TOPDIR)/.module.common.mk
 ifeq ($(findstring simulate,$(MAKECMDGOALS)),simulate)
CFG_SUNXI_SIMULATE_BOOT0=y
CFG_SUNXI_SIM_SIZE_KB?=32
MAIN_OBJ=simulate_main.o
else ifeq ($(findstring $(MAKECMDGOALS),$(NORMAL_GOALS)),)
CFG_SUNXI_SET_SECURE_MODE=y
CFG_SUNXI_WATCHDOG=y
MAIN_OBJ=offline_secure_main.o
else
include $(TOPDIR)/board/$(PLATFORM)/$(MAKECMDGOALS).mk
MAIN_OBJ=boot0_main.o
endif

#This for Tina system, to copy bin to tina SDK
ifeq ($(TARGET_BUILD_VARIANT),tina)
tina_path=$(TOPDIR)/../../../target/allwinner/$(TARGET_PLATFORM)-common/bin
exist = $(shell if [ -d $(tina_path) ]; then echo "exist"; else echo "notexist"; fi;)
ifeq ($(exist),exist)
TARGETDIR=$(TOPDIR)/../../../target/allwinner/$(TARGET_PLATFORM)-common/bin
endif

tina_path=$(TOPDIR)/../../../device/config/chips/$(TARGET_PLATFORM)/bin
exist = $(shell if [ -d $(tina_path) ]; then echo "exist"; else echo "notexist"; fi;)
ifeq ($(exist),exist)
TARGETDIR=$(TOPDIR)/../../../device/config/chips/$(TARGET_PLATFORM)/bin
endif

else
#TOOLS_DIR  := $(TOPDIR)/../../../device/config
#TARGETDIR=$(TOPDIR)/../../../device/config/chips/$(PLATFORM)/bin
ifneq ($(LICHEE_BUSSINESS), )
	TARGETDIR=$(LICHEE_CHIP_CONFIG_DIR)/$(LICHEE_BUSSINESS)/bin
else
	TARGETDIR=$(LICHEE_CHIP_CONFIG_DIR)/bin
endif
endif

CPU ?= armv7

CPUDIR = $(TOPDIR)/arch/$(ARCH)/cpu/$(CPU)
BOOT0_LDSCRIPT := $(CPUDIR)/boot0.lds

# We want the final binaries in this directory
NBOOT_DIR := $(TOPDIR)/nboot/
obj := $(TOPDIR)/nboot/

LIBS-y += arch/$(ARCH)/cpu/$(CPU)/libarch.o
LIBS-y += nboot/main/libmain.o
LIBS-y += drivers/libdrivers.o
LIBS-y += common/libcommon.o
LIBS-y += board/$(PLATFORM)/libboard.o
LIBS-$(CFG_SUNXI_SET_SECURE_MODE) += drivers/secure/libsecure.o

LIBS := $(addprefix $(TOPDIR)/,$(sort $(LIBS-y)))

COBJS-y += $(TOPDIR)/nboot/main/$(MAIN_OBJ)

OBJS = $(sort $(COBJS-y))

ifndef CONFIG_BOOT0_SIZE_LIMIT
	BOOT0SIZE = $(CFG_SYS_INIT_RAM_SIZE)
else
	BOOT0SIZE = $(CFG_BOOT0_SIZE_LIMIT)
endif


# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-DBOOT0ADDR=$(CFG_BOOT0_RUN_ADDR)	 \
	-DBOOT0SIZE=$(BOOT0SIZE)	 \
	-DCPUDIR=$(CPUDIR) \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

define build_boot0_with_suffix
	$(Q)echo bootaddr is $(CFG_BOOT0_RUN_ADDR)
	$(LD) -r -o lib$(PLATFORM)$1.o $(LIBS)
	$(LD) lib$(PLATFORM)$1.o $(OBJS) $(PLATFORM_LIBGCC) $(LDFLAGS) $(LDFLAGS_GC) -Tboot0.lds -o boot0$1.elf -Map boot0$1.map
	$(OBJCOPY) $(OBJCFLAGS) -O binary  boot0$1.elf boot0$1.bin
	@$(TOPDIR)/mk/gen_check_sum $(SRCTREE)/nboot/boot0$1.bin boot0$1_$(PLATFORM)$(DRAM_TYPE_NAME).bin > /dev/null
	@$(TOPDIR)/mk/encrypto_boot0 -f boot0$1_$(PLATFORM)$(DRAM_TYPE_NAME).bin  -c $(LICHEE_IC) >/dev/null
	$(STRIP) -g $(TOPDIR)/nboot/lib$(PLATFORM)$1.o
	$(CP) $(TOPDIR)/nboot/lib$(PLATFORM)$1.o $(TOPDIR)/../spl-pub/board/$(CP_BOARD)/lib$(PLATFORM)$1.a
endef

spinor:	 $(LIBS) boot0.lds
	$(call build_boot0_with_suffix,_spinor)
	@-cp -v boot0_spinor_$(PLATFORM)$(DRAM_TYPE_NAME).bin $(TARGETDIR)/
	@-cp -v boot0_spinor_$(PLATFORM)$(DRAM_TYPE_NAME).bin $(LICHEE_PLAT_OUT)/

mmc:	 $(LIBS) boot0.lds
	$(call build_boot0_with_suffix,_sdcard)
	@-cp -v boot0_sdcard_$(PLATFORM)$(DRAM_TYPE_NAME).bin $(TARGETDIR)/
	@-cp -v boot0_sdcard_$(PLATFORM)$(DRAM_TYPE_NAME).bin $(LICHEE_PLAT_OUT)/

nand:	 $(LIBS) boot0.lds
	$(call build_boot0_with_suffix,_nand)
	@-cp -v boot0_nand_$(PLATFORM)$(DRAM_TYPE_NAME).bin $(TARGETDIR)/
	@-cp -v boot0_nand_$(PLATFORM)$(DRAM_TYPE_NAME).bin $(LICHEE_PLAT_OUT)/

offline_secure_spinor:	 $(LIBS) boot0.lds $(LIBNAND)
	$(call build_boot0_with_suffix,_offline_secure)
	@-cp -v boot0_offline_secure_$(PLATFORM)$(DRAM_TYPE_NAME).bin $(TARGETDIR)/boot0_spinor_$(PLATFORM)$(DRAM_TYPE_NAME).bin
	@-cp -v boot0_offline_secure_$(PLATFORM)$(DRAM_TYPE_NAME).bin $(LICHEE_PLAT_OUT)/boot0_spinor_$(PLATFORM)$(DRAM_TYPE_NAME).bin

offline_secure_mmc:	 $(LIBS) boot0.lds $(LIBNAND)
	$(call build_boot0_with_suffix,_offline_secure)
	@-cp -v boot0_offline_secure_$(PLATFORM)$(DRAM_TYPE_NAME).bin $(TARGETDIR)/boot0_sdcard_$(PLATFORM)$(DRAM_TYPE_NAME).bin
	@-cp -v boot0_offline_secure_$(PLATFORM)$(DRAM_TYPE_NAME).bin $(LICHEE_PLAT_OUT)/boot0_sdcard_$(PLATFORM)$(DRAM_TYPE_NAME).bin

offline_secure_nand:	 $(LIBS) boot0.lds $(LIBNAND)
	$(call build_boot0_with_suffix,_offline_secure)
	@-cp -v boot0_offline_secure_$(PLATFORM)$(DRAM_TYPE_NAME).bin $(TARGETDIR)/boot0_nand_$(PLATFORM)$(DRAM_TYPE_NAME).bin
	@-cp -v boot0_offline_secure_$(PLATFORM)$(DRAM_TYPE_NAME).bin $(LICHEE_PLAT_OUT)/boot0_nand_$(PLATFORM)$(DRAM_TYPE_NAME).bin

simulate:	 $(LIBS) boot0.lds
	$(call build_boot0_with_suffix,_simulate)
	@-mv boot0_simulate_$(PLATFORM)$(DRAM_TYPE_NAME).bin boot0_simulate_$(PLATFORM)_$(CFG_SUNXI_SIM_SIZE_KB)KB.bin

mmc_$(DRAM_TYPE):mmc
nand_$(DRAM_TYPE):nand

$(LIBS): depend
	$(MAKE) -C $(dir $(subst $(OBJTREE),,$@))

boot0.lds: $(BOOT0_LDSCRIPT) depend
	$(Q)$(CPP) $(ALL_CFLAGS) $(LDPPFLAGS) -ansi -D__ASSEMBLY__ -P - <$(BOOT0_LDSCRIPT) >$@

$(NBOOT_DIR)cur.log:
	@git show HEAD --pretty=format:"%H" | head -n 1 > cur.log

sinclude $(TOPDIR)/mk/target_for_conf.mk
depend: .depend build-confs
#########################################################################

# defines $(NBOOT_DIR).depend target
include $(TOPDIR)/mk/rules.mk

sinclude .depend

#########################################################################
