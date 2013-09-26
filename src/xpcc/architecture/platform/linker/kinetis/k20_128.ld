
MEMORY
{
	ROM (rx)      : ORIGIN = 0x00000000, LENGTH = 128k
	RAM (rwx)     : ORIGIN = 0x20000000, LENGTH = 8k-16	/* Main internal SRAM1 */
}

INCLUDE "../memory_sections.ld"
