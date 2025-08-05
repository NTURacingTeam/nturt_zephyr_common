.. _develop_getting_started:

===============
Getting Started
===============

Setting up Zephyr
=================

The Zephyr environment used to develop this project is based on the `Zephyr
Workspace <https://github.com/NTURacingTeam/zephyr_workspace>`_ also developed
by us. Please follow the instructions there to set up the Zephyr
environment.

Including this Project
----------------------

To include this project in your Zephyr application, you can add the following
to your ``west.yml``:

.. code-block:: yaml

   manifest:
     remotes:
       - name: nturt
         url-base: https://github.com/NTURacingTeam

     projects:
       - name: nturt_zephyr_common
         remote: nturt
         revision: master

After another ``west update`` that pull this project into your Zephyr workspace,
you can include it in your application by enabling the ``CONFIG_NTURT`` Kconfig
option.

Learning Resources
==================

Zephyr
------

Zephyr itself is a very involved system that has a deep learning curve.
Fortunately, there are plenty of resources available to help you get started:

- `Introduction to Zephyr
  <https://www.youtube.com/playlist?list=PLEBQazB0HUyTmK2zdwhaf8bLwuEaDH-52>`_
  from the DigiKey YouTube channel is a great place to start.
- `Zephyr 101
  <https://www.youtube.com/playlist?list=PLJKv3qSDEE-lYuq5hMpJ_sSHQcuhO1S-P>`_
  from Circuit Dojo YouTube channel is another good resource for step-by-step
  tutorials on specific topics.

Real-Time Operating Systems (RTOS)
----------------------------------

An operating system is almost imperative for an embedded system to scale up.
Real-Time operating systems (RTOS) are designed to handle real-time tasks
typically found in embedded systems. Here are some resources to help you
understand RTOS concepts:

- `Introduction to RTOS
  <https://www.youtube.com/playlist?list=PLEBQazB0HUyQ4hAPU1cJED6t3DU0h34bz>`_
  from the DigiKey YouTube channel provides a good overview of RTOS concepts
  based on another open-source RTOS, `FreeRTOS <https://www.freertos.org/>`_.
