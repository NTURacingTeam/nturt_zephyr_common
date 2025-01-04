.. _notes_tests:

=================
Testing in Zephyr
=================

Zephyr comes with its own test framework `Ztest
<https://docs.zephyrproject.org/4.0.0/develop/test/ztest.html>`_ utilizing its
comprehensive simulation/emulation capabilities to test code on the host
machine with kernel support in integration tests or without in unit tests. It
also has its own test runner `Twister
<https://docs.zephyrproject.org/4.0.0/develop/test/twister.html>`_ to automate
running multiple tests on multiple platforms in one go.

Mocking
=======

Ztest provides preliminary `test assertion API
<https://docs.zephyrproject.org/4.0.0/doxygen/html/group__ztest__assert.html>`_
for testing the return values of functions under test. However, when it comes to
testing the behavior of a function (for example what other functions it calls
and what parameters are passed into them), it is necessary to mock the function
being called. Ztest also provides a set of preliminary `mocking API
<https://docs.zephyrproject.org/4.0.0/doxygen/html/group__ztest__mock.html>`_
for this purpose.

Ztest mocking requires ``CONFIG_ZTEST_MOCKING`` Kconfig option to be enabled and
its usage is demonstrated in the following example:

If we expect a function :c:func:`foo` to call another function :c:func:`bar`
with a given parameter ``param``, before calling :c:func:`foo` we can mock
:c:func:`bar` by defining :c:func:`bar` as the following:

.. code-block:: c

   #include <zephyr/ztest.h>
   #include <zephyr/ztest_mock.h>

   void bar(int param) {
     ztest_check_expected_value(param)
   }

Then, in the test case, before calling :c:func:`foo`, we can expect the value of
``param`` such as:

.. code-block:: c

   ZTEST(mock, test_mocking) {
     ztest_expect_value(bar, param, <expected_value>);
     foo();
   }

.. note::

   Every invocation of :c:func:`ztest_expect_value` should correspond to one
   invocation of :c:func:`ztest_check_expected_value`. If the number of
   invocations does not match, the test will fail. This applies to all other
   mocking functions in Ztest as well.
