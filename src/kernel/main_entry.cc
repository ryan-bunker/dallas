/**
 * @file main_entry.cc
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
 * Main entry point of kernel after initial environment has been set up
 * by the boot loader.
 */

#include <stdint.h>
#include "core/addressing.h"
#include "boot/multiboot.h"

namespace {

// Our kernel's first function: kmain
extern "C" void kmain(multiboot::Info *mbd, uint32_t magic) {
  if (magic != multiboot::kBootloaderMagic) {
    // Something went not according to specs. Print an error
    // message and halt, but do *not* rely on the multiboot
    // data structure.
  }

  // mbd is currently pointing to physical memory so we need
  // to adjust it for our current GDT offsets
  mbd = (multiboot::Info*) addressing::PhysicalToVirtual(
      reinterpret_cast<addressing::paddress>(mbd));

  uint16_t* const screen = reinterpret_cast<uint16_t*>(0xC00B8000);
  screen[0] = (15 << 8) | 'D';
  screen[1] = (15 << 8) | 'a';
  screen[2] = (15 << 8) | 'l';
  screen[3] = (15 << 8) | 'l';
  screen[4] = (15 << 8) | 'a';
  screen[5] = (15 << 8) | 's';

  // Hang up the computer
  for (;;)
    continue;
}

}  // namespace
