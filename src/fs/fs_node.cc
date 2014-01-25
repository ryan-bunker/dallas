/**
 * @file fs_node.cc
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

#include "fs/fs_node.h"

#include <cstring>

namespace fs {

FSNode::FSNode(FSNodeType node_type)
  : _node_type(node_type),
    _inode(0),
    _length(0),
    _device_id(0),
    _redirect(nullptr) {
  memset(_name, 0, sizeof(_name));
}

FSNode::~FSNode() {
}

FSNode::FSNode(const char *name, FSNodeType node_type, uint32_t inode,
               uint32_t length, uint32_t device_id, FSNode* redirect)
  : _node_type(node_type),
    _inode(inode),
    _length(length),
    _device_id(device_id),
    _redirect(redirect) {
  strcpy(_name, name);
}

}  // namespace fs

