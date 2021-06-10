/*
 * (C) Copyright 2013-2016
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 */
/*
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * Description: MMC driver for General mmc operations
 * Author: Aaron <leafy.myeh@allwinnertech.com>
 * Date: 2012-2-3 14:18:18
 */
#include "mmc_def.h"
#include "mmc_bsp.h"
#include "mmc.h"
#include <private_boot0.h>

/* Set block count limit because of 16 bit register limit on some hardware*/
#ifndef CONFIG_SYS_MMC_MAX_BLK_COUNT
#define CONFIG_SYS_MMC_MAX_BLK_COUNT 65535
#endif

unsigned char mmc_arg_addr[SUNXI_SDMMC_PARAMETER_REGION_SIZE_BYTE];
extern int mmc_config_addr; /*extern const boot0_file_head_t BT0_head; */
static struct mmc *mmc_devices[MAX_MMC_NUM];

int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	return mmc->send_cmd(mmc, cmd, data);
}

int mmc_send_status(struct mmc *mmc, int timeout)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx    = MMC_CMD_SEND_STATUS;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg    = mmc->rca << 16;
	cmd.flags     = 0;

	do {
		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err) {
			mmcinfo("mmc %u Send status failed\n",
				mmc->control_num);
			return err;
		} else if (cmd.response[0] & MMC_STATUS_RDY_FOR_DATA)
			break;

		mdelay(1);

		if (cmd.response[0] & MMC_STATUS_MASK) {
			mmcinfo("mmc %u Status Error: 0x%08X\n",
				mmc->control_num, cmd.response[0]);
			return COMM_ERR;
		}
	} while (timeout--);

	if (!timeout) {
		mmcinfo("mmc %u Timeout waiting card ready\n",
			mmc->control_num);
		return TIMEOUT;
	}

	return 0;
}

int mmc_set_blocklen(struct mmc *mmc, int len)
{
	struct mmc_cmd cmd;

	/* don't set blocklen at ddr mode */
	if ((mmc->speed_mode == HSDDR52_DDR50) || (mmc->speed_mode == HS400)) {
		return 0;
	}

	cmd.cmdidx    = MMC_CMD_SET_BLOCKLEN;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg    = len;
	cmd.flags     = 0;

	return mmc_send_cmd(mmc, &cmd, NULL);
}

struct mmc *find_mmc_device(int dev_num)
{
	if (mmc_devices[dev_num] != NULL)
		return mmc_devices[dev_num];

	mmcinfo("MMC Device %d not found\n", dev_num);

	return NULL;
}

int mmc_read_blocks(struct mmc *mmc, void *dst, unsigned long start,
		    unsigned blkcnt)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int timeout = 1000;

	if (blkcnt > 1)
		cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;

	if (mmc->high_capacity)
		cmd.cmdarg = start;
	else
		cmd.cmdarg = start * mmc->read_bl_len;

	cmd.resp_type = MMC_RSP_R1;
	cmd.flags     = 0;

	data.b.dest    = dst;
	data.blocks    = blkcnt;
	data.blocksize = mmc->read_bl_len;
	data.flags     = MMC_DATA_READ;

	if (mmc_send_cmd(mmc, &cmd, &data)) {
		mmcinfo("mmc %u  read blcok failed\n", mmc->control_num);
		return 0;
	}

	if (blkcnt > 1) {
		cmd.cmdidx    = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg    = 0;
		cmd.resp_type = MMC_RSP_R1b;
		cmd.flags     = 0;
		if (mmc_send_cmd(mmc, &cmd, NULL)) {
			mmcinfo("mmc %u fail to send stop cmd\n",
				mmc->control_num);
			return 0;
		}

		/* Waiting for the ready status */
		mmc_send_status(mmc, timeout);
	}

	return blkcnt;
}

unsigned long mmc_bread(int dev_num, unsigned long start, unsigned blkcnt,
			void *dst)
{
	unsigned cur, blocks_todo = blkcnt;
	struct mmc *mmc = find_mmc_device(dev_num);

	if (blkcnt == 0) {
		mmcinfo("mmc %u blkcnt should not be 0\n", mmc->control_num);
		return 0;
	}
	if (!mmc) {
		mmcinfo("Can not find mmc dev %d\n", dev_num);
		return 0;
	}

	if ((start + blkcnt) > mmc->lba) {
		mmcinfo("mmc %u: block number 0x%x exceeds max(0x%x)\n",
			mmc->control_num, (unsigned int)(start + blkcnt),
			(unsigned int)mmc->lba);
		return 0;
	}

	if (mmc_set_blocklen(mmc, mmc->read_bl_len)) {
		mmcinfo("mmc %u Set block len failed\n", mmc->control_num);
		return 0;
	}

	do {
		cur = (blocks_todo > mmc->b_max) ? mmc->b_max : blocks_todo;
		if (mmc_read_blocks(mmc, dst, start, cur) != cur) {
			mmcinfo("mmc %u block read failed\n", mmc->control_num);
			return 0;
		}
		blocks_todo -= cur;
		start += cur;
		/*dst += cur * mmc->read_bl_len; */
		dst = (char *)dst + cur * mmc->read_bl_len;
	} while (blocks_todo > 0);

	return blkcnt;
}

int mmc_go_idle(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;

	mdelay(1);

	cmd.cmdidx    = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg    = 0;
	cmd.resp_type = MMC_RSP_NONE;
	cmd.flags     = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err) {
		mmcinfo("mmc %u go idle failed\n", mmc->control_num);
		return err;
	}

	mdelay(2);

	return 0;
}

int sd_send_op_cond(struct mmc *mmc)
{
	int timeout = 1000;
	int err;
	struct mmc_cmd cmd;

	do {
		cmd.cmdidx    = MMC_CMD_APP_CMD;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg    = 0;
		cmd.flags     = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err) {
			mmcinfo("mmc %u send app cmd failed\n",
				mmc->control_num);
			return err;
		}

		cmd.cmdidx    = SD_CMD_APP_SEND_OP_COND;
		cmd.resp_type = MMC_RSP_R3;

		/*
		 * Most cards do not answer if some reserved bits
		 * in the ocr are set. However, Some controller
		 * can set bit 7 (reserved for low voltages), but
		 * how to manage low voltages SD card is not yet
		 * specified.
		 */
		cmd.cmdarg =
			mmc_host_is_spi(mmc) ? 0 : (mmc->voltages & 0xff8000);

		if (mmc->version == SD_VERSION_2)
			cmd.cmdarg |= OCR_HCS;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err) {
			mmcinfo("mmc %u send cmd41 failed\n", mmc->control_num);
			return err;
		}

		mdelay(1);
	} while ((!(cmd.response[0] & OCR_BUSY)) && timeout--);

	if (timeout <= 0) {
		mmcinfo("mmc %u wait card init failed\n", mmc->control_num);
		return UNUSABLE_ERR;
	}

	if (mmc->version != SD_VERSION_2)
		mmc->version = SD_VERSION_1_0;

	if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
		cmd.cmdidx    = MMC_CMD_SPI_READ_OCR;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg    = 0;
		cmd.flags     = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err) {
			mmcinfo("mmc %u spi read ocr failed\n",
				mmc->control_num);
			return err;
		}
	}

	mmc->ocr = cmd.response[0];

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	mmc->rca	   = 0;

	return 0;
}

int mmc_send_op_cond(struct mmc *mmc)
{
	int timeout = 10000;
	struct mmc_cmd cmd;
	int err;

	/* Some cards seem to need this */
	mmc_go_idle(mmc);

	/* Asking to the card its capabilities */
	cmd.cmdidx    = MMC_CMD_SEND_OP_COND;
	cmd.resp_type = MMC_RSP_R3;
	cmd.cmdarg    = 0; /*0x40ff8000;*/ /*foresee */
	cmd.flags     = 0;

	/*mmcinfo("mmc send op cond arg not zero !!!\n"); */
	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err) {
		mmcinfo("mmc %u send op cond failed\n", mmc->control_num);
		return err;
	}

	mdelay(1);

	do {
		cmd.cmdidx    = MMC_CMD_SEND_OP_COND;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg =
			(mmc_host_is_spi(mmc) ?
				 0 :
				 (mmc->voltages &
				  (cmd.response[0] & OCR_VOLTAGE_MASK)) |
					 (cmd.response[0] & OCR_ACCESS_MODE));

		if (mmc->host_caps & MMC_MODE_HC)
			cmd.cmdarg |= OCR_HCS;

		cmd.flags = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err) {
			mmcinfo("mmc %u send op cond failed\n",
				mmc->control_num);
			return err;
		}

		mdelay(1);
	} while (!(cmd.response[0] & OCR_BUSY) && timeout--);

	if (timeout <= 0) {
		mmcinfo("mmc %u wait for mmc init failed\n", mmc->control_num);
		return UNUSABLE_ERR;
	}

	if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
		cmd.cmdidx    = MMC_CMD_SPI_READ_OCR;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg    = 0;
		cmd.flags     = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err)
			return err;
	}

	mmc->version = MMC_VERSION_UNKNOWN;
	mmc->ocr     = cmd.response[0];

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	mmc->rca	   = 1;

	return 0;
}

int mmc_send_ext_csd(struct mmc *mmc, char *ext_csd)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;

	/* Get the Card Status Register */
	cmd.cmdidx    = MMC_CMD_SEND_EXT_CSD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg    = 0;
	cmd.flags     = 0;

	data.b.dest    = ext_csd;
	data.blocks    = 1;
	data.blocksize = 512;
	data.flags     = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);
	if (err)
		mmcinfo("mmc %u send ext csd failed\n", mmc->control_num);

	return err;
}

int mmc_update_phase(struct mmc *mmc)
{
	if (mmc->update_phase == NULL)
		return 0;

	return mmc->update_phase(mmc);
}

int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value)
{
	struct mmc_cmd cmd;
	int timeout = 1000;
	int ret;

	cmd.cmdidx    = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg    = (MMC_SWITCH_MODE_WRITE_BYTE << 24) | (index << 16) |
		     (value << 8);
	cmd.flags = 0;

	ret = mmc_send_cmd(mmc, &cmd, NULL);
	if (ret) {
		mmcinfo("mmc %u switch failed\n", mmc->control_num);
	}

	/* for re-update sample phase */
	ret = mmc_update_phase(mmc);
	if (ret) {
		mmcinfo("mmc_switch: update clock failed after send cmd6\n");
		return ret;
	}

	/* Waiting for the ready status */
	mmc_send_status(mmc, timeout);

	return ret;
}

int mmc_change_freq(struct mmc *mmc)
{
	char ext_csd[512];
	char cardtype;
	int err;
	int retry = 5;

	mmc->card_caps = 0;

	if (mmc_host_is_spi(mmc))
		return 0;

	/* Only version 4 supports high-speed */
	if (mmc->version < MMC_VERSION_4)
		return 0;

	mmc->card_caps |= MMC_MODE_4BIT | MMC_MODE_8BIT;
	err = mmc_send_ext_csd(mmc, ext_csd);

	if (err) {
		mmcinfo("mmc %u get ext csd failed\n", mmc->control_num);
		return err;
	}

	cardtype = ext_csd[196] & 0xff;

	/*retry for Toshiba emmc,for the first time Toshiba emmc change to HS */
	/*it will return response crc err,so retry */
	do {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING,
				 1);
		if (!err) {
			break;
		}
		mmcinfo("retry mmc switch(cmd6)\n");
	} while (retry--);

	if (err) {
		mmcinfo("mmc %u change to hs failed\n", mmc->control_num);
		return err;
	}

	/* Now check to see that it worked */
	err = mmc_send_ext_csd(mmc, ext_csd);

	if (err) {
		mmcinfo("mmc %u send ext csd faild\n", mmc->control_num);
		return err;
	}

	/* No high-speed support */
	if (!ext_csd[185])
		return 0;

	/* High Speed is set, there are two types: 52MHz and 26MHz */
	if (cardtype & EXT_CSD_CARD_TYPE_HS) { /*EXT_CSD_CARD_TYPE_52 */
		if (cardtype & EXT_CSD_CARD_TYPE_DDR_52) {
			mmcdbg("%s: get ddr OK!\n", __FUNCTION__);
			mmc->card_caps |= MMC_MODE_DDR_52MHz;
			mmc->speed_mode = HSDDR52_DDR50;
		} else
			mmc->speed_mode = HSSDR52_SDR25;
		mmc->card_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

	} else {
		mmc->card_caps |= MMC_MODE_HS;
		mmc->speed_mode = DS26_SDR12;
	}

	return 0;
}

int mmc_switch_part(int dev_num, unsigned int part_num)
{
	struct mmc *mmc = find_mmc_device(dev_num);

	if (!mmc)
		return -1;

	return mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONF,
			  (mmc->part_config & ~PART_ACCESS_MASK) |
				  (part_num & PART_ACCESS_MASK));
}

int sd_switch(struct mmc *mmc, int mode, int group, u8 value, u8 *resp)
{
	struct mmc_cmd cmd;
	struct mmc_data data;

	/* Switch the frequency */
	cmd.cmdidx    = SD_CMD_SWITCH_FUNC;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg    = ((u32)mode << 31) | 0xffffff;
	cmd.cmdarg &= ~(0xf << (group * 4));
	cmd.cmdarg |= value << (group * 4);
	cmd.flags = 0;

	data.b.dest    = (char *)resp;
	data.blocksize = 64;
	data.blocks    = 1;
	data.flags     = MMC_DATA_READ;

	return mmc_send_cmd(mmc, &cmd, &data);
}

int sd_change_freq(struct mmc *mmc)
{
	int err;
	struct mmc_cmd cmd;
	u32 scr[2];
	u32 switch_status[16];
	struct mmc_data data;
	int timeout;

	mmc->card_caps = 0;

	if (mmc_host_is_spi(mmc))
		return 0;

	/* Read the SCR to find out if this card supports higher speeds */
	cmd.cmdidx    = MMC_CMD_APP_CMD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg    = mmc->rca << 16;
	cmd.flags     = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err) {
		mmcinfo("mmc %u Send app cmd failed\n", mmc->control_num);
		return err;
	}

	cmd.cmdidx    = SD_CMD_APP_SEND_SCR;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg    = 0;
	cmd.flags     = 0;

	timeout = 3;

retry_scr:
	data.b.dest    = (char *)&scr;
	data.blocksize = 8;
	data.blocks    = 1;
	data.flags     = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);

	if (err) {
		if (timeout--)
			goto retry_scr;

		mmcinfo("mmc %u Send scr failed\n", mmc->control_num);
		return err;
	}

	mmc->scr[0] = __mmc_be32_to_cpu(scr[0]);
	mmc->scr[1] = __mmc_be32_to_cpu(scr[1]);

	switch ((mmc->scr[0] >> 24) & 0xf) {
	case 0:
		mmc->version = SD_VERSION_1_0;
		break;
	case 1:
		mmc->version = SD_VERSION_1_10;
		break;
	case 2:
		mmc->version = SD_VERSION_2;
		break;
	default:
		mmc->version = SD_VERSION_1_0;
		break;
	}

	if (mmc->scr[0] & SD_DATA_4BIT)
		mmc->card_caps |= MMC_MODE_4BIT;

	/* Version 1.0 doesn't support switching */
	if (mmc->version == SD_VERSION_1_0)
		return 0;

	timeout = 4;
	while (timeout--) {
		err = sd_switch(mmc, SD_SWITCH_CHECK, 0, 1,
				(u8 *)&switch_status);

		if (err) {
			mmcinfo("mmc %u Check high speed status faild\n",
				mmc->control_num);
			return err;
		}

		/* The high-speed function is busy.  Try again */
		if (!(__be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY))
			break;
	}

	/* If high-speed isn't supported, we return */
	if (!(__be32_to_cpu(switch_status[3]) & SD_HIGHSPEED_SUPPORTED))
		return 0;

	err = sd_switch(mmc, SD_SWITCH_SWITCH, 0, 1, (u8 *)&switch_status);

	if (err) {
		mmcinfo("mmc %u switch to high speed failed\n",
			mmc->control_num);
		return err;
	}

	err = mmc_update_phase(mmc);
	if (err) {
		mmcinfo("update clock failed after send cmd6 to switch to sd high speed mode\n");
		return err;
	}

	if ((__be32_to_cpu(switch_status[4]) & 0x0f000000) == 0x01000000) {
		mmc->card_caps |= MMC_MODE_HS;
		mmc->speed_mode = HSSDR52_SDR25;
	}

	return 0;
}

/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
static const int fbase[] = {
	10000, 100000, 1000000, 10000000,
};

/* Multiplier values for TRAN_SPEED.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
static const int multipliers[] = {
	0, /* reserved */
	10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80,
};

void mmc_set_ios(struct mmc *mmc)
{
	mmc->set_ios(mmc);
}

void mmc_set_clock(struct mmc *mmc, u32 clock)
{
	if (clock > mmc->f_max)
		clock = mmc->f_max;

	if (clock < mmc->f_min)
		clock = mmc->f_min;

	mmc->clock = clock;

	mmc_set_ios(mmc);
}

void mmc_set_bus_width(struct mmc *mmc, u32 width)
{
	mmc->bus_width = width;

	mmc_set_ios(mmc);
}


static void dumphex32(char *name, char *base, int len)
{
	__u32 i;

	printf("dump %s registers:", name);
	for (i = 0; i < len; i += 4) {
		if (!(i & 0xf))
			printf("\n0x%x : ", base + i);
		printf("%x ", *(unsigned int *)(base + i));
	}
	printf("\n");
}



int mmc_read_info(struct mmc *mmc, void *buffer, unsigned short length)
{

	int ret = 0;
	int i = 0;
	int retry_read = 0;
	u32 sum = 0;
	u32 add_sum = 0;
	struct sunxi_sdmmc_parameter_region *pregion =
		(struct sunxi_sdmmc_parameter_region *)buffer;

retry:
	ret = mmc_read_blocks(mmc, buffer, SUNXI_SDMMC_PARAMETER_REGION_LBA_START, length);
	if (ret < 0) {
		printf("%s %d:read region parameter fail, %s \n", __func__, __LINE__,
					(retry_read < 3) ? "retry more time" : "go err");
		dumphex32("info", buffer, 16);
		if (retry_read < 3) {
			retry_read++;
			goto retry;
		} else
			goto err;
	}

	/*check magic and sum*/
	if (pregion->header.magic == SDMMC_PARAMETER_MAGIC) {
		/*add_sum don't participate in check sum verificaton*/
		add_sum = pregion->header.add_sum;
		pregion->header.add_sum = 0;
		sum = 0;
		for (i = 0; i < pregion->header.length; i++) {
			sum += ((unsigned char *)buffer)[i];
		}

		if (sum != add_sum) {
			printf("%s %d:region add sum(%x) is not right(%x), %s \n",
					__func__, __LINE__, sum, add_sum,
					(retry_read < 3) ? "retry more time" : "go err");
			if (retry_read < 3) {
				retry_read++;
				goto retry;
			} else
				goto err;
		}
	} else {
		printf("%s %d:region magic is not right, %s %x\n", __func__, __LINE__,
				(retry_read < 3) ? "retry more time" : "go err", pregion->header.magic);
		if (retry_read < 3) {
			retry_read++;
			goto retry;
		} else {
			dumphex32("info", buffer, 16);
			goto err;
		}
	}


	mmcinfo("RMCA OK!\n");
	return 0;

err:
	mmcinfo("RMCA FAIL!\n");
	return -1;
}
int mmc_startup(struct mmc *mmc)
{
	int err;
	u32 mult, freq;
	u64 cmult, csize, capacity;
	struct mmc_cmd cmd;
	char ext_csd[512];
	int timeout      = 1000;
	char *spd_name[] = { "DS26/SDR12", "HSSDR52/SDR25", "HSDDR52/DDR50",
			     "HS200/SDR104", "HS400" };

	/* Put the Card in Identify Mode */
	cmd.cmdidx =
		mmc_host_is_spi(mmc) ?
			MMC_CMD_SEND_CID :
			MMC_CMD_ALL_SEND_CID; /* cmd not supported in spi */
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg    = 0;
	cmd.flags     = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err) {
		mmcinfo("mmc %u Put the Card in Identify Mode failed\n",
			mmc->control_num);
		return err;
	}

	memcpy(mmc->cid, cmd.response, 16);

	/*
	 * For MMC cards, set the Relative Address.
	 * For SD cards, get the Relatvie Address.
	 * This also puts the cards into Standby State
	 */
	if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
		cmd.cmdidx    = SD_CMD_SEND_RELATIVE_ADDR;
		cmd.cmdarg    = mmc->rca << 16;
		cmd.resp_type = MMC_RSP_R6;
		cmd.flags     = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err) {
			mmcinfo("mmc %u send rca failed\n", mmc->control_num);
			return err;
		}

		if (IS_SD(mmc))
			mmc->rca = (cmd.response[0] >> 16) & 0xffff;
	}

	/* Get the Card-Specific Data */
	cmd.cmdidx    = MMC_CMD_SEND_CSD;
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg    = mmc->rca << 16;
	cmd.flags     = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	/* Waiting for the ready status */
	mmc_send_status(mmc, timeout);

	if (err) {
		mmcinfo("mmc %u get csd failed\n", mmc->control_num);
		return err;
	}

	mmc->csd[0] = cmd.response[0];
	mmc->csd[1] = cmd.response[1];
	mmc->csd[2] = cmd.response[2];
	mmc->csd[3] = cmd.response[3];

	if (mmc->version == MMC_VERSION_UNKNOWN) {
		int version = (cmd.response[0] >> 26) & 0xf;

		switch (version) {
		case 0:
			mmc->version = MMC_VERSION_1_2;
			break;
		case 1:
			mmc->version = MMC_VERSION_1_4;
			break;
		case 2:
			mmc->version = MMC_VERSION_2_2;
			break;
		case 3:
			mmc->version = MMC_VERSION_3;
			break;
		case 4:
			mmc->version = MMC_VERSION_4;
			break;
		default:
			mmc->version = MMC_VERSION_1_2;
			break;
		}
	}

	/* divide frequency by 10, since the mults are 10x bigger */
	freq = fbase[(cmd.response[0] & 0x7)];
	mult = multipliers[((cmd.response[0] >> 3) & 0xf)];

	mmc->tran_speed = freq * mult;

	mmc->read_bl_len = 1 << ((cmd.response[1] >> 16) & 0xf);

	if (IS_SD(mmc))
		mmc->write_bl_len = mmc->read_bl_len;
	else
		mmc->write_bl_len = 1 << ((cmd.response[3] >> 22) & 0xf);

	if (mmc->high_capacity) {
		csize = (mmc->csd[1] & 0x3f) << 16 |
			(mmc->csd[2] & 0xffff0000) >> 16;
		cmult = 8;
	} else {
		csize = (mmc->csd[1] & 0x3ff) << 2 |
			(mmc->csd[2] & 0xc0000000) >> 30;
		cmult = (mmc->csd[2] & 0x00038000) >> 15;
	}

	mmc->capacity = (csize + 1) << (cmult + 2);
	mmc->capacity *= mmc->read_bl_len;

	if (mmc->read_bl_len > 512)
		mmc->read_bl_len = 512;

	if (mmc->write_bl_len > 512)
		mmc->write_bl_len = 512;


	/* Select the card, and put it into Transfer Mode */
	if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
		cmd.cmdidx    = MMC_CMD_SELECT_CARD;
		cmd.resp_type = MMC_RSP_R1b;
		cmd.cmdarg    = mmc->rca << 16;
		cmd.flags     = 0;
		err	   = mmc_send_cmd(mmc, &cmd, NULL);

		if (err) {
			mmcinfo("Select the card failed\n");
			return err;
		}
	}

	if (!IS_SD(mmc)) {
		/*read mmc timing info, first OTA would fail*/
		err = mmc_read_info(mmc, mmc_arg_addr, SUNXI_SDMMC_PARAMETER_REGION_SIZE_BYTE >> 9);
		if (err < 0)
			mmcinfo("mmc read timing info fail\n");

		/*update host caps from timing info*/
		mmc_update_host_caps_r(mmc->control_num);
	}


	mmc_set_clock(mmc, 25000000);
	/*
	 * For SD, its erase group is always one sector
	 */
	mmc->erase_grp_size = 1;
	mmc->part_config    = MMCPART_NOAVAILABLE;
	if (!IS_SD(mmc) && (mmc->version >= MMC_VERSION_4)) {
		/* check  ext_csd version and capacity */
		err = mmc_send_ext_csd(mmc, ext_csd);
		if (!err) {
			/* update mmc version */
			switch (ext_csd[192]) {
			case 0:
				mmc->version = MMC_VERSION_4;
				break;
			case 1:
				mmc->version = MMC_VERSION_4_1;
				break;
			case 2:
				mmc->version = MMC_VERSION_4_2;
				break;
			case 3:
				mmc->version = MMC_VERSION_4_3;
				break;
			case 5:
				mmc->version = MMC_VERSION_4_41;
				break;
			case 6:
				mmc->version = MMC_VERSION_4_5;
				break;
			case 7:
				mmc->version = MMC_VERSION_5_0;
				break;
			case 8:
				mmc->version = MMC_VERSION_5_1;
				break;
			}
		}

		if (!err & (ext_csd[192] >= 2)) {
			/*
			 * According to the JEDEC Standard, the value of
			 * ext_csd's capacity is valid if the value is more
			 * than 2GB
			 */
			capacity = ext_csd[212] << 0 | ext_csd[213] << 8 |
				   ext_csd[214] << 16 | ext_csd[215] << 24;
			capacity *= 512;
			if ((capacity >> 20) > 2 * 1024)
				mmc->capacity = capacity;
		}

		/*
		 * Check whether GROUP_DEF is set, if yes, read out
		 * group size from ext_csd directly, or calculate
		 * the group size from the csd value.
		 */
		if (ext_csd[175])
			mmc->erase_grp_size = ext_csd[224] * 512 * 1024;
		else {
			int erase_gsz, erase_gmul;
			erase_gsz  = (mmc->csd[2] & 0x00007c00) >> 10;
			erase_gmul = (mmc->csd[2] & 0x000003e0) >> 5;
			mmc->erase_grp_size =
				(erase_gsz + 1) * (erase_gmul + 1);
		}

		/* store the partition info of emmc */
		if (ext_csd[160] & PART_SUPPORT)
			mmc->part_config = ext_csd[179];
	}

	if (IS_SD(mmc))
		err = sd_change_freq(mmc);
	else
		err = mmc_change_freq(mmc);

	if (err) {
		mmcinfo("mmc %u Change speed mode failed\n", mmc->control_num);
		return err;
	}

	/* for re-update sample phase */
	err = mmc_update_phase(mmc);
	if (err) {
		mmcinfo("update clock failed\n");
		return err;
	}

	mmcdbg("mmc->card_caps 0x%x, ddr caps:0x%x\n", mmc->card_caps,
	       mmc->card_caps & MMC_MODE_DDR_52MHz);
	/* Restrict card's capabilities by what the host can do */
	mmc->card_caps &= mmc->host_caps;
	mmcdbg("mmc->card_caps 0x%x, ddr caps:0x%x\n", mmc->card_caps,
	       mmc->card_caps & MMC_MODE_DDR_52MHz);
	if (!(mmc->card_caps & MMC_MODE_DDR_52MHz) && !IS_SD(mmc)) {
		if (mmc->speed_mode == HSDDR52_DDR50)
			mmc->speed_mode = HSSDR52_SDR25;
		else
			mmc->speed_mode = DS26_SDR12;
	}

	if (IS_SD(mmc)) {
		if (mmc->card_caps & MMC_MODE_4BIT) {
			cmd.cmdidx    = MMC_CMD_APP_CMD;
			cmd.resp_type = MMC_RSP_R1;
			cmd.cmdarg    = mmc->rca << 16;
			cmd.flags     = 0;

			err = mmc_send_cmd(mmc, &cmd, NULL);
			if (err) {
				mmcinfo("mmc %u send app cmd failed\n",
					mmc->control_num);
				return err;
			}

			cmd.cmdidx    = SD_CMD_APP_SET_BUS_WIDTH;
			cmd.resp_type = MMC_RSP_R1;
			cmd.cmdarg    = 2;
			cmd.flags     = 0;
			err	   = mmc_send_cmd(mmc, &cmd, NULL);
			if (err) {
				mmcinfo("mmc %u sd set bus width failed\n",
					mmc->control_num);
				return err;
			}
			mmc_set_bus_width(mmc, 4);
		}

		if (mmc->card_caps & MMC_MODE_HS)
			mmc->tran_speed = 50000000;
		else
			mmc->tran_speed = 25000000;
	} else {
		if (mmc->card_caps & MMC_MODE_8BIT) {
			/* Set the card to use 8 bit */
			if ((mmc->card_caps & MMC_MODE_DDR_52MHz)) {
				mmcdbg("8bit ddr!!! \n");
				/* Set the card to use 8 bit ddr */
				err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
						 EXT_CSD_BUS_WIDTH,
						 EXT_CSD_BUS_DDR_8);
				if (err) {
					mmcinfo("mmc %u switch bus width failed\n",
						mmc->control_num);
					return err;
				}
				/*mmc_set_bus_mode(mmc,1); */
				mmc_set_bus_width(mmc, 8);
			} else {
				/* Set the card to use 8 bit */
				err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
						 EXT_CSD_BUS_WIDTH,
						 EXT_CSD_BUS_WIDTH_8);

				if (err) {
					mmcinfo("mmc %u switch bus width8 failed\n",
						mmc->control_num);
					return err;
				}
				mmc_set_bus_width(mmc, 8);
			}
		} else if (mmc->card_caps & MMC_MODE_4BIT) {
			if ((mmc->card_caps & MMC_MODE_DDR_52MHz)) {
				mmcdbg("4bit bus ddr!!! \n");
				/* Set the card to use 4 bit ddr */
				err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
						 EXT_CSD_BUS_WIDTH,
						 EXT_CSD_BUS_DDR_4);
				if (err) {
					mmcinfo("mmc %u switch bus width failed\n",
						mmc->control_num);
					return err;
				}
				/*mmc_set_bus_mode(mmc,1); */
				mmc_set_bus_width(mmc, 4);
			} else {
				/* Set the card to use 4 bit */
				err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
						 EXT_CSD_BUS_WIDTH,
						 EXT_CSD_BUS_WIDTH_4);
				if (err) {
					mmcinfo("mmc %u switch bus width failed\n",
						mmc->control_num);
					return err;
				}
				mmc_set_bus_width(mmc, 4);
			}
		}

		if (mmc->card_caps & MMC_MODE_DDR_52MHz) {
			mmc->tran_speed = 52000000;
		} else if (mmc->card_caps & MMC_MODE_HS) {
			if (mmc->card_caps & MMC_MODE_HS_52MHz)
				mmc->tran_speed = 52000000;
			else
				mmc->tran_speed = 26000000;
		} else {
			mmc->tran_speed = 26000000;
		}
	}
	mmcdbg("%s: set clock %u\n", __FUNCTION__, mmc->tran_speed);
	mmc_set_clock(mmc, mmc->tran_speed);

	/* fill in device description */
	mmc->blksz = mmc->read_bl_len;
	/*mmc->lba = mmc->capacity/mmc->read_bl_len;*/   /*for compiler error */
	mmc->lba = mmc->capacity >> 9;

	if (!IS_SD(mmc)) {
		switch (mmc->version) {
		case MMC_VERSION_1_2:
			mmcinfo("MMC 1.2\n");
			break;
		case MMC_VERSION_1_4:
			mmcinfo("MMC 1.4\n");
			break;
		case MMC_VERSION_2_2:
			mmcinfo("MMC 2.2\n");
			break;
		case MMC_VERSION_3:
			mmcinfo("MMC 3.0\n");
			break;
		case MMC_VERSION_4:
			mmcinfo("MMC 4.0\n");
			break;
		case MMC_VERSION_4_1:
			mmcinfo("MMC 4.1\n");
			break;
		case MMC_VERSION_4_2:
			mmcinfo("MMC 4.2\n");
			break;
		case MMC_VERSION_4_3:
			mmcinfo("MMC 4.3\n");
			break;
		case MMC_VERSION_4_41:
			mmcinfo("MMC 4.41\n");
			break;
		case MMC_VERSION_4_5:
			mmcinfo("MMC 4.5\n");
			break;
		case MMC_VERSION_5_0:
			mmcinfo("MMC 5.0\n");
			break;
		case MMC_VERSION_5_1:
			mmcinfo("MMC 5.1\n");
			break;
		default:
			mmcinfo("Unknow MMC ver\n");
			break;
		}
	}

	mmcinfo("%s %u bit\n", spd_name[mmc->speed_mode], mmc->bus_width);
	mmcinfo("%u Hz\n", mmc->clock);
	mmcinfo("%u MB\n", mmc->lba >> 11);
	return 0;
}

int mmc_send_if_cond(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = SD_CMD_SEND_IF_COND;
	/* We set the bit if the host supports voltages between 2.7 and 3.6 V */
	cmd.cmdarg    = ((mmc->voltages & 0xff8000) != 0) << 8 | 0xaa;
	cmd.resp_type = MMC_RSP_R7;
	cmd.flags     = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err) {
		mmcinfo("mmc %u send if cond failed\n", mmc->control_num);
		return err;
	}

	if ((cmd.response[0] & 0xff) != 0xaa)
		return UNUSABLE_ERR;
	else
		mmc->version = SD_VERSION_2;

	return 0;
}

int mmc_init(struct mmc *mmc)
{
	int err;
	/*
	 *struct boot_sdmmc_private_info_t *priv_info =
	 *        (struct boot_sdmmc_private_info_t
	 *                 *)(mmc_config_addr + SDMMC_PRIV_INFO_ADDR_OFFSET);
	 */

	struct boot_sdmmc_private_info_t *priv_info =
		&((struct sunxi_sdmmc_parameter_region *)mmc_arg_addr)->info;

	if (mmc->has_init) {
		mmcinfo("mmc %u Has init\n", mmc->control_num);
		return 0;
	}

	err = mmc->init(mmc);
	if (err) {
		mmcinfo("mmc %u host init failed\n", mmc->control_num);
		return err;
	}
	mmc_set_bus_width(mmc, 1);
	mmc_set_clock(mmc, 1);

retry_card:
	/* Reset the Card */
	err = mmc_go_idle(mmc);
	if (err) {
		mmcinfo("mmc %u reset card failed\n", mmc->control_num);
		return err;
	}

	/* The internal partition reset to user partition(0) at every CMD0 */
	mmc->part_num = 0;

	if (priv_info->card_type == CARD_TYPE_SD) {
		mmcinfo("***Try SD card %u***\n", mmc->control_num);
		/* Test for SD version 2 */
		err = mmc_send_if_cond(mmc);

		/* Now try to get the SD card's operating condition */
		err = sd_send_op_cond(mmc);

		if (err) {
			mmcinfo("SD card %u Card did not respond to voltage select!\n",
				mmc->control_num);
			mmcinfo("***SD/MMC %u init error!!!***\n",
				mmc->control_num);
			return UNUSABLE_ERR;
		}
	} else if (priv_info->card_type == CARD_TYPE_MMC) {
		/* If the command timed out, we check for an MMC card */
		mmcinfo("***Try MMC card %u***\n", mmc->control_num);
		err = mmc_send_op_cond(mmc);

		if (err) {
			mmcinfo("MMC card %u Card did not respond to voltage select!\n",
				mmc->control_num);
			mmcinfo("***SD/MMC %u init error!!!***\n",
				mmc->control_num);
			return UNUSABLE_ERR;
		}
	} else if (mmc->control_num == 2) {
		mmcinfo("Wrong media type 0x%x, but host sdc2, try mmc first\n",
			priv_info->card_type);
		/* If the command timed out, we check for an MMC card */
		mmcinfo("***Try MMC card %u***\n", mmc->control_num);

		err = mmc_send_op_cond(mmc);
		if (err) {
			mmcinfo("MMC card %u Card did not respond to voltage select!\n",
				mmc->control_num);
			mmcinfo("***SD/MMC %u init error!!!***\n",
				mmc->control_num);
			priv_info->card_type = CARD_TYPE_SD;
			goto retry_card;
		}
	} else {
		mmcinfo("Wrong media type 0x%x\n", priv_info->card_type);

		mmcinfo("***Try SD card %u***\n", mmc->control_num);
		/* Test for SD version 2 */
		err = mmc_send_if_cond(mmc);

		/* Now try to get the SD card's operating condition */
		err = sd_send_op_cond(mmc);

		/* If the command timed out, we check for an MMC card */
		if (err) {
			mmcinfo("***Try MMC card %u***\n", mmc->control_num);
			err = mmc_send_op_cond(mmc);

			if (err) {
				mmcinfo("mmc %u Card did not respond to voltage select!\n",
					mmc->control_num);
				mmcinfo("***SD/MMC %u init error!!!***\n",
					mmc->control_num);
				return UNUSABLE_ERR;
			}
		}
	}

	err = mmc_startup(mmc);
	if (err) {
		mmcinfo("***SD/MMC %u init error!!!***\n", mmc->control_num);
		mmc->has_init = 0;
	} else {
		mmc->has_init = 1;
		mmcinfo("***SD/MMC %u init OK!!!***\n", mmc->control_num);
	}

	return err;
}

int mmc_register(int dev_num, struct mmc *mmc)
{
	mmc_devices[dev_num] = mmc;

	if (!mmc->b_max)
		mmc->b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	return mmc_init(mmc);
}

int mmc_unregister(int dev_num)
{
	mmc_devices[dev_num] = NULL;
	mmcdbg("mmc%d unregister\n", dev_num);
	return 0;
}
