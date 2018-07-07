#include <common.h>
#include <command.h>

#include <t6290_edpu.h>

int do_edpu_spi (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long addr;
	unsigned long data;
	unsigned int is_read = 0;
	
	if(argc == 4){/*write, eg: edpu_cpld w 0x12 0x34*/
	
		if(argv[1][0] != 'w')
			return CMD_RET_USAGE;
			
		addr = simple_strtoul(argv[2], NULL, 16); /* address */
		data = simple_strtoul(argv[3], NULL, 16); /* data */
	}else if(argc == 3){ /* read, eg: edpu_cpld r 0x12 */
	
		if(argv[1][0] != 'r')
			return CMD_RET_USAGE;

		is_read = 1;
		addr = simple_strtoul(argv[2], NULL, 16); /* address */
	}
	else{
		return CMD_RET_USAGE;
	}

	if(is_read){
		printf("0x%04x\n", edpu_cpld_read(addr));
	}else{
		edpu_cpld_write(addr, data);
	}
	return 0;
}

/***************************************************/

U_BOOT_CMD(
	edpu_cpld,	5,	1,	do_edpu_spi,
	"T6290 eDPU cpld utility command",
	"<operation> <addr> [data]\n"
	"<operation> - r: read, w: write\n"
	"<addr>      - Hexadecimal string address\n"
	"<data>      - Hexadecimal string write data"
);
