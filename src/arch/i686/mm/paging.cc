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
#include <experimental/optional>

#include "boot/multiboot.h"
#include "mm/frame_allocator.h"
#include "mm/page_fault_handler.h"
#include "sys/kernel.h"
#include "video/text_screen.h"

// /**
//  * Symbol provided by the linker indicating the beginning of the data
//  * section in memory.
//  */
// extern const uint32_t __kernel_data;

// /**
//  * Symbol provided by the linker indicating the end of the kernel in memory.
//  */
// extern const uint32_t __kernel_end;

// using namespace addressing;

namespace paging {

const uint32_t kTableAddressMask = 0xfffff000;

PageDirectory& Directory = *(reinterpret_cast<PageDirectory*>(0xfffff000));

Page Page::ContainingAddress(vaddress address) {
  Page pg;
  pg.index_ = static_cast<size_t>(address) / kPageSize;
  return pg;
} 

optional<Frame> Entry::pointed_frame() {
  if (is(Flags::Present)) {
    return Frame::ContainingAddress(paddress(entry_ & kTableAddressMask));
  } else {
    return {};
  }
}

void Entry::set(Frame frame, Flags flags) {
  ASSERT(frame.start_address().IsPageAligned());
  // screen::Writef("-- set( frame %d, flags %x ) (entry @ 0x%p)\n", frame.index(), flags, &entry_);
  entry_ = static_cast<uint32_t>(frame.start_address()) | static_cast<uint32_t>(flags);
}

void Table::zero() {
  for (int i=0; i<1024; ++i)
    entries_[i].set_unused();
}

optional<size_t> PageDirectory::page_table_address(unsigned int index) const {
  ASSERT(index < 1024);
  // screen::Writef("-- next_table_address( %d )\n", index);
  if (entries_[index].is(Entry::Flags::Present) && !entries_[index].is(Entry::Flags::Size)) {
    auto table_address = reinterpret_cast<size_t>(this);
    // screen::Writef("   table_address: 0x%x\n", table_address);
    return (table_address << 10) | (index << 12);
  }
  // screen::Writef("   entry %d is not present or is huge\n", index);
  return {};
}

PageTable* const PageDirectory::page_table(unsigned int index) const {
  // screen::Writef("-- page_table( %d )\n", index);
  auto addr = page_table_address(index);
  if (addr) {
    // screen::Writef("   address: 0x%x\n", *addr);
    return reinterpret_cast<PageTable* const>(*addr);
  }
  // screen::WriteLine("   not mapped");
  return {};
}

PageTable* const PageDirectory::page_table_create(unsigned int index, IFrameAllocator& allocator) {
  // screen::Writef("-- page_table_create( %d )\n", index);
  auto nxtTab = page_table(index);
  if (nxtTab) {
    // screen::Writef("   page table %d already exists\n", index);
    return nxtTab;
  }
  auto frame = *allocator.Allocate();
  // screen::Writef("   using frame %d as page table\n", frame.index());
  entries_[index].set(frame, Entry::Flags::Present | Entry::Flags::Writable);
  auto table = page_table(index);
  table->zero();
  return table;
}

optional<Frame> ActivePageDirectory::translate_page(Page page) const {
  // screen::Writef("-- translate_page( page #%d (addr=0x%x) )\n", page.index(), page.start_address());
  auto pt = directory_->page_table(page.directory_index());
  if (!pt)
    return {};
  auto entry = (*pt)[page.table_index()];
  // screen::Writef("   tbl idx: %d  entry: 0x%x\n", page.table_index(), entry);
  return entry.pointed_frame();
}

optional<paddress> ActivePageDirectory::translate(vaddress virtual_address) const {
  auto offset = static_cast<size_t>(virtual_address % kPageSize);
  auto pg = Page::ContainingAddress(virtual_address);
  // screen::Writef("-- translate( 0x%x )\n", virtual_address);
  // screen::Writef("   offset: 0x%x\n", offset);
  // screen::Writef("   page: 0x%x (dir: %d, tab: %d)\n", pg.index(), pg.directory_index(), pg.table_index());
  auto frame = translate_page(pg);
  if (frame)
    return frame->start_address() / kPageSize + offset;

  // either directory entry isn't present, directory entry is huge, or page table entry isn't present

  auto entry = (*directory_)[pg.directory_index()];
  // screen::Writef("   entry %d -> %x\n", pg.directory_index(), *(reinterpret_cast<uint32_t*>(&entry)));
  if (entry.is(Entry::Flags::Present | Entry::Flags::Size)) {
    frame = entry.pointed_frame();
    auto offset = static_cast<size_t>(virtual_address) & 0x3FFFFF;
    // screen::Writef("   huge page offset: 0x%x\n", offset);
    // screen::Writef("   huge page frame #%d @ 0x%x\n", frame->index(), frame->index() * (kPageSize * 1024));
    return paddress(frame->index() * (kPageSize * 1024) + offset);
  }

  // screen::Writef("   page table %d is not present\n", pg.directory_index());
  return {};
}

void ActivePageDirectory::map_to(Page page, Frame frame, Entry::Flags flags, IFrameAllocator& allocator) {
  auto pt = directory_->page_table_create(page.directory_index(), allocator);
  ASSERT((*pt)[page.table_index()].is_unused());
  // screen::Writef("   pt: %p, index: %d\n", pt, page.table_index());
  (*pt)[page.table_index()].set(frame, flags | Entry::Flags::Present);
}

void ActivePageDirectory::map(Page page, Entry::Flags flags, IFrameAllocator& allocator) {
  auto frame = *allocator.Allocate();
  map_to(page, frame, flags, allocator);
}

void ActivePageDirectory::identity_map(Frame frame, Entry::Flags flags, IFrameAllocator& allocator) {
  // frames represent physical memory, but pages represent virtual. In this case we're identity mapping
  // so we want them to be the same. Unfortunately that means jumping through some hoops to change types.
  auto page = Page::ContainingAddress(static_cast<vaddress>(static_cast<uint32_t>(frame.start_address())));
  map_to(page, frame, flags, allocator);
}

void ActivePageDirectory::unmap(Page page, IFrameAllocator& allocator) {
  ASSERT(translate(page.start_address()));

  // screen::Writef("-- unmap( page %d [dir: %d, tbl: %d] )\n", page.index(), page.directory_index(), page.table_index());
  auto pt = directory_->page_table(page.directory_index());
  auto frame = (*pt)[page.table_index()].pointed_frame();
  // screen::Writef("   mapped frame %d (starts at 0x%x)\n", frame->index(), frame->start_address());
  // screen::Writef("   pt: %p\n", pt);
  (*pt)[page.table_index()].set_unused();
  void *m = static_cast<void*>(page.start_address());
  __asm__ volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
  //allocator.Free(*frame);
}

void test_paging(IFrameAllocator& allocator) {
  ActivePageDirectory page_dir;

  auto translate = [&page_dir](const char *expected, uint32_t vaddr) {
    screen::Writef("%s = ", expected);
    auto t = page_dir.translate(vaddr);
    if (t)
      screen::Writef("Some(%d)\n", *t);
    else
      screen::WriteLine("None");
  };

  // address 0 is mapped
  translate("Some", 0);
  // second page table entry
  translate("Some", 4096);
  // first byte of second page
  translate("None", 4 * 1024 * 1024);
  // last mapped byte
  translate("Some", 4 * 1024 * 1024 - 1);

  screen::WriteLine("");
  auto addr = 2 * 4 * 1024 * 1024;
  auto page = Page::ContainingAddress(addr);
  auto frame = *allocator.Allocate();
  translate("None", addr);
  screen::Writef("    map to %d\n", frame.index());
  page_dir.map_to(page, frame, Entry::Flags::None, allocator);
  translate("Some", addr);
  auto next_frame = allocator.Allocate();
  if (next_frame)
    screen::Writef("next free frame: Some(%d)\n", next_frame->index());
  else
    screen::WriteLine("next free frame: None");

  auto p = reinterpret_cast<uint32_t*>(static_cast<size_t>(Page::ContainingAddress(addr).start_address()));
  screen::Writef("  before unmap: value @ 0x%p == 0x%x\n", p, *p);

  page_dir.unmap(Page::ContainingAddress(addr), allocator);
  translate("None", addr);

  // the following line will cause a page fault since we just unmapped this address
  // screen::Writef("  after unmap: value @ 0x%p == 0x%x\n", p, *p);
}

// void PageDirectory::zero() {
//   for (auto e : entries_) {
//     e.set_unused();
//   }
// }

// static const unsigned int kPgTblIdxShift = 12;
// static const unsigned int kPgDirIdxShift = 22;

// static inline uint32_t PgTblIdx(vaddress a) {
//   return (a.Raw() >> kPgTblIdxShift) & 0x3FF;
// }

// static inline uint32_t PgDirIdx(vaddress a) {
//   return (a.Raw() >> kPgDirIdxShift) & 0x3FF;
// }

// static inline vaddress PageRoundUp(vaddress a) {
//   return vaddress((a.Raw() + kPageSize - 1) & ~(kPageSize - 1));
// }

// static inline vaddress PageRoundDown(vaddress a) {
//   return vaddress(a.Raw() & ~(kPageSize - 1));
// }

// static inline void lcr3(addressing::paddress addr) {
//   asm volatile("movl %0,%%cr3" : : "r"(addr.Raw()));
// }

// /**
//  * The page directory used by the kernel.
//  */
// Entry kernel_identity_page_table[1024]
// __attribute__((aligned(4096)));

// PageDirectory *PageDirectory::current_directory_ = nullptr;
// PageDirectory *PageDirectory::kernel_directory_ = nullptr;

// /**
//  * The kernel's handler for page faults. Responsible for paging memory to and
//  * from disk, as well as allocating new pages as needed.
//  */
// PageFaultHandler page_fault_handler;

// paddress const Entry::PhysicalAddr() {
//   return frame << kPgTblIdxShift;
// }

// void PageDirectory::Activate() {
//   current_directory_ = this;
//   vaddress v(reinterpret_cast<uint32_t>(tables_physical_));
//   lcr3(v.ToPhysical());
// }

// void PageDirectory::MapPages(AddressPage pgaddr, paddress pa, size_t size,
//                              int perm) {
//   //   vaddress a = PageRoundDown(va);
//   //   vaddress last = PageRoundDown((vaddress)((size_t)va + size - 1));
//   //   for (;;) {
//   //     MapPage(pa, a, perm);
//   //     if (a == last)
//   //       break;
//   //     a += kPageSize;
//   //     pa += kPageSize;
//   //   }
// }

// void PageDirectory::MapPage(paddress physAddress,
//                             addressing::vaddress virtAddress) {
//   //   Entry *pte = WalkPage(virtAddress, true);
//   //   if (pte == nullptr)
//   //     return;

//   //   if (pte->present)
//   //     PANIC("remap");

//   //   pte->present = true;
//   //   pte->address = physAddress >> PgTblIdx;
// }

// Page *PageDirectory::GetPage(addressing::vaddress virtAddress) {
//   //   PageDirectoryEntry *pde =
//   //   &page_directory_entries_[PgDirIdx(virtAddress)];
//   //   if (!pde->present)
//   //     return nullptr;

//   //   Entry pte = page_table_entries_[PgTblIdx(virtAddress)];

//   //   return &PageAllocator::instance().GetPage(pte.frame << 12);
//   return nullptr;
// }

// Page *PageDirectory::UnmapPage(addressing::vaddress virtAddress) {
//   //   // TODO: make this handle releasing the page table when the last frame
//   is
//   //   // unmapped
//   //   Page *frame = GetPage(virtAddress);
//   //   if (!frame)
//   //     return nullptr;

//   //   // we have the frame now, time to unmap from the address space
//   //   Entry *pte =
//   //       &page_table_entries_[reinterpret_cast<uint32_t>(virtAddress) >>
//   12];
//   //   pte->present = false;
//   //   return frame;
//   return nullptr;
// }

// Entry *PageDirectory::WalkPage(vaddress a, bool allocate) {
//   //   PageDirectoryEntry *pde = &page_directory_entries_[PgDirIdx(a)];
//   //   Entry *pte = nullptr;

//   //   if (pde->present)
//   //     pte = reinterpret_cast<Entry *>(
//   //         PhysicalToVirtual(pde->PhysicalAddr()));
//   //   else {
//   //     if (!allocate || (pte = FrameAllocator::instance().Allocate()))
//   //       return nullptr;

//   //     // Make sure all those PTE_P bits are zero.
//   //     memset(pte, 0, kPageSize);
//   //     // The permissions here are overly generous, but they can
//   //     // be further restricted by the permissions in the page table
//   //     // entries, if necessary.
//   //     memset(pde, 0, sizeof(PageDirectoryEntry));
//   //     pde->page_table = VirtualToPhysical(pte) >> kPgDirIdxShift;
//   //     pde->present = true;
//   //     pde->read_write = true;
//   //     pde->user = true;
//   //   }

//   //   return &pte[PgTblIdx(a)];
//   return nullptr;
// }

// // There is one page table per process, plus one that's used when
// // a CPU is not running any process (kpgdir). The kernel uses the
// // current process's page table during system calls and interrupts;
// // page protection bits prevent user code from using the kernel's
// // mappings.

// // setupkvm() and exec() set up every page table like this:

// //   0..KERNBASE: user memory (text+data+stack+heap), mapped to
// //                phys memory allocated by the kernel
// //   KERNBASE..KERNBASE+EXTMEM: mapped to 0..EXTMEM (for I/O space)
// //   KERNBASE+EXTMEM..data: mapped to EXTMEM..V2P(data)
// //                for the kernel's instructions and r/o data
// //   data..KERNBASE+PHYSTOP: mapped to V2P(data)..PHYSTOP,
// //                                  rw data + free physical memory
// //   0xfe000000..0: mapped direct (devices such as ioapic)

// // The kernel allocates physical memory for its heap and for user memory
// // between V2P(end) and the end of physical memory (PHYSTOP)
// // (directly addressable from end..P2V(PHYSTOP)).

// // This table defines the kernel's mappings, which are present in
// // every process's page table.
// // static struct kmap {
// //   vaddress virt;
// //   paddress phys_start;
// //   paddress phys_end;
// //   int perm;
// // } kmap[] = {
// //     {kKernelBase, 0, kExtMemory, 2}, // I/O space
// //     {kKernelLink, VirtualToPhysical(kKernelLink),
// //      VirtualToPhysical(paddress(&data)), 0}, // kern text+rodata
// //     {vaddress(&data), VirtualToPhysical(paddress(&data)), PHYSTOP,
// //      2}, // kern data+memory
// //          // { (void*)DEVSPACE, DEVSPACE,      0,         PTE_W}, // more
// //          devices
// // };

// void InitKernelDirectory() {
//   //   vaddress v = FrameAllocator::instance().Allocate();
//   //   kernel_directory_ = new (v) PageDirectory();

//   //   memset(kernel_directory_, 0, kPageSize);
//   //   for (int k = 0; k < 3; k++)
//   //     kernel_directory_->MapPages(kmap[k]->virt,
//   //                                 kmap[k]->phys_end - kmap[k]->phys_start,
//   //                                 (uint)kmap[k]->phys_start,
//   kmap[k]->perm);
// }

} // namespace paging
