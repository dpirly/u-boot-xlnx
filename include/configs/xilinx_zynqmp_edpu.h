/*
 * Configuration for Welzek T6290E eDPU
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQMP_EDPU_H
#define __CONFIG_ZYNQMP_EDPU_H

#define CONFIG_SYS_I2C_MAX_HOPS		1
#define CONFIG_SYS_NUM_I2C_BUSES	1
#define CONFIG_SYS_I2C_BUSES	{ \
				{0, {I2C_NULL_HOP} }, \
				}

#define CONFIG_SYS_I2C_ZYNQ

#define CONFIG_EXTRA_ENV_SETTINGS \
	"image_addr=0x10000000\0" \
	"scsidev=0\0" \
	"fpga_img=fpga.bit\0" \
	"kernel_img=image.ub\0" \
	"sataboot=scsi scan && " \
		"load scsi $scsidev $image_addr $fpga_img && fpga loadb $scsidev $image_addr 28700853 && " \
		"setenv bootargs earlycon clk_ignore_unused root=/dev/sda2 rw rootwait rootfstype=ext4 && "\
		"load scsi $scsidev $image_addr $kernel_img && " \
		"bootm $image_addr\0"

#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_EDPU_H */
