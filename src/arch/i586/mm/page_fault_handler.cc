/**
 * @file page_fault_handler.cc
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

#include "mm/page_fault_handler.h"

#include <cstdint>

#include "sys/kernel.h"
#include "video/text_screen.h"

void paging::PageFaultHandler::Handle(isr::Registers regs) {
  // A page fault has occurred.
  // The faulting address is stored in the CR2 register.
  uint32_t faulting_address;
  asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

  // The error code gives us details of what happened.
  int present = !(regs.err_code & 0x1);  // Page not present.
  int rw    = regs.err_code & 0x2;       // Write operation?
  int us    = regs.err_code & 0x4;       // Processor was in user-mode?
  // Overwritten CPU-reserved bits of page entry?
  int rsrvd = regs.err_code & 0x8;
  int id    = regs.err_code & 0x10;      // Caused by instruction fetch?

  // Output an error message
  screen::Write("Page fault! (");
  if (present)
    screen::Write("present ");
  if (rw)
    screen::Write("read-only ");
  if (us)
    screen::Write("user-mode ");
  if (rsrvd)
    screen::Write("reserved ");
  if (id)
    screen::Write("instruction-fetch");
  screen::Write(") at 0x");
  screen::WriteHex(faulting_address);
  screen::WriteLine("");

  screen::Write("eip = ");
  screen::WriteHex(regs.eip);
  screen::WriteLine("");

  PANIC("Page fault");
}


