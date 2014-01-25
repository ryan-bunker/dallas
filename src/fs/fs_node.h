/**
 * @file fs_node.h
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


#ifndef SRC_FS_FS_NODE_H_
#define SRC_FS_FS_NODE_H_

#include <cstdint>
#include <cstring>

namespace fs {

enum class FSNodeType {
  kFile,
  kDirectory,
  kCharDevice,
  kBlockDevice,
  kPipe,
  kSymbolicLink,
  kMountPoint
};

struct dirent {
  char name[128];
  uint32_t inode;
};

class FSNode {
 public:
  virtual uint32_t Read(uint32_t offset, uint32_t size, uint8_t* buffer) = 0;

  virtual uint32_t Write(uint32_t offset, uint32_t size, uint8_t* buffer) = 0;

  virtual void Open() = 0;

  virtual void Close() = 0;

  virtual dirent* ReadDirectory(uint32_t index) = 0;

  virtual FSNode* Find(char *name) = 0;

  inline const char *name() { return _name; }
  inline FSNodeType node_type() { return _node_type; }
  inline uint32_t inode() { return _inode; }
  inline uint32_t length() { return _length; }
  inline uint32_t device_id() { return _device_id; }
  inline FSNode* redirect() { return _redirect; }

 protected:
  explicit FSNode(FSNodeType node_type);
  FSNode(const char *name, FSNodeType node_type, uint32_t inode, uint32_t length,
         uint32_t device_id, FSNode* redirect);
  virtual ~FSNode();

  inline void name(const char *name) { strcpy(_name, name); }
  inline void node_type(FSNodeType node_type) { _node_type = node_type; }
  inline void inode(uint32_t inode) { _inode = inode; }
  inline void length(uint32_t length) { _length = length; }
  inline void device_id(uint32_t device_id) { _device_id = device_id; }
  inline void redirect(FSNode* redirect) { _redirect = redirect; }

 private:
  char _name[128];
  FSNodeType _node_type;
  uint32_t _inode;
  uint32_t _length;
  uint32_t _device_id;
  FSNode* _redirect;
};

class FSNodeBasic : public FSNode {
 public:
  virtual uint32_t Read(uint32_t, uint32_t, uint8_t*) {
    return 0;
  }

  virtual uint32_t Write(uint32_t, uint32_t, uint8_t*) {
    return 0;
  }

  virtual void Open() { }

  virtual void Close() { }

  virtual dirent* ReadDirectory(uint32_t) {
    return nullptr;
  }

  virtual FSNode* Find(char*) {
    return nullptr;
  }

 protected:
  explicit FSNodeBasic(FSNodeType node_type) : FSNode(node_type) { }
  FSNodeBasic(const char *name, FSNodeType node_type, uint32_t inode,
              uint32_t length, uint32_t device_id, FSNode* redirect)
    : FSNode(name, node_type, inode, length, device_id, redirect) { }
  virtual ~FSNodeBasic() { }
};

}  // namespace fs

#endif  // SRC_FS_FS_NODE_H_
