Building the Examples
=====================

The example codes in the tutorial are meant to be built using the
*flecsit* tool. To use *flecsit*, you must first configure your path to
find the script, and to include any dynamic library dependencies. We
have provided initialization scripts for bash, csh, and environment
modules to ease this step.

**If you are using the Docker container, you can simply run:**

.. code-block:: console

  $ module load flecsi-tutorial

Otherwise, to use the bash or csh script, source the script
(located in the bin directory of your FleCSI install path):

.. code-block:: console

  $ source CMAKE_INSTALL_PREFIX/bin/flecsi-tutorial.{sh,csh}

To use the environment module, you will need to install the module file
in an appropriate path (This may have been done by your administrator.)
You can then load the module as usual:

.. code-block:: console

  $ module load flecsi-tutorial

Once your environment has been correctly configured, you can build any
of the tutorial examples like:

.. code-block:: console

  $ flecsit compile example.cc

where *example.cc* is the name of the example source file. This will
produce an executable that can be run like:

.. code-block:: console

  $ ./example

For help on the flecsit compile command, type:

.. code-block:: console

  $ flecsit compile --help

Some examples are designed to run in parallel (indicated in the example
documentation). These should be run with a suitable MPI interpreter:

.. code-block:: console

  $ mpirun -np 2 example

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
