.. _test:

=======
Testing
=======

.. code-block:: bash

   # in <nturt_zephyr_common_root/tests>
   west twister -p native_sim -T .

.. code-block:: bash

   # in <nturt_zephyr_common_root/tests>
   west twister --coverage --coverage-basedir .. -p native_posix -T .

.. code-block:: bash

   # in <nturt_zephyr_common_root/tests>
   west twister --device-testing --hardware-map hardware_map.yaml -T .
