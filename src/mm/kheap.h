/**
 * @file kheap.h
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


#ifndef SRC_INCLUDE_MM_KHEAP_H_
#define SRC_INCLUDE_MM_KHEAP_H_

#include <cstdint>
#include "mm/allocator.h"

namespace alloc {

/**
 * Allocator that manages a heap of memory.
 */
class KHeap : public Allocator {
 public:
  /**
   * Creates a new KHeap instance.
   * @param start_address The virtual address of the start of the heap.
   * @param end_address The virtual address of the end of the heap.
   * @param max_address The maximum allowable address for the heap to grow into.
   * @param supervisor Indicates whether the heap is only accessible from
   * kernel-space.
   * @param readonly Indicates whether the heap is read only.
   */
  KHeap(void *start_address, void* end_address, void* max_address,
        bool supervisor, bool readonly);

  virtual void* Allocate(size_t size, bool align = false);
  virtual void  Free(void* ptr);

 private:
  enum class Magic : uint32_t {
    kValue = 0x123890AB
  };

  /**
   * Header information placed at the beginning of every block of allocated
   * memory.
   */
  struct Header {
    /**
     * Magic number used for error check and block boundary recognition.
     */
    Magic magic;

    /**
     * True if this memory is available for allocation.
     */
    bool is_hole;

    /**
     * Size of the block in bytes, including the footer.
     */
    size_t size;
  };

  /**
   * Footer information placed at the end of every block of allocated memory.
   */
  struct Footer {
    /**
     * Magic number, same as in Header.
     */
    Magic magic;

    /**
     * Pointer to the block header.
     */
    Header *header;
  };

  class OrderedHeaderArray {
   public:
    explicit OrderedHeaderArray(uint32_t);
    OrderedHeaderArray(KHeap::Header**, uint32_t);
    ~OrderedHeaderArray();

    bool Insert(const KHeap::Header* item);
    KHeap::Header* operator[](const int index);
    void Remove(int i);

    int size() { return size_; }

  private:
    KHeap::Header** array_;
    int size_;
    int max_size_;
  };

  int32_t FindSmallestHole(size_t size, bool align);
  void Expand(size_t new_size);
  size_t Contract(size_t new_size);

  uint32_t start_address_;
  uint32_t end_address_;
  uint32_t max_address_;
  bool supervisor_;
  bool readonly_;
  OrderedHeaderArray index_;
};

}  // namespace alloc

#endif  // SRC_INCLUDE_MM_KHEAP_H_
