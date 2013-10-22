/**
 * @file allocator.h
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


#ifndef SRC_INCLUDE_MM_ALLOCATOR_H_
#define SRC_INCLUDE_MM_ALLOCATOR_H_

#include <stddef.h>

namespace alloc {

/**
 * Represents an algorithm for allocating memory.
 */
class Allocator {
public:
  virtual ~Allocator() { }

  /**
   * Allocate a block of memory of a specified size.
   * @param size The size, in bytes, to allocate.
   * @param align True to align the memory region on a page boundary.
   */
  virtual void* Allocate(size_t size, bool align = false) = 0;

  /**
   * Releases a previously allocated block of memory. The memory must have
   * been allocated using this same allocator.
   * @param ptr A pointer to the block to release.
   */
  virtual void Free(void* ptr) = 0;
};

}  // namespace alloc

#endif  // SRC_INCLUDE_MM_ALLOCATOR_H_