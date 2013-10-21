/**
 * @file text_screen.cc
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

#include <stdio.h>
#include "video/text_screen.h"
#include "sys/io.h"

namespace screen {

// screen attributes
const int kScreenWidth = 80;
const int kScreenHeight = 25;

const char kHexAlphabet[] = "0123456789ABCDEF";

struct ScreenChar {
  unsigned char character;
  uint8_t color;
} __attribute((packed));

ScreenChar* const screen_buffer = reinterpret_cast<ScreenChar*>(0xC00B8000);
int cursor_x = 0;
int cursor_y = 0;
Color default_fore_color = Color::kWhite;
Color default_back_color = Color::kBlack;

inline uint8_t GetColor(Color fore_color = Color::kDefault,
                      Color back_color = Color::kDefault) {
  if (fore_color == Color::kDefault)
    fore_color = default_fore_color;
  if (back_color == Color::kDefault)
    back_color = default_back_color;
  return (static_cast<uint8_t>(back_color) << 4)
      | (static_cast<uint8_t>(fore_color) & 0x0F);
}

/**
 * Converts an unsigned integer to a null-terminated string using the specified
 * base and stores the result in the provided buffer.
 * @param number The number to be converted to a string.
 * @param base Numerical base used to represent the value as a string, between
 * 2 and 16, where 10 means decimal base, 16 hexadecimal, 8 octal, and 2 binary.
 * @param text_out Array in memory where to store the resulting null-terminated
 * string.
 * @param offset Offset into \p text_out at which to start writing.
 * @param len The minimum number of digits to use to represent \p number.
 * @param groupDigits Whether to separate digits into groups of three.
 */
static void NumberToTextUnsigned(uint32_t number, int base, char *text_out,
                                 int offset, int len, bool group_digits)
{
  int digits = 0;
  uint32_t numberCopy = number;
  while (digits < 10 && numberCopy > 0) {
    numberCopy /= base;
    ++digits;
  }

  if (group_digits)
    digits += digits / 3;

  if (len < 1)
    len = 1;
  if (digits < len)
    digits = len;

  int i = digits - 1;
  for (; i >= 0; --i, number /= base) {
    char c = kHexAlphabet[number % base];
    text_out[i + offset] = c;
  }
}

/**
 * Moves the hardware cursor to the specified location.
 * @param x,y The location to move the cursor to.
 */
static void MoveHardwareCursor(int x, int y)
{
  int cursor_location = y * kScreenWidth + x;
  // Tell the VGA board we are setting the high cursor byte.
  outb(0x3D4, 14);
  // Send the high cursor byte.
  outb(0x3D5, cursor_location >> 8);
  // Tell the VGA board we are setting the low cursor byte.
  outb(0x3D4, 15);
  // Send the low cursor byte.
  outb(0x3D5, cursor_location);
}

/**
 * Scroll the text on the screen up by one line if the cursor is on the last
 * line.
 */
static void Scroll()
{
  // if the cursor isn't on the last line then there's no need to scroll
  if (cursor_y < kScreenHeight)
    return;

  // get a space character with the default color attributes
  ScreenChar blank = { ' ', GetColor() };

  // move the current text chunk that makes up the screen back in the buffer
  // by a line
  int i;
  for (i = 0; i < (kScreenHeight - 1) * kScreenWidth; ++i)
    screen_buffer[i] = screen_buffer[i + kScreenWidth];

  // the last line should now be blank. Do this by writing kScreenWidth spaces
  // to it.
  for (i = (kScreenHeight - 1) * kScreenWidth;
      i < kScreenHeight * kScreenWidth; ++i)
    screen_buffer[i] = blank;

  // the cursor should now be on the last line
  cursor_y = kScreenHeight - 1;
}

void SetForeColor(Color fore_color) {
  default_fore_color = fore_color;
}

void SetBackColor(Color back_color) {
  default_back_color = back_color;
}

void SetCursorPos(int x, int y) {
  cursor_x = x;
  cursor_y = y;
  MoveHardwareCursor(x, y);
}

void PutChar(char c, Color fore_color, Color back_color, int x, int y) {
  ScreenChar* location = screen_buffer + (y * kScreenWidth + x);
  location->character = c;
  location->color = GetColor(fore_color, back_color);
}

void PutChar(char c, Color fore_color, Color back_color) {
  // handle a backspace, by moving the cursor back one space
  if (c == 0x08 && cursor_x > 0) {
    --cursor_x;
    PutChar(' ', fore_color, back_color, cursor_x, cursor_y);
  }

  // handle a tab by increasing the cursor's X, but only to a point
  // where it is divisible by 8.
  else if (c == 0x09)
    cursor_x = (cursor_x + 8) & ~(8 - 1);

  // handle carriage return
  else if (c == '\r')
    cursor_x = 0;

  // handle newline by moving cursor back to left and increasing the row
  else if (c == '\n') {
    cursor_x = 0;
    ++cursor_y;
  }

  // handle any other printable character
  else if (c >= ' ') {
    PutChar(c, fore_color, back_color, cursor_x, cursor_y);
    ++cursor_x;
  }

  // check if we need to insert a new line because we have reached the end
  // of the screen
  if (cursor_x >= kScreenWidth) {
    cursor_x = 0;
    ++cursor_y;
  }

  // Scroll the screen if needed
  Scroll();
  // Move the hardware cursor.
  MoveHardwareCursor(cursor_x, cursor_y);
}

void PutChar(char c) {
  PutChar(c, Color::kDefault, Color::kDefault);
}

void Clear(Color clear_color) {
  ScreenChar blank = { ' ', GetColor(Color::kDefault, clear_color) };

  for (int i = 0; i < kScreenWidth * kScreenHeight; ++i)
    screen_buffer[i] = blank;

  SetCursorPos(0, 0);
}

void Write(const char *str, Color fore_color, Color back_color, int x, int y) {
  for (; *str; ++str) {
    if (++x >= kScreenWidth) {
      x = 0;
      ++y;
    }
    PutChar(*str, fore_color, back_color, x, y);
  }
}

void Write(const char *str, Color fore_color, Color back_color) {
  for (int i=0; str[i]; ++i)
    PutChar(str[i], fore_color, back_color);
}

void Write(const char *str) {
  Write(str, Color::kDefault, Color::kDefault);
}

void WriteLine(const char *str, Color fore_color, Color back_color) {
  Write(str, fore_color, back_color);
  PutChar('\n', fore_color, back_color);
}

void WriteLine(const char *str) {
  WriteLine(str, Color::kDefault, Color::kDefault);
}

void WriteHex(uint32_t n, Color fore_color, Color back_color) {
  char hex[9];
  //TODO(ryan-bunker): sprintf(hex, "%08lx", n);when sbrk syscall is implemented
  NumberToTextUnsigned(n, 16, hex, 0, 8, false);
  Write(hex, fore_color, back_color);
}

void WriteHex(uint32_t n) {
  WriteHex(n, Color::kDefault, Color::kDefault);
}

void WriteDec(uint32_t n, Color fore_color, Color back_color) {
  char dec[20];
  //TODO(ryan-bunker): sprintf(dec, "%ld", n); when sbrk syscall is implemented
  NumberToTextUnsigned(n, 10, dec, 0, 0, false);
  Write(dec, fore_color, back_color);
}

void WriteDec(uint32_t n) {
  WriteDec(n, Color::kDefault, Color::kDefault);
}

}  // namespace screen
