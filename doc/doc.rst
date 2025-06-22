.. _doc:

==================
Doc Implementation
==================

This page explains how the documentation is implemented in the NTU Racing Team
Zephyr Common and how to build it from the source.

Building the Documentation
==========================

Requirements
------------

.. note::

   If you are using the Docker image provided by the `NTU Racing Team's Zephyr
   workspace <https://github.com/NTURacingTeam/zephyr_workspace>`_, the
   requirements are already installed and you can skip this section.

This documentation requires the following packages:

APT packages
~~~~~~~~~~~~

.. code-block:: none

   doxygen=1.12.0
   graphviz
   make

.. note::

   The version of doxygen is important as the configuration file is spciific to
   that version. Since currently the latest version of doxygen in ubuntu is
   1.9.6, you may need to download it manually from `Doxygen Download
   <https://www.doxygen.nl/download.html>`_.

pip modules
~~~~~~~~~~~

.. literalinclude:: requirements.txt
   :language: none

Building the Documentation
--------------------------

To build the documentation, run the following command:

.. code-block:: bash

   # in <nturt_zephyr_common>/doc
   make html

then the main documentation and API reference will be built in the
``_build/html`` and ``_build_doxygen/html`` directories respectively, which can
be viewed in a browser by opening the ``index.html`` file.

Deploying to GitHub Pages with GitHub Actions
=============================================

For ease of access, the main documentation you are reading right now is deployed
to GitHub Pages at `<https://nturacingteam.github.io/nturt_zephyr_common>`_,
which is automated using GiHub Actions. 

The GitHub Actions workflow is defined in `.github/workflows/doc.yml
<https://github.com/NTURacingTeam/nturt_zephyr_common/blob/master/.github/workflows/doc.yml>`_.
And it mainly uses `Sphinx to GitHub Pages
<https://github.com/marketplace/actions/sphinx-to-github-pages>`_ and `GitHub
Pages <https://github.com/marketplace/actions/github-pages-action>`_ actions to
build and deploy the documentation. The installation process is the same as
described above, with the Doxygen installation copied from `Zephyr
<https://github.com/zephyrproject-rtos/zephyr/blob/v4.0.0/.github/workflows/doc-build.yml#L73>`_.

Implementation Details
======================

Main Documentation
------------------

The main documentation that you are reading right now is generated using `Sphinx
<https://www.sphinx-doc.org/en/master/>`_ with `Read the Docs
<https://docs.readthedocs.io/en/stable/>`_ theme. And API references generated
by Doxygen are exported to the main documentation using `Breathe
<https://breathe.readthedocs.io/en/latest/index.html>`_.

API Reference
-------------

The API reference is generated using `Doxygen
<https://www.doxygen.nl/index.html>`_ with `Doxygen Awesome
<https://jothepro.github.io/doxygen-awesome-css/index.html>`_ theme.
