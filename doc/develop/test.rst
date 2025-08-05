.. _develop_test:

=======
Testing
=======

.. todo::

   Finish this section.

Here list some commands to run tests in this project.

.. code-block:: bash

   # in <nturt_zephyr_common_root/tests>
   west twister -p native_sim -T .

.. code-block:: bash

   # in <nturt_zephyr_common_root/tests>
   west twister --coverage --coverage-basedir .. -p native_posix -T .

.. code-block:: bash

   # in <nturt_zephyr_common_root/tests>
   west twister --device-testing --hardware-map hardware_map.yaml -T .

