.code32
.global start
# Our kernel entry point in main_entry.cc
.extern kmain

# These symbols represent the beginning and end of the respective constructors
# and destructors section, declared by the linker script.
.extern start_ctors
.extern end_ctors
.extern start_dtors
.extern end_dtors

# # Declare constants used for creating a multiboot header.
# .set ALIGN,     1<<0              # align loaded modules on page boundaries
# .set MEMINFO,   1<<1              # provide memory map
# .set FLAGS,     ALIGN | MEMINFO   # this is the Multiboot 'flag' field
# .set MAGIC,     0xE85250D6        # 'magic number' lets bootloader find the header
# .set CHECKSUM,  -(MAGIC + FLAGS)  # checksum of above, to prove we are multiboot

# # Declare a header as in the Multiboot Standard. We put this into a special
# # section so we can force the header to be in the start of the final program.
# # You don't need to understand all these details as it is just magic values that
# # is documented in the multiboot standard. The bootloader will search for this
# # magic sequence and recognize us as a multiboot kernel.
# .section .multiboot
# .align 4
# .long MAGIC
# .long FLAGS
# .long CHECKSUM
.section .multiboot
.align 8
header_start:
.int 0xe85250d6
.int 0
.int header_end - header_start
.int -(0xe85250d6 + 0 + (header_end - header_start))

# .short 1
# .short 0
# .int 12    # size
# .int 9

.short 0, 0
.int 8
header_end:

# Tells the assembler to include this data in the '.init' section
.section .init

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded. It
# doesn't make sense to return from this function as the bootloader is gone.
start:
  # The bootloader puts information into EAX and EBX that we don't want to lose,
  # but the stack isn't ready for us to push things so we'll use ECX for the
  # following instructions.

  # Recursive map the page directory (that means point the last entry
  #   in the page directory at itself)
  movl    $(entrypgdir - 0xC0000000), %ecx  # get the physical address of the page directory
  orl     $0x3, %ecx                        # mark the entry present | writable
  movl    %ecx, (entrypgdir - 0xBFFFF004)   # write the entry back into the page table

  # Turn on page size extension for 4Mbyte pages
  movl    %cr4, %ecx
  orl     $0x10, %ecx
  movl    %ecx, %cr4
  # Set page directory
  movl    $(entrypgdir - 0xC0000000), %ecx
  movl    %ecx, %cr3
  # Turn on paging.
  movl    %cr0, %ecx
  orl     $0x80010000, %ecx
  movl    %ecx, %cr0

  # Test value
  movl    $0xdeadbeef, (0x1234)

  # Jump to main(), and switch to executing at
  # high addresses. The indirect call is needed because
  # the assembler produces a PC-relative instruction
  # for a direct jump.
  mov $higherhalf, %ecx
  jmp *%ecx

.section .text
higherhalf:
  # From now on, the CPU will translate automatically every address by adding
  # the base 0x40000000, thus matching the addresses we generated at compile.

  # We now have sufficient code for the bootloader to load and run our operating
  # system. It doesn't do anything interesting yet. Note that the processor is
  # not fully initialized yet and stuff such as floating point instructions are
  # not available yet.

  # To set up a stack, we simply set the esp register to point to the top of
  # our stack (as it grows downwards).
  movl $sys_stack, %esp

  # The bootloader put the multiboot magic number in EAX and the address of the
  # multiboot information structure in EBX, so we need to pass those to kmain.
  push %eax
  addl $0xC0000000,%ebx   # convert the physical address to virtual before passing
                          # it to the kernel
  push %ebx

  # Before we go to the main entry point, we need to call all of our C++
  # constructors so those objects are properly initialized.
  movl $start_ctors, %ebx
  jmp .Lctors_until_end
.Lcall_constructor:
  call *(%ebx)
  addl $4, %ebx
.Lctors_until_end:
  cmpl $end_ctors, %ebx
  jb .Lcall_constructor

  # We are now ready to execute C++ code. Call into our kernel entry point,
  # kmain in main_entry.cc
  call kmain

  # Since the kernel has exited, it is now time to call all of our C++
  # destructors to make sure everything is cleaned up properly.
  movl $end_dtors, %ebx
  jmp .Ldtors_until_end
.Lcall_destructor:
  subl $4, %ebx
  call *%ebx
.Ldtors_until_end:
  cmpl $start_dtors, %ebx
  ja .Lcall_destructor

  # In case the function returns, we'll want to put the computer into an
  # infinite loop. To do that, we use the clear interrupt ('cli') instruction
  # to disable interrupts, the halt instruction ('hlt') to stop the CPU until
  # the next interrupt arrives, and jumping to the halt instruction if it ever
  # continues execution, just to be safe. We will create a local label rather
  # than real symbol and jump to there endlessly.
  cli
  hlt
.Lhang:
  jmp .Lhang
# End of _start


# Load GDT with a specified value.
.global gdt_flush
gdt_flush:
  # Get the pointer to the GDT, passed as a parameter.
  movl 4(%esp), %eax
  # Load the new pointer into GDT.
  lgdt (%eax)

  # 0x10 is the offset in the GDT to our data segment.
  movw $0x10, %ax
  # Load all data segment selectors.
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  movw %ax, %ss
  # 0x08 is the offset to our code segment, far jump to load it
  ljmp $0x08, $.flush
.flush:
  ret

# Load IDT with a specified value.
.global idt_flush
idt_flush:
  # Get the pointer to the IDT, passed as a paramter.
  movl 4(%esp), %eax
  # Load the IDT pointer.
  lidt (%eax)
  ret

.section .pgdir
.align 4096
entrypgdir:
  # 1024 entries at 32 bits/entry
  #  3         2         1
  # 10987654321098765432109876543210
  # |Page table address|AV|GS0ADWURP
  .int 0x83   # entry 0x000 (4K size, read/write, present)
  .fill 767, 4          # 767 entries (767 * 4)
  # 0xC0000 000
  # 1100000000 0000000000 000000000000
  # 0x300      0x0        0x0
  .int 0x83   # entry 0x300 (4K size, read/write, present)
  .fill 255, 4          # 255 entries (255 * 4)

# cga: 0xB8000
#  

# Currently the stack pointer register (esp) points at anything and using it may
# cause massive harm. Instead, we'll provide our own stack. We will allocate
# room for a small temporary stack by allocating 16384 bytes for it, and
# creating a symbol at the top.
.section .bss
.skip 0x4000
sys_stack:

