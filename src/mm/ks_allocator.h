/**
 * @file ks_allocator.h
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


#ifndef SRC_INCLUDE_MM_KS_ALLOCATOR_H_
#define SRC_INCLUDE_MM_KS_ALLOCATOR_H_

#include <cstddef>
#include <cstdint>

#include "mm/allocator.h"

namespace alloc {

/**
 * Allocates memory from the end of the kernel's space. This allocator cannot
 * free memory because it doesn't track it's location.
 */
class KernelSpaceAllocator : public Allocator {
public:
  KernelSpaceAllocator(uint32_t place_address);

  /**
   * Allocates a block of memory from the reserved space at the end of the
   * kernel. The allocated block cannot be freed.
   * @param size The size, in bytes, of the memory to allocate.
   * @param align True to align the returned region on a page boundary.
   */
  virtual void* Allocate(size_t size, bool align = false);

  /**
   * Memory allocated by this allocator cannot be freed.
   * @param ptr
   */
  virtual void Free(void* ptr);

  /**
   * Gets the address of the next region of memory to be allocated by Allocate.
   */
  inline void* next() { return (void*)current_address_; }

private:
  /**
   * Starting address of the kernel space to allocate from.
   */
  uint32_t memory_space_start_;
  /**
   * The next available memory address.
   */
  uint32_t current_address_;
};


}  // namespace alloc

#endif  // SRC_INCLUDE_MM_KS_ALLOCATOR_H_
