#ifndef _T6290_EDPU_H_
#define _T6290_EDPU_H_

#include <common.h>

#define T6290_EDPU_REG_HW_VER_c            (0x00)
#define T6290_EDPU_REG_VER_YEAR_c          (0x01)
#define T6290_EDPU_REG_VER_MMDD_c          (0x02)
#define T6290_EDPU_REG_EDPU_STATUS_c       (0x03)
#define T6290_EDPU_REG_AD9542_1_WR_c       (0x04)
#define T6290_EDPU_REG_AD9542_2_WR_c       (0x05)
#define T6290_EDPU_REG_AD9542_RD_c         (0x06)
#define T6290_EDPU_REG_HMC7044_WR_c        (0x07)
#define T6290_EDPU_REG_HMC7044_RD_c        (0x08)
#define T6290_EDPU_REG_OCXO_WR_c           (0x09)
#define T6290_EDPU_REG_JTAG_CHAIN_SW_c     (0x0A)
#define T6290_EDPU_REG_ERFU_INFO_c         (0x0B)         
#define T6290_EDPU_REG_USB_INFO_c          (0x0C)
#define T6290_EDPU_REG_FAN1_SPEED_c        (0x0D)
#define T6290_EDPU_REG_FAN2_SPEED_c        (0x0E)
#define T6290_EDPU_REG_FAN3_SPEED_c        (0x0F)
#define T6290_EDPU_REG_FAN4_SPEED_c        (0x10)
#define T6290_EDPU_REG_FAN_PWM_c           (0x11)
#define T6290_EDPU_REG_TEMP_TARGET_c       (0x12)
#define T6290_EDPU_REG_TEMP_MEAS_c         (0x13)
#define T6290_EDPU_REG_SYS_LED_GREEN_c     (0x14)
#define T6290_EDPU_REG_SYS_LED_RED_c       (0x15)
#define T6290_EDPU_REG_SYS_OK_c            (0x16)
#define T6290_EDPU_REG_POWER_OFF_c         (0x17)
#define T6290_EDPU_REG_SYS_RESTART_c       (0x18)
#define T6290_EDPU_REG_MODULE_RESET_c      (0x19)
#define T6290_EDPU_REG_DSP_EEPROM_WP_c     (0x1A)
#define T6290_EDPU_REG_UFM_CMD_c           (0x1B)
#define T6290_EDPU_REG_UFM_WR_DATA_c       (0x1C)
#define T6290_EDPU_REG_UFM_RD_DATA0_c      (0x1D)
#define T6290_EDPU_REG_UFM_RD_DATA1_c      (0x1E)
#define T6290_EDPU_REG_UFM_RD_DATA2_c      (0x1F)
#define T6290_EDPU_REG_UFM_RD_DATA3_c      (0x20)
#define T6290_EDPU_REG_UFM_RD_DATA4_c      (0x21)
#define T6290_EDPU_REG_UFM_RD_DATA5_c      (0x22)
#define T6290_EDPU_REG_UFM_RD_DATA6_c      (0x23)
#define T6290_EDPU_REG_UFM_RD_DATA7_c      (0x24)
#define T6290_EDPU_REG_UFM_RD_STATUS_c     (0x25)
#define T6290_EDPU_REG_SCRTCH_c            (0x26)
#define T6290_EDPU_REG_REF_CFG_c           (0x27)
#define T6290_EDPU_REG_CPLD_IRQ_c          (0x28)
#define T6290_EDPU_REG_HW_RESOURCES_c      (0x29)




#define MODULE_RESET_DSP_0				(1<<0)
#define MODULE_RESET_DSP_1				(1<<1)
#define MODULE_RESET_AD9542_0			(1<<4)
#define MODULE_RESET_AD9542_1			(1<<5)
#define MODULE_RESET_DSP_0_PHY		(1<<6)
#define MODULE_RESET_DSP_1_PHY		(1<<7)
#define MODULE_RESET_ETH_SW_0			(1<<8)
#define MODULE_RESET_ETH_SW_1			(1<<9)
#define MODULE_RESET_USB_0				(1<<10)
#define MODULE_RESET_USB_1				(1<<11)
#define MODULE_RESET_SYS_I2C			(1<<12)

  
int edpu_cpld_write(uint32_t addr, uint32_t data);
uint32_t edpu_cpld_read(uint32_t addr);


int edpu_cpld_flash_erase(void);
int edpu_cpld_flash_write(uint16_t page_addr, uint16_t* p);
int edpu_cpld_flash_read(uint32_t page_addr, uint32_t traceid, uint16_t* p);


char* edpu_cpld_version(void);
void edpu_cpld_init(void);

#endif
