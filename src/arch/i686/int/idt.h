/**
 * @file idt.h
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
 * Items related to setting up the Interrupt Descriptor Table (IDT)
 *
 */

#ifndef SRC_ARCH_I586_INCLUDE_INT_IDT_H_
#define SRC_ARCH_I586_INCLUDE_INT_IDT_H_

namespace idt {

/**
 * Initialize interrupts and the IDT table.
 */
void Initialize();

} // namespace idt

#endif // SRC_ARCH_I586_INCLUDE_INT_IDT_H_
