#
# (C) Copyright 2013-2016
# SPDX-License-Identifier: GPL-2.0+
#  Allwinner Technology Co., Ltd. <www.allwinnertech.com>
#
#


include $(TOPDIR)/mk/config.mk

LIB	:= $(obj)libmmc.o

COBJS += mmc.o
COBJS += mmc_bsp.o


COBJS	:= $(COBJS)
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
