#ifndef UTF8_H
#define UTF8_H

/**************************************************************************
 *
 * Copyright (C) 2016, Jonathan S. Shapiro
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the
 * following conditions are met:
 *
 *   - Redistributions of source code must contain the above
 *     copyright notice, this list of conditions, and the following
 *     disclaimer.
 *
 *   - Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions, and the following
 *     disclaimer in the documentation and/or other materials
 *     provided with the distribution.
 *
 *   - Neither the names of the copyright holders nor the names of any
 *     of any contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **************************************************************************/

#include "CodePoint.h"

namespace libucd {
  /// @brief Given a code point, return the length of its UTF-8 encoding.
  static inline size_t
  utf8_encoded_length(CodePoint_t cp)
  {
    if (cp <= 0x7fU)
      return 1;
    else if (cp <= 0x7ffU)
      return 2;
    else if (cp <= 0xffffU)
      return 3;
    else if (cp <= 0x1fffffU)
      return 4;
    else if (cp <= 0x3ffffffU)
      return 5;
    return 6;
  }

  /// @brief Given the @em first byte of a UTF-8 encoded code point, return
  /// the number of bytes that were used to encode the code point.
  static inline size_t
  utf8_decode_length(unsigned char b)
  {
    if (b < 0x80U)
      return 1;
    else if (b < 0xe0U)
      return 2;
    else if (b < 0xf0U)
      return 3;
    else if (b < 0xf8U)
      return 4;
    else if (b < 0xfcU)
      return 5;
    else
      return 6;
  }

  /// @brief Return the mask value that should be used to extract the
  /// encoded code point bits from the first byte of a UTF-8 encoded
  /// code point.
  static inline unsigned char
  utf8_decode_b0_mask(unsigned char b)
  {
    size_t nBytes = utf8_decode_length(b);
    unsigned char mask[] = { 0, 0x7F, 0x1F, 0x0f, 0x07, 0x03, 0x01 };
    return mask[nBytes];
  }

  /// @brief Given a byte string and a length bound on that string,
  /// extract the next UTF-8 encoded code point and return the start
  /// position of the following UTF-8 encoded code point.
  CodePoint_t
  utf8_decode(const char *utf8String,
              const char **next = 0,
              const char *bound = 0);

  /// @brief Given a unicode codepoint, return its encoding to UTF-8.
  std::string utf8_encode(CodePoint_t cp);

  /// @brief Given a unicode codepoint, encode it to UTF-8 at the address
  /// provided by @p s, returning the next byte position as @p *sEnd exactly
  /// if @p sEnd is non-NULL.
  void utf8_encode(CodePoint_t cp, char *s, char **sEnd);

  /// @brief Compute the length of a [sub]string in code points
  size_t utf8_cplen(const char *s, const char *bound = 0);
}

#endif // UTF8_H
