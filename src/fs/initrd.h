/**
 * @file initrd.h
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


#ifndef SRC_FS_INITRD_H_
#define SRC_FS_INITRD_H_

#include "fs/fs_node.h"
namespace initrd {

fs::FSNode& Initialize(void *location);

class RamDiskNode : public fs::FSNodeBasic {
 public:
  RamDiskNode(const char *name, fs::FSNodeType node_type, uint32_t inode,
              uint32_t length, uint32_t device_id, FSNode* redirect)
    : fs::FSNodeBasic(name, node_type, inode, length, device_id, redirect) { }

  virtual uint32_t Read(uint32_t offset, uint32_t size, uint8_t* buffer);

  virtual fs::dirent* ReadDirectory(uint32_t index);

  virtual fs::FSNode* Find(char *name);
};

}  // namespace initrd

#endif  // SRC_FS_INITRD_H_
