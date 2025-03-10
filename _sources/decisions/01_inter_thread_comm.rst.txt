.. _decisions_inter_thread_comm:

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

Status
------

* **PROPOSED** on Feb. 25, 2025

Details
=======

Assumption
----------

Constraints
-----------

* Should be able to meet the requirements of different modules
* Should be easy to use with little boilerplate code required

Positions
---------

Readily available Publisher/Subscriber service in Zephyr is *ZBus*;

* **Pros**:

  * Multi-producer, multi-consumer message passing
  * Various subscription processing type: callbacks, notifications, or queues
  * One subscription can subscribe to multiple channels
  * Dynamic control of whole subscription on/off or individual channel reception
    on/off

* **Cons**:

  * Callbacks are sychronous and can't control the context it runs in
  * Notifications and queues can only be waited one at a time with no current
    waiting API such as k_poll or RTIO
  * No getter methods using publisher/subscriber alone

A widely used method in Zephyr is *Callback-Based Communication*;

* **Pros**:

  * Tailored to different requirements of each module, for example;

    * State machine calls state transition callbacks and waits until all
      callbacks are finished before finishing the transition
    * Delivery of sensor data does not have such requirement

* **Cons**:

  * Different APIs and semantics for each module, increasing the complexity of
    the system

Arguments
---------

For areas where publisher/subscriber model is not suitable, particularly where
actions requires sychronous processing such as;

* Commands that have to respond immediately
* State machines that can only transition after all transition callbacks are
  finished

A callback-based communication model is more suitable, and there is no need to
use ZBus for other parts of that area that could be implemented using
publisher/subscriber model.

Implications
------------

Notes
=====
