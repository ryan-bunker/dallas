/**
 * @file paging.cc
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
 *
 *
 */

#include "mm/paging.h"

#include <cstring>

#include "boot/multiboot.h"
#include "mm/page_allocator.h"
#include "mm/page_fault_handler.h"
#include "sys/kernel.h"

extern const uint32_t __kernel_end;

namespace paging {

// The kernel's page directory
PageTableEntry kernel_identity_page_table[1024] __attribute__((aligned(4096)));

PageDirectory* PageDirectory::current_directory_ = nullptr;
PageDirectory PageDirectory::kernel_directory_(0);

PageFaultHandler page_fault_handler;

PageDirectory::PageDirectory(addressing::paddress address)
  : physical_address_(address) {
  memset(tables_physical_, 0, sizeof(tables_physical_));

  // map the page directory into the last page of the address
  // space so that we can access it once paging is enabled
  tables_physical_[1023].page_table = physical_address_ >> 12;
  tables_physical_[1023].present = true;
  tables_physical_[1023].read_write = true;
  tables_physical_[1023].user = true;

  // now point tables to the upper 4MB of memory
  page_directory_entries_ = reinterpret_cast<PageDirectoryEntry*>(0xFFFFF000);
  page_table_entries_ = reinterpret_cast<PageTableEntry*>(0xFFC00000);
}

void PageDirectory::Activate() {
  current_directory_ = this;
  asm volatile (  "mov %0, %%eax\n"
          "mov %%eax, %%cr3\n"
          "mov %%cr0, %%eax\n"
          "orl $0x80000000, %%eax\n"
          "mov %%eax, %%cr0\n" :: "m" (physical_address_));
}

void PageDirectory::MapPage(Page* frame, addressing::vaddress virtAddress) {
  uint32_t pdeIdx = reinterpret_cast<uint32_t>(virtAddress) >> 22;
  uint32_t pteIdx = reinterpret_cast<uint32_t>(virtAddress) >> 12;
  PageDirectoryEntry* pde = &page_directory_entries_[pdeIdx];
  if (!pde->present) {
    // there is no page table present for this address, we need to map one in
    Page* ptf = PageAllocator::instance().AllocatePage();
    pde->present = true;
    pde->read_write = true;
    pde->user = true;
    pde->page_table = ptf->index;

    memset(&page_table_entries_[pdeIdx * 1024], 0,
           sizeof(PageTableEntry) * 1024);
  }

  PageTableEntry* pte = &page_table_entries_[pteIdx];
  if (pte->present) {
    // we're attempting to map a frame into an address that already has
    // a frame mapped, that is a big no-no
    PANIC("Attempting to map frame onto existing frame");
  }

  // we've got the table entry and there isn't anything already there so map it
  pte->present = true;
  pte->read_write = true;
  pte->user = true;
  pte->frame = frame->physical_address() >> 12;

  //char virt_address_byte = *static_cast<char*>(virtAddress);
  //__asm__ volatile("invlpg %0"::"m" (virt_address_byte));
  __asm__ volatile("invlpg %0"::"m" (virtAddress));
}

Page* PageDirectory::GetPage(addressing::vaddress virtAddress) {
  PageDirectoryEntry* pde =
      &page_directory_entries_[reinterpret_cast<uint32_t>(virtAddress) >> 20];
  if (!pde->present)
    return nullptr;

  PageTableEntry pte =
      page_table_entries_[reinterpret_cast<uint32_t>(virtAddress) >> 12];

  return &PageAllocator::instance().GetPage(pte.frame << 12);
}

Page* PageDirectory::UnmapPage(addressing::vaddress virtAddress) {
  // TODO: make this handle releasing the page table when the last frame is unmapped
  Page* frame = GetPage(virtAddress);
  if (!frame)
    return nullptr;

  // we have the frame now, time to unmap from the address space
  PageTableEntry* pte =
      &page_table_entries_[reinterpret_cast<uint32_t>(virtAddress) >> 12];
  pte->present = false;
  return frame;
}


void Initialize(uint32_t mmap_length, addressing::vaddress mmap_addr) {
  // record the physical address of the kernel's directory
  addressing::paddress phys =
      addressing::VirtualToPhysical(&PageDirectory::kernel_directory_);
  PageDirectory::kernel_directory_.physical_address_ = phys;
  PageDirectory::kernel_directory_.tables_physical_[1023].page_table = phys >> 12;

  // clear out the first page table which will be used for the kernel
  memset(&kernel_identity_page_table, 0, sizeof(kernel_identity_page_table));

  // set up the page table in the directory
  PageDirectoryEntry& zero_pde =
      PageDirectory::kernel_directory_.tables_physical_[0];
  zero_pde.page_table =
      addressing::VirtualToPhysical(&kernel_identity_page_table) >> 12;
  zero_pde.present = true;
  zero_pde.read_write = true;
  zero_pde.user = true;

  // identity map the first 4MB of memory
  for (int k = 0; k < 1024; ++k) {
    PageTableEntry& page = kernel_identity_page_table[k];
    page.present = true;      // mark it as present
    page.read_write = false;  // Should the page be writable?
    page.user = 1;            // Should the page be user-mode?
    page.frame = k;           // map the address onto itself
  }

  // point 3072MB...3076MB to the same page table as 0...4MB (this is where the kernel lives)
  PageDirectory::kernel_directory_.tables_physical_[768] =
      PageDirectory::kernel_directory_.tables_physical_[0];

  // build our free frame stack and initialize it with the remaining frames
  uint64_t total_memory = 0;

  multiboot::MemoryMapEntry *map;
  uint32_t mmap_end = reinterpret_cast<uint32_t>(mmap_addr) + mmap_length;
  // iterate over memory map to find the total amount of available memory
  for (uint32_t k = reinterpret_cast<uint32_t>(mmap_addr); k < mmap_end;) {
    map = reinterpret_cast<multiboot::MemoryMapEntry*>(k);
    if (map->type == multiboot::MemoryMapType::kMemoryAvailable)
      total_memory += map->length;
    k += map->size + sizeof(map->size);
  }

  PageAllocator::InitSingleton(total_memory);
  addressing::paddress end_of_kernel =
      addressing::VirtualToPhysical(const_cast<uint32_t*>(&__kernel_end));
  for (uint32_t k = reinterpret_cast<uint32_t>(mmap_addr); k < mmap_end;) {
    map = reinterpret_cast<multiboot::MemoryMapEntry*>(k);
    if (map->type == multiboot::MemoryMapType::kMemoryAvailable) {
      for (addressing::paddress j = map->address;
          j < map->address + map->length;
          j += paging::kPageSize) {
        // don't push any addresses that are inside the kernel
        if (j > end_of_kernel)
          PageAllocator::instance().FreePageAddress(j);
      }
    }
    k += map->size + sizeof(map->size);
  }

  // Before we enable paging, we must register our page fault handler.
  page_fault_handler.RegisterHandler();

  // Now, enable paging!
  PageDirectory::kernel_directory_.Activate();
}

}  // namespace paging
