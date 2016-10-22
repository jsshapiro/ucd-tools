#ifndef CODEPOINTSET_H
#define CODEPOINTSET_H

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

#include <set>

#include "CodePointRange.h"
#include "utf8.h"

namespace libucd {
  class CodePointSet
  {
      typedef typename std::set<CodePointRange> SetType;
      SetType m_set;

    public:
      typedef typename SetType::key_type key_type;
      typedef typename SetType::value_type value_type;
      typedef typename SetType::iterator iterator;
      typedef typename SetType::const_iterator const_iterator;
      typedef typename SetType::reverse_iterator reverse_iterator;
      typedef typename SetType::const_reverse_iterator const_reverse_iterator;
      typedef typename SetType::key_compare key_compare;
      typedef typename SetType::value_compare value_compare;

      CodePointSet();
      CodePointSet(const CodePointSet& that) {
        insert(that);
      }
      CodePointSet(const std::set<CodePointRange>& s) {
        for (auto it = s.begin(); it != s.end(); it++)
          insert(*it);
      }
      CodePointSet(const char *str) {
        while (*str) {
          CodePoint_t c = utf8_decode(str, &str);
          insert(CodePointRange(c));
        }
      }

      ~CodePointSet();

      iterator begin()        noexcept { return m_set.begin(); }
      const_iterator begin()  const noexcept { return m_set.begin(); }
      const_iterator cbegin() const noexcept { return m_set.cbegin(); }

      iterator end()        noexcept { return m_set.end(); }
      const_iterator end()  const noexcept { return m_set.end(); }
      const_iterator cend() const noexcept { return m_set.cend(); }

      reverse_iterator rbegin()        noexcept { return m_set.rbegin(); }
      const_reverse_iterator rbegin()  const noexcept { return m_set.rbegin(); }
      const_reverse_iterator crbegin() const noexcept { return m_set.crbegin(); }

      reverse_iterator rend()        noexcept { return m_set.rend(); }
      const_reverse_iterator rend()  const noexcept { return m_set.rend(); }
      const_reverse_iterator crend() const noexcept { return m_set.crend(); }

      iterator lower_bound(const CodePointRange& r)
      { return m_set.lower_bound(r); }
      const_iterator lower_bound(const CodePointRange& r) const
      { return m_set.lower_bound(r); }

      iterator upper_bound(const CodePointRange& r)
      { return m_set.upper_bound(r); }
      const_iterator upper_bound(const CodePointRange& r) const
      { return m_set.upper_bound(r); }

      iterator find(const CodePointRange& r)
      { return m_set.find(r); }
      const_iterator find(const CodePointRange& r) const
      { return m_set.find(r); }

      std::pair<iterator, iterator> equal_range(const CodePointRange& r)
      { return m_set.equal_range(r); }
      std:: pair <const_iterator, const_iterator> equal_range(const CodePointRange& r) const
      { return m_set.equal_range(r); }

      value_compare key_comp() const { return m_set.key_comp(); }
      value_compare value_comp() const { return m_set.value_comp(); }


      static CodePointSet Unicode() {
        return CodePointSet() + CodePointRange::Unicode();
      }
      static CodePointSet ASCII() {
        return CodePointSet() + CodePointRange::ASCII();
      }

      size_t size() const { return m_set.size(); }
      bool empty() const { return m_set.empty(); }
      bool contains(const CodePointRange& range) const;
      bool contains(CodePoint_t codePoint) const
      { return contains(CodePointRange(codePoint)); }

      // We want to follow the concept of the standard C++ library, but
      // we are merging entries internally. It isn't immediately clear
      // how to return the iterator for a newly inserted item when that
      // item may have been merged with other items.
      //
      // set insertion *should* be idempotent, so the approach adopted
      // here is to return a pair such that pair::first is an iterator
      // pointing to the range that, after insertion, *contains* the
      // inserted range (i.e. the designated range may be larger), and
      // pair::second is a boolean indicating whether the set was modified
      // to perform the insertion.
      std::pair<iterator, bool> insert(const CodePointRange& r);

      // Since we may have to merge, ignore the hint, but accept the
      // hint-styled signature:
      iterator insert(const_iterator /* pos */, const CodePointRange& val)
      { return insert(val).first; }

      template<class InputIterator>
      void insert(InputIterator first, InputIterator last) {
        for (auto it = first; it != last; it++)
          insert(*it);
      }
      void insert(std::initializer_list<value_type> il)
      { insert(il.begin(), il.end()); }

      // A similar problem arises with erase() in the case where an instance
      // of value_type is passed. We return the number of elements that are
      // **modified** or erased.
      size_t erase(const CodePointRange& r);
      iterator erase(const_iterator position)
      {
        return m_set.erase(position);
      }
      iterator erase(const_iterator first, const_iterator last)
      {
        return m_set.erase(first, last);
      }

      void insert(const CodePointSet& set);
      size_t erase(const CodePointSet& set);

      CodePointSet boundBy(const CodePointRange& r) const;

      CodePointSet& operator += (const CodePointRange& r) {
        insert(r);
        return *this;
      }
      CodePointSet& operator += (const CodePointSet& set) {
        insert(set);
        return *this;
      }
      CodePointSet operator += (const char *str) {
        while (*str) {
          CodePoint_t c = utf8_decode(str, &str);
          insert(CodePointRange(c));
        }
        return *this;
      }

      CodePointSet& operator -= (const CodePointRange& r) {
        erase(r);
        return *this;
      }
      CodePointSet& operator -= (const CodePointSet& set) {
        erase(set);
        return *this;
      }
      CodePointSet operator -= (const char *str) {
        while (*str) {
          CodePoint_t c = utf8_decode(str, &str);
          erase(CodePointRange(c));
        }
        return *this;
      }

      CodePointSet operator + (const CodePointSet& set) const {
        CodePointSet result = *this;
        result += set;
        return result;
      }
      CodePointSet operator + (const char *str) const {
        CodePointSet result = *this;
        while (*str) {
          CodePoint_t c = utf8_decode(str, &str);
          result.insert(CodePointRange(c));
        }
        return result;
      }
      CodePointSet operator + (const CodePointRange& r) const {
        CodePointSet result = *this;
        result += r;
        return result;
      }

      CodePointSet operator - (const CodePointSet& set) const {
        CodePointSet result = *this;
        result -= set;
        return result;
      }
      CodePointSet operator - (const char *str) const {
        CodePointSet result = *this;
        while (*str) {
          CodePoint_t c = utf8_decode(str, &str);
          result -= CodePointRange(c);
        }
        return result;
      }
      CodePointSet operator - (const CodePointRange& r) const {
        CodePointSet result = *this;
        result -= r;
        return result;
      }

      CodePointSet intersect(const CodePointSet& r) const {
        CodePointSet unique = *this;
        unique -= r;
        // What's left in /result/ is the items that were unique to /result/.
        return (*this - unique);
      }

      CodePointSet operator &(const CodePointSet& r) const {
        return this->intersect(r);
      }

      CodePointSet& operator &=(const CodePointSet& r) {
        *this = this->intersect(r);
        return *this;
      }

      CodePointSet operator |(const CodePointSet& r) const {
        return (*this) + r;
      }
      CodePointSet& operator |= (const CodePointSet& r) {
        this->insert(r);
        return *this;
      }

      size_t NumCodePoints() const;
  };
}

std::ostream& operator<< (std::ostream&, const libucd::CodePointSet&);


#endif // CODEPOINTSET_H
