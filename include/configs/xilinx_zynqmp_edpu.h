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
#define CONFIG_PCA953X

#include <configs/xilinx_zynqmp.h>

#endif /* __CONFIG_ZYNQMP_EDPU_H */
