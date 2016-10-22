# ucd-tools
`ucd-tools` provides compiler-oriented tooling for dealing with Unicode
properties and normalization in multiple source languages.

## Introduction and Rationale

The Unicode Character Database is an incredibly useful and somewhat
frustrating resource. The official state of the UCD is spread between
multiple files having inconsistent formats. The XML version of the UCD
omits the state of some newer properties, and also experimental properties.
There is no library (not even ICU) that provides comprehensive support for
_all_ Unicode properties, and no straightforward way to implement
locally-defined custom properties.
If the [International Components for Unicode](http://icu-project.org)
can solve the problem in whatever source language you are using, it's
definitely the right solution to adopt. Unfortunately that leaves most
source languages without native support.

Where ICU aims at broad support of Unicode, many applications in practice
adopt simplifying policies. Examples include:

- External and internal encodings are all done in UTF-8. No
  conversion support is required.
- A particular normalization form (typically NFC) is chosen for
  input; the application may _validate_ normalization, but does not 
  _perform_ normalization.
- Sorting is only performed internally, and is not script- or
  locale-aware.
- All bidiretional text is handled in logical order; no bidirectional
  support is required.

These programs might be viewed as providing "arm's length" Unicode support,
but there are some important applications in this category - most notably
compilers. Compilers are also unusual because

- A Unicode-aware programming language is generally specified with
  respect to some particular version of Unicode. It is not a goal
  (in fact, it is an anti-goal) to always track the latest version of
  the Unicode standard.
- A compiler implementation _really_ wants to be able to access
  Unicode tables and algorithms in the native programming language
  for reasons of text stream processing efficiency.

Enter `ucd-tools`.

## What is `ucd-tools`?

ucd-tools consists of four parts:

1. A collection of XML files. Primarily the XML-formatted UCD
   database, but also some additional XML files that define
   properties not included in the XML version of the UCD. The
   XML files also supply property aliases and property value aliases.

   These files, along with any additional XML files that define
   local, custom properties, will be digested by the first-stage
   tool.

2. A property _compiler_ that digests the XML files and produces a
   monolithic C++ source file that contains all of this information
   in a single, monolithic file.

   The property compiler also performs various validity and cross-version
   stability checks.

3. A property _generator_ that produces compact property tables and
   supporting type definitions for the
   various Unicode and locally-defined properties and their values.
   The property generator can be extended to support additional source
   language targets in a modular way.

   These tables depend in turn on a small, language-specific support
   library that is separately implemented for each target language.

4. A support library for each of the supported source languages. The
   support library provides:

   - Language-specific type definition for Unicode code points.

     This typically is _not_ the same as the language's character
     datatype, because many source languages have adopted a 16-bit
     character type, and Unicode code points range over 21-bit
     unsigned values.

   - A data structure that implements closed code point ranges.

   - A specialized set implementation for code points based on
     the closed code point range type.

     Unicode code point sets often consist of sparse, dense
     ranges, so it is useful to be able to iterate by range
     and to store the set using a dense representation.

   - Validation algorithms for NFC-encoded input.

   - \[Possible, Future:\] Implementations of case conversion and case-aware
     string comparison.

The general idea is to minimize the amount of code that has to be written
for a new source language, and use table-driven algorithms whose tables can
be easily updated as the Unicode version advances. Because the `ucd-tools`
client program requires support for a specific _version_ of Unicode, it
is assumed that the generated tables and the support library are likely
to be statically linked into the client binary.
