/**
 * @file cpp_alloc_operators.cc
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

#include "mm/allocator.h"

#include <cstddef>
#include <cstdint>

#include "mm/ks_allocator.h"
#include "sys/kernel.h"

extern uint32_t ebss;

namespace alloc {

KernelSpaceAllocator g_ks_allocator(reinterpret_cast<uint32_t>(&ebss));
Allocator* g_current_allocator = &g_ks_allocator;

}  // namespace alloc

void *operator new(size_t size) {
  ASSERT(alloc::g_current_allocator != 0);
  return alloc::g_current_allocator->Allocate(size, false);
}

void *operator new[](size_t size) {
  ASSERT(alloc::g_current_allocator != 0);
  return alloc::g_current_allocator->Allocate(size, false);
}

void operator delete(void *ptr) {
  ASSERT(alloc::g_current_allocator != 0);
  alloc::g_current_allocator->Free(ptr);
}

void operator delete[](void *ptr) {
  ASSERT(alloc::g_current_allocator != 0);
  alloc::g_current_allocator->Free(ptr);
}



