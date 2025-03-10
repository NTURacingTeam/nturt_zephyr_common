.. _decisions_file_structure:

==============
File Structure
==============

Summary
=======

Issue
-----

Decision
--------

Status
------

Details
=======

Assumption
----------

* `Clang-Format <https://clang.llvm.org/docs/index.html>`__ is used as the
  formatter based on `Google style
  <https://clang.llvm.org/docs/ClangFormatStyleOptions.html#basedonstyle>`__.

Constraints
-----------

Positions
---------

This record generally follows the `Google C++ Style Guide
<https://google.github.io/styleguide/cppguide.html>`__ and `Linux Kernel Coding
Style <https://www.kernel.org/doc/html/v4.10/process/coding-style.html>`__, and
will refer to them later as "Google Guide" and "Linux Guide" respectively.

The structure of source file (.c) should be as follows:

1. ``#include`` directives that follow;

   * Order of includes from the `Google Guide
     <https://google.github.io/styleguide/cppguide.html#Names_and_Order_of_Includes>`__

     .. note::
     
        Since Clang-Format will remove blink lines between includes, use
        comments to separate different includes.

   * Avoid transitive includes as mentioned in the `Google Guide
     <https://google.github.io/styleguide/cppguide.html#Include_What_You_Use>`__

2. ``#define`` directives for macros and constants
3. Type fordward declarations only for types that will be defined later in the
   file if necesary
4. Typedefs
5. Type declarations in the following order;

   1. Enums
   2. Structs
   3. Unions

6. Static function declarations
7. External variables
8. Static variables
9. Function definitions
10. Static function definitions

With the exception of conditional compilation, which should be grouped together
for readability, refer to the `Linux Guide
<https://kernel.org/doc/html/latest/process/coding-style.html#conditional-compilation>`__
to write code with conditional compilation better.

Within a module, group global variables into a single context structure
:c:struct:`MODULE_NAME_ctx`. Initialize its member in a static initializer if
possible and define a :c:func:`MODULE_NAME_init` function to initialize other
members unable to be initialized in the static initializer.

Arguments
---------

Implications
------------

Notes
=====

* `Google C++ Style Guide <https://google.github.io/styleguide/cppguide.html>`__
* `Zephyr Coding Style Guidelines
  <https://docs.zephyrproject.org/latest/contribute/style/index.html>`__
* `Linux Kernel Coding Style
  <https://kernel.org/doc/html/latest/process/coding-style.html>`__
