.. _develop_notes_canopennode:

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

   Since STM32 series with FDCAN only supports up to 28 standard and 8 extended
   ID filters [#]_, caution must be taken when configuring CANopenNode.

References
----------

.. [#] `Zephyr CAN bus driver documentation
   <https://docs.zephyrproject.org/3.6.0/hardware/peripherals/can/controller.html#receiving>`_
   on receiving messages
.. [#] `STM32H7 FDCAN device tree source code
   <https://github.com/zephyrproject-rtos/zephyr/blob/v4.1.0/dts/arm/st/h7/stm32h7.dtsi#L533>`_,
   where the device tree binding for ``bosch,mram-cfg`` is defined in
   `<https://github.com/zephyrproject-rtos/zephyr/blob/v4.1.0/dts/bindings/can/bosch%2Cm_can-base.yaml>`_

Object Dictionary (OD)
======================

Each OD entry can be read/written by SDO via a callback function registered by
`OD_extension_init()
<https://canopennode.github.io/CANopenNode/group__CO__ODinterface.html#ga41c96feee5da30cd9117a35a307b96e1>`_
instead of straightforwardly from/to the memory. However,
`CO_CONFIG_PDO_OD_IO_ACCESS
<https://canopennode.github.io/CANopenNode/group__CO__STACK__CONFIG__SYNC__PDO.html#gaa20d1b49249b7f5a15963cc1a4611be9>`_
should be set to enable the same behavior for PDOs.

.. note::

   SDO and PDO internally get the OD entry read/write APIs are via
   :c:func:`OD_getSub`. However, SDO calls :c:func:`OD_getSub` everytime a
   request is processed [#]_, so the newly registered callback functions will
   be used. On the other hand, PDOs only call :c:func:`OD_getSub` once when
   initialized [#]_. So in order to make PDO use the callback functions to
   access OD entries, the callbacks should be registered before PDOs are
   initialized.

CANopenNode already registered some common OD entries to provide functionalities
according to the CiA 301 standard. The following is a list of registered ODs:

- 0x1003: Pre-defined error field
- 0x1005: COB-ID SYNC message
- 0x100C: Guard time
- 0x100D: Life time factor
- 0x1010: Store parameters
- 0x1011: Restore default parameters
- 0x1012: COB-ID time stamp object
- 0x1014: COB-ID EMCY
- 0x1015: Inhibit time EMCY
- 0x1016: Consumer heartbeat time
- 0x1017: Producer heartbeat time
- 0x1019: Synchronous counter overflow value
- 0x1200: SDO server parameter
- 0x1400 to 0x15FF: RPDO communication parameter
- 0x1600 to 0x17FF: RPDO mapping parameter
- 0x1800 to 0x19FF TPDO communication parameter
- 0x1A00 to 0x1BFF TPDO mapping parameter

References
----------

.. [#] `CO_SDOserver_process() source code
   <https://github.com/CANopenNode/CANopenNode/blob/master/301/CO_SDOserver.c#L644>`_
   that calls :c:func:`OD_getSub` to get the read/write APIs.
.. [#] `PDOconfigMap() source code
   <https://github.com/CANopenNode/CANopenNode/blob/master/301/CO_PDO.c#L108>`_
   that is called when TPDOs and RPDOs are initialized.

Process Data Objectss (PDOs)
============================

Receive PDOs (RPDOs)
--------------------

OD 0x1400 to 0x15FF define the communication parameters of RPDOs, here are the
overview of them and the implementation of CANopenNode:

- **Transmission type (sub-index 0x02)**: There are two types of reception;
  
  - **Synchronous (0x00 to 0xF0)**: The received data will be *actuated* after a
    SYNC message is received.
  - **Event-Driven (0xFE, 0xFF)**: The received data will be *actuated*
    immediately.

  Here *actuated* means the received data will be copied to the mapped OD
  entries defined in the RPDO mapping parameter (OD 0x1600 to 0x17FF) or the
  registered callback function will be called to process the received data.

- **Event-timer (sub-index 0x05)**: Define the deadline for the reception of
  RPDOs. If the RPDO is not received before the event-timer expires, CANopen
  error ``RPDO timeout (0x8250)`` will be reported. And if the RPDO is
  received after the event-timer expires, the error will be cleared and the
  timer will be reset.

.. note::

   If `CO_CONFIG_PDO_SYNC_ENABLE
   <https://canopennode.github.io/CANopenNode/group__CO__STACK__CONFIG__SYNC__PDO.html#gaa20d1b49249b7f5a15963cc1a4611be9>`_
   is not set, the received synchronous RPDOs will be actuated immediately.

Transmit PDOs (TPDOs)
---------------------

OD 0x1800 to 0x19FF define the communication parameters of TPDOs, here are the
overview of them and the implementation of CANopenNode:

- **Transmission type (sub-index 0x02)**: There are two types of transmission;
  
  - **Synchronous (0x00 to 0xF0)**: Transmitted after a SYNC message is
    received. There are also two sub-types of synchronous transmission;

    - **Acyclic (0x00)**: Transmitted when receiving the next SYNC message after
      requested by an *event*.
    - **Cyclic (0x01 to 0xF0)**: Transmitted after Nth SYNC message is received,
      where N is the value of the transmission type.

  - **Event-Driven (0xFE, 0xFF)**: Transmitted by an *event*.
  
  Here *event* means transmission is requested by the application via
  :c:func:`CO_TPDOsendRequest` or :c:func:`OD_requestTPDO` or when the event
  timer (sub-index 0x05) expires.

- **Inhibit time (sub-index 0x03)**: Define the minimum time between two
  consecutive transmissions of event-driven TPDOs.

- **SYNC start value (sub-index 0x06)**: Define when the TPDO will start being
  transmitted after the SYNC counter is equal to the SYNC start value.

.. note::

   If `CO_CONFIG_PDO_SYNC_ENABLE
   <https://canopennode.github.io/CANopenNode/group__CO__STACK__CONFIG__SYNC__PDO.html#gaa20d1b49249b7f5a15963cc1a4611be9>`_
   is not set, synchronous and cyclic TPDOs will not be transmitted.

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
the ``error status bits`` the real error being tracked of and the CANopen error
codes being the additional information of the error. The number of error status
bits is defined by `CO_CONFIG_EM_ERR_STATUS_BITS_COUNT
<https://canopennode.github.io/CANopenNode/group__CO__STACK__CONFIG__EMERGENCY.html#gab87776d4802748671b234112263760af>`_.

If ``error status bits`` are needed to be accessed via the object dictionary, 
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
macros based on the ``error status bits``. However, the default behavior only
sets the generic bit when ``error status bits`` between ``0x28`` to ``0x2F`` are
set, which does **NOT** adhere to the CANopen specification stating that: "The
generic error shall be signaled at any error situation [#]_."

EMCY write
----------

In order to transmit the ``error status bits`` in the Emergency (EMCY) object,
the first byte of the manufacturer-specific error code is used to store the
``error status bit`` currently reported, **NOT** the ``error status bits`` that
are currently set.

The standard CANopen EMCY write payload has the following format [#]_:

.. code-block:: none

     0        1          2         3                              7
   +------------+----------------+----------------------------------+
   | error code | error register | manufacturer-specific error code |
   +------------+----------------+----------------------------------+

And CANopenNode uses the first byte of manufacturer-specific error code (the
byte of index 3) to transmit the reported ``error status bit``, so the payload
becomes:

.. code-block:: none

     0        1         2                  3           4                              7
   +------------+----------------+------------------------------------------------------+
   | error code | error register | error status bits | manufacturer-specific error code |
   +------------+----------------+------------------------------------------------------+

CANopenNode also recognizes the first byte of manufacturer-specific error code
as ``error status bit`` when receiving EMCY messages from other nodes. The
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

References
----------

.. [#] CiA 301, section 7.5.2.2 Error register
.. [#] CiA 301, section 7.2.7.3.1 Protocol EMCY write
.. [#] `CO_err() source code
   <https://github.com/CANopenNode/CANopenNode/blob/master/301/CO_Emergency.c#L673C14-L673C21>`_
