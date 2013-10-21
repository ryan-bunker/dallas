/**
 * @file isr.h
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
 * Items related to interrupt service routines and interrupt request handlers.
 */


#ifndef SRC_ARCH_I586_INCLUDE_INT_ISR_H_
#define SRC_ARCH_I586_INCLUDE_INT_ISR_H_

#include <stdint.h>

namespace isr {

enum class Interrupts {
  kDivideByZero = 0x00,
  kDebugSingleStep = 0x01,
  kNMI = 0x02,
  kBreakpoint = 0x03,
  kOverflow = 0x04,
  kBoundsCheck = 0x05,
  kUndefinedOpcode = 0x06,
  kNoCoprocessor = 0x07,
  kDoubleFault = 0x08,
  kCoprocessorSegmentOverrun = 0x09,
  kInvalidTSS = 0x0A,
  kSegmentNotPresent = 0x0B,
  kStackSegmentOverrun = 0x0C,
  kGPF = 0x0D,
  kPageFault = 0x0E,
  kCoprocessorError = 0x10
};

enum class IRQs {
  kIRQ0 = 32,
  kIRQ1 = 33,
  kIRQ2 = 34,
  kIRQ3 = 35,
  kIRQ4 = 36,
  kIRQ5 = 37,
  kIRQ6 = 38,
  kIRQ7 = 39,
  kIRQ8 = 40,
  kIRQ9 = 41,
  kIRQ10 = 42,
  kIRQ11 = 43,
  kIRQ12 = 44,
  kIRQ13 = 45,
  kIRQ14 = 46,
  kIRQ15 = 47
};

/**
 * Represents the full set of registers on an x86 system.
 */
struct Registers {
  uint32_t  ds;   /// Data segment selector
  uint32_t  edi,  /// Represents the EDI register.
            esi,  /// Represents the ESI register.
            ebp,  /// Represents the EBP register.
            esp,  /// Represents the ESP register.
            ebx,  /// Represents the EBX register.
            edx,  /// Represents the EDX register.
            ecx,  /// Represents the ECX register.
            eax;  /// Represents the EAX register.

  /**
   * The interrupt number currently executing.
   */
  uint32_t int_num;

  /**
   * The error code (if applicable).
   */
  uint32_t err_code;

  uint32_t eip,      /// The instruction point register
           cs,       /// Code segment selector
           eflags,   /// The current flags register.
           useresp,  /// ???
           ss;       /// ???
};

}  // namespace isr

#endif  // SRC_ARCH_I586_INCLUDE_INT_ISR_H_
