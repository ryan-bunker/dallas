/**
 * @file idt.cc
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
 */

#include "int/idt.h"

#include <cstdint>
#include <cstring>

#include "sys/addressing.h"
#include "sys/io.h"

extern "C" {
/**
 * Places the specified address into the IDT register.
 * @param addr The address to place in the IDT register.
 */
void idt_flush(uint32_t addr);

/// @cond
void isr0();
void isr1();
void isr2();
void isr3();
void isr4();
void isr5();
void isr6();
void isr7();
void isr8();
void isr9();
void isr10();
void isr11();
void isr12();
void isr13();
void isr14();
void isr15();
void isr16();
void isr17();
void isr18();
void isr19();
void isr20();
void isr21();
void isr22();
void isr23();
void isr24();
void isr25();
void isr26();
void isr27();
void isr28();
void isr29();
void isr30();
void isr31();

void irq0();
void irq1();
void irq2();
void irq3();
void irq4();
void irq5();
void irq6();
void irq7();
void irq8();
void irq9();
void irq10();
void irq11();
void irq12();
void irq13();
void irq14();
void irq15();
/// @endcond
}

namespace idt {

/**
 * Specifies the different types of interrupt gates.
 */
enum IDTGateType {
  /**
   * When an interrupt/exception occurs whose entry is a Task Gate, a task
   * switch results.
   */
  k32bitTaskGate = 0x5,
  /**
   * Specifies a 16-bit interrupt service routine.
   */
  k16bitInterruptGate = 0x6,
  /**
   * Specifies a 16-bit exception handler.
   */
  k16bitTrapGate = 0x7,
  /**
   * Specifies a 32-bit interrupt service routine.
   */
  k32bitInterruptGate = 0xE,
  /**
   * Specifies a 32-bit exception handler.
   */
  k32bitTrapGate = 0xF
};

/**
 * Represents an interrupt gate in the IDT.
 */
struct IDTEntry {
  /**
   * The low 16 bits of the address to jump to when this interrupt is raised.
   */
  uint16_t base_low;

  /**
   * Kernel segment selector.
   */
  uint16_t selector;

  /**
   * Reserved field. Must always be zero.
   */
  uint8_t always0;

  /**
   * Indicates what type of interrupt gate this record represents.
   */
  IDTGateType gate_type : 4;

  /**
   * Should be zero for interrupt gates.
   */
  bool storage_segment : 1;

  /**
   * Specifies descriptor privilege level. Gate call protection. Specifies which
   * privilege Level the calling Descriptor minimum should have. So hardware and
   * CPU interrupts can be protected from being called out of user-space.
   */
  uint8_t dpl : 2;

  /**
   * Specifies whether this entry represents a valid handler. Can be set to zero
   * for unused interrupts or for Paging.
   */
  bool is_present : 1;

  /**
   * The high 16 bits of the address to jump to when this interrupt is raised.
   */
  uint16_t base_hi;
} __attribute__((packed));

/**
 * Represents the contents of the IDT register which points to the interrupt
 * descriptor table.
 */
struct IDTRegister {
  /**
   * Defines the length of the IDT in bytes (minimum value is 0x100, a value of
   * 0x1000 means 0x200 interrupts).
   */
  uint16_t limit;

  /**
   * The physical address where the IDT starts (INT 0).
   */
  uint32_t base;
} __attribute__((packed));

/**
 * Global list of IDT entries.
 */
IDTEntry g_idt_entries[256];
/**
 * Global IDT register contents.
 */
IDTRegister g_idtr;

static void IDTSetGate(uint8_t number, void (*handler)(), uint16_t selector,
                       IDTGateType gate_type, bool storage_segment, uint8_t dpl,
                       bool is_present) {
  uint32_t base = reinterpret_cast<uint32_t>(handler);
  g_idt_entries[number].base_low = base & 0xFFFF;
  g_idt_entries[number].base_hi = (base >> 16) & 0xFFFF;

  g_idt_entries[number].selector = selector;
  g_idt_entries[number].always0 = 0;

  g_idt_entries[number].gate_type = gate_type;
  g_idt_entries[number].storage_segment = storage_segment;
  g_idt_entries[number].dpl = dpl;
  g_idt_entries[number].is_present = is_present;
}

void Initialize() {
  g_idtr.limit = sizeof(IDTEntry) * 256 - 1;
  g_idtr.base = addressing::VirtualToPhysical(&g_idt_entries);

  memset(&g_idt_entries, 0, sizeof(IDTEntry) * 256);

#define ISR(num)                                                               \
  IDTSetGate(num, isr##num, 0x08, IDTGateType::k32bitInterruptGate, false, 0,  \
             true)

  ISR(0);
  ISR(1);
  ISR(2);
  ISR(3);
  ISR(4);
  ISR(5);
  ISR(6);
  ISR(7);
  ISR(8);
  ISR(9);
  ISR(10);
  ISR(11);
  ISR(12);
  ISR(13);
  ISR(14);
  ISR(15);
  ISR(16);
  ISR(17);
  ISR(18);
  ISR(19);
  ISR(20);
  ISR(21);
  ISR(22);
  ISR(23);
  ISR(24);
  ISR(25);
  ISR(26);
  ISR(27);
  ISR(28);
  ISR(29);
  ISR(30);
  ISR(31);
#undef ISR

  // Re-map the IRQ table.
  outb(0x20, 0x11);
  io_wait();
  outb(0xA0, 0x11);
  io_wait();
  outb(0x21, 0x20);
  io_wait();
  outb(0xA1, 0x28);
  io_wait();
  outb(0x21, 0x04);
  io_wait();
  outb(0xA1, 0x02);
  io_wait();
  outb(0x21, 0x01);
  io_wait();
  outb(0xA1, 0x01);
  io_wait();
  outb(0x21, 0x0);
  outb(0xA1, 0x0);

#define IRQ(isr, irq_n)                                                        \
  IDTSetGate(isr, irq##irq_n, 0x08, IDTGateType::k32bitInterruptGate, false,   \
             0, true)

  IRQ(32, 0);
  IRQ(33, 1);
  IRQ(34, 2);
  IRQ(35, 3);
  IRQ(36, 4);
  IRQ(37, 5);
  IRQ(38, 6);
  IRQ(39, 7);
  IRQ(40, 8);
  IRQ(41, 9);
  IRQ(42, 10);
  IRQ(43, 11);
  IRQ(44, 12);
  IRQ(45, 13);
  IRQ(46, 14);
  IRQ(47, 15);
#undef IRQ

  idt_flush(reinterpret_cast<uint32_t>(&g_idtr));
}

} // namespace idt
