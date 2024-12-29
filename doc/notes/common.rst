.. _notes_common:

======
Common
======

The C Programming Language
==========================

Built-in Functions
------------------

- :c:func:`__builtin_ffs`
- :c:func:`__builtin_clz`

`GCC built-in functions <https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html>`_

Zephyr
======

Dynamic Memory Allocation
-------------------------

Zephyr provides both ``libc`` :c:func:`malloc` and its own :c:func:`k_melloc`
functions for dynamic memory allocation. It replaces the common ``libc``
implementations with its own one as described in `Zephyr documentation
<https://docs.zephyrproject.org/3.6.0/develop/languages/c/index.html#dynamic-memory-management>`_.

However, :c:func:`malloc` does not share the same heap with :c:func:`k_melloc`
[#]_, namely enabling one does not necessarily enable the other.
:c:func:`malloc` can be enabled by setting the Kconfig option
`CONFIG_COMMON_LIBC_MALLOC_ARENA_SIZE
<https://docs.zephyrproject.org/3.6.0/kconfig.html#CONFIG_COMMON_LIBC_MALLOC_ARENA_SIZE>`_
to a non-zero value. By the same token, :c:func:`k_melloc` can be enabled by
setting the Kconfig option `CONFIG_HEAP_MEM_POOL_SIZE
<https://docs.zephyrproject.org/3.6.0/kconfig.html#CONFIG_HEAP_MEM_POOL_SIZE>`_
to a non-zero value.

Reference
---------

.. [#] `Zephyr malloc.c source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v3.6-branch/lib/libc/common/source/stdlib/malloc.c>`_
