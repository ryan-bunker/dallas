/**
 * @file text_screen.h
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

#ifndef SRC_INCLUDE_VIDEO_TEXT_SCREEN_H_
#define SRC_INCLUDE_VIDEO_TEXT_SCREEN_H_

#include <cstdint>

namespace screen {

/**
 * Represents a foreground or background color of characters on the screen.
 */
enum class Color : uint8_t {
  kDefault = static_cast<uint8_t>(-1), //!< kDefault
  kBlack = 0,                          //!< kBlack
  kBlue,                               //!< kBlue
  kGreen,                              //!< kGreen
  kCyan,                               //!< kCyan
  kRed,                                //!< kRed
  kMagenta,                            //!< kMagenta
  kBrown,                              //!< kBrown
  kLightGray,                          //!< kLightGray
  kDarkGray,                           //!< kDarkGray
  kLightBlue,                          //!< kLightBlue
  kLightGreen,                         //!< kLightGreen
  kLightCyan,                          //!< kLightCyan
  kLightRed,                           //!< kLightRed
  kLightMagenta,                       //!< kLightMagenta
  kYellow,                             //!< kYellow
  kWhite                               //!< kWhite
};

/**
 * Sets the default foreground color.
 * @param fore_color The color to set.
 */
void SetForeColor(Color fore_color);

/**
 * Sets the default background color.
 * @param back_color The color to set.
 */
void SetBackColor(Color back_color);

/**
 * Sets the current cursor position.
 * @param x,y The location to set the cursor to.
 */
void SetCursorPos(int x, int y);

/**
 * Writes a single character to the screen in the specified color at the
 * specified location.
 * @param c The character to write.
 * @param fore_color The foreground color of the character.
 * @param back_color The background color of the character.
 * @param x,y The location to write the character.
 */
void PutChar(char c, Color fore_color, Color back_color, int x, int y);

/**
 * Writes a single character to the screen in the specified color.
 * @param c The character to write.
 * @param fore_color The foreground color of the character.
 * @param back_color The background color of the character.
 */
void PutChar(char c, Color fore_color, Color back_color = Color::kDefault);

/**
 * Writes a single character to the screen.
 * @param c The character to write.
 */
void PutChar(char c);

/**
 * Clear the screen.
 * @param clear_color The background color to clear the screen to.
 */
void Clear(Color clear_color = Color::kDefault);

/**
 * Writes a string to the screen in the specified color at the specified
 * location.
 * @param str The null-terminated ASCII string to write.
 * @param fore_color The foreground color of the character.
 * @param back_color The background color of the character.
 * @param x,y The location to write the character.
 */
void Write(const char *str, Color fore_color, Color back_color, int x, int y);

/**
 * Writes a string to the screen in the specified color.
 * @param str The null-terminated ASCII string to write.
 * @param fore_color The foreground color of the character.
 * @param back_color The background color of the character.
 */
void Write(const char *str, Color fore_color,
           Color back_color = Color::kDefault);

/**
 * Writes a string to the screen.
 * @param str The null-terminated ASCII string to write.
 */
void Write(const char *str);

/**
 * Writes a string to the screen followed by a newline in the specified color.
 * @param str The null-terminated ASCII string to write.
 * @param fore_color The foreground color of the character.
 * @param back_color The background color of the character.
 */
void WriteLine(const char *str, Color fore_color,
               Color back_color = Color::kDefault);

/**
 * Writes a string to the screen followed by a newline.
 * @param str The null-terminated ASCII string to write.
 */
void WriteLine(const char *str);

/**
 * Writes a 32-bit integer to the screen in hexadecimal format in the specified
 * color.
 * @param n The 32-bit integer to write.
 * @param fore_color The foreground color of the character.
 * @param back_color The background color of the character.
 */
void WriteHex(uint32_t n, Color fore_color, Color back_color = Color::kDefault);

/**
 * Writes a 32-bit integer to the screen in hexadecimal format.
 * @param n The 32-bit integer to write.
 */
void WriteHex(uint32_t n);

void WriteHexUnpadded(uint32_t n);

/**
 * Writes a 32-bit integer to the screen in decimal format in the specified
 * color.
 * @param n The 32-bit integer to write.
 * @param fore_color The foreground color of the character.
 * @param back_color The background color of the character.
 */
void WriteDec(uint32_t n, Color fore_color, Color back_color = Color::kDefault);

/**
 * Writes a 32-bit integer to the screen in decimal format.
 * @param n The 32-bit integer to write.
 * @param groupDigits Whether to separate the digits into groups of three.
 */
void WriteDec(uint32_t n);

void Writef(const char *fmt, ...);

} // namespace screen

#endif // SRC_INCLUDE_VIDEO_TEXT_SCREEN_H_
