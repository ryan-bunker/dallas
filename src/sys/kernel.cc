/**
 * @file kernel.cc
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

#include "video/text_screen.h"
#include "sys/addressing.h"

extern void panic(const char *message, const char *file, int line)
{
    // We encountered a massive problem and have to stop.
    asm volatile("cli"); // Disable interrupts.

    screen::SetForeColor(screen::Color::kWhite);
    screen::SetBackColor(screen::Color::kRed);
    screen::Write("PANIC(");
    screen::Write(message);
    screen::Write(") at ");
    screen::Write(file);
    screen::Write(":");
    screen::WriteDec(line);
    screen::Write("\n");

    // Stack contains:
    //  Second function argument
    //  First function argument (MaxFrames)
    //  Return address in calling function
    //  ebp of calling function (pointed to by current ebp)
    unsigned int * ebp = reinterpret_cast<unsigned int *>(&message) - 2;
    screen::Write("Stack trace:\n");
    for (unsigned int frame = 0; frame < 5; ++frame)
    {
        unsigned int eip = ebp[1];
        if (eip == 0)
            // No caller on stack
            break;
        // Unwind to previous stack frame
        ebp = reinterpret_cast<unsigned int *>(ebp[0]);
        //unsigned int * arguments = &ebp[2];
        screen::Write("  0x");
        screen::WriteHex(eip);
        screen::Write("\n");
    }

    // Halt by going into an infinite loop.
    for(;;);
}

extern void panic_assert(const char *file, int line, const char *desc)
{
    // An assertion failed, and we have to panic.
    asm volatile("cli"); // Disable interrupts.

    screen::SetForeColor(screen::Color::kWhite);
    screen::SetBackColor(screen::Color::kRed);
    screen::Write("ASSERTION-FAILED(");
    screen::Write(desc);
    screen::Write(") at ");
    screen::Write(file);
    screen::Write(":");
    screen::WriteDec(line);
    screen::Write("\n");

    // Halt by going into an infinite loop.
    for(;;);
}




