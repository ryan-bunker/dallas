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

#include <cstdint>

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
  /**
   * Align boot modules on i386 page (4KB) boundaries.
   */
  kPageAlign = 1 << 0,
  /**
   * Must pass memory information to OS.
   */
  kMemoryInfo = 1 << 1,
  /**
   * Must pass video information to OS.
   */
  kVideoMode = 1 << 2,
  /**
   * This flag indicates the use of the address fields in the header.
   */
  kAoutKludge = 1 << 16
};

/**
 * Which video mode the boot loader should put the machine into before
 * loading the kernel.
 */
enum class VideoMode : uint32_t {
  /**
   * Linear graphics mode. Header::video_width and Header::video_height
   * specify the dimensions of the screen in pixels.
   */
  kLinearGraphics = 0,
  /**
   * EGA-standard text mode. Header::video_width and Header::video_height
   * specify the dimensions of the screen in characters.
   */
  kEGATextMode = 1
};

/**
 * Flags to be set in the 'flags' member of the multiboot info structure.
 */
enum class InfoFlags : uint32_t {
  /**
   * Info contains lower/upper memory information.
   */
  kInfoMemory = 0x00000001,
  /**
   * Info contains boot device.
   */
  kInfoBootDevice = 0x00000002,
  /**
   * Info contains command line.
   */
  kInfoCommandLine = 0x00000004,
  /**
   * Info contains module information.
   */
  kInfoModules = 0x00000008,

  /**
   * Info contains a symbol table. Mutually exclusive with
   * InfoFlags::kInfoElfSectionHeader.
   */
  kInfoAoutSymbols = 0x00000010,
  /**
   * Info contains an ELF section header table. Mutually exclusive with
   * InfoFlags::kInfoAoutSymbols.
   */
  kInfoElfSectionHeader = 0x00000020,

  /**
   * Info contains a full memory map.
   */
  kInfoMemoryMap = 0x00000040,

  /**
   * Info contains drive information
   */
  kInfoDriveInfo = 0x00000080,

  /**
   * Info contains the configuration table.
   */
  kInfoConfigTable = 0x00000100,

  /**
   * Info contains the boot loader name.
   */
  kInfoBootLoaderName = 0x00000200,

  /**
   * Info contains an APM table.
   */
  kInfoAPMTable = 0x00000400,

  /**
   * Info contains video information.
   */
  kInfoVideoInfo = 0x00000800
};

struct Header {
  /**
   * The magic number identifying the header, which must be HeaderMagic::kMagic.
   */
  HeaderMagic magic;

  /**
   * Specifies features that the OS image requests or requires of a boot loader.
   */
  HeaderFlags flags;

  /**
   * A 32-bit unsigned value which, when added to Header::magic and
   * Header::flags, must have a 32-bit unsigned sum of zero.
   */
  uint32_t checksum;

  /**
   * Contains the address corresponding to the beginning of the Multiboot
   * header - the physical memory location at which the magic value is supposed
   * to be loaded. This field serves to synchronize the mapping between OS
   * image offsets and physical memory addresses.
   */
  uint32_t header_address;
  /**
   * Contains the physical address of the beginning of the text segment. The
   * offset in the OS image file at which to start loading is defined by the
   * offset at which the header was found, minus (Header::header_address -
   * Header::load_address). Header::load_address must be less than or equal
   * to Header::header_address.
   */
  uint32_t load_address;
  /**
   * Contains the physical address of the end of the data segment.
   * (Header::load_end_address - Header::load_address) specifies how much data
   * to load. This implies that the text and data segments must be consecutive
   * in the OS image; this is true for existing a.out executable formats. If
   * this field is zero, the boot loader assumes that the text and data
   * segments occupy the whole OS image file.
   */
  uint32_t load_end_address;
  /**
   * Contains the physical address of the end of the bss segment. The boot
   * loader initializes this area to zero, and reserves the memory it occupies
   * to avoid placing boot modules and other data relevant to the operating
   * system in that area. If this field is zero, the boot loader assumes that
   * no bss segment is present.
   */
  uint32_t bss_end_address;
  /**
   * The physical address to which the boot loader should jump in order to
   * start running the operating system.
   */
  uint32_t entry_address;

  /**
   * Specifies the preferred graphics mode. Note that that is only a
   * recommended mode by the OS image. If the mode exists, the boot loader
   * should set it, when the user doesn't specify a mode explicitly. Otherwise,
   * the boot loader should fall back to a similar mode, if available.
   */
  VideoMode video_mode;
  /**
   * Contains the number of columns. This is specified in pixels in a graphics
   * mode, and in characters in a text mode. The value zero indicates that the
   * OS image has no preference.
   */
  uint32_t video_width;
  /**
   * Contains the number of lines. This is specified in pixels in a graphics
   * mode, and in characters in a text mode. The value zero indicates that the
   * OS image has no preference.
   */
  uint32_t video_height;
  /**
   * Contains the number of bits per pixel in a graphics mode, and zero in a
   * text mode. The value zero indicates that the OS image has no preference.
   */
  uint32_t video_depth;
};

/**
 * Indicates where the symbol table from an a.out kernel image can be found.
 */
struct AoutSymbolTable {
  /**
   * Equal to its size parameter (found at the beginning of the symbol section).
   * Note that this field may be 0, indicating no symbols, even if the
   * InfoFlags::kInfoAoutSymbols flag of Info::flags is set.
   */
  uint32_t table_size;
  /**
   * Equal to its size parameter (found at the beginning of the string section)
   * of the following string table to which the symbol table refers.
   */
  uint32_t string_size;
  /**
   * The physical address of the size (4-byte unsigned long) of an array of
   * a.out format nlist structures, followed immediately by the array itself,
   * then the size (4-byte unsigned long) of a set of zero-terminated ASCII
   * strings (plus sizeof(unsigned long) in this case), and finally the set of
   * strings itself.
   */
  uint32_t address;
  /**
   * Reserved by the specification (set to zero).
   */
  uint32_t reserved;
};

/**
 * Indicates where the section header table from an ELF kernel is, the size of
 * each entry, number of entries, and the string table used as the index of
 * names. They correspond to the ‘shdr_*’ entries (‘shdr_num’, etc.) in the
 * Executable and Linkable Format (elf) specification in the program header.
 * All sections are loaded, and the physical address fields of the elf section
 * header then refer to where the sections are in memory (refer to the i386 elf
 * documentation for details as to how to read the section header(s)).
 */
struct ElfSectionHeaderTable {
  /**
   * Note that this field may be 0, indicating no symbols, even if the
   * InfoFlags::kInfoElfSectionHeader flag of Info::flags is set.
   */
  uint32_t num;
  uint32_t size;
  uint32_t address;
  uint32_t shndx;
};

/**
 * Contains information about a loaded module.
 */
struct ModuleInfo {
  /**
   * The start address of the boot module.
   */
  uint32_t module_start;
  /**
   * The end address of the boot module.
   */
  uint32_t module_end;
  /**
   * Contains an arbitrary string to be associated with the particular boot
   * module; it is a zero-terminated ASCII string, just like the kernel
   * command line. This field may be 0 if there is no string associated with
   * the module. Typically the string might be a command line (e.g. if the
   * operating system treats boot modules as executable programs), or a
   * pathname (e.g. if the operating system treats boot modules as files in a
   * file system), but its exact use is specific to the operating system.
   */
  unsigned char *string;  // NOLINT
  /**
   * Must be set to 0 by the boot loader and ignored by the operating system.
   */
  uint32_t reserved;
};

/**
 * Indicates which type of memory region a particular MemoryMapEntry is.
 */
enum class MemoryMapType : uint32_t {
  /**
   * The memory region represents available memory.
   */
  kMemoryAvailable = 1,
  /**
   * The memory region represents reserved memory. Note that any non-1 value
   * indicates reserved memory.
   */
  kMemoryReserved
};

/**
 * Contains information about a region in memory.
 */
struct MemoryMapEntry {
  /**
   * The size of the remaining structure in bytes, which can be greater than
   * the minimum of 20 bytes
   */
  uint32_t size;
  /**
   * The starting address of the region.
   */
  uint64_t address;
  /**
   * The size of the memory region in bytes.
   */
  uint64_t length;
  /**
   * Type of address range represented.
   */
  MemoryMapType type;
}__attribute__((packed));

/**
 * Represents the types of drive access modes used by the boot loader.
 */
enum class DriveMode : uint8_t {
  /**
   * CHS mode (traditional cylinder/head/sector addressing mode).
   */
  kCHS = 0,
  /**
   * LBA mode (Logical Block Addressing mode).
   */
  kLBA = 1
};

struct DriveInfo {
  /**
   * Specifies the size of this structure. The size varies, depending on the
   * number of ports. Note that the size may not be equal to (10 + 2 * the
   * number of ports), because of an alignment.
   */
  uint32_t size;
  /**
   * Contains the BIOS drive number.
   */
  uint8_t drive_number;
  /**
   * Represents the access mode used by the boot loader.
   */
  DriveMode drive_mode;
  /**
   * Contains the number of cylinders, as detected by the BIOS.
   */
  uint16_t drive_cylinders;
  /**
   * Contains the number of heads, as detected by the BIOS.
   */
  uint8_t drive_heads;
  /**
   * Contains the number of sectors per track, as detected by the BIOS.
   */
  uint8_t drive_sectors;
  /**
   * An array of the I/O ports used for the drive in the BIOS code. The array
   * consists of zero or more unsigned two-byte integers, and is terminated
   * with a zero. Note that the array may contain any number of I/O ports that
   * are not related to the drive actually (such as DMA controller's ports).
   */
  uint16_t drive_ports[];
};

struct APMTable {
  /**
   * Version number of the APM table.
   */
  uint16_t version;
  /**
   * Contains the protected mode 32-bit code segment.
   */
  uint16_t cseg;
  /**
   * Contains the offset of the entry point.
   */
  uint32_t offset;
  /**
   * Contains the protected mode 16-bit code segment.
   */
  uint16_t cseg_16;
  /**
   * Contains the protected mode 16-bit data segment.
   */
  uint16_t dseg;
  /**
   * Contains the flags.
   */
  uint16_t flags;
  /**
   * Contains the length of the protected mode 32-bit code segment.
   */
  uint16_t cseg_len;
  /**
   * Contains the length of the protected mode 16-bit code segment.
   */
  uint16_t cseg_16_len;
  /**
   * Contains the length of the protected mode 16-bit data segment.
   */
  uint16_t dseg_len;
};

struct Info {
  /**
   * Indicates the presence and validity of other fields in the Multiboot
   * information structure. All as-yet-undefined bits must be set to zero by
   * the boot loader. Any set bits that the operating system does not understand
   * should be ignored. Thus, this field also functions as a version indicator,
   * allowing the Multiboot information structure to be expanded in the future
   * without breaking anything.
   */
  InfoFlags flags;

  /**
   * Indicates the amount of lower memory, in kilobytes. Lower memory starts at
   * address 0. The maximum possible value for lower memory is 640 kilobytes.
   */
  uint32_t memory_lower;
  /**
   * Indicates the amount of upper memory, in kilobytes. Upper memory starts at
   * address 1 megabyte. The value returned for upper memory is maximally the
   * address of the first upper memory hole minus 1 megabyte. It is not
   * guaranteed to be this value.
   */
  uint32_t memory_upper;

  /**
   * Indicates which BIOS disk device the boot loader loaded the OS image from.
   * If the OS image was not loaded from a BIOS disk, then this field must not
   * be present (flag InfoFlags::kInfoBootDevice must be clear). The operating
   * system may use this field as a hint for determining its own root device,
   * but is not required to.
   * The ‘boot_device’ field is laid out in four one-byte subfields as follows:
   * <table>
   * <tr><td>part3</td><td>part2</td><td>part1</td><td>drive</td></tr>
   * </table>
   * The first byte contains the BIOS drive number as understood by the BIOS
   * INT 0x13 low-level disk interface: e.g. 0x00 for the first floppy disk or
   * 0x80 for the first hard disk.
   *
   * The three remaining bytes specify the boot partition. ‘part1’ specifies
   * the top-level partition number, ‘part2’ specifies a sub-partition in the
   * top-level partition, etc. Partition numbers always start from zero. Unused
   * partition bytes must be set to 0xFF. For example, if the disk is
   * partitioned using a simple one-level DOS partitioning scheme, then ‘part1’
   * contains the DOS partition number, and ‘part2’ and ‘part3’ are both 0xFF.
   * As another example, if a disk is partitioned first into DOS partitions, and
   * then one of those DOS partitions is subdivided into several BSD partitions
   * using BSD's disklabel strategy, then ‘part1’ contains the DOS partition
   * number, ‘part2’ contains the BSD sub-partition within that DOS partition,
   * and ‘part3’ is 0xFF.
   *
   * DOS extended partitions are indicated as partition numbers starting from 4
   * and increasing, rather than as nested sub-partitions, even though the
   * underlying disk layout of extended partitions is hierarchical in nature.
   * For example, if the boot loader boots from the second extended partition
   * on a disk partitioned in conventional DOS style, then ‘part1’ will be 5,
   * and ‘part2’ and ‘part3’ will both be 0xFF.
   */
  uint32_t boot_device;

  /**
   * Contains the physical address of the command line to be passed to the
   * kernel. The command line is a normal C-style zero-terminated string.
   */
  uint32_t command_line;

  /**
   * Contains the number of modules loaded. May be zero, indicating no boot
   * modules were loaded, even if flag InfoFlags::kInfoModules of
   * Info::flags is set.
   */
  uint32_t modules_count;
  /**
   * Information about the loaded modules.
   */
  ModuleInfo *modules;

  union {
    /**
     * Contains the symbol table from an a.out kernel image.
     */
    AoutSymbolTable aout_symbols;
    /**
     * Contains the section header table from an ELF kernel.
     */
    ElfSectionHeaderTable elf_section;
  } u;

  /**
   * The total size of the memory map buffer.
   */
  uint32_t mmap_length;
  /**
   * The memory map buffer containing zero or more MemoryMapEntry structures.
   */
  MemoryMapEntry *mmap;

  /**
   * Total size of the drives information buffer.
   */
  uint32_t drives_length;
  /**
   * The drives information structures.
   */
  DriveInfo drives;

  /**
   * Indicates the address of the ROM configuration table returned by the
   * GET CONFIGURATION BIOS call. If the BIOS call fails, then the size of the
   * table must be zero.
   */
  uint32_t config_table;

  /**
   * Contains the physical address of the name of a boot loader booting the
   * kernel. The name is a normal C-style zero-terminated string.
   */
  unsigned char *boot_loader_name;

  /**
   * Contains an APM table.
   */
  APMTable apm_table;

  /**
   * Contains the physical address of VBE control information returned by the
   * VBE function 00h.
   */
  uint32_t vbe_control_info;
  /**
   * Contains the physical address of VBE mode information returned by the VBE
   * function 01h.
   */
  uint32_t vbe_mode_info;
  /**
   * Indicates current video mode in the format specified in VBE 3.0.
   */
  uint16_t vbe_mode;
  /**
   * Contains the segment to a protected mode interface defined in VBE 2.0+. If
   * this information is not available, this field contains zero.
   */
  uint16_t vbe_interface_seg;
  /**
   * Contains the offset to a protected mode interface defined in VBE 2.0+. If
   * this information is not available, this field contains zero.
   */
  uint16_t vbe_interface_off;
  /**
   * Contains the length of a protected mode interface defined in VBE 2.0+. If
   * this information is not available, this field contains zero.
   */
  uint16_t vbe_interface_len;
};

}  // namespace multiboot

#endif  // SRC_INCLUDE_BOOT_MULTIBOOT_H_
