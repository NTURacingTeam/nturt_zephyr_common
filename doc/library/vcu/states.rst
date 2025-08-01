.. _library_vcu_states:

=============
State Machine
=============

Overview
========

The state machine is part of the control system that manages the various states
of the system. It provides a way to handle the transition of states using
callback functions.

The state machine is defined as a `Hierarchical state machine
<https://en.wikipedia.org/wiki/UML_state_machine#Hierarchically_nested_states>`_,
where the black filled circles represent the initial state and the initial state
transition of the state machine, meaning which substate to enter when
transitioning to the parent state. So the initial state of the VCU is
``RTD_BLINK``.

.. figure:: /_static/images/state_transition_diagram.svg
   :width: 90%
   :align: center

   `UML state diagram
   <https://sparxsystems.com/resources/tutorials/uml2/state-diagram.html>`_ of
   the VCU.

State machine can be added to VCU by enabling the ``CONFIG_VCU_STATES`` Kconfig
option.

Core Concepts
-------------

- **States**: Each state is represented by a bit in the :c:enum:`states_state`
  and can be queried using :c:func:`states_get`.

- **State Transitions**: State transitions are requested using
  :c:func:`states_transition` with a command from the
  :c:enum:`states_trans_cmd`. However, the command is only executed if the
  source state of the command matches the current state. The validity can be
  checked using :c:func:`states_valid_transition` or get the information of the
  command from :c:func:`states_transition_info`.

- **Callbacks**: Modules can register callbacks via
  :c:macro:`STATES_CALLBACK_DEFINE` to be called when entering or exiting a
  state.

Runtime Behavior
----------------

- **Execution Context**: State transition callbacks are executed in the same
  context as the thread requesting the transition, ensuring thread safety.

Usage
=====

Requesting State Transitions
----------------------------

Use :c:func:`states_transition` to request a state transition:

.. code-block:: c

   states_transition(TRANS_CMD_DISABLE);

Handling State Transitions
--------------------------

Define callbacks for state transitions using :c:macro:`STATES_CALLBACK_DEFINE`.
For example:

.. code-block:: c

    static void my_callback(enum states_state state, bool is_entry, void *user_data) {
        if (is_entry) {
            printf("Entered state: %s\n", states_state_str(state));
        } else {
            printf("Exited state: %s\n", states_state_str(state));
        }
    }

    STATES_CALLBACK_DEFINE(STATE_RUNNING, my_callback, NULL);

API Reference
=============

.. doxygengroup:: stat
