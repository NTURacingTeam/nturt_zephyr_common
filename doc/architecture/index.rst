.. _archecture:

============
Architecture
============

Hardware Overview
=================

This is the architecture of the electrical system of the vehicle.

.. figure:: /_static/images/electrical_system.svg
   :width: 100%
   :align: center

   The electrical system of the vehicle.

Within the vehicle, there are currently 6 independent processors that control
different subsystems of the vehicle, including:

- Vehicle Control Unit (VCU) using STM32 MCU
- Embedded computer for wireless communication and data logging using Raspberry
  Pi
- Motor Controllers using STM32 MCU
- Battery Management System (BMS) for both HV and LV batteries using STM32 MCU

Currently, only vehicle control unit is using this software framework, but it
should also be extended to support other microcontrollers of the race car.

.. toctree::
   :maxdepth: 1

   principles
   vehicle_control_unit
   survey
   sensing
