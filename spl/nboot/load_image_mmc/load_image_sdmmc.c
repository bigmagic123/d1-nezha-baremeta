// * SPDX-License-Identifier:	GPL-2.0+

#include <common.h>
#include <spare_head.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <private_toc.h>
#include <mmc_boot0.h>

int mmc_config_addr;

enum {
	E_SDMMC_OK = 0,
	E_SDMMC_NUM_ERR = 1,
	E_SDMMC_INIT_ERR = 2,
	E_SDMMC_READ_ERR = 3,
	E_SDMMC_FIND_BOOT1_ERR =4,
};


typedef struct _boot_sdcard_info_t
{
	__s32	card_ctrl_num;                //总共的卡的个数
	__s32	boot_offset;                  //指定卡启动之后，逻辑和物理分区的管理
	__s32	card_no[4];                   //当前启动的卡号, 16-31:GPIO编号，0-15:实际卡控制器编号
	__s32	speed_mode[4];                //卡的速度模式，0：低速，其它：高速
	__s32	line_sel[4];                  //卡的线制，0: 1线，其它，4线
	__s32	line_count[4];                //卡使用线的个数
}
boot_sdcard_info_t;

//card num: 0-sd 1-card3 2-emmc
int get_card_num(void)
{
	int card_num = 0;

	card_num = BT0_head.boot_head.platform[0] & 0xf;
	card_num = (card_num == 1)? 3: card_num;
	return card_num;
}

void update_flash_para(phys_addr_t uboot_base)
{
	int card_num;
	struct spare_boot_head_t  *bfh = (struct spare_boot_head_t *) uboot_base;

	card_num = get_card_num();
	set_mmc_para(card_num, (void *)&BT0_head.prvt_head.storage_data, uboot_base);
#ifdef CONFIG_ARCH_SUN50IW11
	int card_type = get_card_type();
	if (card_num == 0) {
		if (card_type == CARD_TYPE_MMC)
			bfh->boot_data.storage_type = STORAGE_EMMC0;
		else
			bfh->boot_data.storage_type = STORAGE_SD;
	}
#else
	if (card_num == 0) {
		bfh->boot_data.storage_type = STORAGE_SD;
	}
#endif
	else if (card_num == 2) {
		bfh->boot_data.storage_type = STORAGE_EMMC;
	} else if (card_num == 3) {
		bfh->boot_data.storage_type = STORAGE_EMMC3;
	}
}


int load_toc1_from_sdmmc(char *buf)
{
	u8  *tmp_buff = (u8 *)CONFIG_BOOTPKG_BASE;
	uint total_size;
	sbrom_toc1_head_info_t	*toc1_head;
	int  card_no;
	int ret =0;
	int start_sector,i;
	int error_num = E_SDMMC_OK;
	int start_sectors[4] = {UBOOT_START_SECTOR_IN_SDMMC,UBOOT_BACKUP_START_SECTOR_IN_SDMMC,0,0};
	boot_sdcard_info_t  *sdcard_info = (boot_sdcard_info_t *)buf;
	mmc_config_addr = (u32)((phys_addr_t)(BT0_head.prvt_head.storage_data));

	card_no = get_card_num();

	printf("card no is %d\n", card_no);
	if(card_no < 0)
	{
		error_num = E_SDMMC_NUM_ERR;
		goto __ERROR_EXIT;
	}

	if(!sdcard_info->line_sel[card_no])
	{
		sdcard_info->line_sel[card_no] = 4;
	}
	printf("sdcard %d line count %d\n", card_no, sdcard_info->line_sel[card_no] );

	if( sunxi_mmc_init(card_no, sdcard_info->line_sel[card_no], BT0_head.prvt_head.storage_gpio, 16) == -1) 
	{
		error_num = E_SDMMC_INIT_ERR;
		goto __ERROR_EXIT;;
	}

	for(i=0; i < 4; i++)
	{
		start_sector = start_sectors[i];
		tmp_buff = (u8 *)CONFIG_BOOTPKG_BASE;
		if(start_sector == 0)
		{
			error_num = E_SDMMC_FIND_BOOT1_ERR;
			goto __ERROR_EXIT;
		}
		ret = mmc_bread(card_no, start_sector, 64, tmp_buff);
		if(!ret)
		{
			error_num = E_SDMMC_READ_ERR;
			goto __ERROR_EXIT;
		}
		toc1_head = (struct sbrom_toc1_head_info *)tmp_buff;
		if(toc1_head->magic != TOC_MAIN_INFO_MAGIC)
		{
			printf("error:bad magic.\n");
			continue;
		}
		total_size = toc1_head->valid_len;
		if(total_size > 64 * 512)
		{
			tmp_buff += 64*512;
			ret = mmc_bread(card_no, start_sector + 64, (total_size - 64*512 + 511)/512, tmp_buff);
			if(!ret)
			{
				error_num = E_SDMMC_READ_ERR;
				goto __ERROR_EXIT;
			}
		}

		if( verify_addsum( (__u32 *)CONFIG_BOOTPKG_BASE, total_size) != 0 )
		{
			printf("error:bad checksum.\n");
			continue;
		}
		break;
	}
	printf("Loading boot-pkg Succeed(index=%d).\n",
		(BT0_head.boot_head.platform[0] & 0xf0)>>4);
	sunxi_mmc_exit( card_no, BT0_head.prvt_head.storage_gpio, 16 );
	return 0;

__ERROR_EXIT:
	printf("Loading boot-pkg fail(error=%d)\n",error_num);
	sunxi_mmc_exit(card_no, BT0_head.prvt_head.storage_gpio, 16 );
	return -1;

}


int load_package(void)
{
	//memcpy((void *)DRAM_PARA_STORE_ADDR, (void *)BT0_head.prvt_head.dram_para, 
	//	SUNXI_DRAM_PARA_MAX * 4);

	return load_toc1_from_sdmmc((char *)BT0_head.prvt_head.storage_data);
}
