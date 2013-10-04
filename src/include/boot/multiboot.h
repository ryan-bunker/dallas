/**
 * @file multiboot.h
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
 * Values and data structures from the Multiboot specification.
 */

#ifndef SRC_INCLUDE_BOOT_MULTIBOOT_H_
#define SRC_INCLUDE_BOOT_MULTIBOOT_H_

#include <stdint.h>

/**
 * Contains types and constants related to the Multiboot specification.
 */
namespace multiboot {

/**
 * Indicates how far into the file to offset to find the header (in bytes).
 */
const int kSearch = 8192;

/**
 * Header::flags must contain kMagic this to be valid.
 */
enum class HeaderMagic : uint32_t { kMagic = 0x1BADB002 };

/**
 * When a multiboot kernel begins executing, EAX should contain this value.
 */
const int kBootloaderMagic = 0x2BADB002;

/**
 * The bits in the required part of flags field we don't support.
 */
const int kUnsupported = 0x0000fffc;

/**
 * Alignment of multiboot modules.
 */
const int kModAlign = 0x00001000;

/**
 * Alignment of the multiboot info structure.
 */
const int kInfoAlign = 0x00000004;

/**
 * Flags set in the flags member of the multiboot header.
 */
enum class HeaderFlags : uint32_t {
  kPageAlign = 1 << 0,   /// Align boot modules on i386 page (4KB) boundaries
  kMemoryInfo = 1 << 1,  /// Must pass memory information to OS.
  kVideoMode = 1 << 2,   /// Must pass video information to OS.
  kAoutKludge = 1 << 16  /// This flag indicates the use of the address fields
                         /// in the header.
};

enum class VideoMode : uint32_t {
  kLinearGraphics = 0,
  kEGATextMode = 1
};

/**
 * Flags to be set in the 'flags' member of the multiboot info structure.
 */
enum class InfoFlags : uint32_t {
  kInfoMemory = 0x00000001,   /// The info structure contains lower/upper
                              /// memory information
  /* is there a boot device set? */
  kInfoBootDevice = 0x00000002,
  /* is the command-line defined? */
  kInfoCommandLine = 0x00000004,
  /* are there modules to do something with? */
  kInfoModules = 0x00000008,

  /* These next two are mutually exclusive */

  /* is there a symbol table loaded? */
  kInfoAoutSymbols = 0x00000010,
  /* is there an ELF section header table? */
  kInfoElfSectionHeader = 0x00000020,

  /* is there a full memory map? */
  kInfoMemoryMap = 0x00000040,

  /* Is there drive info? */
  kInfoDriveInfo = 0x00000080,

  /* Is there a config table? */
  kInfoConfigTable = 0x00000100,

  /* Is there a boot loader name? */
  kInfoBootLoaderName = 0x00000200,

  /* Is there a APM table? */
  kInfoAPMTable = 0x00000400,

  /* Is there video information? */
  kInfoVideoInfo = 0x00000800
};

struct Header {
  /* Must be MULTIBOOT_MAGIC - see above. */
  HeaderMagic magic;

  /* Feature flags. */
  HeaderFlags flags;

  /* The above fields plus this one must equal 0 mod 2^32. */
  uint32_t checksum;

  /* These are only valid if MULTIBOOT_AOUT_KLUDGE is set. */
  uint32_t header_address;
  uint32_t load_address;
  uint32_t load_end_address;
  uint32_t bss_end_address;
  uint32_t entry_address;

  /* These are only valid if MULTIBOOT_VIDEO_MODE is set. */
  VideoMode video_mode;
  uint32_t video_width;
  uint32_t video_height;
  uint32_t video_depth;
};

/* The symbol table for a.out. */
struct AoutSymbolTable {
  uint32_t table_size;
  uint32_t string_size;
  uint32_t address;
  uint32_t reserved;
};

/* The section header table for ELF. */
struct ElfSectionHeaderTable {
  uint32_t num;
  uint32_t size;
  uint32_t address;
  uint32_t shndx;
};

struct Info {
  /* Multiboot info version number */
  InfoFlags flags;

  /* Available memory from BIOS */
  uint32_t memory_lower;
  uint32_t memory_upper;

  /* "root" partition */
  uint32_t boot_device;

  /* Kernel command line */
  uint32_t command_line;

  /* Boot-Module list */
  uint32_t modules_count;
  uint32_t modules_address;

  union {
    AoutSymbolTable aout_symbols;
    ElfSectionHeaderTable elf_section;
  } u;

  /* Memory Mapping buffer */
  uint32_t mmap_length;
  uint32_t mmap_address;

  /* Drive Info buffer */
  uint32_t drives_length;
  uint32_t drives_address;

  /* ROM configuration table */
  uint32_t config_table;

  /* Boot Loader Name */
  unsigned char *boot_loader_name;

  /* APM table */
  uint32_t apm_table;

  /* Video */
  uint32_t vbe_control_info;
  uint32_t vbe_mode_info;
  uint16_t vbe_mode;
  uint16_t vbe_interface_seg;
  uint16_t vbe_interface_off;
  uint16_t vbe_interface_len;
};

enum class MemoryMapType : uint32_t {
  kMemoryAvailable = 1,
  kMemoryReserved
};

struct MemoryMapEntry {
  uint32_t size;
  uint64_t address;
  uint64_t length;
  MemoryMapType type;
}__attribute__((packed));

struct ModuleList {
  /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
  uint32_t module_start;
  uint32_t module_end;

  /* Module command line */
  uint32_t command_line;

  /* padding to take it to 16 bytes (must be zero) */
  uint32_t pad;
};

}  // namespace multiboot

#endif  // SRC_INCLUDE_BOOT_MULTIBOOT_H_
