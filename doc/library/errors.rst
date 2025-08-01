.. _library_errors:

==============
Error Handling
==============

Overview
========

The error handling system provides a centralized way to define, report, and
handle errors. It can be added to any application by enabling the
``CONFIG_NTURT_ERR`` Kconfig option.

Core concepts
-------------

- **Severity Levels**: Errors are categorized into different severity levels in
  :c:enum:`err_sev`.

- **Error Codes**: Each error is identified by a unique error code defined using
  :c:macro:`ERR_DEFINE`, which does not need to be contiguous.

- **Callbacks**: Modules can register callbacks via
  :c:macro:`ERR_CALLBACK_DEFINE` to be called when an error is reported (set or
  cleared). The callbacks may be optionally registered with filters to specify
  which errors they should handle using :c:macro:`ERR_FILTER_CODE` or
  :c:macro:`ERR_FILTER_SEV`.

Runtime Behavior
----------------

- **Initialization**: During system initialization, reported errors are deferred
  until ``CONFIG_NTURT_ERR_CB_DISPATCH_PRIORITY`` is reached. This allows error
  callbacks from modules not yet initialized to be processed later.

- **Order of Callback Dispatch**: At ``CONFIG_NTURT_ERR_CB_DISPATCH_PRIORITY``
  initialization priority, errors defined with :c:macro:`ERR_FLAG_SET` are first
  processed, followed by the same order errors were reported.

  .. warning::

     Since the error callbacks are deferred, during callback dispatch, handlers
     may observe `future` errors that were reported after the current error was
     set.

- **Execution Context**: Error callbacks are executed in the same context of the
  thread that reported the error, and they are reentrant and thread-safe.

Usage
=====

Defining Errors
---------------

Use :c:macro:`ERR_DEFINE` to define one error. For example:

.. code-block:: c

    ERR_DEFINE(my_error, 0x01, ERR_SEV_FATAL, "My error occurred");

Errors may also be defined with additional flags to control their behavior,
those include:

- :c:macro:`ERR_FLAG_SET`: Indicates that the error is set after initialization.
- :c:macro:`ERR_FLAG_DISABLED`: Indicates that the error is disabled and setting
  or clearing the error will not have any effect.

Reporting Errors
----------------

Use :c:func:`err_report` to report an error using its code:

.. code-block:: c

    err_report(0x01, true);  // set the error
    err_report(0x01, false); // clear the error

Handling Errors
---------------

Use the :c:macro:`ERR_CALLBACK_DEFINE` to define a callback to handle errors in
event-driven fasion. For example:

.. code-block:: c

    static void my_error_handler(uint32_t errcode, bool set, void* user_data) {
        // handle the error
    }

    ERR_CALLBACK_DEFINE(my_error_handler, NULL);

You can also filter which errors the callback should handle using
:c:macro:`ERR_FILTER_CODE` or :c:macro:`ERR_FILTER_SEV`:

.. code-block:: c

    ERR_CALLBACK_DEFINE(my_error_handler, NULL, ERR_FILTER_CODE(0x01));

    // or

    ERR_CALLBACK_DEFINE(my_error_handler, NULL, ERR_FILTER_SEV(ERR_SEV_FATAL));

When two filters are specified, they are combined using logical AND, meaning the
callback will only be invoked if both conditions are met.

Errors may also be polled using :c:func:`err_is_set` or
:c:macro:`ERR_FOREACH_SET` to iterate over all currently set errors.

API Reference
=============

.. doxygengroup:: err
