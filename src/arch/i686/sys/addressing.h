/**
 * @file addressing.h
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
 * Functions and symbols used for translating between physical and virtual
 * memory addresses.
 */

#ifndef SRC_ARCH_I586_INCLUDE_CORE_ADDRESSING_H_
#define SRC_ARCH_I586_INCLUDE_CORE_ADDRESSING_H_

#include <cstdint>

/**
 * Symbol provided by the linker for the base address of physical memory.
 */
extern const unsigned int __region_physical_base;

/**
 * Symbol provided by the linker for the base address of virtual memory.
 */
extern const unsigned int __region_virtual_base;

/**
 * Contains functions for translating between physical and virtual memory
 * addresses.
 */
namespace addressing {

/**
 * Represents a physical address in memory.
 */
typedef uint32_t paddress;

/**
 * Represents a virtual address in memory.
 */
typedef void *vaddress;

/**
 * Calculates the offset between physical and virtual memory using symbols
 * passed from the linker.
 * @return Offset (in bytes) between physical and virtual memory.
 */
inline uint32_t GetVirtualPhysicalOffset() {
  const uint32_t kPhysicalBase =
      reinterpret_cast<uint32_t>(&__region_physical_base);
  const uint32_t kVirtualBase =
      reinterpret_cast<uint32_t>(&__region_virtual_base);
  return kPhysicalBase - kVirtualBase;
}

/**
 * Converts a virtual memory address into its corresponding physical address.
 * @param address The virtual address to convert.
 * @return The physical address that represents the same address.
 */
inline paddress VirtualToPhysical(vaddress address) {
  return reinterpret_cast<paddress>(address) + GetVirtualPhysicalOffset();
}

/**
 * Converts a physical memory address into its corresponding virtual address.
 * @param address The physical address to convert.
 * @return The virtual address that represents the same address.
 */
inline vaddress PhysicalToVirtual(paddress address) {
  return reinterpret_cast<vaddress>(address - GetVirtualPhysicalOffset());
}

} // namespace addressing

#endif // SRC_ARCH_I586_INCLUDE_CORE_ADDRESSING_H_
