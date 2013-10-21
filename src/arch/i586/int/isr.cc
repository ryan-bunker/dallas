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
#include "sys/io.h"
#include "sys/kernel.h"

namespace isr {

InterruptHandler* InterruptHandler::interrupt_handlers_[256];

InterruptHandler::InterruptHandler(Interrupts interrupt_number,
                                   bool register_immediately)
  : interrupt_number_(interrupt_number),
    is_registered_(false),
    next_(nullptr) {
  if (register_immediately)
    RegisterHandler();
}

//InterruptHandler::~InterruptHandler() {
//  if (is_registered_)
//    UnregisterHandler();
//}

void InterruptHandler::RegisterHandler() {
  InterruptHandler* handler =
      interrupt_handlers_[static_cast<int>(interrupt_number_)];
  next_ = handler;
  interrupt_handlers_[static_cast<int>(interrupt_number_)] = this;
  is_registered_ = true;
}

void InterruptHandler::UnregisterHandler() {
  is_registered_ = false;
  InterruptHandler* handler =
      interrupt_handlers_[static_cast<int>(interrupt_number_)];
  if (handler == this) {
    // we are at the head of the list, so make our next_ the head
    interrupt_handlers_[static_cast<int>(interrupt_number_)] = next_;
    return;
  }

  for (InterruptHandler* h = handler; h; h = h->next_) {
    if (h->next_ != this)
      continue;

    // we found the handler before us in the list, point it's next_ at our next_
    h->next_ = next_;
    return;
  }
}

void InterruptHandler::EndOfInterrupt(Registers regs) {
  // Send an EOI (end of interrupt) signal to the PICs.
  // If this interrupt involved the slave.
  if (regs.int_num >= 40) {
    // Send reset signal to slave.
    outb(0xA0, 0x20);
  }

  // Send reset signal to master. (As well as slave, if necessary)
  outb(0x20, 0x20);
}

bool InterruptHandler::FindAndHandle(Registers regs) {
  InterruptHandler* handler =
      InterruptHandler::interrupt_handlers_[regs.int_num];
  if (!handler)
    return false;   // no handler was registered

  // run through the handler chain, executing each one in turn
  for (; handler; handler = handler->next_)
    handler->Handle(regs);

  return true;
}

void GlobalISRHandler_CPP(Registers regs) {
  if (!InterruptHandler::FindAndHandle(regs))
    PANIC("Unhandled exception");
}

void GlobalIRQHandler_CPP(Registers regs) {
  if (!InterruptHandler::FindAndHandle(regs))
    // there is no registered handler for this interrupt so we have to
    // send an EOI (end of interrupt) ourselves, so the PICs don't
    // stop sending us interrupts
    InterruptHandler::EndOfInterrupt(regs);
}

}  // namespace isr

/**
 * Called from an assembly defined ISR stub in order to handle individual
 * exceptions. Routes each exception onto its declared handlers.
 * @param regs The register values at the time of the exception.
 */
extern "C" void GlobalISRHandler(isr::Registers regs) {
  GlobalISRHandler_CPP(regs);
}

/**
 * Called from an assembly defined ISR stub in order to handle individual
 * interrupts. Routes each interrupt onto its declared handlers.
 * @param regs The register values at the time of the interrupt.
 */
extern "C" void GlobalIRQHandler(isr::Registers regs) {
  GlobalIRQHandler_CPP(regs);
}
