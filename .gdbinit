symbol-file build/kernel.elf
directory src/arch/i686
directory src/kernel
break start
target remote tcp::1234layout asm
