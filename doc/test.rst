.. _test:

=======
Testing
=======

.. code-block:: bash

   # in <nturt_zephyr_common_root>
   west twister -p native_sim -T .

.. code-block:: bash

   # in <nturt_zephyr_common_root>
   west twister --device-testing --hardware-map hardware_map.yaml -T .
