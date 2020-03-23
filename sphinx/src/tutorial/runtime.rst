.. |br| raw:: html

   <br />

Runtime
=======

Using FleCSI requires proper initialization and configuration of the
FleCSI runtime. These examples illustrate some of the basic steps and
options that are available.

Example 1: Minimal
******************

The core FleCSI runtime has three control points: *initialize*, *start*, and
*finalize*. These must be invoked by the user's application code in that
order.

.. sidebar:: Top-level Action

  The top-level action is a C++ function created by the user to tell
  FleCSI what it should do when the start control point is invoked.

* **initialize** |br|
  This control point spins up the FleCSI runtime, and (optionally) any
  other runtimes on which FleCSI depends.

* **start** |br|
  This control point begins the actual runtime execution that forms the
  bulk of any simulation that is performed using FleCSI. The user must
  pass a top-level action that FleCSI will execute.

* **finalize** |br|
  This control point shuts down the FleCSI runtime, and any other
  runtimes that FleCSI itself initialized.

This example demonstrates a minimal use of FleCSI that just executes an
action to print out *Hello World*. Code for this example can be found in
*tutorial/1-runtime/1-minimal.cc*

.. literalinclude:: ../../../tutorial/1-runtime/1-minimal.cc
  :language: cpp

.. note::

  - The top-level action can be any C/C++ function that takes (int,
    char**) and returns an int.  In this simple example, we only print a
    message to indicate that the top-level action was actually executed
    by FleCSI. However, in a real application, the top-level action
    would execute FleCSI tasks and other functions to implement the
    simulation.  

  - The main function must invoke initialize, start, and finalize on the
    FleCSI runtime. Otherwise, the implementation of main is left to the
    user.

  - The status returned by FleCSI's initialize method should be
    inspected to see if the end-user specified --help on the command
    line. FleCSI has built-in command-line support using Boost Program
    Options. This is documented in the next example.

Example 2: Program Options
**************************

FleCSI supports a program options capability based on `Boost Program
Options`__ to simplify the creation and management of user-defined
command-line options. The basic syntax for adding and accessing program
options is semantically similar to the Boost interface (You can find
documentation using the above link.) However, there are some notable
differences: FleCSI internally manages the boost::program_options::value
variables for you, using boost::optional; Positional options are the
mechanism that should be used for *required* options; Default, implicit,
zero, and multi value attributes are specified in the
flecsi::program_option constructor as an std::initializer_list. This
section of the tutorial provides examples of how to use FleCSI's program
option capability.

__ https://www.boost.org/doc/libs/1_63_0/doc/html/program_options.html

Example Program
+++++++++++++++

In this example, imagine that you have a program that takes information
about a taxi service (The options are silly and synthetic. However they
demonstrate some basic usage of flecsi::program_option type.) The
command-line options and arguments for the program allow specification
of the following: trim level, transmission, child seat, purpose
(personal or business), light speed, and a passenger list. The first two
options will be in a *Car Options* section, while the purpose will be
under the *Ride Options* section. The passenger list is a positional
argument to the program. The help output for the entire program looks
like this:

.. code-block:: console

  Usage: program_options <passenger-list>

  Basic Options:
    -h [ --help ]                         Print this message and exit.

  Car Options:
    -l [ --level ] arg (= 1)              Specify the trim level [1-10].
    -t [ --transmission ] arg (= manual)  Specify the transmission type
                                          ["automatic", "manual"].
    -c [ --child-seat ] [=arg(= 1)] (= 0) Request a child seat.

  Ride Options:
    -p [ --purpose ] arg (= 1)            Specify the purpose of the trip
                                          (personal=0, business=1).
    --lightspeed                          Travel at the speed of light.

  Positional Options:
    passenger-list The list of passengers for this trip

  FleCSI Options:
    --flog-tags arg (=all)         Enable the specified output tags, e.g.,
                                   --flog-tags=tag1,tag2.
    --flog-verbose [=arg(=1)] (=0) Enable verbose output. Passing '-1' will strip
                                   any additional decorations added by flog and
                                   will only output the user's message.
    --flog-process arg (=-1)       Restrict output to the specified process id.

  Available FLOG Tags (FleCSI Logging Utility):
    execution
    task_wrapper
    legion_mapper
    unbind_accessors
    topologies
    reduction_wrapper
    context
    registration
    bind_accessors
    task_prologue

This shows the program usage, the basic options, e.g., *--help*, the
command-line and positional options for the example, and some auxiliary
options for controlling the FleCSI logging utility *FLOG* (described in the
next section of this tutorial).

Declaring Options
+++++++++++++++++

.. note::

  FleCSI program options must be declared at namespace scope, i.e.,
  outside of any function, class, or enum class. This is not a problem! It
  is often convenient to declare them in a header file (in which case,
  they must be declared *inline*), or directly before the *main* function.
  We use the latter for this example simply for conciseness.

Let's consider the first *Car Options* option: *--level*. To declare
this option, we use the following declaration:

.. literalinclude:: ../../../tutorial/1-runtime/2-program_options.cc
  :language: cpp
  :lines: 18-31

First, notice that the flecsi::program_option type is templated on the
underlying option type *int*. In general, this can be any valid C++
type.

This constructor to flecsi::program_option takes the following
parameters:

* *section ("Car Options")*: |br|
  Identifies the section. Sections are generated automatically, simply
  by referencing them in a program option.

* *flag ("level,l")*: |br|
  The long and short forms of the option. If the string contains a
  comma, it is split into *long name,short name*. If there is no comma,
  the string is used as the long name with no short name.

* *help ("Specify...")* |br|
  The help description that will be displayed when the usage message
  is printed.

* *values ({{flecsi::option_default, ...}})* |br|
  This is a
  std::initializer_list<flecsi::program_option::initializer_value<int>>.
  The possible values are flecsi::option_default,
  flecsi::option_implicit, flecsi::option_zero, and
  flecsi::option_multi. The default value is used if the option is not
  passed at invocation. The implicit value is used if the option is
  passed without a value. If zero is specified, the option does not take
  an argument, and an implicit value must be provided. If multi is
  specified, the option takes multiple values.

* *check ([](flecsi::any const &...)* |br|
  An optional, user-defined predicate to validate the value passed by
  the user.

The next option *--transmission* is similar, but uses a std::string
value type:

.. literalinclude:: ../../../tutorial/1-runtime/2-program_options.cc
  :language: cpp
  :lines: 33-46

The only real difference is that (because the underlying type is
std::string) the default value is also a string.

The last option in the "Car Options" section demonstrates the use of
flecsi::option_implicit:

.. literalinclude:: ../../../tutorial/1-runtime/2-program_options.cc
  :language: cpp
  :lines: 48-60

Providing an implicit value, defines the behavior for the case that the
user invokes the program with the given flag, but does not assign a
value, e.g., *--child-seat* vs. *--child-seat=1*. The value is *implied*
by the flag itself.

.. caution::

  This style of option should not be used with positional arguments
  because Boost appears to have a bug when such options are invoked
  directly before a positional option (gets confused about separation).
  We break that convention here for the sake of completeness. If you
  need an option that simply acts as a switch, i.e., it is either *on*
  or *off*, consider usint the --lightspeed style option below, as this
  type of option is safe to use with positional options.

In the *Ride Options* section, the *--purpose* option takes an integer
value *0* or *1*. This option is declared with the following code:

.. literalinclude:: ../../../tutorial/1-runtime/2-program_options.cc
  :language: cpp
  :lines: 62-76

This option demonstrates how an enumeration can be used to define
possible values. Although FleCSI does not enforce correctness, the
enumeration can be used to check that the option is within a particular
range.

The next option in the *Ride Options* section defines an implicit value
and zero values (meaning that it takes no values). The *--lightspeed*
option acts as a switch, taking the implicit value if the flag is
passed.  This will be useful to demonstrate how we can check whether or
not an option was passed in the next section:

.. literalinclude:: ../../../tutorial/1-runtime/2-program_options.cc
  :language: cpp
  :lines: 78-86

The final option in this example is a positional option, i.e., it is an
argument to the program itself:

.. literalinclude:: ../../../tutorial/1-runtime/2-program_options.cc
  :language: cpp
  :lines: 88-100

Postional options are required, i.e., the program will error and print
the usage message if the option is not present.

Checking & Using Options
++++++++++++++++++++++++

FleCSI option variables are implemented using an *optional* C++ type.
The utility of this implementation is that *optional* already captures
the behavior that we want from an option, i.e., it either has a value,
or it does not. If the option has a value, the specific value depends on
whether or not the user explicitly passed the option on the command
line, and its default and implicit values.

Options that have a default value defined do not need to be tested:

.. literalinclude:: ../../../tutorial/1-runtime/2-program_options.cc
  :language: cpp
  :lines: 111-139

Here, we simply need to access the value of the option using the
*value()* method.

For options with no default value, we can check whether or not the
option has a value using the *has_value()* method:

.. literalinclude:: ../../../tutorial/1-runtime/2-program_options.cc
  :language: cpp
  :lines: 141-148

Our one positional option works like the defaulted options (because it
is required), and can be accessed using the *value()* method:

.. literalinclude:: ../../../tutorial/1-runtime/2-program_options.cc
  :language: cpp
  :lines: 150-161

Here is the full source for this tutorial example:

.. literalinclude:: ../../../tutorial/1-runtime/2-program_options.cc
  :language: cpp

Example 3: FLOG (FleCSI Logging Utility)
****************************************

FLOG provides users with a mechanism to print logging information to
various output descriptors, e.g., standard out, standard error, or to a
file descriptor. Multiple descriptors can be used simaltaneously, so
that information about the running state of a program can be captured
and displayed, possibly selecting different parts to go to different
descriptors.

If you wish to use FLOG, you must initialize and configure its behavior
after flecsi::initialize has been invoked, but before flecsi::start.

.. important::

  One of the challenges of using distributed-memory and tasking runtimes
  is that output written to the console often get clobbered because
  multiple threads of execution are all writing to the same descriptor
  concurrently. FLOG fixes this by collecting output from different
  threads and serializing it. This is an important and useful feature of
  FLOG.



.. literalinclude:: ../../../tutorial/1-runtime/3-flog.cc
  :language: cpp

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
