.. _architecture_survey:

======
Survey
======

Here are some projects that can be referenced for designing an embedded control
system.

Motor Controllers
=================

SimpleFOC
---------

Simple framework based on Arduino that emphasize on simplicity. Motor is
controled in a while loop that;

1. First acquires sensor data such as phase current from ADC or position/speed
   sensor.
2. Then caculates the PWM duty cycles.
3. Finally, applies the PWM duty cycles to each phase.

References
~~~~~~~~~~

* `SimpleFOC code <https://docs.simplefoc.com/code>`_

ST Motor Workbench
------------------

Hardware timer-driven system where time-critical tasks are executed in ISRs and
utilizes Cortex-M's NVIC to have preemptible interrupts with priorities. Since
the hardware is optimized for motor control (specialized timer for three phase
PWM), so the software can be simple.

References
~~~~~~~~~~

* `ST motor control course: FOC library about interrupt
  <https://www.youtube.com/watch?v=ctN6wYqE970&list=PLnMKNibPkDnFxzg5RExF_MNOxX6wfT95M&index=10>`_

Spinner
-------

Motor controller based on Zephyr as a proof of concept project.

For STM32 implementation (currently the only one available), it uses Zephyr's
`Direct ISR
<https://docs.zephyrproject.org/latest/kernel/services/interrupts.html#defining-a-direct-isr>`_
for;

* Phase current sampling using ADC [#]_.
* Hall effect encoder using timer [#]_.

PWM duty cycle is calcuated and set in the current sampling loop [#]_.

References
~~~~~~~~~~

* `Spinner Documentation <https://teslabs.github.io/spinner/index.html>`_
* `Spinner GitHub <https://github.com/teslabs/spinner>`_

.. [#] `Spinner current sampling source code
   <https://github.com/teslabs/spinner/blob/main/drivers/currsmp/currsmp_shunt_stm32.c#L48>`_
.. [#] `Spinner hall effect sensor source code
   <https://github.com/teslabs/spinner/blob/main/drivers/feedback/halls_stm32.c#L54>`_
.. [#] `currsmp_configure() API reference
   <https://teslabs.github.io/spinner/components/currsmp/index.html#c.currsmp_configure>`_
   that sets current regulation callback.

Vechicle Controllers
====================

CogniPilot - Cerebri
--------------------

Vehicle management unit based on Zephyr for autonomous vehicles.

It is publisher / subscriber model similar to ROS.

References
~~~~~~~~~~

* `GitHub: CogniPilot/cerebri <https://github.com/CogniPilot/cerebri>`_

Flight Controllers
==================

ArduPilot
---------

.. image:: https://ardupilot.org/dev/_images/copter-architecture.png
   :alt: ArduPilot Architecture
   :align: center
   :width: 100%

References
~~~~~~~~~~

* `ArduPilot code overview <https://ardupilot.org/dev/docs/apmcopter-code-overview.html>`_

PX4
---

.. image:: https://docs.px4.io/main/assets/PX4_Architecture.sDy5Z0TR.svg
   :alt: PX4 Architecture
   :align: center
   :width: 100%

References
~~~~~~~~~~

* `Software architecture <https://docs.px4.io/main/en/concept/architecture.html>`_

3D Printers
===========

Marlin
------

`Code structure <https://marlinfw.org/docs/development/code_structure.html>`_

Kilpper
-------

`Kilpper code overview <https://www.klipper3d.org/Code_Overview.html>`_

Other Zephyr projects
=====================

List of Zephyr applications and frameworks: `awesome Zephyr
<https://github.com/golioth/awesome-zephyr-rtos>`_

Some popular Zephyr projects:

* `ZMK Firmware <https://zmk.dev/>`_
* `ZSWatch <https://github.com/jakkra/ZSWatch>`_
