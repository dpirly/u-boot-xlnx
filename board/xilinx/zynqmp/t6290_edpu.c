#include <common.h>
#include <command.h>
#include <dm.h>
#include <errno.h>
#include <spi.h>
#include <i2c.h>

#include "t6290_edpu.h"

#define		DSP_EEPROM_ADDR (0x50)

static char* to_lower(char* s){
	char* r = s;
	char c;
	while (*s != '\0'){
		c = *s;
		if(c >= 'A' && c <= 'Z'){
			*s = c + 0x20;
		}
		s++;
	}
	return r;
}

/*
	ZynqMP> i2c bus
	Bus 0:	zynq_1
	Bus 1:	zynq_1->PCA9548@0x70:0 (PS-PMBUS)
	Bus 2:	zynq_1->PCA9548@0x70:1 (DSP1-EEPROM: 0x50) --!!
	Bus 3:	zynq_1->PCA9548@0x70:2 (DSP2-EEPROM: 0x50) --!!
	Bus 4:	zynq_1->PCA9548@0x70:3 (ADT75: 0x48)
	Bus 5:	zynq_1->PCA9548@0x70:4 (PS-DDR4 SPD)
	Bus 6:	zynq_1->PCA9548@0x70:5 (PL-DDR4 SPD)
	Bus 7:	zynq_1->PCA9548@0x70:6 (PS-USB-SDA)
	Bus 8:	zynq_1->PCA9548@0x70:7 (QSFP+)
*/

static bool edpu_ddr4_select_page(uint8_t page){
	uint8_t dummy;
	int ret;
	
	if(page == 0){
		ret = i2c_write(0x36, 0, 0, &dummy, 1);
	}else{
		ret = i2c_write(0x37, 0, 0, &dummy, 0);
	}
	return ret;
}

static const char* edpu_ddr4_company(uint8_t bank, uint8_t code){
	static char unknown[32];
	if (bank == 1 && code == 0x2c){
		return "Micron Technology";
	}else if (bank == 1 && code == 0xad){
		return "SK Hynix";
	}else if (bank == 2 && code == 0x98){
		return "Kingston";
	}else if(bank == 5 && code == 0xcb){
		return "A-DATA Technology";
	}else if(bank == 3 && code == 0x9E){
		return "Corsair";
	}else if(bank == 8 && code == 0x46){
		return "Gloway International";
	}else if(bank == 9 && code == 0x13){
		return "Gloway International";
	}else if((bank == 0x80 || bank == 0x00 || bank == 0x01) && code == 0xCE){
		return "Samsung";
	}else if(bank == 5 && code == 0xCD){
		return "G.Skill Intl";
	}else if(bank == 10 && code == 0x68){
		return "Kimtigo Semiconductor";
	}else if(bank == 5 && code == 0xef){
		return "Team Group Inc";
	}else if(bank == 6 && code == 0x9b){
		return "Crucial Technology";
	}else if(bank == 9 && code == 0xc8){
		return "Lenovo";
	}else if(bank == 2 && code == 0x7a){
		return "Apacer Technology";
	}
	sprintf(unknown, "Unknown(bank=%d,code=%02x)", bank, code);
	return unknown;
}

static void edpu_ddr4_probe(const char *prefix){
	static uint8_t spd[512];
	char variable[32] = {0};
	char value[32] = {0};
	uint32_t offset;
	
	uint32_t rank;
	uint32_t cap;
	uint32_t bus_width;
	uint32_t sdram_width;
	uint32_t total_memory;

	int8_t tckmin_mtb;
	int8_t tckmin_offset_ftb;
	float tckavg_min_result;
	float speed;

	/* read all page 0 contents */
	if(edpu_ddr4_select_page(0) != 0){
		printf("%s: None!\n", prefix);
		return;
	}
	for(offset=0; offset<256; offset+=32){
		i2c_read(0x51, 			/* chip address */
				offset, 				/* byte address */
				1, 							/* address length */
				&spd[offset], 	/* buffer */
				32); 						/* read length */
	}

	/* read all page 1 contents */
	if(edpu_ddr4_select_page(1) != 0){
		printf("%s: error!\n", prefix);
		return;
	}
	for(offset=0; offset<256; offset+=32){
		i2c_read(0x51,			/* chip address */
				offset, 				/* byte address */
				1,							/* address length */
				&spd[256+offset],/* buffer, have offset */
				32);						/* read length */
	}

	/* parse SPD content */
	if(spd[0x02] != 0x0c){
		printf("%s-DDR is not DDR4, except 0x0c, got 0x%x\n", prefix, spd[0x02]);
		return;
	}
	if(spd[0x03] != 0x03){
		printf("%s-DDR is not SO-DIMM, except 0x03, got 0x%x\n", prefix, spd[0x03]);
		return;
	}
	if(spd[0x06] != 0x00){
		printf("%s-DDR is unknown package type, except 0x00, got 0x%x\n", prefix, spd[0x06]);
		return;
	}

	printf("%s: %s, ", prefix, edpu_ddr4_company((spd[0x140] & 0x7f)+1, spd[0x141]));
	//printf("ID CODE:%02x%02x => bank%d, code:%02x\n", spd[0x140], spd[0x141],  (spd[0x140] & 0x7f)+1, spd[0x141]);

	// calc speed
	//Tckavg_min Result = Tck_min MTB(reg0x12) + Tck_min Offset FTB(reg0x7d)
	tckmin_mtb = (int8_t)spd[0x12];
	tckmin_offset_ftb = (int8_t)spd[0x7d];
	tckavg_min_result = tckmin_mtb*0.125 + tckmin_offset_ftb*0.001;
	speed = (2.0* (1/tckavg_min_result))*1000.0;

	// calc capcatity
	rank = ((spd[0x0c] & 0b111000) >> 3) + 1; /* 1 2 3 4 */
	bus_width = (spd[0x0d] & 0b111); 
	bus_width = 8 << bus_width; /* 8 16 32 64*/
	sdram_width = (spd[0x0c] & 0b111);
	sdram_width = 4 << sdram_width; /* 4 8 16 32 */
	cap = (spd[0x04] & 0b1111);
	cap = 256 << cap;
	/*  Total = SDRAM Capacity / 8 * Primary Bus Width / SDRAM Widht * Logical Ranks per DIMM */
	//total_memory = cap / 8 * bus_width / sdram_width * rank;
	total_memory = (cap * bus_width * rank) / (8*sdram_width);
	
	printf("%4dMB-%dMHz, Rank %d, Bus width %d, SDRAM width %d, SDRAM capcatity %dMb, ", 
			total_memory, (uint32_t)speed, rank, bus_width, sdram_width, cap);

	memcpy(value, &spd[0x149], 20);
	printf("PN:%s\n", value);

	/* export variable */
	sprintf(variable, "ddr_%s_size", prefix);
	sprintf(value, "%d", total_memory);
	env_set(to_lower(variable), value);

	sprintf(variable, "ddr_%s_rank", prefix);
	sprintf(value, "%d", rank);
	env_set(to_lower(variable), value);

	sprintf(variable, "ddr_%s_speed", prefix);
	sprintf(value, "%d", (uint32_t)speed);
	env_set(to_lower(variable), value);
}

static void edpu_ddr4_init(void){
	printf("\nMemory:\n");

	/* PS-DDR4 */
	i2c_set_bus_num(5); /*pca9548a channel 4 */
	i2c_init(100000, 0x51);
	edpu_ddr4_probe("PS");

	/* PL-DDR4 */
	i2c_set_bus_num(6); /*pca9548a channel 5 */
	edpu_ddr4_probe("PL");
}

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

	udelay(1000);

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

	udelay(1000);

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

void edpu_cpld_get_hardware_resources(uint32_t* rf_channels, uint32_t* rf_ports, uint32_t* dsp_bitmap){
	char buf[32];
	uint32_t reg = edpu_cpld_read(T6290_EDPU_REG_HW_RESOURCES_c);

	*rf_channels = (reg & 0x000f);
	*rf_ports    = (reg & 0x00f0) >> 4;
	*dsp_bitmap  = (reg & 0x0f00) >> 8;

	*rf_ports = *rf_ports + 1; /* 0~15 to 1~16 */

	printf("DSP bitmap:0x%x, RF channels:%d, RF ports:%d\n", 
			*dsp_bitmap, *rf_channels, *rf_ports);

	/* rf channel */
	sprintf(buf, "%d", *rf_channels);
	env_set("rfch", buf);

	/* rf port */
	sprintf(buf, "%d", *rf_ports);
	env_set("rfport", buf);

	/* dsp */
	sprintf(buf, "%x", *dsp_bitmap);
	env_set("dsp", buf);

	/* dsp */
	sprintf(buf, "rfc%drfp%d", *rf_channels, *rf_ports);
	env_set("hwcfg", buf);
}

static void edpu_module_reset(uint32_t en, uint32_t bits){
	uint32_t r;

	r = edpu_cpld_read(T6290_EDPU_REG_MODULE_RESET_c);

	if(en)
		r = r | bits;
	else
		r = r & ~bits;

	edpu_cpld_write(T6290_EDPU_REG_MODULE_RESET_c, r);
}

void board_poweroff(void){
	edpu_cpld_write(T6290_EDPU_REG_POWER_OFF_c, 0x7ff);
}

void board_reset(void){
	edpu_cpld_write(T6290_EDPU_REG_SYS_RESTART_c, 0x7ff);
}

#define EDPU_CPLD_FLASH_CMD_SHIFT (16)
#define EDPU_CPLD_FLASH_READ     (0b000)
#define EDPU_CPLD_FLASH_WRITE    (0b010)
#define EDPU_CPLD_FLASH_ENABLE   (0b100)
#define EDPU_CPLD_FLASH_DISABLE  (0b101)
#define EDPU_CPLD_FLASH_ERASE    (0b111)
#define EDPU_CPLD_FLASH_TRACEID  (0b110)

#define EDPU_CPLD_FLASH_PAGE_INSTRU_SN  (6)
#define EDPU_CPLD_FLASH_PAGE_BOARD_SN   (7)
#define EDPU_CPLD_FLASH_PAGE_MAC_PS     (10)
#define EDPU_CPLD_FLASH_PAGE_MAC_DSP    (11)

static int edpu_cpld_flash_ready(void){
	uint32_t i,r;
	
	/* waiting for done */
	i = 1000;
	do{
		udelay(10*1000);
		r = edpu_cpld_read(T6290_EDPU_REG_UFM_RD_STATUS_c);
		if ((r & 0x01) == 0)
			break;
	}while(i-->0);

	/* check */
	if ((r & 0x01) == 0)
		return 0;
	else
		return -EBUSY;
}

static int edpu_cpld_flash_enable(uint32_t en){

	if(0 != edpu_cpld_flash_ready())
		return -EIO;

	if(en)
		edpu_cpld_write(T6290_EDPU_REG_UFM_CMD_c, (EDPU_CPLD_FLASH_ENABLE << EDPU_CPLD_FLASH_CMD_SHIFT));
	else
		edpu_cpld_write(T6290_EDPU_REG_UFM_CMD_c, (EDPU_CPLD_FLASH_DISABLE << EDPU_CPLD_FLASH_CMD_SHIFT));

	if(0 != edpu_cpld_flash_ready())
		return -EIO;
	else
		return 0;
}

int edpu_cpld_flash_write(uint16_t page_addr, uint16_t* p){
	uint32_t i;
	uint32_t data; 

	if(p == 0)
		return -EINVAL;

	/* enable flash */
	if(0 != edpu_cpld_flash_enable(1))
		return -EIO;

	/* load data */
	for (i=0; i<8; i++){
		data = htons(*p); /* to network-order(big-ending) */
		edpu_cpld_write(T6290_EDPU_REG_UFM_WR_DATA_c, data | (i << 16));
		p++;
	}

	/* issue write */
	edpu_cpld_write(T6290_EDPU_REG_UFM_CMD_c, (EDPU_CPLD_FLASH_WRITE << EDPU_CPLD_FLASH_CMD_SHIFT) | (page_addr & 0xfff));

	/* waiting for done */
	if(0 != edpu_cpld_flash_ready())
		return -EIO;

	/* disable flash */
	if(0 != edpu_cpld_flash_enable(0))
		return -EIO;

	return 0;
}

int edpu_cpld_flash_erase(void){

	/* enable flash */
	if(0 != edpu_cpld_flash_enable(1))
		return -EIO;

	/* erase all flash */
	edpu_cpld_write(T6290_EDPU_REG_UFM_CMD_c, (EDPU_CPLD_FLASH_ERASE << EDPU_CPLD_FLASH_CMD_SHIFT));

	/* wait */
	if(0 != edpu_cpld_flash_ready())
		return -EIO;

	/* disable flash */
	if(0 != edpu_cpld_flash_enable(0))
		return -EIO;

	return 0;
}

int edpu_cpld_flash_read(uint32_t page_addr, uint32_t traceid, uint16_t* p){
	int i;
	
	if(p == 0)
		return -EINVAL;

	/* enable flash */
	if(0 != edpu_cpld_flash_enable(1))
		return -EIO;

	/* issue read */
	if(traceid)
		edpu_cpld_write(T6290_EDPU_REG_UFM_CMD_c, (EDPU_CPLD_FLASH_TRACEID << EDPU_CPLD_FLASH_CMD_SHIFT));
	else
		edpu_cpld_write(T6290_EDPU_REG_UFM_CMD_c, (page_addr & 0xfff));

	/* wait */
	if(0 != edpu_cpld_flash_ready())
		return -EIO;

	/* disable flash */
	if(0 != edpu_cpld_flash_enable(0))
		return -EIO;

	/* read the result */
	for(i=0; i<8; i++){
		*p = (edpu_cpld_read(T6290_EDPU_REG_UFM_RD_DATA0_c + i) & 0xffff);
		*p = ntohs(*p);  /* to host order */
		p++;
	}

	return 0;
}

int edpu_cpld_get_ps_mac(uint8_t* mac1, uint8_t* mac2){
	uint16_t buf[8]={0};
	int i;
	
	edpu_cpld_flash_read(EDPU_CPLD_FLASH_PAGE_MAC_PS, 0, buf);
	
	if(buf[0] != 0){
		uint8_t* p = ((uint8_t*)buf)+2;
		for(i=0; i<6; i++){
			*mac1 = *p;
			//printf("%02x:", *mac1);
			mac1++; p++;
		}
		//printf("\n");
		for(i=0; i<6; i++){
			*mac2 = *p;
			//printf("%02x:", *mac2);
			mac2++; p++;
		}
		//printf("\n");
		return 0;
	}
	
	return -EIO;
}

void edpu_init_eth_mac(void){
	static char fmt[] = "%02X:%02X:%02X:%02X:%02X:%02X";
	char ethaddr[20];
	uint8_t mac1[6],mac2[6];
	if (0 == edpu_cpld_get_ps_mac(mac1, mac2)){
		sprintf(ethaddr, fmt,
				mac1[0], mac1[1], mac1[2],
				mac1[3], mac1[4], mac1[5]);
		env_set("ethaddr", ethaddr); /* not eth0addr */

		sprintf(ethaddr, fmt,
				mac2[0], mac2[1], mac2[2],
				mac2[3], mac2[4], mac2[5]);
		env_set("eth1addr", ethaddr);
	}
}

void edpu_cpld_init(void)
{
	uint32_t rf_channels, rf_ports, dsp_bitmap;

	/* check DDR4 module */
	edpu_ddr4_init();

	/* check CPLD */
	printf("\neDPU CPLD:%s\n", edpu_cpld_version());
	edpu_init_eth_mac(); /* read and set ps ethernet mac address from CPLD*/
	edpu_cpld_get_hardware_resources(&rf_channels, &rf_ports, &dsp_bitmap);

	udelay(1*1000);

	printf("\n");
	udelay(10*1000);

}
