#
# (C) Copyright 2013-2016
# SPDX-License-Identifier: GPL-2.0+
#  Allwinner Technology Co., Ltd. <www.allwinnertech.com>
#
#


include $(TOPDIR)/mk/config.mk

LIB	:= $(obj)libnand.o

COBJS-y += nand/adv_NF_read.o
COBJS-y += nand/basic_nf_read.o

COBJS-y += nand_bsp/nand_for_boot0/nand_for_boot0.o  nand_bsp/nand_for_boot0/osal/nand_osal_boot0.o
COBJS-y += nand_bsp/nfc_for_boot0/src/nand_scan_for_boot.o  nand_bsp/nfc_for_boot0/src/nfc_for_boot0.o  nand_bsp/nfc_for_boot0/src/nand_simple_for_boot0.o

COBJS-$(CFG_SUNXI_SPINAND) += spinand/spic_op.o
COBJS-$(CFG_SUNXI_SPINAND) += spinand/spinand.o
COBJS-$(CFG_SUNXI_SPINAND) += spinand/spinand_basic_op.o
COBJS-$(CFG_SUNXI_SPINAND) += spinand/spinand_osal_boot0.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(TOPDIR)/mk/rules.mk

sinclude $(obj).depend

#########################################################################
