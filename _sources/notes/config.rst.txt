.. _notes_config:

===========================
Zephyr Configuration System
===========================

Device Tree
===========

Using device tree to describe hardware configurations across different platforms
is a great design in my opinion, especially for microcontrollers where there are
lots of vendors with their unique features for similar peripherals. However,
just like Zephyr's disadvantages, the documentation and examples are few,
especially for vendor specific configurations. Here are some tips for using
device tree in Zephyr:

Device tree includes
--------------------

Decive tree supports including other device tree files just like how C does, but
without a language service provider (LSP) like IntelliSense for C/C++ or Pylance
for Python, it is hard to find where the included files are. The following are
some common places where Zephyr may put these files:

- Device tree header files (.dtsi) for a specific architecture in ``zephyr/dts``
- C header files (.h) for constants and macros in
  ``zephyr/include/zephyr/dt-bindings``
- Board support package (BSP) files in ``zephyr/boards``
- Vendor specific device tree files (especially for ping control) in
  ``modules/hal/<vendor>/dts``

Device tree macros
------------------

Since device tree in Zephyr is processed into C macros for the compiler to
further process, it supports including C hearder files as well as using macros
defined in it.

For example, you may see ``clock-frequency = <DT_FREQ_K(48)>``, in clock
configs where ``DT_FREQ_K`` is a macro defined as ``DT_FREQ_M(x) ((x) * 1000)``
in ``zephyr/dts/common/freq.h``.

STM32 device tree
-----------------

For STM32, it is a good idea to use STM32CubeMX as a reference when configuring
the device tree as most of the time is just copying the configurations from
STM32CubeMX to the device tree.

Kconfig
=======

Kconfig Macros
--------------

Kconfig also `supports macros
<https://www.kernel.org/doc/html/latest/kbuild/kconfig-macro-language.html>`_,
which is mainly used in Zephyr for accessing the device tree. It is useful for
example to enable a feature only when a specific property is defined in the
device tree. The supported macros can be referenced from `Custom Kconfig
Preprocessor Functions
<https://docs.zephyrproject.org/4.0.0/build/kconfig/preprocessor-functions.html>`_.

Porting New Boards to Zephyr
============================

A board that is not supported by Zephyr can be added to it by following the
official `Board Porting Guid
<https://docs.zephyrproject.org/4.0.0/hardware/porting/board_porting.html>`_

In Zephyr v3.7.0, a new hardware model is introduced for defining the
configuration of boards. Though a new qualification system for determining the
SoC, CPU cluster, and variant of a target is introduced. It however does not
support using different SoCs across different versions of the same board (or at
least I don't think it is designed to do so). For example, the
``Kconfig.<board>`` file for selecting the SoC is not versioned [#]_, and the
``<board>.dts`` file that selects the SoC of the board file only allows
``<board>_<revision>.overlay`` files to be merged into the base device tree
file, but no way to replace the it [#]_. Hence, though the versioning system is
still useful for the same set of SoCs, it is required to create a new board when
a different SoC is used.

A wierd thing is that the YAML file extensions for board metadata file
``board.yml`` and twister metadata file ``<board>.yaml`` are different [#]_. The
board may not be indentified if the extensions are not correct.

References
----------

.. [#] `Board Porting Guid: Write Kconfig files
   <https://docs.zephyrproject.org/4.0.0/hardware/porting/board_porting.html#write-kconfig-files>`_
.. [#] `Board Porting Guid: Board revision configuration adjustment
   <https://docs.zephyrproject.org/4.0.0/hardware/porting/board_porting.html#board-revision-configuration-adjustment>`_
.. [#] `Board Porting Guid: Create your board directory
   <https://docs.zephyrproject.org/4.0.0/hardware/porting/board_porting.html#create-your-board-directory>`_
