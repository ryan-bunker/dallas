ENTRY (start)
OUTPUT_FORMAT(elf32-i386)

_phys_virt_offset = 0xC0000000; 
__region_physical_base = 0x100000;
__region_virtual_base = _phys_virt_offset + __region_physical_base;

SECTIONS{
  . = __region_physical_base;
  
  .setup :
  {
    *(.setup)
  }
  
  . += _phys_virt_offset;
  
  .text : AT(ADDR(.text) - _phys_virt_offset)
  {
    *(.text)
    *(.gnu.linkonce.t*)
  }
  
  .rodata ALIGN (0x1000) : AT(ADDR(.rodata) - _phys_virt_offset)
    {
        start_ctors = .;
        *(.ctor*)
        end_ctors = .;

        start_dtors = .;
        *(.dtor*)
        end_dtors = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)
    }
  
  .data ALIGN (4096) : AT(ADDR(.data) - _phys_virt_offset)
  {
    *(.data)
    *(.gnu.linkonce.d*)
  }
  
  .bss ALIGN(4096) : AT(ADDR(.bss) - _phys_virt_offset)
  {
    sbss = .;
    *(.COMMON*)
    *(.bss*)
    *(.gnu.linkonce.b*)
    ebss = .;
  }
  
  /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame) /* discard this, unless you are implementing runtime support for C++ exceptions. */
    }
}