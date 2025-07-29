.. _library_telemetry:

================
Telemetry System
================

Overview
========

The telemetry system provides a type-safe interface for defining, updating,
and aggregating telemetry data to be consumed by various backends. It can be
added to any application by enabling the ``CONFIG_NTURT_TM`` Kconfig option.

Core concepts
-------------

- **Data Address**: Since most of the embedded protocols uses an address to
  identify data such as I2C, Modbus, or CAN Open, the telemetry system uses the
  same concept. Each piece of data is defined by :c:macro:`TM_DATA_DEFINE` using
  its name, type, and address.

- **Data Aliases**: To support multiple protocols with different addresses for
  the same data, data aliases can be defined using :c:macro:`TM_ALIAS_DEFINE`.
  This allows the same data to be accessed using different addresses and when
  either the data or any of its aliases is updated, all aliases as well as the
  original data are updated.

- **Publishing Groups**: To support the situation where multiple pieces of data
  need to be published together such as different sensor data within a single
  CAN frame, data and aliases can be grouped via :c:macro:`TM_GROUP_DEFINE` to
  be aggregated into a single frame.

- **Data Retrieval**: Aside from actively publishing data to a backend, data
  also can be queried by passive protocols such as I2C or Modbus using
  :c:func:`tm_data_get`.

All data operations are reentrant and thread-safe.

Usage
=====

Defining Telemetry Data
-----------------------

Use :c:macro:`TM_DATA_DEFINE` to define one piece of data. If that data would be
accessed across multiple files, use :c:macro:`TM_DATA_DECLARE` to declare it in
a header file. 

To define a data named ``foo`` of type ``int`` with address ``0x01`` and ``bar``
of type ``float`` with address ``0x02``:

.. code-block:: c

   // in a header file
   TM_DATA_DECLARE(foo, int);
   TM_DATA_DECLARE(bar, float);

   // in a source file
   TM_DATA_DEFINE(foo, int, 0x01);
   TM_DATA_DEFINE(bar, float, 0x02);

Data aliases are defined and declared in similar way using
:c:macro:`TM_ALIAS_DEFINE` and :c:macro:`TM_ALIAS_DECLARE`. To define an alias
for the data ``bar`` with the name ``baz`` and alias address ``0x100``:

.. code-block:: c

   // in a header file
   TM_ALIAS_DECLARE(baz, bar);

   // in a source file
   TM_ALIAS_DEFINE(baz, bar, 0x100);

Updating and Retrieving Data
----------------------------

Data and aliases can be accessed either by their names via
:c:macro:`TM_DATA_GET`, :c:macro:`TM_DATA_UPDATE` macros, or by their addresses
via :c:func:`tm_data_get`, :c:func:`tm_data_update` functions.

Since the type of the data is known at compile time, accessing by their names is
internally done via assignment operator ``=``. For example, to update the data
``foo`` that is an integer with a float value or retrieve it as a boolean:

.. code-block:: c

   TM_DATA_UPDATE(foo, 3.14F);

   bool is_non_zero = TM_DATA_GET(foo);

However, since accessing data by their addresses is internally done via
:c:func:`memcpy`, the argument must be of the same type as the data. For
example, to update the data ``foo`` of address ``0x01`` with a float value:

.. code-block:: c

   float new_foo = 3.14F;
   int new_foo_int = new_foo;
   tm_data_update(0x01, &new_foo_int);

Grouping Data
-------------

Telemetry system uses :ref:`library_aggregation` to aggregate data into groups
for publishing. For example:

.. code-block:: c

   TM_GROUP_DEFINE(my_group,
       period, min_separation, watermark, flag, publish, user_data
       TM_DATA(foo),
       TM_DATA(baz, AGG_MEMBER_FLAG_OPTIONAL)
   );

defines a group named ``my_group`` that aggregates the data ``foo`` and
``baz``. Where ``period``, ``min_separation``, ``watermark``, and ``flag`` are
parameters for aggregation.

After the data is aggregated, ``publish`` of type :c:type:`tm_publish_t` is
called for every data in the group.

API Reference
=============

.. doxygengroup:: tm
