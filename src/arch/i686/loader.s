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

# Declare constants used for creating a multiboot header.
.set ALIGN,     1<<0              # align loaded modules on page boundaries
.set MEMINFO,   1<<1              # provide memory map
.set FLAGS,     ALIGN | MEMINFO   # this is the Multiboot 'flag' field
.set MAGIC,     0x1BADB002        # 'magic number' lets bootloader find the header
.set CHECKSUM,  -(MAGIC + FLAGS)  # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard. We put this into a special
# section so we can force the header to be in the start of the final program.
# You don't need to understand all these details as it is just magic values that
# is documented in the multiboot standard. The bootloader will search for this
# magic sequence and recognize us as a multiboot kernel.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded. It
# doesn't make sense to return from this function as the bootloader is gone.
.section .init
start:
  # First things first, we need to load the trick GDT so addressing everything
  # works correctly. Load a GDT with a base address of 0x40000000 for the
  # code (0x08) and data (0x10) segments.
  lgdt trickgdt
  movw $0x10, %cx
  movw %cx, %ds
  movw %cx, %es
  movw %cx, %fs
  movw %cx, %gs
  movw %cx, %ss

  # Now we perform a jump into the higher half kernel to switch into the just
  # loaded GDT registers.
  jmp $0x08, $higherhalf

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

# Tells the assembler to include this data in the '.init' section
.section .init
trickgdt:
  # Size of the GDT
  .word gdt_end - gdt
  # Linear address of GDT
  .long gdt

gdt:
  .long 0, 0
  # Code selector 0x08: base 0x40000000, limit 0xFFFFFFFF, type 0x9A, granularity 0xCF
  .byte 0xFF, 0xFF, 0, 0, 0, 0x9A, 0xCF, 0x40
  # Data selector 0x10: base 0x40000000, limit 0xFFFFFFFF, type 0x92, granularity 0xCF
  .byte 0xFF, 0xFF, 0, 0, 0, 0x92, 0xCF, 0x40
gdt_end:

# Currently the stack pointer register (esp) points at anything and using it may
# cause massive harm. Instead, we'll provide our own stack. We will allocate
# room for a small temporary stack by allocating 16384 bytes for it, and
# creating a symbol at the top.
.section .bss
.skip 0x4000
sys_stack:
