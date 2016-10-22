This is the C++ implementation of the target language support library.

In contrast to all other target languages, this one is built as part of
the ucd-tools build. The property compiler uses parts of the library to
produce the monolithic property database, which is then linked into the
property generator binary. The source code that is in turn produced by
the property generator also makes reference to various elements of
the support library.
