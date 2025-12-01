.. _develop_cli:

======================
Command Line Interface
======================

The firmware exposes a `Zephyr shell
<https://docs.zephyrproject.org/4.2.0/services/shell/index.html>`_
for interactive inspection and control. This page lists the custom commands
implemented in this repository and the additional commands provided by
``nturt_vcu`` application. Please refer to the `Zephyr documentation
<https://docs.zephyrproject.org/4.2.0/index.html>`_ for other commands provided
by Zephyr.

.. note::
   All custom commands support `tab completion feature
   <https://docs.zephyrproject.org/4.2.0/services/shell/index.html#tab-feature>`_
   and `command help
   <https://docs.zephyrproject.org/4.2.0/services/shell/index.html#command-help>`_
   for ease of use.

Available Commands
==================

canopen
-------

CANopen diagnostics.

Subcommands
~~~~~~~~~~~

- ``error``

  - ``get``: Print currently set CANopen errors.

err
---

Error handling commands provided by :ref:`library_errors`.

Subcommands
~~~~~~~~~~~

- ``list``: Print all registered errors with severity tagging.
- ``get``: Show currently set errors.
- ``set``: Set an error by name.
- ``clear``: Clear a set error by name.

msg
---

Message passing diagnostics.

Subcommands
~~~~~~~~~~~

- ``stats``: Display statistics for each channel.
- ``dump``: Dump every message published by a given channel to console.

sensor_axis
-----------

Sensor axis input diagnostics and calibration utilities.

Subcommands
~~~~~~~~~~~

- ``channel``

  - ``list``: List the underlying sensors that form a channel.
  - ``calib_set``: Calibrate all sensors in the channel using the current
    readings and save them to settings.

- ``sensor``

  - ``calib_get``: Read the stored calibration points for a specific sensor.
  - ``calib_set``: Calibrate a single sensor using its current reading abd  save
    it to settings.
  - ``dump_raw``: Dump raw sensor output to console.

nturt_vcu Commands
==================

The following commands are provided by ``nturt_vcu``.

ctrl
----

Control system management.

Subcommands
~~~~~~~~~~~

- ``param``

  - ``list``: Print all control parameters with current values and types.
  - ``get``: Gat the value of a parameter.
  - ``set``: Update a parameter and save it to settings. Values must match the
    parameter type.

- ``inv``

  - ``fault_reset``: Clear inverter fault.

dashboard
---------

Dashboard and display controls.

Subcommands
~~~~~~~~~~~

- ``brightness``

  - ``get``: Get the current dashboard brightness level.
  - ``set``: Set dashboard brightness level and save it to settings.

- ``mode``

  - ``get`` Display the active dashboard mode.
  - ``set`` Set the dashboard display mode.

states
------

Control state machine commands provided by :ref:`library_vcu_states`.

Subcommands
~~~~~~~~~~~

- ``get`` Print the current state bitmask along with human-readable names.
- ``trans`` Request a state transition.
