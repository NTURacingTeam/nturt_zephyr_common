.. _library_aggregation:

================
Data Aggregation
================

Overview
========

The data aggregation module provides a flexible mechanism for monitoring,
collecting, and publishing updates to a collection of data or struct members.
It is designed to track changes of individual data items, aggregate updates, and
trigger publishing actions based on configurable timing and update policies.
This is particularly useful for aggregating data scattered from various sources,
such as sensor data from ADC and CAN bus, which must be collected and sent at
regular intervals or upon change. This module can be added to any application by
enabling the ``CONFIG_NTURT_MSG`` Kconfig option.

Core concepts
-------------

Timing Parameters
~~~~~~~~~~~~~~~~~

There are three timing parameters that control the behavior of the aggregation:

- **Period**: The typical interval at which the aggregated data is published.
- **Minimum Separation**: The minimum time between two consecutive publications.
- **Watermark**: The additional time to wait for late-arriving data.

.. figure:: ../_static/images/aggregation_timing.svg
   :width: 100%
   :align: center

   Timing parameters for data aggregation.

Basically, only after every data item in an aggregation are updated will the
aggregated data be published. However, if after **Period** time has elapsed
since the last publication and some data items are still not updated,
additional **Watermark** time will be waited for late-arriving data. If after
that time some data items are still not updated, the aggregated data will be
published anyway, using the last known values of those data items.

If the updates of specific data items are not necessary,
:c:macro:`AGG_MEMBER_FLAG_IGNORED` or :c:macro:`AGG_MEMBER_FLAG_OPTIONAL` flags
can be set in :c:macro:`AGG_MEMBER` to mark a member as ignored or optional.

Since the data aggregation module is designed to be used for processing
real-time data, aggregated data is published immediately after all data items
are updated. And **Minimum Separation** is used to ensure that the publication
does not happen too frequently. Hence the aggregated data can only be published
in the **Publish Interval**.

Dormant and Cold Start
~~~~~~~~~~~~~~~~~~~~~~

If after **Period** plus **Watermark** time has elapsed and no data items were
updated, the aggregation will be **dormant**, meaning it will stop publishing
until any one of its data items is updated again. This is useful to stop
unnecessary publication when the source modules are not active or the data is
not changing.

After the aggregation is **cold started** by an update of a data item (including
data marked by :c:macro:`AGG_MEMBER_FLAG_OPTIONAL`), it will only wait
**Watermark** time for other data items to be updated before publishing.

Dormant can be turned off by setting :c:macro:`AGG_FLAG_ALWAYS_PUBLISH` flag in
:c:macro:`AGG_DEFINE` or :c:macro:`AGG_TYPED_DEFINE`, which will force the
aggregation to publish every **Period** plus **Watermark** even if no data items
are updated.

.. note::

   Currently, the aggregation module does not start automatically even if
   :c:macro:`AGG_FLAG_ALWAYS_PUBLISH` is set. It will only start when
   the first data item is updated. This may be changed in the future to
   automatically start the aggregation after initialization.

Data Update
~~~~~~~~~~~

If one data item is updated multiple times before next publication, only the
latest value will be published. This is to ensure that the aggregated data
reflects the most recent state of the system. If a data item is not updated
before the next publication, its last known value will be used.

Usage
=====

Defining an Aggregation
-----------------------

The aggregation module can be used in two main ways:

- For external or unrelated data items, use the :c:struct:`agg` and define it
  using :c:macro:`AGG_DEFINE` or initialize it with :c:macro:`AGG_INITIALIZER`
  within a struct. This allows you to aggregate updates to data items by their
  index, suitable for cases where the data items are not part of a single
  struct.

- For struct members, use :c:macro:`AGG_TYPED_DEFINE` to define a typed
  aggregation. This macro sets up an aggregation for a specific struct type,
  allowing you to monitor and update individual members. This is useful for
  aggregating updates to fields within a message in `message types
  <https://nturacingteam.github.io/nturt_zephyr_common/doxygen/group__msg__interface.html>`_
  or data structure.

Suppose we have a struct representing a message:

.. code-block:: c

   struct my_msg {
       int foo;
       struct {
           float x;
           float y;
       } bar;
   };

An aggregation can be defined to monitor updates to these members:

.. code-block:: c

   AGG_TYPED_DEFINE(my_msg_agg, struct my_msg,
       AGG_DATA_INIT({0, {0.0f, 0.0f}}),              // initial value
       K_MSEC(100),                                   // period
       K_MSEC(10),                                    // minimum separation
       K_MSEC(20),                                    // watermark
       0,                                             // aggregation flags
       my_publish_func,                               // publish callback
       NULL,                                          // user data for the callback
       AGG_MEMBER(foo),                               // members to monitor
       AGG_MEMBER(bar.x, AGG_MEMBER_FLAG_OPTIONAL)
   );

.. note::

   Not all members need to be monitored. But if a member is not monitored, only
   the initial value will be used when the aggregation is published.

Updating Members
----------------

To signal that a member has been updated, use :c:func:`agg_update` for
:c:struct:`agg` or :c:macro:`AGG_TYPED_UPDATE` for typed aggregations:

.. code-block:: c

   AGG_TYPED_UPDATE(&my_msg_agg, struct my_msg, foo, 42);
   AGG_TYPED_UPDATE(&my_msg_agg, struct my_msg, bar.x, 3.14F);

.. warning::

   Only members declared in the :c:macro:`AGG_TYPED_DEFINE` can be updated. If
   a unknown member is updated, an assertion will fail at runtime.

Publish Function
----------------

The aggregation module publishes the aggregated data by calling a user-defined
publish function of type :c:type:`agg_publish_t` for :c:struct:`agg` or
:c:type:`agg_typed_publish_t` for typed aggregations:

.. code-block:: c

   void my_publish(const void *data, void *user_data) {
       const struct my_msg *agg = data;

       // publish the aggregated data
   }

API Reference
=============

.. doxygengroup:: msg_agg
