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

#include <assert.h>
#include "utf8.h"

namespace libucd {
  CodePoint_t
  utf8_decode(const char *utf8String, const char **next, const char *bound)
  {
    if (!bound)
      bound = utf8String + 4;

    if ((bound - utf8String) < 1)
      return CODEPOINT_EOF;

    char b0 = *utf8String;
    ptrdiff_t nBytes = utf8_decode_length(b0);

    if ((bound - utf8String) < nBytes)
      return CODEPOINT_EOF;

    CodePoint_t c = (b0 & utf8_decode_b0_mask(b0));

    for (ptrdiff_t i = 1; i < nBytes; i++) {
      c <<= 6;
      unsigned char b = utf8String[i];
      if ((b & 0xc0) != 0x80)
        return CODEPOINT_EOF;

      c |= (utf8String[1] & 0x3F);
    }

    if (c > CODEPOINT_MAX)
      return CODEPOINT_EOF;

    if (next)
      *next = utf8String + nBytes;

    return c;
  }

  void
  utf8_encode(CodePoint_t cp, char *s, char **sEnd)
  {
    assert ( cp <= CODEPOINT_MAX );

    if (cp <= 0x7f) {
      *s++ = cp;
    }
    else if (cp <= 0x07ff) {
      *s++ = (0xc0 | (cp >> 6));
      *s++ = (0x80 | (cp & 0x3f));
    }
    else if (cp <= 0x0FFF) {
      *s++ = (0xE0 | (cp >> 12));
      *s++ = (0x80 | ((cp >> 6) & 0x3f));
      *s++ = (0x80 | (cp & 0x3f));
    }
    else {
      *s++ = (0xF0 | (cp >> 18));
      *s++ = (0x80 | ((cp >> 12) & 0x3f));
      *s++ = (0x80 | ((cp >> 6) & 0x3f));
      *s++ = (0x80 | (cp & 0x3f));
    }

    if (sEnd)
      *sEnd = s;
  }

  std::string
  utf8_encode(CodePoint_t cp)
  {
    assert ( cp <= CODEPOINT_MAX );

    char encoding[5];
    char *sEnd;

    utf8_encode(cp, encoding, &sEnd);
    *sEnd = 0;
    return encoding;
  }

  size_t
  utf8_cplen(const char *s, const char *bound)
  {
    size_t len = 0;
    while (*s && (s != bound)) {
      s += utf8_decode_length(*s);
      len++;
    }

    return len;
  }
}
