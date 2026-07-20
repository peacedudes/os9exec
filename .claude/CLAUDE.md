# OS9exec project rules

The `Source/OS9exec_core/os9defs/` headers — `*_from_book.h` and
`errno_from_docs.h` — are SECOND-HAND. Their contents were reconstructed from
published books and documentation, never from Microware source. We tried to get
them right and believe they are close, but nothing in them is guaranteed to be
Microware-compatible.

Treat that as a live possibility when something does not behave as expected: a
disagreement between one of these headers and real OS-9 is at least as likely to
be an error in the reconstruction as a bug in the code using it. Verify against
observed behaviour before concluding the emulator is wrong.

The proprietary Microware source files that were previously in this directory
have been removed from the working directory. No access restrictions apply.
