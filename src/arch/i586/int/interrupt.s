/** @file interrupt.s
 *
 * @section LICENSE
 *
 * Copyright (C) 2013  Ryan Bunker
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see [http://www.gnu.org/licenses/].
 *
 * @section DESCRIPTION
 *
 * Assembly stubs for interrupt service routines and interrupt handlers.
 */
.code32

// Creates an ISR that does not accept an error code.
.macro ISR_NOERRCODE isr_num
  .global isr\isr_num
  isr\isr_num:
    cli
    pushw $0
    pushw $\isr_num
    jmp isr_common_stub
.endm

// Creates an ISR that does accept an error code.
.macro ISR_ERRCODE isr_num
  .global isr\isr_num
  isr\isr_num:
    cli
    pushw $\isr_num
    jmp isr_common_stub
.endm

// Creates an IRQ.
// @param irq_num The IRQ number to handle.
// @param isr_num The ISR number this IRQ is remapped to.
.macro IRQ irq_num, isr_num
  .global irq\irq_num
  irq\irq_num:
    cli
    pushw $0
    pushw $\isr_num
    jmp irq_common_stub
.endm

// Define all the interrupt service routine stubs.
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

// Now define all the interrupt request handlers.
IRQ  0, 32
IRQ  1, 33
IRQ  2, 34
IRQ  3, 35
IRQ  4, 36
IRQ  5, 37
IRQ  6, 38
IRQ  7, 39
IRQ  8, 40
IRQ  9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47


// This is our common ISR stub. It saves the processor state, sets up for kernel
// mode segments, calls the C-level fault handler, and finally restores the stack
// frame.
isr_common_stub:
  pusha           // Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

  movw %ds, %ax   // Lower 16-bits of eax = ds
  pushl %eax      // save the data segment descriptor

  movw $0x10, %ax // load the kernel data segment selector
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs

  call GlobalISRHandler

  popl %eax       // reload the original data segment descriptor
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs

  popa            // Pops edi,esi,ebp...
  addl $8, %esp   // Cleans up the pushed error code and pushed ISR number
  sti
  iret            // pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP


// This is our common IRQ stub. It saves the processor state, sets up for kernel
// mode segments, calls the C-level fault handler, and finally restores the
// stack frame.
irq_common_stub:
  pusha           // Pushes edi,esi,ebp,esp,ebx,edx,ecx,eax

  movw %ds, %ax   // Lower 16-bits of eax = ds
  pushl %eax      // save the data segment descriptor

  movw $0x10, %ax // load the kernel data segment selector
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs

  call GlobalIRQHandler

  popl %ebx       // reload the original data segment descriptor
  movw %bx, %ds
  movw %bx, %es
  movw %bx, %fs
  movw %bx, %gs

  popa            // Pops edi,esi,ebp...
  addl $8, %esp   // Cleans up the pushed error code and pushed ISR number
  sti
  iret            // Pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
