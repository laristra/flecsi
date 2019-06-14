.. |br| raw:: html

   <br />

..

********************************************************************************
 FleCSI Static Analyzer
********************************************************************************

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Subtitle?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

See if there's a way to make the above appear as a centered title, and larger
than paragraph headings as we have below.

Good info: ``http://docutils.sourceforge.net/docs/user/rst/quickstart.html``



================================================================================
Introduction
================================================================================

zzz

--------------------
Overview
--------------------

The FleCSI Static Analyzer, or ``Flecstan``, is a code analysis tool that
provides FleCSI users with additional analysis of their codes, above and beyond
what a C++ compiler provides. Our basic underlying design philosophy is that
we can use our knowledge of FleCSI constructs, in particular their proper uses
versus possible misuses, to immediately detect certain problems that might not
normally arise until run-time.

``Flecstan`` itself is a C++ application, built using the Clang and LLVM APIs.
As such, it is, in some sense, a compiler. When you use ``flecstan`` to analyze
a C++ code, however, it doesn't "compile" your code in the usual sense of the
term. Specifically, it doesn't take your C++ and produce either object files,
or an executable. While we could have designed ``flecstan`` to do this - in
effect, to act as a modified compiler - we decided that it was cleaner and more
flexible to design it as a separate, standalone analysis tool. You may, after
all, want to fully compile your code using a C++ compiler other than Clang
(``g++``, say, or the Intel compiler). Or, you may wish to run our analysis
tool periodically, while building your app, without incurring the extra overhead
of having it fully compile.

At present, for our initial version of ``flecstan``, we have focused in
particular on various uses and misuses of *FleCSI's macro interface*. As FleCSI
users know, FleCSI provides a rich set of macros for performing various tasks.
For example, FleCSI at present defines four macros (regular, simplified, MPI,
and MPI simplified) for task registration, and another four macros for task
execution. An easy user mistake is to execute a function that hasn't been
registered. This could happen due to the simple oversight of never having
invoked the registration macro; or, perhaps, to invoking it, but with a typo
in the task name or the namespace.

For technical reasons that we won't detail here, such a mistake won't generally
be detected by the C++ compiler itself. The error, therefore, will appear only
at run-time, possibly, in a particularly unfortunate scenario, only after much
work has already been done, and the error is triggered when the code takes a
different path than it has before.

While FleCSI's macro interface has so far been our primary focus for analysis,
``flecstan`` is designed and written to be somewhat open-ended, so that it can
grow and evolve as FleCSI does. As more people use FleCSI and report on their
experiences, we can consider what additional static analysis capabilities might
be possible and beneficial for us to include.



--------------------
Design
--------------------



--------------------
Quick Tour
--------------------



================================================================================
Input
================================================================================

--------------------
Compilation Databases
--------------------

--------------------
Json Files
--------------------

--------------------
Make Output
--------------------

--------------------
Direct C++ Source
--------------------



================================================================================
Basic Examples
================================================================================

--------------------
Example 01
--------------------

--------------------
Example 02
--------------------

--------------------
Example 03
--------------------



================================================================================
Analyzing FleCSI Tutorial Codes
================================================================================



================================================================================
Analyzing FleCSALE-MM Codes
================================================================================



================================================================================
Report Content
================================================================================

--------------------
Quiet, Verbose
--------------------

--------------------
General
--------------------

--------------------
Sections
--------------------

--------------------
Diagnostics
--------------------

--------------------
Auxiliary
--------------------



================================================================================
Report Formatting
================================================================================

--------------------
General
--------------------

--------------------
Visual Candy
--------------------

--------------------
File printing
--------------------



================================================================================
In-Depth Examples
================================================================================

--------------------
Example 01
--------------------

--------------------
Example 02
--------------------

--------------------
Example 03
--------------------



================================================================================
Advanced Topics
================================================================================

--------------------
Input: Direct C++ Source
--------------------

--------------------
YAML
--------------------

**Input** |br|

**Output** |br|

--------------------
Diagnostic Traces
--------------------

--------------------
Echoing Compilation Commands
--------------------

--------------------
Debug Mode
--------------------



================================================================================
Appendices
================================================================================

--------------------
Command-Line Options
--------------------

--------------------
Variants
--------------------

--------------------
Categorized
--------------------

--------------------
Alphabetical
--------------------



================================================================================
Remove This Later
================================================================================

Some headings...

   - Heading 1
      - subheading A
      - subheading B

   - Heading 2
      - subheading A
      - subheading B
      - subheading C

A code block...

.. code-block:: console

     flecstan 01-task-good-register-execute-inside.json

This ``filename`` is in fixed-width font.

*This is italics*

**This is bold**

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
