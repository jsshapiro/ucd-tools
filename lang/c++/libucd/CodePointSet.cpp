/**************************************************************************
 *
 * Copyright (C) 2014, Jonathan S. Shapiro
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
#include <iostream>
#include "CodePointSet.h"

#define DEBUG if (0)

namespace libucd {
  CodePointSet::CodePointSet()
  {
  }

  CodePointSet::~CodePointSet()
  {
  }

  std::pair<CodePointSet::iterator, bool>
  CodePointSet::insert(const CodePointRange& r)
  {
    if (r.empty())
      return {end(), false};

    DEBUG std::cout << "Call to insert(" << r << ')' << std::endl;

    if (empty()) {
      DEBUG std::cout << "  Insert into empty set" << std::endl;
      return m_set.insert(r);
    }

    CodePointRange range = r;

    auto lb = lower_bound(range);

    // /lb/ is NOT LESS THAN. If it exists, then (a) it is strictly above, or
    // (b) there is overlap. Note that lb cannot be the empty range, because
    // that sorts as the smallest range.
    //
    // Note further that since lb is NOT LESS THEN, if lb abuts then it
    // necessarily abuts from above.

    DEBUG {
      if (lb != end())
        std::cout << "  Found lb at " << (*lb) << std::endl;
      else
        std::cout << "  No LB found" << std::endl;
    }

    if (lb != end() && lb->contains(range)) {
      DEBUG std::cout << "Already contained." << std::endl;
      return {lb, false};
    }

    // Find the least constituent range that might merge with /range/.
    // That is either /lb/ (iff lb->m_min < range.m_min) or it is the
    // predecessor to /lb/:
    if (lb == end() || (lb->min() >= range.min())) {
      if (lb != begin()) {
        auto prev = lb;
        prev--;
        if (range.canMergeWith(*prev)) {
          DEBUG std::cout << "Range " << range
                          << " can merge with " << *prev << std::endl;
          lb = prev;
          range = (*prev) | range;
        }
      }
    }
    else if (lb->min() < range.min()) {
      // /lb/ overlaps /range/ from below.
      range = (*lb) | range;
    }

    auto upper = upper_bound(range);

    // /upper/ is STRICTLY ABOVE /range/

    DEBUG {
      if (upper == end())
        std::cout << "Upper bound at top" << std::endl;
      else
        std::cout << "Upper bound at " << (*upper) << std::endl;
    }

    if (upper != end() && upper->abuts(range)) {
      range = range | (*upper);
      upper++; // Keep /upper/ strictly above.
    }
  #if 0
    else if (upper != begin()) {
      // upper
      iterator prev = upper;
      prev--;
      if (prev->canMergeWith(range)) {
        DEBUG std::cout << "Merge " << range
                        << " with prev " << (*prev) << std::endl;
        range = range | (*prev);
      }
    }
  #endif

    DEBUG {
      if (lb == end())
        std::cout << "Nothing to erase" << std::endl;
      else if (upper == end())
        std::cout << "Erasing from " << (*lb) << " to top" << std::endl;
      else
        std::cout << "Erasing from " << (*lb)
                  << " to " << (*upper) << std::endl;
    }

    m_set.erase(lb, upper);

    DEBUG std::cout << "Do insert of " << range << std::endl;

    auto pr = m_set.insert(range);
    return { pr.first, true };
  }

  // Note that removing a range will never cause existing ranges to
  // coalesce, so we don't need to be concerned about range size limits.
  size_t
  CodePointSet::erase(const CodePointRange& r)
  {
    size_t count = 0;

    if (r.empty())
      return 0;

    if (empty())
      return 0;

    // Lower holds the first range that is not strictly less than /r/.
    // So anything *beneath* that is definitely going to be removed
    // but the lower bound might partially overlap.
    for(;;) {
      iterator lb = lower_bound(r);

      if ((lb == end()) || ((*lb) > r))
        return 0;

      // Not strictly greater and not strictly less than, which leaves:
      assert(lb->overlaps(r));

      CodePointRange below = lb->portionBelow(r);
      CodePointRange above = lb->portionAbove(r);

      m_set.erase(lb);
      count++;

      if (!below.empty()) insert(below);
      if (!above.empty()) insert(above);
    }

    return count;
  }

  void
  CodePointSet::insert(const CodePointSet &set)
  {
    for (auto it = set.begin(); it != set.end(); it++)
      insert(*it);
  }

  size_t
  CodePointSet::erase(const CodePointSet &set)
  {
    size_t count = 0;

    for (auto it = set.begin(); it != set.end(); it++)
      count += erase(*it);

    return count;
  }

  CodePointSet
  CodePointSet::boundBy(const CodePointRange& r) const
  {
    if (r.empty())
      return CodePointSet(); // an empty one

    CodePointSet rs(*this);

    if (r.min() > 0)
      rs.erase(CodePointRange(0, r.min()));

    if (r.max() < CODEPOINT_EOF)
      rs.erase(CodePointRange(r.max() + 1, CODEPOINT_EOF));

    return rs;
  }

  bool CodePointSet::contains(const CodePointRange& range) const
  {
    auto lb = lower_bound(range);

    // /lb/ is NOT LESS THAN. If it exists, then (a) it is strictly above, or
    // (b) there is overlap. Note that lb cannot be the empty range, because
    // that sorts as the smallest range.

    if (lb == end())
      return false;

    // Ranges in a range set are dense, meaning that abutting ranges are
    // merged on insert. Therefore, if /range/ is wholly contained, it
    // is necessarily the case that it is wholly contained by /lb/.
    return lb->contains(range);
  }


  size_t
  CodePointSet::NumCodePoints() const
  {
    size_t nElem = 0;
    for (auto it = begin(); it != end(); it++)
      nElem += it->size();

    return nElem;
  }
}

std::ostream&
operator<< (std::ostream& os, const libucd::CodePointSet& r)
{
  os << "{";
  const char *sep = " ";

  for (auto it = r.begin();
       it != r.end();
       it++) {
    os << sep << (*it);
    sep = "\n  ";
  }
  os << " }";

  return os;
}

