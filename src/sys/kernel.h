/**
 * @file kernel.h
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


#ifndef SRC_INCLUDE_SYS_KERNEL_H_
#define SRC_INCLUDE_SYS_KERNEL_H_

/**
 * Halt the kernel and display an error message on screen.
 * @param msg The message to display on screen.
 */
#define PANIC(msg) panic(msg, __FILE__, __LINE__);
/**
 * Assert that a given condition is true, and if false, halt the kernel and
 * display an error message on screen.
 * @param b The condition to assert is true.
 */
#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))

/**
 * Halt the kernel and display an error message on screen.
 * @param message The message to display on screen.
 * @param file The file that the panic originated from.
 * @param line The line in the file that the panic originated from.
 */
extern void panic(const char *message, const char *file, int line);

/**
 * Halt the kernel due to an assertion failing. Prints the failed assertion to
 * the screen.
 * @param file The file that contained the failed assertion.
 * @param line The line that the failed assertion was on.
 * @param desc The failed assertion.
 */
extern void panic_assert(const char *file, int line, const char *desc);

#endif  // SRC_INCLUDE_SYS_KERNEL_H_
