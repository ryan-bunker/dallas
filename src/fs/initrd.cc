/**
 * @file initrd.cc
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
 */

#include "initrd.h"

#include <cstdint>

#include "fs/fs_node.h"
#include "mm/allocator.h"

namespace initrd {

struct initrd_header {
  uint32_t nfiles;
};

struct initrd_file_header {
  uint32_t magic;
  char name[64];
  uint32_t offset;
  uint32_t length;
};

initrd_header* header;
initrd_file_header* file_headers;
RamDiskNode initrd_root("initrd", fs::FSNodeType::kDirectory, 0, 0, 0, nullptr);
RamDiskNode initrd_dev("dev", fs::FSNodeType::kDirectory, 0, 0, 0, nullptr);
RamDiskNode* root_nodes;
uint32_t nroot_nodes;

fs::dirent dirent;

fs::FSNode& Initialize(void *location) {
  header = static_cast<initrd_header*>(location);
  file_headers = reinterpret_cast<initrd_file_header*>(
      reinterpret_cast<uint32_t>(location) + sizeof(initrd_header));

  root_nodes =
      static_cast<RamDiskNode*>(kmalloc(sizeof(RamDiskNode) * header->nfiles));
  nroot_nodes = header->nfiles;

  for (uint32_t i=0; i<header->nfiles; ++i) {
    file_headers[i].offset += reinterpret_cast<uint32_t>(location);
    new (root_nodes + i) RamDiskNode(file_headers[i].name,
                                     fs::FSNodeType::kFile, i,
                                     file_headers[i].length, 0, nullptr);
  }

  return initrd_root;
}

uint32_t RamDiskNode::Read(uint32_t offset, uint32_t size, uint8_t* buffer) {
  initrd_file_header& header = file_headers[inode()];
  if (offset > header.length)
    return 0;
  if (offset + size > header.length)
    size = header.length - offset;
  memcpy(buffer, reinterpret_cast<uint8_t*>(header.offset + offset), size);
  return size;
}

fs::dirent* RamDiskNode::ReadDirectory(uint32_t index) {
  if (this == &initrd_root && index == 0) {
    strcpy(dirent.name, "dev");
    dirent.inode = 0;
    return &dirent;
  }

  if (index - 1 >= nroot_nodes)
    return 0;
  strcpy(dirent.name, root_nodes[index - 1].name());
  dirent.inode = root_nodes[index-1].inode();
  return &dirent;
}

fs::FSNode* RamDiskNode::Find(char *name) {
  if (this == &initrd_root && !strcmp(name, "dev") )
    return &initrd_dev;

  for (uint32_t i = 0; i < nroot_nodes; ++i)
    if (!strcmp(name, root_nodes[i].name()))
      return &root_nodes[i];

  return nullptr;
}


}  // namespace initrd

