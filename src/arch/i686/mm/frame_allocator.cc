/**
 * @file frame_allocator.cc
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

#include "mm/frame_allocator.h"

#include "sys/kernel.h"
// #include "video/text_screen.h"

extern uint32_t __kernel_start, __kernel_end;

namespace paging {

AreaFrameAllocator::AreaFrameAllocator(paddress kernelStart, paddress kernelEnd, paddress multibootStart, paddress multibootEnd)
    : next_free_frame_(0), kernel_start_(0), kernel_end_(0), multiboot_start_(0), multiboot_end_(0), current_area_(), areas_count_(0) {
    kernel_start_ = Frame::ContainingAddress(kernelStart);
    kernel_end_ = Frame::ContainingAddress(kernelEnd);
    multiboot_start_ = Frame::ContainingAddress(multibootStart);
    multiboot_end_ = Frame::ContainingAddress(multibootEnd);
}

void AreaFrameAllocator::RegisterMemoryArea(paddress start, size_t size) {
  if (++areas_count_ >= 32) {
    PANIC("Exceeded maximum available memory areas");
  }

  areas_[areas_count_ - 1].address = start;
  areas_[areas_count_ - 1].size = size;

  if (areas_count_ == 1)
    ChooseNextArea();
}

Maybe<Frame> AreaFrameAllocator::Allocate() {
  // screen::WriteLine("-- Allocate()...");
  // screen::Writef("      current_area_.is_set() = %d, areas_count_ = %d, next_free_frame_ = %d\n",
  //   current_area_.is_set() ? 1 : 0, areas_count_, next_free_frame_.index());
  return maybe_if(current_area_, [&](MemoryArea area) {
    // make a copy of next_free_frame_ that we can return
    Frame frame = next_free_frame_;

    auto current_area_last_frame = Frame::ContainingAddress(area.address + area.size - 1);

    // screen::Writef("      area = {address: %x, size: %d}, frame = %d, ca_last_frame = %d\n",
    //                area.address, area.size, frame.index(),
    //                current_area_last_frame.index());
    if (frame > current_area_last_frame) {
      // all frames of current area are used, switch to next area
      // screen::WriteLine(
      //     "      all frames in current area in use, moving to next area");
      ChooseNextArea();
    } else if (frame >= kernel_start_ && frame <= kernel_end_) {
      // 'frame' is used by the kernel
      // screen::WriteLine("      frame is in use by the kernel");
      next_free_frame_ = kernel_end_ + 1;
    } else if (frame >= multiboot_start_ && frame <= multiboot_end_) {
      // 'frame' is used by the multiboot information structure
      // screen::WriteLine("      frame is in use by the multiboot information structure");
      next_free_frame_ = multiboot_end_ + 1;
    } else {
      // frame is unused, increment next_free_frame_ and return it
      // screen::WriteLine("      frame is available");
      ++next_free_frame_;
      return Maybe<Frame>(frame);
    }
    // frame was not valid, try again with the updated next_free_frame_
    // screen::WriteLine("      frame was not valid, retrying");
    return Allocate();
  });
}

void AreaFrameAllocator::ChooseNextArea() {
  current_area_ = Maybe<MemoryArea>::None;
  // screen::WriteLine("-- ChooseNextArea()...");
  // screen::Writef("      next_free_frame_ = %d\n", next_free_frame_.index());
  // screen::Writef("      areas_count_ = %d\n", areas_count_);
  for (int i = 0; i < areas_count_; ++i) {
    auto address = areas_[i].address + areas_[i].size - 1;
    // screen::Writef("      areas_[%d] = {address: %x, size: %d}\n", i,
    //                areas_[i].address, areas_[i].size);
    if (Frame::ContainingAddress(address) < next_free_frame_)
      continue;

    maybe_if(current_area_, [&](MemoryArea area) {
      // screen::Writef("      current_area_ = {address: %x, size: %d}\n", area.address, area.size);
      if (areas_[i].address < area.address) {
        // screen::Writef("      areas_[%d] is smaller, setting current_area_\n", i);
        current_area_ = areas_[i];
      }
    }).otherwise([&]() {
      // screen::Writef("      current_area_ = None, setting current_area_\n");
      current_area_ = areas_[i];
    });
  }

  maybe_if(current_area_, [&](const MemoryArea& area) {
    auto start_frame = Frame::ContainingAddress(area.address);
    if (next_free_frame_ < start_frame) {
      // screen::Writef("      advancing next_free_frame_ to %d\n", start_frame.index());
      next_free_frame_ = start_frame;
    }
  });
}

void AreaFrameAllocator::Free(Frame f) {
  PANIC("AreaFrameAllocator::Free not implemented!");
}

}
