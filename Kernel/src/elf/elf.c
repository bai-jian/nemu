#include "common.h"
#include "memory.h"

#include <elf.h>
#include <string.h>


#ifdef HAS_DEVICE
	#define ELF_OFFSET_IN_DISK 0
#endif

#define STACK_SIZE (1 << 20)

#define BUF_H_SIZE 4096
#define BUF_T_SIZE 4096
static uint8_t buf_h[BUF_H_SIZE];  // buffer head
static uint8_t buf_t[BUF_T_SIZE];  // buffer temp

void ide_read(uint8_t*, uint32_t, uint32_t);
void create_video_mapping();
uint32_t get_ucr3();
uint32_t loader( )
{
	Elf32_Ehdr* elf;

	#ifdef HAS_DEVICE
		ide_read(buf_h, ELF_OFFSET_IN_DISK, 4096);
		elf = (void*)buf_h;
	#else
		// The ELF file is located at memory address 0
		elf = (void*)0x0;
	#endif

	Elf32_Phdr* ph = (void*)elf->e_phoff;

	// Load each program segment 
	uint32_t i;
	for (i = 0; i < elf->e_phnum; ++i)
	{
		// Scan the program header table, load each segment into memory
		if (ph[i].p_type == PT_LOAD)
		{
			#ifdef IA32_PAGE

				uint32_t pa = mm_malloc(ph[i].p_vaddr, ph[i].p_memsz);

				assert(ph[i].p_filesz < BUF_T_SIZE);
				ide_read(buf_t, ph[i].p_offset, ph[i].p_filesz);

				uint32_t j;
				for (j = 0; j < ph[i].p_filesz; ++j)
					*(uint8_t*)(pa + j) = buf_t[j];
				for (     ; j < ph[i].p_memsz;  ++j)
					*(uint8_t*)(pa + j) = (uint8_t)0;

				int k;
				for (k = 0; k < ph[i].p_filesz; ++k)
					Log("0x%x ", buf_t[k]);
				assert(0);
			#else
				uint32_t j;
				// read the content of the segment from the ELF file to the memory region [VirtAddr, VirtAddr + FileSiz)
				for (j = 0; j < ph[i].p_filesz; ++j)
					*(uint8_t*)((uint8_t*)ph[i].p_vaddr + j) = *(uint8_t*)((uint8_t*)ph[i].p_offset + j);
				// zero the memory region [VirtAddr + FileSiz, VirtAddr + MemSiz)
				for (     ; j < ph[i].p_memsz;  ++j)
					*(uint8_t*)((uint8_t*)ph[i].p_vaddr + j) = 0; 
			#endif

			/* Record the prgram break for future use. */
			extern uint32_t brk;
			uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
			if(brk < new_brk) { brk = new_brk; }
		}
	}

	volatile uint32_t entry = elf->e_entry;

	#ifdef IA32_PAGE
		mm_malloc(KOFFSET - STACK_SIZE, STACK_SIZE);
		#ifdef HAS_DEVICE
			create_video_mapping();
		#endif
		write_cr3(get_ucr3());
	#endif

	return entry;
}
