.. _notes_canopennode:

===========
CANopenNode
===========

`Zephyr's branch on CANopenNode
<https://github.com/zephyrproject-rtos/canopennode>`_ is currently outdated
comapring to the latest version of `CANopenNode
<https://github.com/CANopenNode/CANopenNode>`_, and the `CANopenEditor
<https://github.com/CANopenNode/CANopenEditor>`_ does not support this legacy
version of CANopenNode (it has a legacy exporter, but the generated code lacks
some type definitions that it can't compile).

Also, CANopen itself as a protocol controlled by `CAN in Automation (CiA)
<https://www.can-cia.org>`_ is not that really that open as a lot of the
specifications are not free. For example, now I need CiA 302 and it costs 512
Euros.

However, since CANopen provides the necessary application layer over the
original CAN protocol that only covers the transmission layer, with the legacy
Zephyr integration of CANopenNode available, it's too good to not use it. We
maintain `a fork of Zephyr <https://github.com/NTURacingTeam/zephyr>`_ that
contains the modifications necessary to use the latest version of CANopenNode.

Here are some notes for using the latest version of CANopenNode in Zephyr:

CAN Reception
=============

Callbacks
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

In addition to the standard CANopen error codes defined in CiA 301, CANopenNode
defines a set of `error status bits
<https://canopennode.github.io/CANopenNode/group__CO__EM__errorStatusBits__t.html>`_
that can be used to indicate what errors are currently happening in the node.
When an error is reported or cleared using :c:func:`CO_error`, the error status
bits will be set or cleared accordingly and if the same bit is already set or
cleared, no processing will happen. Such mutually exclusivity effectively making
the error status bits the real error being tracked of and the CANopen error
codes being the additional information of the error. The number of error status
bits is defined by `CO_CONFIG_EM_ERR_STATUS_BITS_COUNT
<https://canopennode.github.io/CANopenNode/group__CO__STACK__CONFIG__EMERGENCY.html#gab87776d4802748671b234112263760af>`_.

If error status bits are needed to be accessed via the object dictionary, 
`CO_CONFIG_EM_STATUS_BITS
<https://canopennode.github.io/CANopenNode/group__CO__STACK__CONFIG__EMERGENCY.html#ga16aa1479ffd52a627d1053c20f844b62>`_
should be set as well as define a OD entry ``Error status bits`` of type
``OCTET_STRING`` with length of `CO_CONFIG_EM_ERR_STATUS_BITS_COUNT / 4`. You
are responsible for defining the OD entry and register it to CANopenNode using
:c:func:`CO_EM_init`.

Error register
--------------

CANopenNode also manages ``Error register`` of OD 0x1001 via a set of
`CO_CONFIG_ERR_CONDITION_*
<https://canopennode.github.io/CANopenNode/group__CO__STACK__CONFIG__EMERGENCY.html>`_
macros based on the error status bits. However, the default behavior only sets
the generic bit when error status bits between ``0x28`` to ``0x2F`` are set,
which does **NOT** adhere to the CANopen specification stating that: "The
generic error shall be signaled at any error situation [#]_."

EMCY write
----------

In order to transmit the error status bits in the Emergency (EMCY) object, the
first byte of the manufacturer-specific error code is used to store the error
status bit currently reported, **NOT** the error status bits that are currently
set.

The standard CANopen EMCY write payload has the following format [#]_:

.. code-block:: none

     0        1          2         3                              7
   +------------+----------------+----------------------------------+
   | error code | error register | manufacturer-specific error code |
   +------------+----------------+----------------------------------+

And CANopenNode uses the first byte of manufacturer-specific error code (the
byte of index 3) to transmit the reported error status bit, so the payload
becomes:

.. code-block:: none

     0        1         2                  3           4                              7
   +------------+----------------+------------------------------------------------------+
   | error code | error register | error status bits | manufacturer-specific error code |
   +------------+----------------+------------------------------------------------------+

CANopenNode also recognizes the first byte of manufacturer-specific error code
as error status bit when receiving EMCY messages from other nodes. The
callback for receiving EMCY registered using :c:func:`CO_EM_initCallbackRx` has
the prototype:

.. code-block:: c

   void pFunctSignalRx(const uint16_t ident,
                       const uint16_t errorCode,
                       const uint8_t errorRegister,
                       const uint8_t errorBit,
                       const uint32_t infoCode);

Where ``infoCode`` is the rest of the manufacturer-specific error code.

Pre-defined error fields
------------------------

CANopenNode also helps to maintain ``Pre-defined error fields`` of OD 0x1003 for
recording errors that happened if `CO_CONFIG_EM_HISTORY
<https://canopennode.github.io/CANopenNode/group__CO__STACK__CONFIG__EMERGENCY.html#ga16aa1479ffd52a627d1053c20f844b62>`_
is set. Once an error is reported using :c:func:`CO_error`, it will be recorded
to ``Pre-defined error fields`` in the following format [#]_:

.. code-block:: none

   32               24     16           0
   +----------------+------+------------+
   | error register | 0x00 | error code |
   +----------------+------+------------+
   MSB                                LSB

Reference
=========

.. [#] `Zephyr CAN bus driver documentation
   <https://docs.zephyrproject.org/3.6.0/hardware/peripherals/can/controller.html#receiving>`_
   on receiving messages
.. [#] CiA 301, section 7.5.2.2 Error register
.. [#] CiA 301, section 7.2.7.3.1 Protocol EMCY write
.. [#] `CO_err() source code
   <https://github.com/CANopenNode/CANopenNode/blob/master/301/CO_Emergency.c#L673C14-L673C21>`_
