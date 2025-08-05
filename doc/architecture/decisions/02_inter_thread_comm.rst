.. _architecture_decisions_inter_thread_comm:

=========================
Inter-Thread Communcation
=========================

Summary
=======

Issue
-----

Inter-thread communication is required to scale the system to multiple modules
running in different threads. And a well designed communication model can
greately reduce the dependency between each modules.

Decision
--------

Generally, Zephyr's `zbus
<https://docs.zephyrproject.org/4.2.0/services/zbus/index.html>`_ should be used
whenever possible, but for some areas where synchronous processing is
required, a callback-based communication model is more suitable.

Status
------

- **PROPOSED** on Feb. 25, 2025

Details
=======

Assumption
----------

The data is passed in multi-producer, multi-consumer model, and simple use case
only involving one producer and one consumer such as data aggregation is not in
the scope of this decision.

Constraints
-----------

- Should be able to meet the requirements of different modules, especially the
  real-time requirements of the control algorithms.
- Should be easy to use with little boilerplate code required.

Positions
---------

A readily available Publisher/Subscriber service in Zephyr is `zbus
<https://docs.zephyrproject.org/4.2.0/services/zbus/index.html>`_;

- **Pros**:

  - Multi-producer, multi-consumer message passing.
  - Various subscription processing type: callbacks, notifications, or queues.
  - One subscription can subscribe to multiple channels.
  - Dynamic control of whole subscription on/off or individual channel reception
    on/off.

- **Cons**:

  - Callbacks are sychronous and runs in the same context as the publisher.

Another widely used method in Zephyr is `Callback-Based Communication`, such as
`Input <https://docs.zephyrproject.org/v4.2.0/services/input/index.html>`_;

- **Pros**:

  - Can be tailored to meet different requirements of different modules, for
    example;

    - State machine needs to wait until all state transition callbacks are
      finished before finishing the transition.
    - Delivery of sensor data does not have such requirement.

- **Cons**:

  - Different APIs and semantics for each module, increasing the complexity of
    the system.

Arguments
---------

For areas where publisher/subscriber model is not suitable, particularly where
actions requires sychronous processing such as;

- Commands that have to respond immediately.
- State machines that can only transition after all transition callbacks are
  finished.

A callback-based communication model is more suitable, and there is no need to
use zbus in order to implement other parts of the system that can be implemented
using publisher/subscriber model.

Implications
------------

Notes
=====
