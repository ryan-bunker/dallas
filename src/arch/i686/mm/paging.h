/**
 * @file paging.h
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
 * Definitions and signatures relating to memory management through paging.
 *
 */

#ifndef SRC_ARCH_I586_INCLUDE_MM_PAGING_H_
#define SRC_ARCH_I586_INCLUDE_MM_PAGING_H_

#include <cstddef>
#include <cstdint>
#include <experimental/optional>
#include <memory>

#include "mm/frame_allocator.h"
#include "sys/addressing.h"

using namespace addressing;
using namespace std::experimental;

namespace paging {

class Page {
public:
  static Page ContainingAddress(vaddress address);

  inline vaddress start_address() const { return index_ * kPageSize; }

  inline size_t directory_index() const { return (index_ >> 10) & 0x3FF; }
  inline size_t table_index() const { return index_ & 0x3FF; }

  inline size_t index() const { return index_; }
private:
  size_t index_;
};

/**
 * Represents a single entry inside of a page table.
 */
class Entry {
public:
  enum class Flags : uint32_t {
    // G0DACWURP
    // G - Global
    // D - Dirty
    // A - Accessed
    // C - Cache disabled
    // W - Write through
    // U - User/supervisor
    // R - Read/Write
    // P - Present
    None = 0,
    Present = 1 << 0,
    Writable = 1 << 1,
    UserAccessible = 1 << 2,
    WriteThrough = 1 << 3,
    CacheDisabled = 1 << 4,
    Accessed = 1 << 5,
    Dirty = 1 << 6,
    Size = 1 << 7,
    Global = 1 << 8,
  };

  inline bool is_unused() const { return entry_ == 0; }

  inline void set_unused() { entry_ = 0; }

  inline Flags flags() const { return static_cast<Flags>(entry_); }

  inline bool is(Flags testFlags) const;

  optional<Frame> pointed_frame();

  void set(Frame frame, Flags flags);

private:
  uint32_t entry_;

  //addressing::paddress const PhysicalAddr();
};

inline constexpr Entry::Flags operator|(Entry::Flags lhs, Entry::Flags rhs) {
  return (Entry::Flags)(static_cast<uint32_t>(lhs) |
                                 static_cast<uint32_t>(rhs));
}

inline constexpr Entry::Flags operator&(Entry::Flags lhs, Entry::Flags rhs) {
  return Entry::Flags(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

inline bool Entry::is(Flags testFlags) const {
  return (flags() & testFlags) == testFlags;
}

class Table {
public:
  Entry& operator[](const unsigned int index) { return entries_[index]; }
  const Entry& operator[](const unsigned int index) const { return entries_[index]; }

  void zero();

protected:
  Entry entries_[1024];
};

class PageTable : public Table {
};

class PageDirectory : public Table {
public:
  PageTable* const page_table(unsigned int index) const;

  PageTable* const page_table_create(unsigned int index, IFrameAllocator& allocator);

private:
  optional<size_t> page_table_address(unsigned int index) const;
};

class ActivePageDirectory {
public:
  ActivePageDirectory() : directory_(reinterpret_cast<PageDirectory*>(0xfffff000)) {}

  optional<paddress> translate(vaddress virtual_address) const;

  void map_to(Page page, Frame frame, Entry::Flags flags, IFrameAllocator& allocator);

  void map(Page page, Entry::Flags flags, IFrameAllocator& allocator);

  void identity_map(Frame frame, Entry::Flags flags, IFrameAllocator& allocator);

  void unmap(Page page, IFrameAllocator& allocator);

private:
  inline PageDirectory& directory() const { return *directory_; }

  optional<Frame> translate_page(Page page) const;

  std::unique_ptr<PageDirectory> directory_;
};

extern PageDirectory& Directory;

void test_paging(IFrameAllocator& allocator);

// class PageDirectoryEntry {
// public:
//   enum class Flags : uint32_t {
//     Present = 1 << 0,
//     Writable = 1 << 1,
//     UserAccessible = 1 << 2,
//     WriteThrough = 1 << 3,
//     Dirty = 1 << 4,
//     Accessed = 1 << 5,
//     PageSize = 1 << 7,
//     Global = 1 << 8,
//   };

//   inline bool is_unused() const { return entry_ == 0; }

//   inline void set_unused() { entry_ = 0; }

//   inline Flags flags() { return (Flags)entry_; }

// private:
//   uint32_t entry_;
// };

// inline PageDirectoryEntry::Flags operator|(PageDirectoryEntry::Flags lhs,
//                                            PageDirectoryEntry::Flags rhs) {
//   return (PageDirectoryEntry::Flags)(static_cast<uint32_t>(lhs) |
//                                      static_cast<uint32_t>(rhs));
// }

// inline bool operator&(PageDirectoryEntry::Flags lhs,
//                       PageDirectoryEntry::Flags rhs) {
//   return (static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)) != 0;
// }

// class PageDirectory {
// public:
//   static PageDirectory &I() {
//     return *reinterpret_cast<PageDirectory *>(0xFFC00000);
//   }

//   PageDirectoryEntry &operator[](const int index) { return entries_[index]; }
//   const PageDirectoryEntry &operator[](const int index) const {
//     return entries_[index];
//   }

//   void zero();

// private:
//   PageDirectoryEntry entries_[1024];
// };

// /**
//  * Represents a single entry inside a page directory.
//  */
// struct PageDirectoryEntry {
//   /**
//    * If true, the page is actually in physical memory at the moment. For
//    * example, when a page is swapped out, it is not in physical memory and
//    * therefore not 'present'. If a page is called, but not present, a page
//    * fault will occur, and the operating system should handle it.
//    */
//   bool present : 1;

//   /**
//    * If true, the page is read/write. Otherwise the page is read-only. The WP
//    * bit in CR0 determines if this is only applied to userspace, always
//    giving
//    * the kernel write access (the default) or both userspace and the kernel
//    * (see Intel Manuals 3A 2-20).
//    */
//   bool read_write : 1;

//   /**
//    * Controls access to the page based on privilege level. If true, then the
//    * page may be accessed by all. Otherwise, only the supervisor can access
//    it.
//    * If you wish to make a page a user page, you must set the user bit in the
//    * relevant page directory entry as well as the page table entry.
//    */
//   bool user : 1;

//   /**
//    * If true, write-through caching is enabled. Otherwise, write-back is
//    * enabled instead.
//    */
//   bool write_through : 1;

//   /**
//    * If true, the page will not be cached.
//    */
//   bool cache_disable : 1;

//   /**
//    * Used to discover whether a page has been read or written to. If it has,
//    * then accessed will be true. Note that this flag will not be cleared by
//    the
//    * CPU, so that burden falls on the operating system.
//    */
//   bool accessed : 1;

//   /**
//    * This flag is unused and must always be false.
//    */
//   bool always_false : 1;

//   /**
//    * If true, then pages are 4 MiB in size. Otherwise, they are 4 KiB. Please
//    * note that for 4 MiB pages PSE must be enabled.
//    */
//   bool size_4mb : 1;

//   /**
//    * Unused bits in the entry.
//    */
//   uint32_t unused : 4;

//   /**
//    * The upper 20 bits of the physical address of the page table that manages
//    * the four megabytes at that point. Please note that it is very important
//    * that this address be 4 KiB aligned. This is needed, due to the fact that
//    * the last bits of the DWORD are overwritten by access bits and such.
//    */
//   uint32_t page_table : 20;
// };

// /**
//  * Initialize's the kernel's paging directory.
//  */
// void InitKernelDirectory();

// /**
//  * Represents a memory mapped page directory.
//  */
// class PageDirectory {
// public:
//   /**
//    * Load this directory into the CR3 register, making it the active
//    directory
//    * that the CPU is using.
//    */
//   void Activate();

//   /**
//    * Gets the memory page that currently contains the specified virtual
//    address.
//    * @param address The virtual address to lookup.
//    * @return The memory page that \p address is currently mapped to.
//    */
//   Page *GetPage(addressing::vaddress address);

//   /**
//    * Unmaps the memory page mapped to the specified virtual address.
//    * @param address The virtual address to unmap.
//    * @return The page that was mapped to \p address and that is now unmapped.
//    */
//   Page *UnmapPage(addressing::vaddress address);

//   /**
//    * Gets the currently active page directory.
//    * @return The page directory that is currently active.
//    */
//   inline static PageDirectory *current_directory() {
//     return current_directory_;
//   }

//   /**
//    * Gets the kernel's page directory.
//    * @return The kerne's page directory.
//    */
//   inline static PageDirectory &kernel_directory() { return
//   *kernel_directory_; }

// private:
//   void MapPages(AddressPage pgaddr, addressing::paddress pa, size_t size,
//                 int perm);

//   /**
//    * Map a specified physical address to the specified virtual address.
//    * @param physAddress The physical address of memory to map.
//    * @param virtAddress The virtual address to map \p physAddress to.
//    */
//   void MapPage(addressing::paddress physAddress,
//                addressing::vaddress virtAddress);

//   /**
//    * Returns the address of the page table entry in this page directory that
//    * corresponds to the virtual address a.
//    * @param a The virtual address to lookup.
//    * @param allocate If true, allocate any required page table pages.
//    * @return The address of the Entry that corresponds to the given
//    * virtual address.
//    */
//   Entry *WalkPage(const addressing::vaddress a, bool allocate);

//   /**
//    * A pointer to the currently active page directory.
//    */
//   static PageDirectory *current_directory_;

//   /**
//    * The kerne's page directory.
//    */
//   static PageDirectory *kernel_directory_;

//   /**
//    * The physical stored page directory entries making up this page
//    directory.
//    */
//   PageDirectoryEntry tables_physical_[1024];
// };

} // namespace paging

#endif // SRC_ARCH_I586_INCLUDE_MM_PAGING_H_
