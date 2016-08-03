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

#include "sys/io.h"
#include "video/text_screen.h"

namespace screen {

/**
 * The width of the default screen buffer, in characters.
 */
const int kScreenWidth = 80;
/**
 * The height of the default screen buffer, in rows.
 */
const int kScreenHeight = 25;

/**
 * Represents a single character in the screen buffer.
 */
struct ScreenChar {
  /**
   * The ASCII character to display.
   */
  unsigned char character;
  /**
   * The foreground and background color of the character.
   */
  uint8_t color;
} __attribute__((packed));

/**
 * The active screen buffer.
 */
ScreenChar *const screen_buffer = reinterpret_cast<ScreenChar *>(0xC00B8000);
/**
 * The cursor's current x (column) position.
 */
int cursor_x = 0;
/**
 * The cursor's current y (row) position.
 */
int cursor_y = 0;
/**
 * The current default foreground color to use if no color is specified
 * when printing.
 */
Color default_fore_color = Color::kWhite;
/**
 * The current default background color to use if no color is specified
 * when printing.
 */
Color default_back_color = Color::kBlack;

/**
 * Packs color information into an unsigned byte, for placing into the screen
 * buffer.
 * @param fore_color The foreground color to use.
 * @param back_color The background color to use.
 * @return The foreground and background color packed into a single byte.
 */
inline uint8_t GetColor(Color fore_color = Color::kDefault,
                        Color back_color = Color::kDefault) {
  if (fore_color == Color::kDefault)
    fore_color = default_fore_color;
  if (back_color == Color::kDefault)
    back_color = default_back_color;
  return (static_cast<uint8_t>(back_color) << 4) |
         (static_cast<uint8_t>(fore_color) & 0x0F);
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */

/**
 * Converts an integer into a string.
 * @param[in] value The integer to convert.
 * @param[out] result The buffer to place the resulting string.
 * @param[in] base The base to use when converting. Can be any integer between
 * 2 and 36 inclusive.
 * @return result
 */
char *itoa(uint32_t value, char *result, int base) {
  // check that the base if valid
  if (base < 2 || base > 36) {
    *result = '\0';
    return result;
  }

  char *ptr = result, *ptr1 = result, tmp_char;
  uint32_t tmp_value;

  const char *alphabet =
      "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz";
  do {
    tmp_value = value;
    value /= base;
    *ptr++ = alphabet[35 + (tmp_value - value * base)];
  } while (value);

  *ptr-- = '\0';
  while (ptr1 < ptr) {
    tmp_char = *ptr;
    *ptr-- = *ptr1;
    *ptr1++ = tmp_char;
  }

  return result;
}

/**
 * Moves the hardware cursor to the specified location.
 * @param x,y The location to move the cursor to.
 */
static void MoveHardwareCursor(int x, int y) {
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
static void Scroll() {
  // if the cursor isn't on the last line then there's no need to scroll
  if (cursor_y < kScreenHeight)
    return;

  // get a space character with the default color attributes
  ScreenChar blank = {' ', GetColor()};

  // move the current text chunk that makes up the screen back in the buffer
  // by a line
  int i;
  for (i = 0; i < (kScreenHeight - 1) * kScreenWidth; ++i)
    screen_buffer[i] = screen_buffer[i + kScreenWidth];

  // the last line should now be blank. Do this by writing kScreenWidth spaces
  // to it.
  for (i = (kScreenHeight - 1) * kScreenWidth; i < kScreenHeight * kScreenWidth;
       ++i)
    screen_buffer[i] = blank;

  // the cursor should now be on the last line
  cursor_y = kScreenHeight - 1;
}

void SetForeColor(Color fore_color) { default_fore_color = fore_color; }

void SetBackColor(Color back_color) { default_back_color = back_color; }

void SetCursorPos(int x, int y) {
  cursor_x = x;
  cursor_y = y;
  MoveHardwareCursor(x, y);
}

void PutChar(char c, Color fore_color, Color back_color, int x, int y) {
  ScreenChar *location = screen_buffer + (y * kScreenWidth + x);
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

void PutChar(char c) { PutChar(c, Color::kDefault, Color::kDefault); }

void Clear(Color clear_color) {
  ScreenChar blank = {' ', GetColor(Color::kDefault, clear_color)};

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
  for (int i = 0; str[i]; ++i)
    PutChar(str[i], fore_color, back_color);
}

void Write(const char *str) { Write(str, Color::kDefault, Color::kDefault); }

void WriteLine(const char *str, Color fore_color, Color back_color) {
  Write(str, fore_color, back_color);
  PutChar('\n', fore_color, back_color);
}

void WriteLine(const char *str) {
  WriteLine(str, Color::kDefault, Color::kDefault);
}

void WriteHex(uint32_t n, Color fore_color, Color back_color) {
  char hex[9];
  // TODO(ryan-bunker): sprintf(hex, "%08lx", n);when sbrk syscall is
  // implemented
  itoa(n, hex, 16);
  Write(hex, fore_color, back_color);
}

void WriteHex(uint32_t n) { WriteHex(n, Color::kDefault, Color::kDefault); }

void WriteDec(uint32_t n, Color fore_color, Color back_color) {
  char dec[20];
  // TODO(ryan-bunker): sprintf(dec, "%ld", n); when sbrk syscall is implemented
  itoa(n, dec, 10);
  Write(dec, fore_color, back_color);
}

void WriteDec(uint32_t n) { WriteDec(n, Color::kDefault, Color::kDefault); }

} // namespace screen
