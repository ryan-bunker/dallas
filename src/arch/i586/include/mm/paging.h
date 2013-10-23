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

#include <cstdint>
#include "sys/addressing.h"

namespace paging {

struct Page;

/**
 * Represents a single entry inside of a page table.
 */
struct PageTableEntry {
  /**
   * If true, the page is actually in physical memory at the moment. For
   * example, when a page is swapped out, it is not in physical memory and
   * therefore not 'present'. If a page is called, but not present, a page
   * fault will occur, and the operating system should handle it.
   */
  bool present : 1;

  /**
   * If true, the page is read/write. Otherwise the page is read-only. The WP
   * bit in CR0 determines if this is only applied to userspace, always giving
   * the kernel write access (the default) or both userspace and the kernel
   * (see Intel Manuals 3A 2-20).
   */
  bool read_write : 1;

  /**
   * Controls access to the page based on privilege level. If true, then the
   * page may be accessed by all. Otherwise, only the supervisor can access it.
   * If you wish to make a page a user page, you must set the user bit in the
   * relevant page directory entry as well as the page table entry.
   */
  bool user : 1;

  /**
   * If true, write-through caching is enabled. Otherwise, write-back is
   * enabled instead.
   */
  bool write_through : 1;

  /**
   * If true, the page will not be cached.
   */
  bool cache_disable : 1;

  /**
   * Used to discover whether a page has been read or written to. If it has,
   * then accessed will be true. Note that this flag will not be cleared by the
   * CPU, so that burden falls on the operating system.
   */
  bool accessed : 1;

  /**
   * If true, then the page has been written to. This flag is not updated by the
   * CPU, and once set will not clear itself.
   */
  bool dirty : 1;

  /**
   * This flag is unused and must always be false.
   */
  bool always_false : 1;

  /**
   * If true, prevents the TLB from updating the address in it's cache if CR3 is
   * reset. Note, that the page global enable bit in CR4 must be set to enable
   * this feature.
   */
  bool global : 1;

  /**
   * Unused bits in the entry.
   */
  uint32_t unused : 3;

  /**
   * The upper 20 bits of a 4kb aligned physical address. The address points to
   * a 4kb block of physical memory that is then mapped to that location in the
   * page table and directory.
   */
  uint32_t frame : 20;
};

/**
 * Represents a single entry inside a page directory.
 */
struct PageDirectoryEntry {
  /**
   * If true, the page is actually in physical memory at the moment. For
   * example, when a page is swapped out, it is not in physical memory and
   * therefore not 'present'. If a page is called, but not present, a page
   * fault will occur, and the operating system should handle it.
   */
  bool present : 1;

  /**
   * If true, the page is read/write. Otherwise the page is read-only. The WP
   * bit in CR0 determines if this is only applied to userspace, always giving
   * the kernel write access (the default) or both userspace and the kernel
   * (see Intel Manuals 3A 2-20).
   */
  bool read_write : 1;

  /**
   * Controls access to the page based on privilege level. If true, then the
   * page may be accessed by all. Otherwise, only the supervisor can access it.
   * If you wish to make a page a user page, you must set the user bit in the
   * relevant page directory entry as well as the page table entry.
   */
  bool user : 1;

  /**
   * If true, write-through caching is enabled. Otherwise, write-back is
   * enabled instead.
   */
  bool write_through : 1;

  /**
   * If true, the page will not be cached.
   */
  bool cache_disable : 1;

  /**
   * Used to discover whether a page has been read or written to. If it has,
   * then accessed will be true. Note that this flag will not be cleared by the
   * CPU, so that burden falls on the operating system.
   */
  bool accessed : 1;

  /**
   * This flag is unused and must always be false.
   */
  bool always_false : 1;

  /**
   * If true, then pages are 4 MiB in size. Otherwise, they are 4 KiB. Please
   * note that for 4 MiB pages PSE must be enabled.
   */
  bool size_4mb : 1;

  /**
   * Unused bits in the entry.
   */
  uint32_t unused : 4;

  /**
   * The upper 20 bits of the physical address of the page table that manages
   * the four megabytes at that point. Please note that it is very important
   * that this address be 4 KiB aligned. This is needed, due to the fact that
   * the last bits of the DWORD are overwritten by access bits and such.
   */
  uint32_t page_table : 20;
};

/**
 * Initializes the paging system and enables paging.
 * @param mmap_length The overall length of the memory map provided by the
 * bootloader.
 * @param mmap_addr The virtual address of the first memory map entry.
 */
void Initialize(uint32_t mmap_length, addressing::vaddress mmap_addr);

/**
 * Represents a memory mapped page directory.
 */
class PageDirectory {
 public:
  /**
   * Creates a new PageDirectory instance.
   * @param address The address of the new directory.
   */
  explicit PageDirectory(addressing::paddress address = 0);

  /**
   * Load this directory into the CR3 register, making it the active directory
   * that the CPU is using.
   */
  void Activate();

  /**
   * Map a specified physical page of memory to the specified virtual address.
   * @param page The physical page of memory to map.
   * @param address The virtual address to map \p page to.
   */
  void MapPage(Page* page, addressing::vaddress address);

  /**
   * Gets the memory page that currently contains the specified virtual address.
   * @param address The virtual address to lookup.
   * @return The memory page that \p address is currently mapped to.
   */
  Page* GetPage(addressing::vaddress address);

  /**
   * Unmaps the memory page mapped to the specified virtual address.
   * @param address The virtual address to unmap.
   * @return The page that was mapped to \p address and that is now unmapped.
   */
  Page* UnmapPage(addressing::vaddress address);

  /**
   * Gets the currently active page directory.
   * @return The page directory that is currently active.
   */
  inline static const PageDirectory* current_directory() {
    return current_directory_;
  }

  /**
   * Gets the kernel's page directory.
   * @return The kerne's page directory.
   */
  inline static const PageDirectory& kernel_directory() {
    return kernel_directory_;
  }

 private:
  /**
   * A pointer to the currently active page directory.
   */
  static PageDirectory* current_directory_;

  /**
   * The kerne's page directory.
   */
  static PageDirectory kernel_directory_;

  /**
   * The physical stored page directory entries making up this page directory.
   */
  PageDirectoryEntry tables_physical_[1024] __attribute__((aligned(4096)));

  /**
   * The physical address of this page directory?
   */
  addressing::paddress physical_address_;

  /**
   * A virtual pointer to tables_physical_. Intended to be used once paging is
   * initialized, since physical addresses can no longer be accessed.
   */
  PageDirectoryEntry* page_directory_entries_;

  PageTableEntry* page_table_entries_;

  friend void Initialize(uint32_t, addressing::vaddress);
};


}  // namespace paging

#endif  // SRC_ARCH_I586_INCLUDE_MM_PAGING_H_
