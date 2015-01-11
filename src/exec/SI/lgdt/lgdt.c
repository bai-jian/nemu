#include "exec/helper.h"


#include "cpu/reg.h"
#include "cpu/modrm.h"


// lgdt  %GDTR, m16&m32(linear base address)
uint32_t hwaddr_read(hwaddr_t addr, size_t len);
make_helper(lgdt)
{
	// ModR_M:  mod  reg  R_M
	//           00  010  101
	ModR_M m;  m.val = instr_fetch(eip+1, 1);
	assert( m.val == 0x15 );

	uint32_t ph_addr = hwaddr_read(eip+2, 4);
	cpu.GDTR.limit = hwaddr_read(ph_addr    , 2);
	cpu.GDTR.base  = hwaddr_read(ph_addr + 2, 4);

	print_asm("lgdt   0x%x", ph_addr);

	return 1 + 1 + 4;
}
