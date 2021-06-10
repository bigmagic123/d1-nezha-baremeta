#
# (C) Copyright 2000-2003
# SPDX-License-Identifier: GPL-2.0+
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/mk/config.mk
LIB	=  libdrivers.o

COBJS-y	+= serial.o
COBJS-y	+= pinmux.o
COBJS-y	+= rtc.o
COBJS-$(CFG_SUNXI_SPI) += spi.o
COBJS-$(CFG_SUNXI_RSB) += rsb.o
COBJS-$(CFG_SUNXI_TWI) += i2c.o
COBJS-$(CFG_SUNXI_PMIC) += pmic_bus.o
COBJS-$(CFG_SUNXI_STANDBY) += standby.o
COBJS-$(CFG_SUNXI_WATCHDOG) += watchdog.o
COBJS-$(CFG_SUNXI_DMA) += sunxi_dma.o

ifdef CFG_SUNXI_PHY_KEY
COBJS-$(CFG_LRADC_KEY) += lrkey.o
COBJS-$(CFG_GPADC_KEY) += gpkey.o
endif

LIBS-$(CFG_SUNXI_POWER) += drivers/power/libpower.o
LIBS-y += drivers/dram/libdram.o

LIBS-$(CFG_DDR_SOFT_TRAIN) += drivers/secure/libsecure.o
LIBS-$(CFG_SUNXI_SBOOT) += drivers/secure/libsecure.o
LIBS-$(CFG_SUNXI_SBOOT) += sboot/openssl/libopenssl.o


LIBS-$(CFG_SUNXI_NAND) += drivers/nand/$(PLATFORM)/libnand.o
ifeq ($(CFG_SUNXI_SBOOT),)
	LIBS-$(CFG_SUNXI_NAND) += nboot/load_image_nand/libloadnand.o
endif

check_spinand=$(shell if [ -d $(TOPDIR)/nboot/load_image_spinand ]; then echo yes; else echo no; fi;)
ifeq (x$(check_spinand), xyes)
	LIBS-$(CFG_SUNXI_SPINAND) += drivers/nand/$(PLATFORM)/libspinand.o
	ifeq ($(CFG_SUNXI_SBOOT),)
	LIBS-$(CFG_SUNXI_SPINAND) += nboot/load_image_spinand/libloadspinand.o
endif
endif

LIBS-$(CFG_SUNXI_SDMMC)$(CFG_SUNXI_MMC) += drivers/mmc/$(PLATFORM)/libmmc.o
ifeq ($(CFG_SUNXI_SBOOT),)
	LIBS-$(CFG_SUNXI_SDMMC) += nboot/load_image_mmc/libloadmmc.o
endif

LIBS-$(CFG_SUNXI_SPINOR) += drivers/spinor/libspinor.o
ifeq ($(CFG_SUNXI_SBOOT),)
	LIBS-$(CFG_SUNXI_SPINOR) += nboot/load_image_spinor/libloadspinor.o
endif

LIBS := $(addprefix $(TOPDIR)/,$(sort $(LIBS-y)))


SRCS	:= $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
OBJS	+= $(LIBS)
all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(LIBS): depend
	$(MAKE) -C $(dir $(subst $(OBJTREE),,$@))
#########################################################################

depend:	$(obj).depend
.PHONY: depend

# defines $(obj).depend target
include $(TOPDIR)/mk/rules.mk
