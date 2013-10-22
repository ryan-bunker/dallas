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

#include "boot/multiboot.h"
#include "int/idt.h"
#include "sys/addressing.h"
#include "sys/io.h"
#include "video/text_screen.h"

namespace {

/**
 * Main entry point into kernel from loader assembly.
 * @param mbd The multiboot information structure.
 * @param magic Must match kBootloaderMagic to verify that mbd is valid.
 */
extern "C" void kmain(multiboot::Info *mbd, uint32_t magic) {
  if (magic != multiboot::kBootloaderMagic) {
    // Something went not according to specs. Print an error
    // message and halt, but do *not* rely on the multiboot
    // data structure.
    return;
  }

  // mbd is currently pointing to physical memory so we need
  // to adjust it for our current GDT offsets
  mbd = (multiboot::Info*) addressing::PhysicalToVirtual(
      reinterpret_cast<addressing::paddress>(mbd));

  idt::Initialize();

  enable_interrupts();

  screen::Clear();
  screen::WriteLine("Dallas");

  int* a = new int;
  int* b = new int;
  int* c = new int;

  *a = 1;
  *b = 2;
  *c = 3;

  screen::Write("0x");
  screen::WriteHex(reinterpret_cast<uint32_t>(a));
  screen::Write(" = ");
  screen::WriteDec(*a);
  screen::WriteLine("");

  screen::Write("0x");
  screen::WriteHex(reinterpret_cast<uint32_t>(b));
  screen::Write(" = ");
  screen::WriteDec(*b);
  screen::WriteLine("");

  screen::Write("0x");
  screen::WriteHex(reinterpret_cast<uint32_t>(c));
  screen::Write(" = ");
  screen::WriteDec(*c);
  screen::WriteLine("");

  // Hang up the computer
  for (;;)
    continue;
}

}  // namespace
