.. _architecture_survey:

======
Survey
======

Motor Controllers
=================

SimpleFOC
---------

Very simple while loop where sensor data is acquired before motor control.

`SimpleFOC code <https://docs.simplefoc.com/code>`_

ST Motor Workbench
------------------

Interrupt-based loops with sensor data acquired in the control loop.

The hardware is optimized for motor control, so the software can be simple.

`ST motor control course: FOC library about interrupt
<https://www.youtube.com/watch?v=ctN6wYqE970&list=PLnMKNibPkDnFxzg5RExF_MNOxX6wfT95M&index=10>`_

Flight Controllers
==================

ArduPilot
---------

.. image:: https://ardupilot.org/dev/_images/copter-architecture.png
   :alt: ArduPilot Architecture
   :align: center
   :width: 100%

`ArduPilot code overview <https://ardupilot.org/dev/docs/apmcopter-code-overview.html>`_

PX4
---

.. image:: https://docs.px4.io/main/assets/PX4_Architecture.BOZwmjrc.svg
   :alt: PX4 Architecture
   :align: center
   :width: 100%

`Software architecture <https://docs.px4.io/main/en/concept/architecture.html>`_

3D Printers
===========

Marlin
------

`Code structure <https://marlinfw.org/docs/development/code_structure.html>`_

Kilpper
-------

`Kilpper code overview <https://www.klipper3d.org/Code_Overview.html>`_
