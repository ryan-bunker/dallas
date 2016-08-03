/**
 * @file page_allocator.cc
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

#include "mm/page_allocator.h"

#include <cstring>

namespace paging {

PageAllocator *PageAllocator::instance_ = nullptr;

PageAllocator::PageAllocator(size_t memory_available)
    : free_pages_(0), last_freed_index_(0) {
  page_count_ = memory_available / kPageSize;
  pages_ = new Page[page_count_];
  memset(pages_, 0, sizeof(Page) * page_count_);
  for (uint32_t i = 0; i < page_count_; i++)
    pages_[i].index = i;
}

PageAllocator::~PageAllocator() { delete[] pages_; }

Page *PageAllocator::AllocatePages(int count) {
  Page *free_page = nullptr;
  uint32_t i = last_freed_index_;
  do {
    if (pages_[i].available) {
      // we found a free page so begin looping to see if there are 'count' free
      // pages here
      uint32_t j = i + 1;
      for (; j < i + count; j++)
        if (!pages_[j].available)
          break;
      if (j == i + count)
        // count pages are free so return them
        free_page = pages_ + i;
      else
        // count pages not free..start looking again
        i = j;
    }

    if (++i >= page_count_)
      i = 0;
  } while (!free_page && i != last_freed_index_);

  if (free_page) {
    // we found a set of free pages, so mark them all used
    for (int i = 0; i < count; ++i)
      free_page[i].available = false;
  }

  return free_page;
}

void PageAllocator::FreePage(Page &page) {
  page.available = true;
  page.virtual_address = nullptr;
  last_freed_index_ = page.index;
  ++free_pages_;
}

} // namespace paging
