/**
 * @file main_entry.cc
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
 * Main entry point of kernel after initial environment has been set up
 * by the boot loader.
 */

#include <cstdint>

#include "boot/multiboot.h"
#include "int/idt.h"
#include "mm/gdt.h"
#include "mm/kheap.h"
#include "mm/paging.h"
#include "mm/page_allocator.h"
#include "sys/addressing.h"
#include "sys/io.h"
#include "video/text_screen.h"

namespace {

unsigned char heap_memory[sizeof(alloc::KHeap)];
alloc::KHeap* kernel_heap = nullptr;

void InitializeKernelHeap() {
  // set up the heap boundaries (1MiB initial, 2MiB max)
  // 0xC0400000  <-- 1MiB -->  0xC0500000  <-- 1MiB -->  0xC0600000
  uint32_t heap_start = static_cast<uint32_t>(0xC0400000);
  if (heap_start % paging::kPageSize)
    heap_start += paging::kPageSize - (heap_start % paging::kPageSize);
  uint32_t heap_end = heap_start + 0x100000;
  uint32_t max_heap = heap_end + 0x100000;

  // now map in the necessary pages for the heap
  paging::Page* page = paging::PageAllocator::instance().AllocatePages(
      (heap_end - heap_start) / paging::kPageSize);
  for (uint32_t addr = heap_start; addr < heap_end; addr += paging::kPageSize)
    paging::PageDirectory::kernel_directory().MapPage(
        page++, reinterpret_cast<void*>(addr));

  // create the heap instance and tell it the memory to manage
  kernel_heap = new (static_cast<void*>(heap_memory))
      alloc::KHeap(reinterpret_cast<void*>(heap_start),
                   reinterpret_cast<void*>(heap_end),
                   reinterpret_cast<void*>(max_heap),
                   false, false);

  // make the heap our active allocator
  alloc::SetActiveAllocator(*kernel_heap);
}

/**
 * Main entry point into kernel from loader assembly.
 * @param mbd The multiboot information structure.
 * @param magic Must match kBootloaderMagic to verify that mbd is valid.
 */
extern "C" void kmain(multiboot::Info *mbd, uint32_t magic) {
  if (magic != multiboot::kBootloaderMagic) {
    // Something went not according to specs. Print an error
    // message and halt, but do *not* rely on the multiboot
    // data structure.
    return;
  }

  // mbd is currently pointing to physical memory so we need
  // to adjust it for our current GDT offsets
  mbd = (multiboot::Info*) addressing::PhysicalToVirtual(
      reinterpret_cast<addressing::paddress>(mbd));

  // first step is to initialize paging
  paging::Initialize(mbd->mmap_length,
                     addressing::PhysicalToVirtual(mbd->mmap_addr));
  // then we can restore the GDT back to normal and initialize interrupts
  gdt::Initialize();
  idt::Initialize();

  // allocate this before we set up the heap
  int* a = new int;

  // get the Heap ready
  InitializeKernelHeap();

  // now that everything is set up we can enable interrupts again
  enable_interrupts();

  screen::Clear();
  screen::WriteLine("Dallas");

  int* b = new int;

  *a = 1;
  *b = 2;

  screen::Write("0x");
  screen::WriteHex(reinterpret_cast<uint32_t>(a));
  screen::Write(" = ");
  screen::WriteDec(*a);
  screen::WriteLine(" (allocated from kernel space)");

  screen::Write("0x");
  screen::WriteHex(reinterpret_cast<uint32_t>(b));
  screen::Write(" = ");
  screen::WriteDec(*b);
  screen::WriteLine(" (allocated from the heap)");

  screen::WriteLine("Now we're going to page fault at 0x500000...");

  // this memory is not in the first 4MB or in the kernel's 4MB
  *reinterpret_cast<int*>(0x500000) = 1234;

  for (;;)
    continue;
}

}  // namespace
