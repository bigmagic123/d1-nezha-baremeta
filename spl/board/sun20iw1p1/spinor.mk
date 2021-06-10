#
#config file for sun50iw11
#
FILE_EXIST=$(shell if [ -f $(TOPDIR)/board/$(PLATFORM)/common.mk ]; then echo yes; else echo no; fi;)
ifeq (x$(FILE_EXIST),xyes)
include $(TOPDIR)/board/$(PLATFORM)/common.mk
else
include $(TOPDIR)/board/$(CP_BOARD)/common.mk
endif

MODULE=spinor
CFG_SUNXI_SPI =y
CFG_SUNXI_DMA =y
CFG_SPI_USE_DMA =y
CFG_SUNXI_SPINOR =y
CFG_SPINOR_UBOOT_OFFSET=128
