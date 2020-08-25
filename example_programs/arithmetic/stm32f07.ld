/* Define memory regions. */
MEMORY
{
    rom (rx)   : ORIGIN = 0x08000000, LENGTH = 0x2000
    ram (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x4000
}

SECTIONS {
	.text : {
		KEEP(SORT(*)(.vectors))
	} >rom
}

/* Include the common ld script. */
INCLUDE cortex-m-generic.ld
