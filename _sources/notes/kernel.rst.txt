.. _notes_kernel:

=============
Zephyr Kernel
=============

Boot Sequence
=============

The boot sequence of Zephyr is not in the documentation, but can be referenced
form `this conference recording <https://www.youtube.com/watch?v=tAH5fy0rSo4>`_
as well as `its slides
<https://static.sched.com/hosted_files/osselc21/3c/ELC-2021-Zephyr%20Startup%20-%20v0.2.pdf>`_.

Basically, the boot sequence is as follows:

.. figure:: ../_static/images/notes/startup_early_driver.jpg
   :width: 60%
   :align: center

   Early driver initialization.

.. figure:: ../_static/images/notes/startup_kernel.jpg
   :width: 100%
   :align: center

   Kernel initialization.

.. figure:: ../_static/images/notes/startup_post_kernel.jpg
   :width: 60%
   :align: center

   Kernel initialization.
