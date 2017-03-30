/**
 * @file frame_allocator.h
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
 */

#ifndef SRC_ARCH_I586_INCLUDE_MM_FRAME_ALLOCATOR_H_
#define SRC_ARCH_I586_INCLUDE_MM_FRAME_ALLOCATOR_H_

#include <cstddef>
#include <experimental/optional>

#include "sys/addressing.h"

using namespace addressing;
using std::experimental::optional;

namespace paging {

/**
 * The size of a memory page in bytes.
 */
const size_t kPageSize = 0x1000;

class PageTableEntry;

/**
 * Contains information about a frame of physical memory.
 */
class Frame {
public:
  Frame(size_t index) : index_(index) {}

  inline size_t index() const { return index_; }

  inline bool operator==(const Frame &rhs) { return index_ == rhs.index_; }
  inline bool operator>(const Frame &rhs) { return index_ > rhs.index_; }
  inline bool operator<(const Frame &rhs) { return index_ < rhs.index_; }
  inline bool operator>=(const Frame &rhs) { return index_ >= rhs.index_; }
  inline bool operator<=(const Frame &rhs) { return index_ <= rhs.index_; }
  inline Frame operator+(int n) const { return Frame(index_ + n); }
  inline Frame operator-(int n) const { return Frame(index_ - n); }
  inline Frame operator++() { ++index_; return *this; }
  inline Frame operator--() { --index_; return *this; }

  addressing::paddress start_address() { return index_ * kPageSize; }

  inline static Frame ContainingAddress(paddress a) {
    return Frame(static_cast<size_t>(a) / kPageSize);
  }

private:

  /**
   * The index of the frame in the system. The physical address of
   * the beginning of the frame is index * frame size.
   */
  size_t index_;

  friend class PageTableEntry;
};

class IFrameAllocator {
public:
  virtual optional<Frame> Allocate() = 0;
  virtual void Free(Frame f) = 0;
};

class AreaFrameAllocator : public IFrameAllocator {
public:
  AreaFrameAllocator(paddress kernelStart, paddress kernelEnd, paddress multibootStart, paddress multibootEnd);

  void RegisterMemoryArea(paddress start, size_t size);

  optional<Frame> Allocate();

  void Free(Frame f);

private:
  struct MemoryArea {
    paddress address;
    size_t size;
    MemoryArea() : address(0), size(0) {}
  };

  void ChooseNextArea();

  Frame next_free_frame_;
  Frame kernel_start_;
  Frame kernel_end_;
  Frame multiboot_start_;
  Frame multiboot_end_;
  optional<MemoryArea> current_area_;
  int areas_count_;
  MemoryArea areas_[32];
};

}

#endif