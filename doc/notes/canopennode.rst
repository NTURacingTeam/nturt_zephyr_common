.. _notes_canopennode:

===========
CANopenNode
===========

The `CANopenNode track in Zephy
<https://github.com/zephyrproject-rtos/canopennode>`_ is currently outdated to
the latest version of `CANopenNode
<https://github.com/CANopenNode/CANopenNode>`_, and the `CANopenEditor
<https://github.com/CANopenNode/CANopenEditor>`_ does not support this legacy
version of CANopenNode (it has a legacy exporter, but the generated code lacks
some type definitions that it can't compile).

But CAN open is not that really open as a lot of the specifications are not
free. For now I need CiA 302.

However, since the current version of CANopenNode in Zephyr is still operational
and provides the necessary features, it's too good to not use it. Here are some
notes for using CANopenNode in Zephyr:

CAN Reception
=============

callbacks
---------

CAN bus driver in Zephyr uses hardware filters to filter out messages, and only
messages that pass the filter will be received by the application using callback
functions from an interrupt context [#]_. Since callbacks are called from ISR,
caution must be taken when setting callbacks related to CAN bus reception in
CANopenNode. For example, :c:func:`CO_EM_initCallbackRx` and
:c:func:`CO_NMT_initCallback` both execute in the ISR context, please
investigate the source code to see what context the callback is executed.

Filters
-------

Filters are added using :c:func:`CO_CANrxBufferInit` defined in ``CO_driver.c``.
The following is a list of filters added by CANopenNode:

- :c:func:`CO_EM_init` in ``CO_Emergency.c``
- :c:func:`CO_HBcons_monitoredNodeConfig` in ``CO_HBconsumer.c``, one for each
  monitored node
- :c:func:`CO_LSSmaster_init` in ``CO_LSSmaster.c`` *number not investigated*
- :c:func:`CO_LSSslave_init` in ``CO_LSSslave.c`` *number not investigated*
- :c:func:`CO_NMT_init` in ``CO_NMT_Heartbeat.c``
- :c:func:`CO_RPDO_init` in ``CO_PDO.c``, called once for each RPDO by
  :c:func:`CO_CANopenInitPDO` in ``CANopen.c``
- :c:func:`CO_SDO_init` in ``CO_SDO.c``
- :c:func:`CO_SDOclient_setup` in ``CO_SDOmaster.c``, one for each SDO client
- :c:func:`CO_SYNC_init` in ``CO_SYNC.c``
- :c:func:`CO_TIME_init` in ``CO_TIME.c``

.. note::

  Since STM32 only supports up to 28 standard ID filters, caution must be taken
  when configuring CANopenNode.

Service Data Object (SDO)
=========================

Each object dictionary (OD) entry can add additional functionalities by
registering a callback function using :c:func:`CO_OD_configure`. And,
CANopenNode already registered some common OD entries to provide functionalities
according to the CiA 301 standard. The following is a list of registered ODs:

- 0x1003: Pre-defined error field
- 0x1005: COB-ID SYNC message
- 0x1006: Communication cycle period
- 0x1010: Store parameters
- 0x1011: Restore default parameters
- 0x1014: COB-ID EMCY
- 0x1016: Consumer heartbeat time
- 0x1019: Synchronous counter overflow value
- 0x1200: SDO server parameter
- 0x1400 to 0x15FF: RPDO communication parameter
- 0x1600 to 0x17FF: RPDO mapping parameter
- 0x1800 to 0x19FF TPDO communication parameter
- 0x1A00 to 0x1BFF TPDO mapping parameter

Error Handling
==============

Error status bits
-----------------

CANopenNode uses an optional OD entry ``Error status bits`` of type
``OCTET_STRING`` and length more than 12 to store error status. You are
responsible for setting it in OD and register it to CANopenNode using
:c:func:`CO_EM_init`. The first 6 bytes (and hence the minimum length of 12 of
the octet string) is used internally by CANopenNode to store error status, and
the rest 26 bytes can be used for manaufacturer specific errors. The definitions
of the error status bits can be found in `CO_EM_errorStatusBits_t
<https://canopennode.github.io/CANopenSocket/group__CO__Emergency.html#ga587034df9d350c8e121c253f1d4eeacc>`_.

.. note::

  The length of ``Error status bits`` must grow coorespondingly to the number of
  manufacturer specific errors.

Error register
--------------

CANopenNode also helps mamnge generic, communication and manufacturer-specific
bits of ``Error register`` at OD 0x1001 [#]_. It sets communication bits when
internal communication error occurs, and manufacturer-specific bits when any of
the manaufacturer specific errors in ``Error status bits`` are set. However, it
only set generic bit when ``CO_EM_errorStatusBits_t`` between 0x28 to 0x2F are
set, which does **NOT** adhere to the standard stating that: "The generic error
shall be signaled at any error situation [#]_."

EMCY write
----------

In CANopen standard the EMCY write payload has the following format [#]_:

.. code-block:: none

    0        1          2         3                              7
  +------------+----------------+----------------------------------+
  | error code | error register | manufacturer-specific error code |
  +------------+----------------+----------------------------------+

CANopenNode uses the first byte of manaufacturer-specific error code to transmit
its ``Error status bits``, so the payload becomes:

.. code-block:: none

    0        1         2                  3           4                              7
  +------------+----------------+------------------------------------------------------+
  | error code | error register | error status bits | manufacturer-specific error code |
  +------------+----------------+------------------------------------------------------+

CANopenNode also recognizes the first byte of manaufacturer-specific error code
as ``Error status bits`` when receiving EMCY messages from other nodes. The
callback for receiving EMCY registered using :c:func:`CO_EM_initCallbackRx` has
the prototype:

.. code-block:: c

  void pFunctSignalRx(const uint16_t ident,
                      const uint16_t errorCode,
                      const uint8_t errorRegister,
                      const uint8_t errorBit,
                      const uint32_t infoCode);

Where ``errorBit`` is for ``Error status bits`` (and ``infoCode`` for the rest
of the manufacturer-specific error code).

Pre-defined error fields
------------------------

CANopenNode also helps to maintain ``Pre-defined error fields`` at OD 0x1003 for
recording errors that happened [#]_. Once an error is reported using
:c:func:`CO_errorReport`, it will be recorded to ``Pre-defined error fields`` in
the following format:

.. code-block:: none

  32     24               16           0
  +------+----------------+------------+
  | 0x00 | error register | error code |
  +------+----------------+------------+
  MSB                                LSB

where error code is one of the standard error codes defined in CiA 301.

EMCY reception
--------------

CANopenNode will receive all EMCY messages from the bus [#]_ and call the
callback registered using :c:func:`CO_EM_initCallbackRx`. It does not provide
support for ``Emergency consumer object`` at OD 0x1014.

Reference
=========

.. [#] `Zephyr CAN bus driver documentation
  <https://docs.zephyrproject.org/3.6.0/hardware/peripherals/can/controller.html#receiving>`_
  on receiving messages
.. [#] `CANopenNode CO_EM_process() source code (1)
  <https://github.com/zephyrproject-rtos/canopennode/blob/zephyr/stack/CO_Emergency.c#L251>`_ 
  that manages error register
.. [#] CiA 301, section 7.5.2.2 Error register
.. [#] CiA 301, section 7.2.7.3.1 Protocol EMCY write
.. [#] `CANopenNode CO_EM_process() source code (2)
  <https://github.com/zephyrproject-rtos/canopennode/blob/zephyr/stack/CO_Emergency.c#L310>`_
  that mantains pre-defined error fields
.. [#] `CANopenNode CO_EM_init() source code
  <https://github.com/zephyrproject-rtos/canopennode/blob/zephyr/stack/CO_Emergency.c#L179>`_
  that receives all EMCY messages
