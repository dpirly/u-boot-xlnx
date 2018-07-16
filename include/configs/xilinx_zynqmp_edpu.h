/*
 * Configuration for Welzek T6290E eDPU
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_EDPU_H
#define __CONFIG_ZYNQMP_EDPU_H

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


#define CONFIG_EXTRA_ENV_SETTINGS \
	"image_addr=0x10000000\0" \
	"scsidev=0\0" \
	"fpga_img=fpga.bit\0" \
	"kernel_img=image.ub\0" \
	"sataboot=scsi scan && " \
		"load scsi $scsidev $image_addr $fpga_img && fpga loadb $scsidev $fileaddr $filesize && " \
		"setenv bootargs earlycon clk_ignore_unused root=/dev/sda2 rw rootwait rootfstype=ext4 && "\
		"load scsi $scsidev $image_addr $kernel_img && " \
		"bootm $fileaddr\0"
#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_EDPU_H */
