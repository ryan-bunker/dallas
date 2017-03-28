// /**
//  * @file page_allocator.h
//  *
//  * @section LICENSE
//  *
//  * Copyright (C) 2013  Ryan Bunker
//  *
//  * This program is free software: you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License as published by
//  * the Free Software Foundation, either version 3 of the License, or
//  * (at your option) any later version.
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program.  If not, see [http://www.gnu.org/licenses/].
//  *
//  */

// #ifndef SRC_ARCH_I586_INCLUDE_MM_PAGE_ALLOCATOR_H_
// #define SRC_ARCH_I586_INCLUDE_MM_PAGE_ALLOCATOR_H_

// #include <cstddef>
// #include <cstdint>

// #include "sys/addressing.h"

// namespace paging {

// /**
//  * The size of a memory page in bytes.
//  */
// const size_t kPageSize = 0x1000;

// /**
//  * Bitmask to page align an integer.
//  */
// const uint32_t kPageAlignMask = 0xFFFFF000;

// /**
//  * Contains information about an individual memory page.
//  */
// struct Page {
//   //  uint32_t flags;

//   /**
//    * The index of the page in a page table.
//    */
//   uint32_t index;

//   /**
//    * Indicates whether the page is available for mapping.
//    */
//   bool available;

//   /**
//    * The virtual address that the page is currently mapped to.
//    */
//   addressing::vaddress virtual_address;

//   /**
//    * Gets the physical address of the page in memory.
//    * @return The physical address of this page in memory.
//    */
//   inline addressing::paddress physical_address() const {
//     return index * kPageSize;
//   }
// };

// /**
//  * Allocator used to manage free and used memory pages.
//  */
// class PageAllocator {
// public:
//   /**
//    * Initializes the single instance of this allocator.
//    * @param memory_available The amount of physical memory available to the
//    * machine.
//    */
//   static void InitSingleton(size_t memory_available) {
//     instance_ = new PageAllocator(memory_available);
//   }

//   /**
//    * Gets the single instance of this allocator.
//    * @return The single allocator instance.
//    */
//   static PageAllocator &instance() { return *instance_; }

//   /**
//    * Gets the number of available memory pages.
//    * @return The number of available memory pages.
//    */
//   inline uint32_t free_pages() const { return free_pages_; }

//   /**
//    * Gets the number of used memory pages.
//    * @return The number of used memory pages.
//    */
//   inline uint32_t used_pages() const { return page_count_ - free_pages_; }

//   /**
//    * Allocate a specified number of memory pages.
//    * @param count The number of pages to allocate.
//    * @return The first allocated page.
//    */
//   Page *AllocatePages(int count);

//   /**
//    * Allocate a single memory page.
//    * @return The allocated page.
//    */
//   inline Page *AllocatePage() { return AllocatePages(1); }

//   /**
//    * Return a page back to the system and mark it available.
//    * @param page The page to free.
//    */
//   void FreePage(Page &page);

//   /**
//    * Return the page that corresponds to the specified address back to the
//    * system and mark it available.
//    * @param address The physical address of the memory page to free.
//    */
//   inline void FreePageAddress(addressing::paddress address) {
//     FreePage(GetPage(address));
//   }

//   /**
//    * Returns the memory page that corresponds to the specified physical address.
//    * @param address The physical address of the page to retrieve.
//    * @return The memory page that corresponds to address.
//    */
//   inline Page &GetPage(addressing::paddress address) const {
//     return pages_[address / kPageSize];
//   }

// private:
//   /**
//    * Creates a new PageAllocator instance.
//    * @param memory_available The amount of physical memory available.
//    */
//   explicit PageAllocator(size_t memory_available);

//   virtual ~PageAllocator();

//   /**
//    * The single instance of this allocator.
//    */
//   static PageAllocator *instance_;

//   /**
//    * The complete list of memory pages, both used and free.
//    */
//   Page *pages_;

//   /**
//    * The total number of memory pages contained in pages_.
//    */
//   uint32_t page_count_;

//   /**
//    * The number of available memory pages.
//    */
//   uint32_t free_pages_;

//   /**
//    * The index of the last page to have been freed.
//    */
//   uint32_t last_freed_index_;
// };

// } // namespace paging

// #endif // SRC_ARCH_I586_INCLUDE_MM_PAGE_ALLOCATOR_H_
