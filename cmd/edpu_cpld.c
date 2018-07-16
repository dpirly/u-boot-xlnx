#include <common.h>
#include <command.h>

#include <t6290_edpu.h>

int do_edpu_pld (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint16_t buf[8]={0};
	int ret;
	unsigned long addr = 0;
	unsigned long data = 0;
	unsigned int is_read = 0;
	unsigned int is_flash = 0;
	unsigned int is_erase = 0;

	printf("argc=%d\n", argc);
	
	if(argc >= 4){/*write, eg: edpu_pld w 0x12 0x34*/
		if(argv[1][0] == 'w' || 
			(argv[1][0] == 'f' && argv[1][1] == 'w')){
			addr = simple_strtoul(argv[2], NULL, 16); /* address */
			if(argc == 4)
				data = simple_strtoul(argv[3], NULL, 16); /* data */
			else if (argc == (3 + 8)){
				int i;
				for(i=0; i<8; i++)
					buf[i] = simple_strtoul(argv[3+i], NULL, 16); /* flash write data */
			}else
				return CMD_RET_USAGE;
		}else
			return CMD_RET_USAGE;
	}else if(argc == 3){ /* read, eg: edpu_pld r 0x12 */
	
		if(argv[1][0] != 'r' || 
			(argv[1][0] == 'f' && argv[1][1] == 'r')){
			is_read = 1;
			addr = simple_strtoul(argv[2], NULL, 16); /* address */
		}else
			return CMD_RET_USAGE;
	}else if(argc == 2){ /* edpu_pld fe */
		if (argv[1][0] == 'f' && argv[1][1] == 'e'){
			is_erase = 1;
			is_flash = 1;
		}
	}else{
		return CMD_RET_USAGE;
	}

	if (argv[1][0] == 'f')
		is_flash = 1;

	if(is_flash){
		if(is_erase){
			ret = edpu_cpld_flash_erase();
			if(ret == 0){
				printf("flash erase successfully\n");
			}else{
				printf("flash erase failed\n");
			}
		}else if(is_read){
			ret = edpu_cpld_flash_read(addr, 0, buf);
			if(ret == 0){
				int i;
				for(i=0; i<8; i++)
					printf("0x%04x\n", buf[i]);
				printf("\n");
			}else{
				printf("flash read failed\n");
			}
		}else{
			ret = edpu_cpld_flash_write(addr, buf);
			if(ret == 0){
				printf("flash write successfully\n");
			}else{
				printf("flash write failed\n");
			}
		}
	}else{
		if(is_read){
			printf("0x%04x\n", edpu_cpld_read(addr));
		}else{
			edpu_cpld_write(addr, data);
		}
	}
	
	return 0;
}

/***************************************************/

U_BOOT_CMD(
	edpu_pld,	11,	0,	do_edpu_pld,
	"T6290 eDPU cpld utility command",
	"<operation> <addr> [data]\n"
	"<operation> - r: reg read, w: reg write, fr:flash read, fw: flash write, fe: flash erase\n"
	"<addr>      - Hexadecimal string address\n"
	"<data>      - Hexadecimal string write data"
);
