.. _architecture_vehicle_control_unit:

====================
Vehicle Control Unit
====================

Architecture
============

The Vehicle Control Unit (VCU) is architected in a layered manner to ensure
modularity and maintainability. Most of the drivers and the middlewares are
provided by Zephyr, with only parts in blue being implemented by us.

.. figure:: /_static/images/architecture.svg
   :width: 100%
   :align: center

   The architecture of the VCU.

Data Flow
=========

The system's data mainly comes from the sensors and CAN Open, which is
aggregated first into appropriate `messages types
<https://nturacingteam.github.io/nturt_zephyr_common/doxygen/group__msg__interface.html>`_
using :ref:`library_aggregation` and then published to Zephyr `Zbus
<https://docs.zephyrproject.org/4.2.0/services/zbus/index.html>`_. CAN Open may
also send commands to change the state of the system, the parameters of the
control algorithms, etc., which is handled by :ref:`library_command` and then
dispatched to appropriate modules via function calls. Finally, in order to
transmit telemetry data, :ref:`library_telemetry` is used as the broker to
receive messages from Zbus for various backend protocols, including data
logginger, to publish.

.. figure:: /_static/images/data_flow_diagram.svg
   :width: 75%
   :align: center

   The data flow diagram of the VCU.

State Transition Diagram
========================

:ref:`library_vcu_states` uses Zephyr `state machine framework
<https://docs.zephyrproject.org/4.2.0/services/smf/index.html>`_, which is based
on `Hierarchical state machine
<https://en.wikipedia.org/wiki/UML_state_machine#Hierarchically_nested_states>`_,
to manage the complex state transitions of VCU's control system. Here the black
filled circles represent the initial state and the initial state transition of
the state machine, meaning which substate to enter when transitioning to the
parent state. So the initial state of the VCU is ``RTD_BLINK``.

.. figure:: /_static/images/state_transition_diagram.svg
   :width: 90%
   :align: center

   `UML state diagram
   <https://sparxsystems.com/resources/tutorials/uml2/state-diagram.html>`_ of
   the VCU.
