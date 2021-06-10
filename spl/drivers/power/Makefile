#
# Copyright (C) 2019 Allwinner.
# SPDX-License-Identifier: GPL-2.0+
# weidonghui <weidonghui@allwinnertech.com>
#
#
include $(TOPDIR)/mk/config.mk
LIB	=  libpower.o

COBJS-y += axp.o
COBJS-$(CFG_AXP809_POWER) += axp809.o
COBJS-$(CFG_AXP81X_POWER) += axp81X.o
COBJS-$(CFG_AXP858_POWER) += axp858.o
COBJS-$(CFG_AXP2101_POWER) += axp2101.o
COBJS-$(CFG_AXP2202_POWER) += axp2202.o
COBJS-$(CFG_AXP152_POWER) += axp152.o
COBJS-$(CFG_AXP1530_POWER) += axp1530.o
COBJS-$(CFG_AXP806_POWER) += axp806.o


SRCS	:= $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

depend:	$(obj).depend
.PHONY: depend

# defines $(obj).depend target
include $(TOPDIR)/mk/rules.mk
