#/*
# * Copyright 2000-2009
# SPDX-License-Identifier: GPL-2.0+
#  * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#   *
#    */
include $(TOPDIR)/mk/config.mk
LIB	=  libboard.o


COBJS	+= board.o
COBJS	+= clock.o

SRCS	:= $(COBJS:.o=.c) $(SOBJS:.o=.S)
OBJS	:= $(addprefix $(obj),$(COBJS) $(SOBJS))

all:	 $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(TOPDIR)/mk/rules.mk

sinclude .depend

#########################################################################
