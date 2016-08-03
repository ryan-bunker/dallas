/**
 * @file io.h
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


#ifndef SRC_ARCH_I586_INCLUDE_SYS_IO_H_
#define SRC_ARCH_I586_INCLUDE_SYS_IO_H_

#include <cstdint>

/**
 * Writes a byte value to the specified port.
 * @param port The port to send data to.
 * @param value The byte to send.
 */
inline void outb(uint16_t port, uint8_t value) {
  asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

/**
 * Reads a byte value from the specified port.
 * @param port The port to receive data from.
 * @return The byte that was read from the port.
 */
inline uint8_t inb(uint16_t port) {
   uint8_t ret;
   asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

/**
 * Reads a word value from the specified port.
 * @param port The port to receive data from.
 * @return The word that was read from the port.
 */
inline uint16_t inw(uint16_t port) {
   uint16_t ret;
   asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

/**
 * Wait for the IO port to be ready.
 */
inline void io_wait() {
  asm volatile ("outb %%al, $0x80" : : "a" (0));
}

/**
 * Enables interrupts.
 */
inline void enable_interrupts() {
  asm volatile ("sti");
}

/**
 * Disables interrupts.
 */
inline void disable_interrupts() {
  asm volatile ("cli");
}

#endif  // SRC_ARCH_I586_INCLUDE_SYS_IO_H_
