/**
 * @file isr.cc
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

#include "int/isr.h"

/**
 * Called from an assembly defined ISR stub in order to handle individual
 * exceptions. Routes each exception onto its declared handlers.
 * @param regs The register values at the time of the exception.
 */
extern "C" void GlobalISRHandler(isr::Registers regs) {
  // nothing to do for now
  uint16_t* const screen = reinterpret_cast<uint16_t*>(0xC00B8000);
  screen[0] = (15 << 8) | 'I';
  screen[1] = (15 << 8) | 'N';
  screen[2] = (15 << 8) | 'T';
  screen[3] = (15 << 8) | ' ';
  if (regs.int_num < 10)
    screen[4] = (15 << 8) | ('0' + regs.int_num);
  if (regs.int_num < 100) {
    screen[4] = (15 << 8) | ('0' + regs.int_num / 10);
    screen[5] = (15 << 8) | ('0' + regs.int_num % 10);
  }

  for (;;)
    continue;
}

/**
 * Called from an assembly defined ISR stub in order to handle individual
 * interrupts. Routes each interrupt onto its declared handlers.
 * @param regs The register values at the time of the interrupt.
 */
extern "C" void GlobalIRQHandler(isr::Registers /*regs*/) {
  // nothing to do for now
}
