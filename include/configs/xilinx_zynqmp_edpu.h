/*
 * Configuration for Welzek T6290E eDPU
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_EDPU_H
#define __CONFIG_ZYNQMP_EDPU_H

#define CONFIG_LAST_STAGE_INIT 1


#define CONFIG_SYS_I2C_MAX_HOPS		1
#define CONFIG_SYS_NUM_I2C_BUSES	9
#define CONFIG_SYS_I2C_BUSES	{ \
				{0, {I2C_NULL_HOP} }, \
				{0, {{I2C_MUX_PCA9548, 0x70, 0} } }, \
				{0, {{I2C_MUX_PCA9548, 0x70, 1} } }, \
				{0, {{I2C_MUX_PCA9548, 0x70, 2} } }, \
				{0, {{I2C_MUX_PCA9548, 0x70, 3} } }, \
				{0, {{I2C_MUX_PCA9548, 0x70, 4} } }, \
				{0, {{I2C_MUX_PCA9548, 0x70, 5} } }, \
				{0, {{I2C_MUX_PCA9548, 0x70, 6} } }, \
				{0, {{I2C_MUX_PCA9548, 0x70, 7} } }, \
				}

#define CONFIG_SYS_I2C_ZYNQ

#define CONFIG_SYS_I2C_EEPROM_ADDR        0x50    /* EEPROM at24c08        */
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN     2      /* Bytes of address        */
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS    10  /* delay of each i2c eeprom write */


#define CONFIG_EXTRA_ENV_SETTINGS \
	"image_addr=0x10000000\0" \
	"scsidev=0\0" \
	"fpga_idx=0\0" \
	"generate_file_names=" \
		"if test -e $ddr_pl_size; then " \
			"setenv pl_image_file plm\"$ddr_pl_size\"r\"$ddr_pl_rank\".bit; " \
		"fi; " \
		"if test -e $ddr_ps_size; then " \
			"setenv ps_image_file psm\"$ddr_ps_size\"r\"$ddr_ps_rank\".ub; " \
		"fi; \0" \
	"load_pl=" \
		"env delete fileaddr; " \
		"if test -e scsi $scsidev $pl_image_file; then " \
			"load scsi $scsidev $image_addr $pl_image_file; " \
		"else " \
			"if test -e scsi $scsidev fpga.bit; then " \
				"load scsi $scsidev $image_addr fpga.bit; "\
			"fi; " \
		"fi; " \
		"if test -e $fileaddr; then " \
			"fpga loadb $fpga_idx $fileaddr $filesize; " \
		"fi; \0" \
	"load_ps=" \
		"env delete fileaddr; " \
		"if test -e scsi $scsidev $ps_image_file; then " \
			"load scsi $scsidev $image_addr $ps_image_file; " \
		"else " \
			"load scsi $scsidev $image_addr image.ub; "\
		"fi; " \
		"if test -e $fileaddr; then " \
			"bootm $fileaddr; " \
		"fi; \0" \
	"sataboot=scsi scan && " \
		"run generate_file_names && " \
		"run load_pl && ; " \
		"run load_ps; \0" \
		
#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_EDPU_H */
