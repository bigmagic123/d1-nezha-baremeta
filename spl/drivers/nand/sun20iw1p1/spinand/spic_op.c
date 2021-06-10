/*
 * (C) Copyright 2017-2020
* SPDX-License-Identifier:	GPL-2.0+
 *Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */

#include "spic.h"
#include "spinand.h"
#include "spinand_osal_boot0.h"

#define USE_DMA
#ifdef USE_DMA
#include <arch/dma.h>
static ulong rx_dma_chan;
#endif

#define spi_wb(v, a) writeb(v, (volatile void __iomem *)(ulong)a)
#define spi_wl(v, a) writel(v, (volatile void __iomem *)(ulong)a)
#define spi_rb(a) readb((volatile void __iomem *)(ulong)a)
#define spi_rl(a) readl((volatile void __iomem *)(ulong)a)

__u32 SPIC_IO_BASE;

__s32 Wait_Tc_Complete(void)
{
	__u32 timeout = 0xffffff;

	/*wait transfer complete*/
	while (!(spi_rl(SPI_ISR) & (0x1 << 12))) {
		timeout--;
		if (!timeout)
			break;
	}
	if (timeout == 0) {
		SPINAND_Print("wait xfer complete timeout!\n");
		return -ERR_TIMEOUT;
	}

	return 0;
}

__s32 Spic_init(__u32 spi_no)
{
	__u32 rval;

#ifdef USE_DMA
	sunxi_dma_init();
	rx_dma_chan = sunxi_dma_request(DMAC_DMATYPE_NORMAL);
#endif
	SPINAND_PIORequest(spi_no);
	SPIC_IO_BASE = SPINAND_GetIOBaseAddr();
	SPINAND_ClkRequest(spi_no);
	SPINAND_SetClk(spi_no, 100);

	rval = SPI_SOFT_RST | SPI_TXPAUSE_EN | SPI_MASTER | SPI_ENABLE;
	spi_wl(rval, SPI_GCR);

	/*set ss to high,discard unused burst,SPI select signal
	 * polarity(low,1=idle)*/
	rval = SPI_SET_SS_1 | SPI_DHB | SPI_SS_ACTIVE0 | SPI_SAMPLE_CTRL;
	spi_wl(rval, SPI_TCR);

	spi_wl(SPI_TXFIFO_RST | (SPI_TX_WL << 16) | (SPI_RX_WL), SPI_FCR);
	spi_wl(SPI_ERROR_INT, SPI_IER);

	return 0;
}

__s32 Spic_exit(__u32 spi_no)
{
	__u32 rval;

	rval = spi_rl(SPI_GCR);
	rval &= (~(SPI_SOFT_RST | SPI_MASTER | SPI_ENABLE));
	spi_wl(rval, SPI_GCR);

	SPINAND_ClkRelease(spi_no);

	SPINAND_PIORelease(spi_no);

	/* set ss to high,discard unused burst,SPI select signal
	  * polarity(low,1=idle) */
	rval = SPI_SET_SS_1 | SPI_DHB | SPI_SS_ACTIVE0;
	spi_wl(rval, SPI_TCR);

#ifdef USE_DMA
	sunxi_dma_release(rx_dma_chan);
	sunxi_dma_exit();
#endif
	return 0;
}

void Spic_sel_ss(__u32 spi_no, __u32 ssx)
{
	__u32 rval = spi_rl(SPI_TCR) & (~(3 << 4));
	rval |= ssx << 4;
	spi_wl(rval, SPI_TCR);
}

void Spic_config_dual_mode(__u32 spi_no, __u32 rxdual, __u32 dbc, __u32 stc)
{
	spi_wl((rxdual << 28) | (dbc << 24) | (stc), SPI_BCC);
}

#ifndef USE_DMA
/*
 * spi txrx
 * _ _______ ______________
 *  |_______|/_/_/_/_/_/_/_|
 */
static __s32 xfer_by_cpu(__u32 spi_no, __u32 tcnt, __u8 *txbuf, __u32 rcnt, __u8 *rxbuf, __u32 dummy_cnt)
{
	__u32 i = 0, fcr;

	spi_wl(0, SPI_IER);
	spi_wl(0xffffffff, SPI_ISR); /*clear status register*/

	spi_wl(tcnt, SPI_MTC);
	spi_wl(tcnt + rcnt + dummy_cnt, SPI_MBC);

	/*read and write by cpu operation*/
	if (tcnt) {
		i = 0;
		while (i < tcnt) {
			if (((spi_rl(SPI_FSR) >> 16) & 0x7f) == SPI_FIFO_SIZE)
				SPINAND_Print("TX FIFO size error!\n");
			spi_wb(*(txbuf + i), SPI_TXD);
			i++;
		}
	}
	/* start transmit */
	spi_wl(spi_rl(SPI_TCR) | SPI_EXCHANGE, SPI_TCR);
	if (rcnt) {
		i = 0;
		while (i < rcnt) {
			/*receive valid data*/
			while (((spi_rl(SPI_FSR)) & 0x7f) == 0)
				;
			*(rxbuf + i) = spi_rb(SPI_RXD);
			i++;
		}
	}

	if (Wait_Tc_Complete()) {
		SPINAND_Print("wait tc complete timeout!\n");
		return -ERR_TIMEOUT;
	}

	fcr = spi_rl(SPI_FCR);
	fcr &= ~(SPI_TXDMAREQ_EN | SPI_RXDMAREQ_EN);
	spi_wl(fcr, SPI_FCR);
	/* (1U << 11) | (1U << 10) | (1U << 9) | (1U << 8)) */
	if (spi_rl(SPI_ISR) & (0xf << 8)) {
		SPINAND_Print("FIFO status error: 0x%x!\n", spi_rl(SPI_ISR));
		return NAND_OP_FALSE;
	}

	if (spi_rl(SPI_TCR) & SPI_EXCHANGE) {
		SPINAND_Print("XCH Control Error!!\n");
	}

	spi_wl(0xffffffff, SPI_ISR); /* clear  flag */
	return NAND_OP_TRUE;
}
#else
int spic0_dma_start(unsigned int tx_mode, unsigned int addr, unsigned length)
{
	int ret = 0;
	sunxi_dma_set dma_set;

	dma_set.loop_mode       = 0;
	dma_set.wait_cyc	= 32;
	dma_set.data_block_size = 0;

	if (!tx_mode) {
		dma_set.channal_cfg.src_drq_type     = DMAC_CFG_TYPE_SPI0;
		dma_set.channal_cfg.src_addr_mode    = DMAC_CFG_SRC_ADDR_TYPE_IO_MODE;
		dma_set.channal_cfg.src_burst_length = DMAC_CFG_SRC_8_BURST;
		dma_set.channal_cfg.src_data_width   = DMAC_CFG_SRC_DATA_WIDTH_32BIT;
		dma_set.channal_cfg.reserved0	= 0;

		dma_set.channal_cfg.dst_drq_type     = DMAC_CFG_TYPE_DRAM;
		dma_set.channal_cfg.dst_addr_mode    = DMAC_CFG_DEST_ADDR_TYPE_LINEAR_MODE;
		dma_set.channal_cfg.dst_burst_length = DMAC_CFG_DEST_8_BURST;
		dma_set.channal_cfg.dst_data_width   = DMAC_CFG_DEST_DATA_WIDTH_32BIT;
		dma_set.channal_cfg.reserved1	= 0;
	}

	if (!tx_mode) {
		ret = sunxi_dma_setting(rx_dma_chan, &dma_set);
		if (ret < 0) {
			SPINAND_Print("rx dma set fail\n");
			return -1;
		}
	}

	/* cache is disabled in spi0 */
	if (!tx_mode) {
		/* cache is disabled in boot0, needn`t flush cache */
		ret = sunxi_dma_start(rx_dma_chan, (__u32)SPI_RXD, addr, length);
	}
	if (ret < 0) {
		SPINAND_Print("rx dma start fail\n");
		return -1;
	}

	return 0;
}

static int spic0_wait_dma_finish(unsigned int tx_flag, unsigned int rx_flag)
{
	__u32 timeout = 0xffffff;

	if (rx_flag) {
		timeout = 0xffffff;
		while (sunxi_dma_querystatus(rx_dma_chan)) {
			timeout--;
			if (!timeout)
				break;
		}

		if (timeout <= 0) {
			SPINAND_Print("RX DMA wait status timeout!\n");
			return -ERR_TIMEOUT;
		}
	}

	return 0;
}
static __s32 xfer_by_dma(__u32 spi_no, __u32 tcnt, __u8 *txbuf, __u32 rcnt, __u8 *rxbuf, __u32 dummy_cnt)
{
	__u32 i		  = 0, fcr;
	__u32 tx_dma_flag = 0;
	__u32 rx_dma_flag = 0;

	spi_wl(0, SPI_IER);
	spi_wl(0xffffffff, SPI_ISR); /*clear status register*/

	spi_wl(tcnt, SPI_MTC);
	spi_wl(tcnt + rcnt + dummy_cnt, SPI_MBC);

	/*read and write by cpu operation*/
	if (tcnt) {
		i = 0;
		while (i < tcnt) {
			if (((spi_rl(SPI_FSR) >> 16) & 0x7f) == SPI_FIFO_SIZE)
				SPINAND_Print("TX FIFO size error!\n");
			spi_wb(*(txbuf + i), SPI_TXD);
			i++;
		}
	}
	/* start transmit */
	spi_wl(spi_rl(SPI_TCR) | SPI_EXCHANGE, SPI_TCR);
	if (rcnt) {
		if (rcnt <= 64) {
			i = 0;

			while (i < rcnt) {
				/*receive valid data*/
				while (((spi_rl(SPI_FSR)) & 0x7f) == 0)
					;
				*(rxbuf + i) = spi_rb(SPI_RXD);
				i++;
			}
		} else {
			rx_dma_flag = 1;
			spi_wl((spi_rl(SPI_FCR) | SPI_RXDMAREQ_EN), SPI_FCR);
			spic0_dma_start(0, (unsigned long)rxbuf, rcnt);
		}
	}

	if (spic0_wait_dma_finish(tx_dma_flag, rx_dma_flag)) {
		SPINAND_Print("DMA wait status timeout!\n");
		return -ERR_TIMEOUT;
	}

	if (Wait_Tc_Complete()) {
		SPINAND_Print("wait tc complete timeout!\n");
		return -ERR_TIMEOUT;
	}

	fcr = spi_rl(SPI_FCR);
	fcr &= ~(SPI_TXDMAREQ_EN | SPI_RXDMAREQ_EN);
	spi_wl(fcr, SPI_FCR);
	/* (1U << 11) | (1U << 10) | (1U << 9) | (1U << 8)) */
	if (spi_rl(SPI_ISR) & (0xf << 8)) {
		SPINAND_Print("FIFO status error: 0x%x!\n", spi_rl(SPI_ISR));
		return NAND_OP_FALSE;
	}

	if (spi_rl(SPI_TCR) & SPI_EXCHANGE) {
		SPINAND_Print("XCH Control Error!!\n");
	}

	spi_wl(0xffffffff, SPI_ISR); /* clear  flag */
	return NAND_OP_TRUE;
}
#endif
/*
 * spi txrx
 * _ _______ ______________
 *  |_______|/_/_/_/_/_/_/_|
 */
__s32 Spic_rw(__u32 spi_no, __u32 tcnt, __u8 *txbuf, __u32 rcnt, __u8 *rxbuf, __u32 dummy_cnt)
{
#ifndef USE_DMA
	return xfer_by_cpu(spi_no, tcnt, txbuf, rcnt, rxbuf, dummy_cnt);
#else
	return xfer_by_dma(spi_no, tcnt, txbuf, rcnt, rxbuf, dummy_cnt);
#endif
}
