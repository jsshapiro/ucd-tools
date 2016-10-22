#ifndef CODEPOINTRANGE_H
#define CODEPOINTRANGE_H

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

#include <type_traits>
#include <limits>
#include <ostream>

#include "CodePoint.h"

#if 0
#include "BasicTypeName.h"
#endif

#if 0
/// @brief Iterator support for CodePointRange objects.
///
/// This is necessary in order to deal
/// that integral types have no "out of band" value that can be used to
/// signal an open upper bound.
///
/// In the parlance of C++ iterators, this is an input iterator.
template <class T>
class CodePointRangeIterator {
    bool m_isEnd;
    T m_value;

  public:
    CodePointRangeIterator(T value)
    {
      m_isEnd = false;
      m_value = value;
    }

    CodePointRangeIterator(const CodePointRangeIterator& other)
    {
      m_isEnd = other.m_isEnd;
      m_value = other.m_value;
    }

    bool operator == (const CodePointRangeIterator& other) const {
      return ((m_isEnd == other.m_isEnd)  && (m_value == other.m_value));
    }
    bool operator != (const CodePointRangeIterator& other) const {
      return ((m_isEnd != other.m_isEnd)  || (m_value != other.m_value));
    }

    T operator *() const { return m_value; }

    // Increment variants
    //
    // if m_value is maximal, then we treat it as "clamped" and set
    // m_isEnd. This is how we manage iterating at 255 in a single byte
    // value to arrive at an out-of-range end value.
    //
    // Note that ++ on end() is generally undefined, and the limit
    // test used by convention is != rather than <.

    // Pre-increment:
    CodePointRangeIterator& operator++() {
      if (m_value == std::numeric_limits<CodePoint_t>::max())
        m_isEnd = true;
      else
        m_value++;
      return *this;
    }
    // Post-increment
    CodePointRangeIterator& operator++(int) {
      if (m_value == std::numeric_limits<CodePoint_t>::max())
        m_isEnd = true;
      else
        m_value++;
      return *this;
    }
};
#endif
namespace libucd {
  /// @brief Implementation of Unicode code point ranges.
  ///
  /// The main operations of this class are range comparisons and range
  /// containment checks.
  ///
  /// Given two instances a,b:
  /// - a &lt; b iff a.bound <= b.base
  /// - a &gt; b iff b.base >= a.bound
  /// - a == b iff a.base == b.base &amp;&amp; a.bound == b.bound
  /// - a != b iff a.base != b.base || a.bound != b.bound
  ///
  ///  note that overlapping instances are not orderable!

  class CodePointRange {
      /// @brief Minimum and maximum values for this range.
      ///
      /// A range is empty if m_max < m_min, but we specifically use
      /// - m_min = std::numeric_limits<CodePoint_t>::max() and
      /// - m_max = std::numeric_limits<CodePoint_t>::min() in this case.
      CodePoint_t m_min;
      CodePoint_t m_max; // INCLUSIVE

    public:
      typedef CodePoint_t value_type;
      typedef CodePoint_t iterator;

      inline CodePoint_t base() const { return m_min; }
      inline CodePoint_t bound() const { return m_max+1; }

      CodePointRange() {
        m_min = std::numeric_limits<CodePoint_t>::max();
        m_max = std::numeric_limits<CodePoint_t>::min();
      }

      CodePointRange(const CodePoint_t min, const CodePoint_t max)
      { m_min = min;
        m_max = max; }

      CodePointRange(const CodePoint_t value)
      { m_min = value;
        m_max = value; }

      CodePointRange(const CodePointRange & other)
      { m_min = other.m_min;
        m_max = other.m_max; }

      CodePoint_t min() const { return m_min; }
      CodePoint_t max() const { return m_max; }

      static CodePointRange closed(const CodePoint_t min, const CodePoint_t max)
      { return CodePointRange(min, max); }
      static CodePointRange open(const CodePoint_t base, const CodePoint_t bound)
      { return CodePointRange(base, bound-1); }

      static CodePointRange ASCII()
      { return CodePointRange::closed(0, 127); }

      static CodePointRange Unicode()
      { return CodePointRange::closed(0, 0x10ffff); }

      iterator begin() const {
        return m_min;
      }
      iterator end() const {
        return empty() ? m_min : (m_max + 1);
      }

      size_t size() const { return (m_max - m_min) + 1; }

      bool empty() const {
        return ((m_min == std::numeric_limits<CodePoint_t>::max()) &&
                (m_max == std::numeric_limits<CodePoint_t>::min()));
      }
      bool isSingleton() const { return (size() == 1); }

      bool contains(CodePoint_t cp) const
      { return !empty() && ((m_min <= cp) && (cp <= m_max)); }

      // All ranges, including the empty range, contain the empty range.
      bool contains(const CodePointRange & r) const
      { if (r.empty()) return true;
        if (empty()) return false;
        return ((m_min <= r.m_min) && (r.m_max <= m_max));
      }

      // All ranges abut the empty range
      bool abuts(const CodePointRange & r) const
      { if (empty() || r.empty()) return true;
        if ((m_min > std::numeric_limits<CodePoint_t>::min()) && ((m_min - 1) == r.m_max))
          return true;
        if ((m_max < std::numeric_limits<CodePoint_t>::max()) && ((m_max + 1) == r.m_min))
          return true;
        return false;
      }

      bool overlaps(const CodePointRange & r) const
      // Four cases of interest; the third is easy to get wrong:
      //    |---- r ----|
      //       |---- *this ----|
      //
      //    |---- *this ---- |
      //              |---- r ----|
      //
      //    |---- *this ----|
      //      |---- r ----|
      //
      //      |---- *this ----|
      //    |-------- r --------|
      { return (contains(r.m_min) || contains(r.m_max)) || r.contains(m_min); }

      bool canMergeWith(const CodePointRange & r) const
      { return abuts(r) || overlaps(r); }

      // Union
      CodePointRange operator |(const CodePointRange& r) const
      { if (empty()) return r;
        if (r.empty()) return *this;

        return CodePointRange(std::min(m_min, r.m_min),
                              std::max(m_max, r.m_max));
      }
      // Intersection
      CodePointRange operator &(const CodePointRange& r) const
      { if (!overlaps(r)) return CodePointRange(); // empty range

        return CodePointRange(std::max(m_min, r.m_min),
                              std::min(m_max, r.m_max));
      }

      // Strictly beneath. I've adopted the convention that empty
      // ranges sort below non-empty ranges.
      bool operator < (const CodePointRange& r) const
      { if (empty()) return !r.empty();
        if (r.empty()) return false;
        return m_max < r.m_min;
      }
      // Strictly above. Empty ranges are non-comparable.
      bool operator > (const CodePointRange& r) const
      { if (r.empty()) return !empty();
        if (empty()) return false;
        return m_min > r.m_max;
      }

      // Empty ranges can be compared for equality
      bool operator == (const CodePointRange& r) const
      { return ((m_min == r.m_min) && (m_max == r.m_max)); }
      bool operator != (const CodePointRange& r) const
      { return ((m_min != r.m_min) || (m_max != r.m_max)); }

      bool isStrictlyBelow(const CodePointRange& r) const
      { return (*this < r); }
      bool isStrictlyAbove(const CodePointRange& r) const
      { return (*this > r); }

      CodePointRange portionBelow(const CodePointRange& r) const
      {
        if (isStrictlyBelow(r))
          return *this;
        if(isStrictlyAbove(r) || (m_min >= r.m_min))
          return CodePointRange();

        // If we are here, then there is overlap from beneath
        return CodePointRange(m_min, r.m_min-1);
      }
      CodePointRange portionAbove(const CodePointRange& r) const
      {
        if (isStrictlyAbove(r))
          return *this;
        if (isStrictlyBelow(r) || (m_max <= r.m_max))
          return CodePointRange();

        // There is overlap from above
        return CodePointRange(r.m_max+1, m_max);
      }

#if 0
      friend
      std::ostream& operator<< (std::ostream&, CodePointRange const&);
#endif
  };

}

inline std::ostream&
operator << (std::ostream& os, const libucd::CodePointRange& r) {
  std::ios_base::fmtflags oflags = os.flags();

  os << std::hex << std::showbase;
  if (r.isSingleton()) {
    os << "CodePointRange(" << r.min() << ")";
    os.flags(oflags);
  } else {
    os << "CodePointRange(" << r.min() << ',' << ' ' << r.max() << ')';
    os.flags(oflags);
  }
  os << " /* len=" << r.size() << " */";
  return os;
}

#endif // CODEPOINTRANGE_H
