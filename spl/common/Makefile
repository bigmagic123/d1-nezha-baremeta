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
LIB	=  libcommon.o

COBJS	+= string.o
COBJS	+= printf.o
COBJS   += boot_utils.o
COBJS   += eabi_compat.o
COBJS   += malloc.o
COBJS   += iobase_sunxi.o

ifdef CFG_SUNXI_MEMOP
COBJS   += memset_sunxi.o
COBJS   += memcpy_sunxi.o
endif
COBJS   += debug.o

ifdef CFG_SUNXI_GUNZIP
COBJS   += crc32.o
COBJS   += gunzip.o
COBJS   += zlib/zlib.o
endif

ifdef CFG_SUNXI_LZ4
COBJS   += lz4/lz4_wrapper.o
endif

ifdef CFG_SUNXI_LZMA
COBJS   += lzma/LzmaDec.o
COBJS   += lzma/LzmaTools.o
endif
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
