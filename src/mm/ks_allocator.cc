/**
 * @file ks_allocator.cc
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

#include "mm/ks_allocator.h"

namespace alloc {

KernelSpaceAllocator::KernelSpaceAllocator(uint32_t place_address)
//  : memory_space_start_(place_address),
//    current_address_(place_address) {
{
  memory_space_start_ = place_address;
  current_address_ = place_address;
}

void* KernelSpaceAllocator::Allocate(size_t size, bool align) {
  // if the address is not already page-aligned and we're supposed to align it
  if (align && (current_address_ & 0xFFFFF000)) {
    // Align it
    current_address_ &= 0xFFFFF000;
    current_address_ += 0x1000;
  }

  uint32_t tmp = current_address_;
  current_address_ += size;
  return (void*)tmp;
}

void KernelSpaceAllocator::Free(void*) {
  // TODO: force some sort of error here since kernel space allocated memory can't be freed
}

}  // namespace alloc
