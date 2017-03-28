#ifndef SRC_INCLUDE_BOOT_MULTIBOOT2_H_
#define SRC_INCLUDE_BOOT_MULTIBOOT2_H_

#include <stdint.h>

namespace multiboot2 {
/**
 * When a multiboot2 kernel begins executing, EAX should contain this value.
 */
const int kBootloaderMagic = 0x36d76289;

enum class TagType : uint32_t {
    kTerminatingTag = 0,
    kBasicMemoryInfo = 4,
    kBiosBootDevice = 5,
    kMemoryMap = 6,
    kElfSymbols = 9
};

struct Tag {
    TagType type;
    uint32_t size;

    Tag* next() {
        auto addr = reinterpret_cast<uint32_t>(this) + size;
        auto aligned = addr & ~7;
        if (aligned != addr)
            aligned += 8;
        auto tag = reinterpret_cast<Tag*>(aligned);
        if (tag->type == TagType::kTerminatingTag && tag->size == 8)
            return nullptr;
        return tag;
    }
};

struct BasicMemoryTag : Tag {
    uint32_t mem_lower;
    uint32_t mem_upper;
};

struct BiosBootDeviceTag : Tag {
    uint32_t biosdev;
    uint32_t partition;
    uint32_t sub_partition;
};

struct MemoryMapEntry {
    uint32_t base_addr_lo;
    uint32_t base_addr_hi;
    uint32_t length_lo;
    uint32_t length_hi;
    uint32_t type;
    uint32_t reserved;
};

struct MemoryMapTag : Tag {
    uint32_t entry_size;
    uint32_t entry_version;

    int entries() { return (size - 16) / entry_size; }

    MemoryMapEntry* entry(int index) {
        auto addr = reinterpret_cast<uint32_t>(&entry_version + 1);
        return reinterpret_cast<MemoryMapEntry*>(addr + index * entry_size);
    }
};

struct ElfSection {
    uint32_t sh_name,
             sh_type,
             sh_flags,
             sh_addr,
             sh_offset,
             sh_size,
             sh_link,
             sh_info,
             sh_addralign,
             sh_entsize;
};

struct ElfSymbolsTag : Tag {
    uint16_t num;
    uint32_t entsize;
    uint32_t shndx;
    //uint16_t reserved;
    //uint32_t pad;
    ElfSection sections[0];

    const char* section_name(int index) {
        auto strtab = sections[shndx];
        auto s = sections[index];
        return reinterpret_cast<char *>(strtab.sh_addr + s.sh_name);
    }
};

/**
 * Represents the multiboot2 information structure provided by the bootloader.
 * See
 * http://nongnu.askapache.com/grub/phcoder/multiboot.pdf
 * for more information.
 */
struct Info {
    uint32_t total_size;
    uint32_t reserved;
    Tag first_tag;

    BasicMemoryTag* basic_memory() {
        return reinterpret_cast<BasicMemoryTag*>(find_tag(TagType::kBasicMemoryInfo));
    }

    BiosBootDeviceTag* bios_bootdevice() {
        return reinterpret_cast<BiosBootDeviceTag*>(find_tag(TagType::kBiosBootDevice));
    }

    MemoryMapTag* memory_map() {
        return reinterpret_cast<MemoryMapTag*>(find_tag(TagType::kMemoryMap));
    }

    ElfSymbolsTag* elf_symbols() {
        return reinterpret_cast<ElfSymbolsTag*>(find_tag(TagType::kElfSymbols));
    }

    private:
    Tag* find_tag(TagType type) {
        for (auto t = &first_tag; t != nullptr; t = t->next()) {
            if (t->type == type)
                return t;
        }
        return nullptr;
    }
};

}

#endif // SRC_INCLUDE_BOOT_MULTIBOOT2_H_