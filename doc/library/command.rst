.. _library_command:

==============
Command System
==============

Overview
========

The command system provides an standard interface for a module to provide
commands for other modules or communication protocols to control it. A command
consists of the following parts:

- **Opcode**: An integer that uniquely identifies the command.
- **Immediate Handler**: An optional function that is called when the command is
  invoked, which is used to execute the command in the same context of the
  invocation or to validate the operand if the command has a deffered handler.
- **Deferred Handler**: An optional function that is queued to be executed in a
  dedicated thread when the command requires a longer execution time or cannot
  be executed in ISRs.

Usage
=====

Design
======

Architecture
------------

API Reference
=============

.. doxygengroup:: cmd
