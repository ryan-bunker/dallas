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

Table* const Directory = reinterpret_cast<Table*>(0xfffff000);

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
  entry_ = static_cast<uint32_t>(frame.start_address()) | static_cast<uint32_t>(flags);
}

void Table::zero() {
  for (auto e : entries_) {
    e.set_unused();
  }
}

optional<size_t> Table::next_table_address(unsigned int index) const {
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

Table* const Table::next_table(unsigned int index) const {
  // screen::Writef("-- next_table( %d )\n", index);
  auto addr = next_table_address(index);
  if (addr)
    // screen::Writef("   address: 0x%x\n", address);
    return reinterpret_cast<Table* const>(*addr);
  return {};
}

Table& Table::next_table_create(unsigned int index, IFrameAllocator& allocator) {
  auto nxtTab = next_table(index);
  if (nxtTab)
    return *nxtTab;
  auto frame = allocator.Allocate().expect("no frames available");
  entries_[index].set(frame, Entry::Flags::Present | Entry::Flags::Writable);
  auto table = next_table(index);
  table->zero();
  return *table;
}

optional<Frame> translate_page(Page page) {
  // screen::Writef("-- translate_page( page #%d (addr=0x%x) )\n", page.index(), page.start_address());
  auto pt = Directory->next_table(page.directory_index());
  if (!pt)
    return {};
  auto entry = (*pt)[page.table_index()];
  // screen::Writef("   tbl idx: %d  entry: 0x%x\n", page.table_index(), entry);
  return entry.pointed_frame();
}

optional<paddress> translate(vaddress virtual_address) {
  auto offset = static_cast<size_t>(virtual_address % kPageSize);
  auto pg = Page::ContainingAddress(virtual_address);
  // screen::Writef("-- translate( 0x%x )\n", virtual_address);
  // screen::Writef("   offset: 0x%x\n", offset);
  auto frame = translate_page(pg);
  if (frame)
    return frame->start_address() / kPageSize + offset;

  auto entry = (*Directory)[pg.directory_index()];
  if (entry.is(Entry::Flags::Present)) {
    frame = entry.pointed_frame();
    if (!frame)
      return {};
    auto offset = static_cast<size_t>(virtual_address) & 0x3FFFFF;
    // screen::Writef("   huge page offset: 0x%x\n", offset);
    // screen::Writef("   huge page frame #%d @ 0x%x\n", f.index(), f.index() * (kPageSize * 1024));
    return paddress(frame->index() * (kPageSize * 1024) + offset);
  }

  // screen::Writef("   page table %d is not present\n", pg.directory_index());
  return {};
}

void map_to(Page page, Frame frame, Entry::Flags flags, IFrameAllocator& allocator) {
  auto pt = Directory->next_table_create(page.directory_index(), allocator);
  ASSERT(pt[page.table_index()].is_unused());
  pt[page.table_index()].set(frame, flags | Entry::Flags::Present);
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
