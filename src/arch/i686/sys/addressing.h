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

#include <cstddef>
#include <cstdint>

extern const unsigned int _phys_virt_offset;
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
 * Calculates the offset between physical and virtual memory using symbols
 * passed from the linker.
 * @return Offset (in bytes) between physical and virtual memory.
 */
inline uint32_t GetVirtualPhysicalOffset() {
  // const uint32_t kPhysicalBase =
  //     reinterpret_cast<uint32_t>(&__region_physical_base);
  // const uint32_t kVirtualBase =
  //     reinterpret_cast<uint32_t>(&__region_virtual_base);
  // return kPhysicalBase - kVirtualBase;
  return reinterpret_cast<uint32_t>(&_phys_virt_offset);
}

class AddressVirtual;

class AddressPhysical {
public:
  AddressPhysical(uint32_t address) { address_ = address; }

  inline uint32_t Raw() { return address_; }
  inline void *Address() { return reinterpret_cast<void *>(address_); }

  inline AddressPhysical operator+(size_t sz) const {
    return AddressPhysical(address_ + sz);
  }
  inline AddressPhysical operator-(size_t sz) const {
    return AddressPhysical(address_ - sz);
  }
  inline AddressPhysical operator+(AddressPhysical addr) const {
    return AddressPhysical(address_ + addr.address_);
  }
  inline AddressPhysical operator-(AddressPhysical addr) const {
    return AddressPhysical(address_ - addr.address_);
  }
  inline AddressPhysical operator/(size_t sz) const {
    return AddressPhysical(address_ / sz);
  }
  inline AddressPhysical operator%(size_t sz) const {
    return AddressPhysical(address_ % sz);
  }
  inline bool operator<(AddressPhysical &rhs) { return address_ < rhs.address_; }

  inline explicit operator uint32_t() const { return address_; }

  AddressVirtual ToVirtual();

  // TODO: use page alignment constant here instead of literal
  inline bool IsPageAligned() const { return address_ % 0x1000 == 0; }

private:
  uint32_t address_;
};

class AddressVirtual {
public:
  AddressVirtual(uint32_t address) { address_ = address; }
  AddressVirtual(const void *address) { address_ = reinterpret_cast<uint32_t>(address); }

  inline uint32_t Raw() { return address_; }
  inline void *Address() { return reinterpret_cast<void *>(address_); }

  inline AddressVirtual operator+(size_t sz) const {
    return AddressVirtual(address_ + sz);
  }
  inline AddressVirtual operator-(size_t sz) const {
    return AddressVirtual(address_ - sz);
  }
  inline AddressVirtual operator+(AddressVirtual addr) const {
    return AddressVirtual(address_ + addr.address_);
  }
  inline AddressVirtual operator-(AddressVirtual addr) const {
    return AddressVirtual(address_ - addr.address_);
  }
  inline AddressVirtual operator/(size_t sz) const {
    return AddressVirtual(address_ / sz);
  }
  inline AddressVirtual operator%(size_t sz) const {
    return AddressVirtual(address_ % sz);
  }

  inline explicit operator size_t() const { return address_; }
  inline explicit operator void*() const { return reinterpret_cast<void*>(address_); }

  AddressPhysical ToPhysical();

private:
  uint32_t address_;
};

/**
 * Represents a physical address in memory.
 */
typedef AddressPhysical paddress;

/**
 * Represents a virtual address in memory.
 */
typedef AddressVirtual vaddress;

const paddress kExtMemory(0x100000);
const paddress kPhysStop(0xE000000);

const vaddress kKernelBase(0xC0000000);
const vaddress kKernelLink(0xC0000000 + 0x100000); // TODO: reuse constants here

/**
 *
 */
inline vaddress KernelBase() {
  const uint32_t kPhysicalBase =
      reinterpret_cast<uint32_t>(&__region_physical_base);
  const uint32_t kVirtualBase =
      reinterpret_cast<uint32_t>(&__region_virtual_base);
  return vaddress(kVirtualBase - kPhysicalBase);
}

} // namespace addressing

#endif // SRC_ARCH_I586_INCLUDE_CORE_ADDRESSING_H_
