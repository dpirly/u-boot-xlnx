#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <spi.h>

#include "t6290_edpu.h"

static int edpu_spi_xfer(uint32_t dout, uint32_t *din)
{
	unsigned int bus = 1;
	unsigned int cs = 0;
	unsigned int mode = SPI_MODE_0;
#ifdef CONFIG_DM_SPI
		char name[30], *str;
		struct udevice *dev;
#endif
	struct spi_slave *slave;
	int ret = 0;

#ifdef CONFIG_DM_SPI
	snprintf(name, sizeof(name), "generic_%d:%d", bus, cs);
	str = strdup(name);
	if (!str)
		return -ENOMEM;
	ret = spi_get_bus_and_cs(bus, cs, 1000000, mode, "spi_generic_drv",
				 str, &dev, &slave);
	if (ret)
		return ret;
#else
	slave = spi_setup_slave(bus, cs, 1000000, mode);
	if (!slave) {
		printf("Invalid device %d:%d\n", bus, cs);
		return -EINVAL;
	}
#endif

	ret = spi_claim_bus(slave);
	if (ret)
		goto done;

	ret = spi_xfer(slave, 32, &dout, din,
					 SPI_XFER_BEGIN | SPI_XFER_END);

#ifndef CONFIG_DM_SPI
		/* We don't get an error code in this case */
		if (ret)
			ret = -EIO;
#endif
		if (ret) {
			printf("Error %d during SPI transaction\n", ret);
		}
	done:
		spi_release_bus(slave);
#ifndef CONFIG_DM_SPI
		spi_free_slave(slave);
#endif

	return ret;
}

int edpu_cpld_write(uint32_t addr, uint32_t data)
{
	uint32_t data_in;
	uint32_t data_out;
	
	//printf("edpu_cpld_write(0x%04x, 0x%06x)\n", addr, data);
	
	data_out = (addr << 24) | data;
	data_out = htonl(data_out);

	return edpu_spi_xfer(data_out, &data_in);
}

uint32_t edpu_cpld_read(uint32_t addr)
{
	uint32_t data_in;
	uint32_t data_out;
	int ret;
	uint32_t r;
	
	data_out = (1 << 31) | (addr << 24);
	data_out = htonl(data_out);

	ret = edpu_spi_xfer(data_out, &data_in);

	if(ret){
		printf("edpu_cpld_read error:%d\n", ret);
	}

	r = ntohl(data_in) & 0xffffff;
	//printf("edpu_cpld_read(0x%06x) = 0x%04x)\n", addr, r);
	
	return r;
}

char* edpu_cpld_version(void){
	static char cpld_ver[32];

	sprintf(cpld_ver, "%x%04x-%x", edpu_cpld_read(T6290_EDPU_REG_VER_YEAR_c),
		edpu_cpld_read(T6290_EDPU_REG_VER_MMDD_c),
		edpu_cpld_read(T6290_EDPU_REG_HW_VER_c));
	return cpld_ver;
}

void board_poweroff(void){
	edpu_cpld_write(T6290_EDPU_REG_POWER_OFF_c, 0xffff);
}

void board_reset(void){
	edpu_cpld_write(T6290_EDPU_REG_SYS_RESTART_c, 0xffff);
}

#define EDPU_CPLD_FLASH_READ     (0b000)
#define EDPU_CPLD_FLASH_WRITE    (0b010)
#define EDPU_CPLD_FLASH_ERASE    (0b111)
#define EDPU_CPLD_FLASH_TRACEID  (0b110)

int edpu_cpld_flash_write(uint16_t page_addr, uint32_t* p){
	uint32_t r;
	uint32_t i;

	if(p == 0)
		return -EINVAL;

	/* load data */
	for (i=0; i<8; i++){
		edpu_cpld_write(T6290_EDPU_REG_UFM_WR_DATA_c, *p + (i << 16));
		p++;
	}

	/* issue write */
	edpu_cpld_write(T6290_EDPU_REG_UFM_CMD_c, (EDPU_CPLD_FLASH_WRITE << 12) | (page_addr & 0xfff));

	/* waiting for done */
	i = 10;
	do{
		udelay(10);
		r = edpu_cpld_read(T6290_EDPU_REG_UFM_RD_STATUS_c);
		if ((r & 0x01) == 0)
			break;
	}while(i-->0);

	/* check */
	if ((r & 0x01) == 0)
		return 0;
	else
		return -EIO;
}

int edpu_cpld_flash_erase(void){
	uint32_t i;
	uint32_t r;

	edpu_cpld_write(T6290_EDPU_REG_UFM_CMD_c, (EDPU_CPLD_FLASH_ERASE <<12));

	/* wait */
	i = 10;
	do{
		udelay(100);
		r = edpu_cpld_read(T6290_EDPU_REG_UFM_RD_STATUS_c);
		if ((r & 0x01) == 0)
			break;
	}while(i-->0);

	return 0;
}

int edpu_cpld_flash_read(uint32_t page_addr, uint32_t traceid, uint32_t* p){
	uint32_t r;
	uint32_t i;

	if(p == 0)
		return -EINVAL;

	/* issue read */
	if(traceid)
		edpu_cpld_write(T6290_EDPU_REG_UFM_CMD_c, (EDPU_CPLD_FLASH_TRACEID <<12));
	else
		edpu_cpld_write(T6290_EDPU_REG_UFM_CMD_c, (page_addr & 0xfff));

	/* wait */
	i = 10;
	do{
		udelay(10);
		r = edpu_cpld_read(T6290_EDPU_REG_UFM_RD_STATUS_c);
		if ((r & 0x01) == 0)
			break;
	}while(i-->0);

	/* read the result */
	if ((r & 0x01) == 0){
		for(i=0; i<8; i++){
			*p = edpu_cpld_read(T6290_EDPU_REG_UFM_RD_DATA0_c + i);
			p++;
		}
		return 0;
	}else{
		return -EIO;
	}
}

void ufm_flash_test(void){
	static uint32_t buf[8];
	uint32_t i;

	for(i=0; i<8; i++)
		buf[i] = i;

	/*
	edpu_cpld_flash_write(2, buf);

	for(i=0; i<8; i++)
		buf[i] = 0x10 + i;

	edpu_cpld_flash_write(3, buf);
*/
	/* read serial number */
	if(0 == edpu_cpld_flash_read(2, 0, buf)){
		int i;
		for (i=0; i<8; i++){
			printf("%06x ", buf[i]);
		}
		printf("\n");
	}
}
void edpu_cpld_init(void)
{
	printf("\neDPU CPLD:%s\n", edpu_cpld_version());

	/* reset */
	//edpu_cpld_write(T6290_EDPU_REG_MODULE_RESET_c, 0xffff);
	//udelay(200*1000);
	//edpu_cpld_write(T6290_EDPU_REG_MODULE_RESET_c, 0);
	//udelay(2000*1000);

	//edpu_cpld_flash_erase();
	
	//ufm_flash_test();
}
