#include "gdt.h"
#include "panic.h"
#include "log.h"

#include <stdint.h>

void gdt_init(void)
{
	// Flat GDT setup, 4GiB of untranslated addresses
	gdt_encode_entry(gdt[0], 0, 0, 0); // null 
	gdt_encode_entry(gdt[1], 0, 0xFFFFFFFF, 0x9A); // kernel code
	gdt_encode_entry(gdt[2], 0, 0xFFFFFFFF, 0x92); // kernel data
	gdt_encode_entry(gdt[3], 0, 0xFFFFFFFF, 0xFA); // user code
	gdt_encode_entry(gdt[4], 0, 0xFFFFFFFF, 0xF2); // user data
	gdt_encode_entry(gdt[5], (uint32_t) &gdt_tss, sizeof(gdt_tss), 0x89); // TSS

	gdt_pointer.offset = (uint32_t) &gdt;
	gdt_pointer.size = sizeof(gdt) - 1;

	gdt_flush((size_t) &gdt_pointer);

	gdt_tss.ss0 = 0x10; // kernel data descriptor
	gdt_tss.esp0 = (uint32_t) &stack_top; // kernel stack top
	gdt_tss.iomap = (uint16_t) sizeof(gdt_tss); // don't care about iomap
	gdt_tss_flush();

	log(LOG_OK, "GDT module initialized\n");
}

void gdt_encode_entry(uint8_t target[8], uint32_t base, uint32_t limit, uint8_t type)
{
	// If limit is big, it must be a multiple of 4KiB
	if ((limit > 0x10000) && (limit & 0xFFF) != 0xFFF)
		PANIC("Invalid GDT limit specified, this shouldn't ever happen\n");

	if (limit > 0x10000) {
		// Divide limit by 4K, change granularity to 4KiB
		limit >>= 12;
		target[6] = 0xC0;
	} else {
		target[6] = 0x40;
	}

	target[0] = limit & 0xFF;
	target[1] = (limit >> 8) & 0xFF;
	target[6] |= (limit >> 16) & 0xF;

	target[2] = base & 0xFF;
	target[3] = (base >> 8) & 0xFF;
	target[4] = (base >> 16) & 0xFF;
	target[7] = (base >> 24) & 0xFF;

	target[5] = type;
}