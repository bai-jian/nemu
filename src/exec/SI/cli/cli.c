#include "exec/helper.h"


#include "cpu/reg.h"

make_helper(cli)
{
	cpu.IF = 0;

	print_asm("cli");

	return 1;
}
