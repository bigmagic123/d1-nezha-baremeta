OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)
SECTIONS
{
	. = FES1ADDR;
	. = ALIGN(4);

	.head   :
	{
		main/fes1_head.o	(.rodata)
	}
	.text :
	{
		CPUDIR/fes1_entry.o (.text)
		*(.text)
	}
	. = ALIGN(16);
  	.rodata : { *(.rodata) }
	. = ALIGN(16);
  	.data : { *(.data) }

	. = ALIGN(4);
	.bss :
	{
	fes1_res_addr = .;
	__bss_start = .;
		*(.bss)
	}
	. = ALIGN(4);
	__bss_end = .;

	_end = .;
}



