/**
 * @file allocator.cc
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

/**
 * Symbol provided by the linker that indicates the end of the bss section.
 */
extern const uint32_t ebss;

namespace alloc {

/**
 * Singleton instance of the KernelSpaceAllocator used before the heap is
 * initialized.
 */
KernelSpaceAllocator g_ks_allocator(reinterpret_cast<uint32_t>(&ebss));

/**
 * Points the currently active allocator that will be used to satisfy C++
 * new/delete requests.
 */
Allocator *g_current_allocator = &g_ks_allocator;

void SetActiveAllocator(Allocator &allocator) {
  g_current_allocator = &allocator;
}

} // namespace alloc

/// @cond
void *operator new(size_t size) {
  return alloc::g_current_allocator->Allocate(size, false);
}

void *operator new[](size_t size) {
  return alloc::g_current_allocator->Allocate(size, false);
}

void operator delete(void *ptr) { alloc::g_current_allocator->Free(ptr); }

void operator delete[](void *ptr) { alloc::g_current_allocator->Free(ptr); }

/// @endcond
