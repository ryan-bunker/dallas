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

#include <cstddef>
#include <cstdint>

#include "boot/multiboot2.h"
#include "mm/frame_allocator.h"
#include "mm/paging.h"
#include "sys/addressing.h"
#include "video/text_screen.h"

extern const uint32_t __kernel_start, __kernel_data, __kernel_end;

namespace {

// unsigned char heap_memory[sizeof(alloc::KHeap)];
// alloc::KHeap *kernel_heap = nullptr;

// void InitializeKernelHeap() {
//   // set up the heap boundaries (1MiB initial, 2MiB max)
//   // 0xC0400000  <-- 1MiB -->  0xC0500000  <-- 1MiB -->  0xC0600000
//   uint32_t heap_start = static_cast<uint32_t>(0xC0400000);
//   if (heap_start % paging::kPageSize)
//     heap_start += paging::kPageSize - (heap_start % paging::kPageSize);
//   uint32_t heap_end = heap_start + 0x100000;
//   uint32_t max_heap = heap_end + 0x100000;

//   // now map in the necessary pages for the heap
//   paging::Page *page = paging::PageAllocator::instance().AllocatePages(
//       (heap_end - heap_start) / paging::kPageSize);
//   for (uint32_t addr = heap_start; addr < heap_end; addr +=
//   paging::kPageSize)
//     paging::PageDirectory::kernel_directory().MapPage(
//         page++, reinterpret_cast<void *>(addr));

//   // create the heap instance and tell it the memory to manage
//   kernel_heap = new (static_cast<void *>(heap_memory)) alloc::KHeap(
//       reinterpret_cast<void *>(heap_start), reinterpret_cast<void
//       *>(heap_end),
//       reinterpret_cast<void *>(max_heap), false, false);

//   // make the heap our active allocator
//   alloc::SetActiveAllocator(*kernel_heap);
// }

/**
 * Main entry point into kernel from loader assembly.
 * @param mbd The multiboot information structure.
 * @param magic Must match kBootloaderMagic to verify that mbd is valid.
 */
extern "C" void kmain(multiboot2::Info *mbd, uint32_t magic) {
  screen::Clear();
  screen::Writef("Dallas\n");
  screen::Writef("mbd: 0x%p\n", mbd);

  if (magic != multiboot2::kBootloaderMagic) {
    // Something went not according to specs. Print an error
    // message and halt, but do *not* rely on the multiboot
    // data structure.
    screen::Write("BAD MULTIBOOT!!!", screen::Color::kWhite, screen::Color::kRed);
    return;
  }

  screen::Write("GOOD MULTIBOOT\n\n", screen::Color::kGreen, screen::Color::kBlack);

  // auto mem_tag = mbd->basic_memory();
  // if (mem_tag == nullptr)
  //   screen::WriteLine("-- no basic memory info --");
  // else
  //   screen::Writef("mem_lower=%d  mem_upper=%d\n", mem_tag->mem_lower, mem_tag->mem_upper);
  
  // auto boot_tag = mbd->bios_bootdevice();
  // if (boot_tag == nullptr)
  //   screen::WriteLine("-- no bios bootdevice --");
  // else
  //   screen::Writef("biosdev=%d  partition=%d  sub_partition=%d\n", boot_tag->biosdev, boot_tag->partition, boot_tag->sub_partition);
  
  // uint32_t kernel_start = 0xFFFFFFFF, kernel_end = 0;
  // auto elf_tag = mbd->elf_symbols();
  // if (elf_tag == nullptr)
  //   screen::WriteLine("-- no elf symbols --");
  // else {
  //   screen::Writef("size=%d  num=%d  entsize=%d  shndx=%d\n",
  //     elf_tag->size, elf_tag->num, elf_tag->entsize, elf_tag->shndx);
  //   for (int i=1; i<elf_tag->num; i++) {
  //     screen::Writef("    [%d] %s @ 0x%x, size: 0x%x, flags: 0x%x\n",
  //       i, elf_tag->section_name(i),
  //       elf_tag->sections[i].sh_addr, elf_tag->sections[i].sh_size, elf_tag->sections[i].sh_flags);

  //     if (elf_tag->sections[i].sh_addr < kernel_start)
  //       kernel_start = elf_tag->sections[i].sh_addr;
  //     if (elf_tag->sections[i].sh_addr + elf_tag->sections[i].sh_size > kernel_end)
  //       kernel_end = elf_tag->sections[i].sh_addr + elf_tag->sections[i].sh_size;
  //   }

  //   kernel_start = static_cast<uint32_t>(addressing::vaddress(kernel_start).ToPhysical());
  //   kernel_end = static_cast<uint32_t>(addressing::vaddress(kernel_end).ToPhysical());

  //   screen::Writef("__kernel_start: 0x%x, __kernel_end: 0x%x\n", &__kernel_start, &__kernel_end);
  // }

  addressing::vaddress kernel_start(&__kernel_start);
  addressing::vaddress kernel_end(&__kernel_end);
  screen::Writef("kernel_start   : 0x%x, kernel_end   : 0x%x\n", kernel_start, kernel_end);

  auto multiboot_start = static_cast<addressing::vaddress>(mbd);
  auto multiboot_end = multiboot_start + static_cast<size_t>(mbd->total_size);
  screen::Writef("multiboot_start: 0x%x, multiboot_end: 0x%x\n", multiboot_start, multiboot_end);

  paging::AreaFrameAllocator allocator(
    kernel_start.ToPhysical(),
    kernel_end.ToPhysical(),
    multiboot_start.ToPhysical(),
    multiboot_end.ToPhysical());

  auto mem_map = mbd->memory_map();
  if (mem_map == nullptr)
    screen::WriteLine("-- no memory map --");
  else {
    //screen::Writef("entry_size=%d  entry_version=%d  %d entries\n", mem_map->entry_size, mem_map->entry_version, mem_map->entries());
    screen::WriteLine("memory areas:");
    for (int i=0; i<mem_map->entries(); i++) {
      auto memEntry = mem_map->entry(i);
      if (memEntry->type != 1)
        continue;
      screen::Writef("    start: 0x%x, length: 0x%x\n", memEntry->base_addr_lo, memEntry->length_lo);
      allocator.RegisterMemoryArea(static_cast<addressing::paddress>(memEntry->base_addr_lo), memEntry->length_lo);
    }
  }

  paging::test_paging(allocator);

  for (;;)
    continue;

/*
  // mbd->mmap_length
  screen::Writef("Memory map (%d bytes @ 0x%x)\n", mbd->mmap_length,
                 mbd->mmap_addr);
  for (uint32_t addr = 0; addr < mbd->mmap_length;) {
    auto memEntry =
        reinterpret_cast<multiboot::MemoryMapEntry *>(mbd->mmap_addr + addr);

    screen::Writef("  [%d @ %x]  0x%x %d bytes (type %d)", memEntry->size,
                   mbd->mmap_addr + addr, uint32_t(memEntry->address),
                   uint32_t(memEntry->length), uint32_t(memEntry->type));

    if (memEntry->type == multiboot::MemoryMapType::kMemoryAvailable) {
      screen::Write("  registered");
      paging::FrameAllocator::I().RegisterMemoryArea(
          paddress(memEntry->address), size_t(memEntry->length));
    }

    screen::WriteLine("");

    addr += memEntry->size + 4;
  }

  screen::Writef("ELF sections (%d entries, %d bytes @ 0x%x)\n", mbd->u.elf_section.num, mbd->u.elf_section.size, mbd->u.elf_section.address);

  screen::Writef("multiboot - start: 0x%p  data: 0x%p  end: 0x%p\n",
                 &__kernel_start, &__kernel_data, &__kernel_end);

  for (;;)
    continue;
*/
  // Free the 4K frames that exist within the first 4M that we already
  // mapped, but that aren't being used by kernel code. This is important
  // because we need some basic memory in order to setup the kernel page
  // directory next.
  // addressing::paddress end_of_kernel =
  // addressing::VirtualToPhysical(const_cast<uint32_t *>(&__kernel_end));
  // addressing::vaddress vStart(reinterpret_cast<uint32_t>(&__kernel_end));
  // addressing::vaddress vEnd = addressing::paddress(4 * 1024 *
  // 1024).ToVirtual();
  // paging::FreeFrameRange(vStart, vEnd);

  // paging::InitKernelDirectory();

  // Now we want to setup the kernel's page table. This is the page table
  // that will be used by things that exist solely in the kernel, such
  // as the process scheduler.
  // paging::PageDirectory::kernel_directory().Activate();

  /*
    // first step is to initialize paging
    paging::Initialize(mbd->mmap_length,
                       addressing::PhysicalToVirtual(mbd->mmap_addr));
    // then we can restore the GDT back to normal and initialize interrupts
    gdt::Initialize();
    idt::Initialize();

    // allocate this before we set up the heap
    int *a = new int;

    // get the Heap ready
    InitializeKernelHeap();

    // now that everything is set up we can enable interrupts again
    enable_interrupts();
  */

  /*
    int *b = new int;

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
    *reinterpret_cast<int *>(0x500000) = 1234;
  */
  for (;;)
    continue;
}

} // namespace
