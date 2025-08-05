====================================
NTU Racing Team Zephyr Documentation
====================================

**Welcome to the NTU Racing Team Zephyr documentation.**

Project Overview
================

In NTU Racing Team we build embedded controllers for racing vehicles, including
inverters to drive the motors, battery management systems to monitor and control
the batteries, and vehicle control unit to manage the overall vehicle operations
and run the control algorithms.

This project contains the common codebase and utilities for the Zephyr
RTOS-based embedded controllers used by the NTU Racing Team. It is designed to
be modular and reusable across different systems, providing a foundation for
developing high-performance, real-time applications in the context of
motorsport. Though it is currently primarily used for the vehicle control unit,
it should also be extended to support other controllers of the race car.

.. toctree::
   :caption: Architecture
   :maxdepth: 1
   :hidden:

   architecture/index
   architecture/decisions/index

.. toctree::
   :caption: Libraries
   :maxdepth: 1
   :hidden:

   library/index
   library/vcu/index

.. toctree::
   :caption: Developing
   :maxdepth: 1
   :hidden:

   develop/getting_started
   develop/doxygen
   develop/test
   develop/notes/index
   develop/doc

Indices and Tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
