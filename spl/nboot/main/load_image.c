/*
 * (C) Copyright 2018-2020
* SPDX-License-Identifier:	GPL-2.0+
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 *
 */

#include <common.h>
#include <spare_head.h>
#include <nand_boot0.h>
#include <private_toc.h>
#include <private_boot0.h>
#include <private_uboot.h>
#include <private_tee.h>
#include <private_atf.h>
#include <u-boot/zlib.h>
#include <lzma/LzmaTools.h>
#include <u-boot/lz4.h>

typedef struct _sunxi_image_head
{
	unsigned int  jump_instruction;
	unsigned char magic[MAGIC_SIZE];
	unsigned int  res1;
	unsigned int  res2;
	unsigned int  res3;
	unsigned int  res4;
	unsigned char res5[8];
	unsigned char res6[8];
	int           run_addr;
}sunxi_image_head;


extern const boot0_file_head_t  BT0_head;

int toc1_flash_read(u32 start_sector, u32 blkcnt, void *buff)
{
	void __iomem *addr = sunxi_get_iobase(CONFIG_BOOTPKG_BASE + 512 * start_sector);
	memcpy(buff, (addr), 512 * blkcnt);

	return blkcnt;
}

phys_addr_t toc1_get_image_addr(u32 start_sector)
{
	sunxi_image_head *image_head = (sunxi_image_head *)(sunxi_get_iobase(CONFIG_BOOTPKG_BASE + 512 * start_sector));

	return image_head->run_addr;
}

int load_image(phys_addr_t *uboot_base, phys_addr_t *optee_base, \
		phys_addr_t *monitor_base, phys_addr_t *rtos_base, phys_addr_t *opensbi_base)
{
	int i;
	//int len;
	__maybe_unused void *dram_para_addr = (void *)BT0_head.prvt_head.dram_para;
	phys_addr_t image_base;
	void __iomem *bootpkg_base = sunxi_get_iobase(CONFIG_BOOTPKG_BASE);

	struct sbrom_toc1_head_info  *toc1_head = NULL;
	struct sbrom_toc1_item_info  *item_head = NULL;

	struct sbrom_toc1_item_info  *toc1_item = NULL;

	toc1_head = (struct sbrom_toc1_head_info *)bootpkg_base;
	item_head = (struct sbrom_toc1_item_info *)(bootpkg_base + sizeof(struct sbrom_toc1_head_info));

#ifdef BOOT_DEBUG
	printf("*******************TOC1 Head Message*************************\n");
	printf("Toc_name          = %s\n",   toc1_head->name);
	printf("Toc_magic         = 0x%x\n", toc1_head->magic);
	printf("Toc_add_sum	      = 0x%x\n", toc1_head->add_sum);

	printf("Toc_serial_num    = 0x%x\n", toc1_head->serial_num);
	printf("Toc_status        = 0x%x\n", toc1_head->status);

	printf("Toc_items_nr      = 0x%x\n", toc1_head->items_nr);
	printf("Toc_valid_len     = 0x%x\n", toc1_head->valid_len);
	printf("TOC_MAIN_END      = 0x%x\n", toc1_head->end);
	printf("***************************************************************\n\n");
#endif
	//init
	toc1_item = item_head;
	for(i=0;i<toc1_head->items_nr;i++,toc1_item++)
	{
#ifdef BOOT_DEBUG
		printf("\n*******************TOC1 Item Message*************************\n");
		printf("Entry_name        = %s\n",   toc1_item->name);
		printf("Entry_data_offset = 0x%x\n", toc1_item->data_offset);
		printf("Entry_data_len    = 0x%x\n", toc1_item->data_len);

		printf("encrypt	          = 0x%x\n", toc1_item->encrypt);
		printf("Entry_type        = 0x%x\n", toc1_item->type);
		printf("run_addr          = 0x%x\n", toc1_item->run_addr);
		printf("index             = 0x%x\n", toc1_item->index);
		printf("Entry_end         = 0x%x\n", toc1_item->end);
		printf("***************************************************************\n\n");
#endif
		printf("Entry_name        = %s\n",   toc1_item->name);

		image_base = toc1_get_image_addr(toc1_item->data_offset/512);
		if(strncmp(toc1_item->name, ITEM_UBOOT_NAME, sizeof(ITEM_UBOOT_NAME)) == 0) {
			*uboot_base = image_base;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
		}
		else if (strncmp(toc1_item->name, ITEM_OPTEE_NAME, sizeof(ITEM_OPTEE_NAME)) == 0) {
			*optee_base = image_base;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
			struct spare_optee_head *tee_head = (struct spare_optee_head *)image_base;
			memcpy(tee_head->dram_para, BT0_head.prvt_head.dram_para, 32*sizeof(int));
			memcpy(tee_head->chipinfo, &BT0_head.prvt_head.jtag_gpio[4], 8);
		}
		else if(strncmp(toc1_item->name, ITEM_MONITOR_NAME, sizeof(ITEM_MONITOR_NAME)) == 0) {
			*monitor_base = image_base;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
			struct private_atf_head *atf_head = (struct private_atf_head *)image_base;
			memcpy(atf_head->dram_para, BT0_head.prvt_head.dram_para, 32 * sizeof(int));
			memcpy(atf_head->platform, &BT0_head.prvt_head.jtag_gpio[4], 8);
		}
		else if(strncmp(toc1_item->name, ITEM_SCP_NAME, sizeof(ITEM_SCP_NAME)) == 0) {
#ifdef SCP_SRAM_BASE
#ifdef SCP_DTS_BASE
			struct sbrom_toc1_item_info  *toc1_item_scp_dts = item_head;
			int scp_j;
			for (scp_j = 0; scp_j < toc1_head->items_nr; scp_j++, toc1_item_scp_dts++) {
				if (strncmp(toc1_item_scp_dts->name, ITEM_DTB_NAME, sizeof(ITEM_DTB_NAME)) == 0) {
					if (toc1_item_scp_dts->data_len > SCP_DTS_SIZE) {
						printf("error: dtb size larger than scp dts size\n");
					} else {
						toc1_flash_read(toc1_item_scp_dts->data_offset/512, (toc1_item_scp_dts->data_len+511)/512, (void *)SCP_DTS_BASE);
					}
					break;
				}
			}
			if (scp_j == toc1_head->items_nr)
				printf("error: dtb not found for scp\n");
#endif
#ifdef SCP_DEASSERT_BY_MONITOR
			toc1_flash_read(toc1_item->data_offset / 512,
					(SCP_SRAM_SIZE + SCP_DRAM_SIZE + 511) / 512,
					(void *)SCP_TEMP_STORE_BASE);
#else
			toc1_flash_read(toc1_item->data_offset / 512,
					(SCP_SRAM_SIZE + 511) / 512,
					(void *)SCP_SRAM_BASE);
			toc1_flash_read((toc1_item->data_offset +
					 SCP_CODE_DRAM_OFFSET) /
						512,
					(SCP_DRAM_SIZE + 511) / 512,
					(void *)SCP_DRAM_BASE);
			memcpy((void *)(SCP_SRAM_BASE + HEADER_OFFSET +
					SCP_DRAM_PARA_OFFSET),
			       dram_para_addr, SCP_DARM_PARA_NUM * sizeof(int));
			sunxi_deassert_arisc();
#endif
#endif
		}
		else if(strncmp(toc1_item->name, ITEM_DTB_NAME, sizeof(ITEM_DTB_NAME)) == 0) {
			/* note , uboot must be less than 2M*/
			image_base = *uboot_base + 2*1024*1024;
			struct private_atf_head *atf_head = (struct private_atf_head *)(sunxi_get_iobase(*monitor_base));
			atf_head->dtb_base = image_base;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
		} else if (strncmp(toc1_item->name, ITEM_DTBO_NAME, sizeof(ITEM_DTBO_NAME)) == 0) {
			image_base = *uboot_base + 1*1024*1024;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
		} else if (strncmp(toc1_item->name, ITEM_LOGO_NAME,
				   sizeof(ITEM_LOGO_NAME)) == 0) {
			image_base	    = *uboot_base + 3 * 1024 * 1024;
			*(uint *)(image_base) = toc1_item->data_len;
			toc1_flash_read(toc1_item->data_offset / 512,
					(toc1_item->data_len + 511) / 512,
					(void *)(image_base + 16));
			set_uboot_func_mask(UBOOT_FUNC_MASK_BIT_BOOTLOGO);
		} else if (strncmp(toc1_item->name, ITEM_OPENSBI_NAME, sizeof(ITEM_OPENSBI_NAME)) == 0) {
			*opensbi_base = image_base;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
		} else if (strncmp(toc1_item->name, ITEM_RTOS_NAME, sizeof(ITEM_RTOS_NAME)) == 0 ||
			strncmp(toc1_item->name, ITEM_MELIS_NAME, sizeof(ITEM_MELIS_NAME)) == 0) {
			*rtos_base = image_base;
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base);
		} else if (strncmp(toc1_item->name, ITEM_MELIS_CONFIG_NAME, sizeof(ITEM_MELIS_CONFIG_NAME)) == 0) {
			image_base = 0x43000000; /* hardcode here, as we can't get image_base from gz header */
			printf("image_base:%x\n", image_base);
			memcpy((void *)image_base, &toc1_item->data_len, sizeof(toc1_item->data_len));
			toc1_flash_read(toc1_item->data_offset/512, (toc1_item->data_len+511)/512, (void *)image_base + 512);
		}
#ifdef CFG_SUNXI_GUNZIP
		else if ((strncmp(toc1_item->name, ITEM_MELIS_GZ_NAME, sizeof(ITEM_MELIS_GZ_NAME)) == 0)) {
			image_base = 0x40000000; /* hardcode here, as we can't get image_base from gz header */
			*rtos_base = image_base;
			void *dst = (void *)image_base;
			int dstlen = *(unsigned long *)((unsigned char *)CONFIG_BOOTPKG_BASE + toc1_item->data_offset + toc1_item->data_len - 4);
			unsigned char *src = (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
			unsigned long srclen = toc1_item->data_len;
			unsigned long *lenp = &srclen;
			int ret = gunzip(dst, dstlen, src, lenp);
			if (ret) {
				printf("Error: gunzip returned %d\n", ret);
				return -1;
			}
		}
#endif
#ifdef CFG_SUNXI_LZ4
		else if ((strncmp(toc1_item->name, ITEM_MELIS_LZ4_NAME, sizeof(ITEM_MELIS_LZ4_NAME)) == 0)) {
			image_base = 0x40000000; /* hardcode here, as we can't get image_base from lz4 header */
			*rtos_base = image_base;
			void *dst = (void *)image_base;
			unsigned int dstlen = 0x800000;
			unsigned char *src = (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
			unsigned long srclen = toc1_item->data_len;
			int ret = ulz4fn(src, srclen, dst, (size_t *)&dstlen);
			if (ret) {
				printf("Error: ulz4fn returned %d\n", ret);
				return -1;
			}

		}
#endif
#ifdef CFG_SUNXI_LZMA
		else if (strncmp(toc1_item->name, ITEM_MELIS_LZMA_NAME, sizeof(ITEM_MELIS_LZMA_NAME)) == 0) {
			image_base = 0x40000000; /* hardcode here, as we can't get image_base from gz header */
			*rtos_base = image_base;
			size_t src_len = ~0U, dst_len = ~0U;
			void *dst = (void *)image_base;
			unsigned char *src = (unsigned char *)(CONFIG_BOOTPKG_BASE) + toc1_item->data_offset;
			int ret = lzmaBuffToBuffDecompress(dst, &src_len, src, dst_len);
			if (ret) {
				printf("Error: lzmaBuffToBuffDecompress returned %d\n", ret);
				return -1;
			}

		}
#endif

	}

	return 0;
}
