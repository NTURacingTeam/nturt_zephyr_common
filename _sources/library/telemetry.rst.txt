.. _library_telemetry:

================
Telemetry System
================

Overview
========

The telemetry system acts as the interface between temeletry data producers and
data consumers. It consists of the following core components:

- **Data**: Data is a collection of address-value pairs, where each of them
  represnets one piece of data that is being monitored such as battery voltage
  of segment 1 or IMU acceleration in the x-axis. The address is an integer that
  uniquely identifies the data which does not need to be contiguously defined.
- **Group**: A group is a collection of data that are logically related to each
  other. For example, a group can be a collection of data that are related to
  the IMU sensor. And only after all data in a group is updated by the producer
  side will the data be updated on the consumer side or published by the backend
  if the group is associated with one. Each group also has a unique ID to
  identify it which also does not need to be contiguously defined.
- **Backend**: A backend is used to transmit data to the consumers such as
  through UART to the host or to another thread when a group associated with it
  is updated. Backends must be defined by the user to implement the actual
  transmission of data.

Usage
=====

Design
======

Architecture
------------

API Reference
=============

.. doxygengroup:: tm
