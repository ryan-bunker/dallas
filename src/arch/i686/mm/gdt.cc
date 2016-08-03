/**
 * @file gdt.cc
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

#include "mm/gdt.h"

#include <cstdint>

#include "sys/addressing.h"

/**
 * Places the specified address into the GDT register.
 * @param addr The address to place in the GDT register.
 */
extern "C" void gdt_flush(uint32_t addr);

namespace gdt {

/**
 * An entry in the GDT table representing a single segment.
 */
struct GDTEntry {
  /**
   * The lower 16-bits of a 20-bit value containing the maximum addressable unit
   * (either in 1 byte units, or in pages).
   */
  uint16_t limit_low;

  /**
   * The lower 16-bits of a 32-bit value containing the linear address where
   * the segment begins.
   */
  uint16_t base_low;

  /**
   * The next 8-bits of base.
   */
  uint8_t base_middle; // The next 8 bits of the base.

  /**
   * The CPU sets this field to true when the segment is accessed.
   */
  bool accessed : 1;

  /**
   * For code selectors: indicates whether read access is allowed. Write access
   * is never allowed for code segments.
   * For data selectors: indicates whether write access is allowed. Read access
   * is always allowed for data segments.
   */
  bool read_write_access : 1;

  /**
   * For code selectors: If true code in this segment can be executed from an
   * equal or lower privilege level. For example, code in ring 3 can far-jump to
   * conforming code in a ring 2 segment. The privilege field represents the
   * highest privilege level that is allowed to execute the segment. For
   * example, code in ring 0 cannot far-jump to a conforming code segment with
   * privilege==0x2, while code in ring 2 and 3 can. Note that the privilege
   * level remains the same, ie. a far-jump from ring 3 to a segment with
   * privilege==0x2 remains in ring 3 after the jump. If false, code in this
   * segment can only be executed from the ring set in privilege.
   * For data selectors: Indicates the direction that the segment grows. If
   * false, the segment grows up. If true, the segment grows down, ie. the
   * offset has to be greater than the base.
   */
  bool direction_conforming : 1;

  /**
   * Indicates whether the segment is executable, ie. whether it is a code
   * segment or data segment.
   */
  bool is_executable : 1;

  /**
   * This field should always be true.
   */
  bool always_true : 1;

  /**
   * Contains the ring level, 0 = highest (kernel), 3 = lowest (user
   * applications).
   */
  uint8_t privilege : 2;

  /**
   * Must be true for all valid segments.
   */
  bool present : 1;

  /**
   * The upper 4-bits of limit.
   */
  uint8_t limit_high : 4;

  /**
   * Unused, always zero.
   */
  uint8_t always_zero : 2;

  /**
   * The size of the segment. If 0 the selector defines 16-bit protected mode.
   * If 1 it defines 32-bit protected mode. You can have both 16-bit and 32-bit
   * selectors at once.
   */
  uint8_t size : 1;

  /**
   * If 0 the limit is in 1B blocks (byte granularity), if 1 the limit is in
   * 4KiB blocks (page granularity).
   */
  uint8_t granularity : 1;

  /**
   * The upper 8-bits of base.
   */
  uint8_t base_high;
} __attribute__((packed));

/**
 * Represents the contents of the GDT register which points to the global
 * descriptor table.
 */
struct GDTRegister {
  /**
   * Defines the length of the GDT +1 in bytes.
   */
  uint16_t limit;

  /**
   * The physical address where the GDT starts. Note that paging applies to
   * this address.
   */
  uint32_t base;
} __attribute__((packed));

/**
 * Global list of GDT entries.
 */
GDTEntry g_gdt_entries[5];
/**
 * Global GDT register contents.
 */
GDTRegister g_gdtr;

static void GDTSetGate(int num, uint32_t base, uint32_t limit,
                       bool read_write_access, bool direction_conforming,
                       bool is_executable, uint8_t privilege, uint8_t size,
                       uint8_t granularity) {
  g_gdt_entries[num].base_low = (base & 0xFFFF);
  g_gdt_entries[num].base_middle = (base >> 16) & 0xFF;
  g_gdt_entries[num].base_high = (base >> 24) & 0xFF;

  g_gdt_entries[num].limit_low = (limit & 0xFFFF);
  g_gdt_entries[num].limit_high = (limit >> 16) & 0xF;

  g_gdt_entries[num].accessed = false;
  g_gdt_entries[num].read_write_access = read_write_access;
  g_gdt_entries[num].direction_conforming = direction_conforming;
  g_gdt_entries[num].is_executable = is_executable;
  g_gdt_entries[num].always_true = true;
  g_gdt_entries[num].privilege = privilege;
  g_gdt_entries[num].present = true;
  g_gdt_entries[num].always_zero = 0;
  g_gdt_entries[num].size = size;
  g_gdt_entries[num].granularity = granularity;
}

void Initialize() {
  g_gdtr.limit = sizeof(GDTEntry) * 5 - 1;
  g_gdtr.base = addressing::VirtualToPhysical(&g_gdt_entries);

  GDTSetGate(0, 0, 0, false, false, false, 0, 0, 0);         // Null segment
  GDTSetGate(1, 0, 0xFFFFFFFF, true, false, true, 0, 1, 1);  // Code segment
  GDTSetGate(2, 0, 0xFFFFFFFF, true, false, false, 0, 1, 1); // Data segment
  GDTSetGate(3, 0, 0xFFFFFFFF, true, false, true, 3, 1, 1);  // User mode code
  GDTSetGate(4, 0, 0xFFFFFFFF, true, false, false, 3, 1, 1); // User mode data

  gdt_flush(reinterpret_cast<uint32_t>(&g_gdtr));
}
}
