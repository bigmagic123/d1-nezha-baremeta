# # (C) Copyright 2013-2016
# SPDX-License-Identifier: GPL-2.0+
#  Allwinner Technology Co., Ltd. <www.allwinnertech.com>
#
#  SPDX-License-Identifier:     GPL-2.0+
#


include $(TOPDIR)/mk/config.mk

LIB	:= $(obj)libdram.o

COBJS += mctl_hal.o
# COBJS += mctl_hal.o chip_id_sd.o


COBJS	:= $(COBJS)
SRCS	:= $(COBJS:.o=.S)
OBJS	:= $(addprefix $(obj),$(COBJS))

#LIBASM  = mctl_hal.S
#asmgcc = /home/wuhuabin/workspace/tina-d1/lichee/brandy-2.0/tools/toolchain/riscv64-linux-x86_64-20200528/bin/riscv64-unknown-linux-gnu-gcc

all:	$(LIB)
#all:	$(LIB) $(LIBASM)
#	$(info "****************************************************************************")
#	$(info $(obj))
#	$(info $(SRCS))
#	$(info $(COBJS))
#	$(info "****************************************************************************")
#$(LIBASM): $(SRCS)
#	$(info "############################################################################")
#	$(info $(LIBASM))
#	$(info $(asmgcc))
#	$(info $(OBJS))
##	$(call cmd_link_o_target, $(OBJS))
#	$(asmgcc) -S -o $@ $<
#	$(info "############################################################################")
#	$(info "1111111111111111111111111111111111111111111111111111111111111111111111111111")
$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))
	#$(strip) -s libdram.o
	cp $(LIB) ../libdram

#########################################################################

# defines $(obj).depend target
include $(TOPDIR)/mk/rules.mk

sinclude $(obj).depend

#########################################################################
