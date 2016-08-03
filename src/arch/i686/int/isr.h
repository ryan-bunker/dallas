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

#include <cstdint>

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
  kCoprocessorError = 0x10,

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
  /**
   * Data segment selector.
   */
  uint32_t ds;

  /**
   * Represents the EDI register.
   */
  uint32_t edi;

  /**
   * Represents the ESI register.
   */
  uint32_t esi;

  /**
   * Represents the EBP register.
   */
  uint32_t ebp;

  /**
   * Represents the ESP register.
   */
  uint32_t esp;

  /**
   * Represents the EBX register.
   */
  uint32_t ebx;

  /**
   * Represents the EDX register.
   */
  uint32_t edx;

  /**
   * Represents the ECX register.
   */
  uint32_t ecx;

  /**
   * Represents the EAX register.
   */
  uint32_t eax;

  /**
   * The interrupt number currently executing.
   */
  uint32_t int_num;

  /**
   * The error code (if applicable).
   */
  uint32_t err_code;

  /**
   * The instruction pointer register.
   */
  uint32_t eip;

  /**
   * The code segment selector.
   */
  uint32_t cs;

  /**
   * The current flags register.
   */
  uint32_t eflags;

  /**
   * The value of the ESP register.
   */
  uint32_t user_esp;

  /**
   * The value of the SS register.
   */
  uint32_t ss;
};

/**
 * Abstract base class representing an interrupt handler.
 */
class InterruptHandler {
public:
  /**
   * Create a new InterruptHandler instance and optionally register it.
   * @param interrupt_number The interrupt number for this handler.
   * @param register_immediately If true, the handler will be registered with
   * the system during construction.
   */
  InterruptHandler(Interrupts interrupt_number,
                   bool register_immediately = false);

  virtual ~InterruptHandler();

  /**
   * Register this handler with the system.
   */
  void RegisterHandler();

  /**
   * Unregister this handler with the system. It will no longer be raised.
   */
  void UnregisterHandler();

  /**
   * Gets whether the handler is currently registered with the system.
   * @return True if the handler is currently registered.
   */
  inline bool is_registered() { return is_registered_; }

protected:
  /**
   * Sends an EOI (end of interrupt) signal to the PICs. Should only be called
   * for interrupts (not exceptions).
   * @param regs The value of the registers when the interrupt was raised.
   */
  static void EndOfInterrupt(Registers regs);

  /**
   * Called when the interrupt is raised. Perform all handling here.
   * @param regs The value of the registers when the interrupt was raised.
   */
  virtual void Handle(Registers regs) = 0;

private:
  /**
   * Iterates and executes the handler chain for the specified interrupt.
   * @param regs The value of the registers when the interrupt was raised.
   * @return True if at least one handler is registered for the interrupt.
   */
  static bool FindAndHandle(Registers regs);

  /**
   * The table of all registered interrupt handlers.
   */
  static InterruptHandler *interrupt_handlers_[];

  /**
   * The interrupt number that this handler has been setup for.
   */
  Interrupts interrupt_number_;

  /**
   * Whether the handler is currently registered.
   */
  bool is_registered_;

  /**
   * The next handler in the handler chain for this interrupt number.
   */
  InterruptHandler *next_;

  friend void GlobalISRHandler_CPP(Registers);
  friend void GlobalIRQHandler_CPP(Registers);
};

} // namespace isr

#endif // SRC_ARCH_I586_INCLUDE_INT_ISR_H_
