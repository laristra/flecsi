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

--------------------
Overview
--------------------

The FleCSI Static Analyzer, or ``Flecstan``, is a code analysis tool that
provides FleCSI users with additional analysis of their codes, above and
beyond what a C++ compiler provides.

Our basic underlying design philosophy is that we can use our knowledge of
FleCSI constructs, in particular their proper uses versus possible misuses,
to immediately detect certain problems that might not
normally arise until run-time.

At present, for our initial version of the tool, we have focused on various
uses and misuses of FleCSI's *macro interface*. As FleCSI users know, FleCSI
provides a rich set of macros for performing various activities. For example,
FleCSI at present defines four macros (regular, simplified, MPI, and MPI
simplified) for task registration, and four similar macros for task execution.
An easy mistake is to execute a function that hasn't been registered. This
could happen due to the simple oversight of never having invoked the necessary
registration macro - or perhaps to invoking it, but with a typo in the task
name or the namespace.

For technical reasons that we won't detail here, such a mistake won't generally
be detected by the C++ compiler, and the error will therefore appear only at
run-time. In a particularly unfortunate scenario, the error may be triggered,
and cause program failure, only after much work has already been done, and
after the code takes a different path than it has before.

While FleCSI's macro interface has thus far been our primary focus for analysis,
``flecstan`` is designed and written to be somewhat open-ended, so that it can
grow and evolve as FleCSI does. As more people use FleCSI and report on their
experiences, we can consider what additional static analysis capabilities might
be possible and beneficial for us to include.

Perhaps of particular interest to some potential users:

   - Analysis can be done on *any* C++ code - it doesn't *need* to be based on
     FleCSI. Naturally, however, if invoked on a non-FleCSI code, the analyzer
     won't find any FleCSI constructs to evaluate.

   - ``Flecstan`` intercepts the diagnostics (warnings, errors, etc.) that the
     underlying ``clang++`` compiler produces, and emits them in its own format.
     C++ compilers are notorious for generating confusing and visually messy
     diagnostics. While automatically interpreting and rewriting such cryptic
     content would involve some form of magic that we don't possess, we can
     (and do) re-format and report the compiler's diagnostics in a manner that
     some users may find to be clearer.

The second point above may lead to some users actually running our analyzer
on non-FleCSI codes - the first point above - if only to get diagnostics that,
arguably, are presented more clearly than the compiler's. We'd like to hear
about it if you do choose to use ``flecstan`` this way, especially if you have
additional ideas on how we can, in effect, improve the compiler's reporting.



--------------------
Design
--------------------

**Basic Use** |br|
``Flecstan`` is a command-line tool. You invoke it on the command line,

.. code-block:: console

   $ flecstan *flags, input, etc.*

with any of various command-line flags, and, most importantly, with input that
describes *precisely* how your FleCSI-based code must be compiled. ``Flecstan``
then prints the results of its analysis to standard output - to your terminal
window.

**Flags** |br|
Numerous options allow you to control, in detail, the content and formatting
of ``flecstan``'s output.

**Input** |br|
There are three basic methods for describing a FleCSI code's compilation:

   - Json-format "compilation databases"

   - Pulling information from a *make* process

   - Direct specification of compiler flags and C++ file

Each of these is described in detail in the *Input* chapter.

We wish to emphasize the *fundamental importance* of ``flecstan`` being told
*precisely* how a FleCSI code is compiled. Consider, for example, one of the
FleCSALE-MM applications; say, ``hydro_2d``. Building ``hydro_2d`` requires
the compilation of several C++ source files that are spread out across several
directories. Each such compilation involves the use of many compilation flags,
in order to stipulate such things as ``#include`` directories (as with ``-I``)
and ``#defines`` (as with ``-D``). The compilations of an individual C++ source
file can, and often does, depend on context: a build system will enter into
a specific directory before compiling a file, and the directory context will,
or at least can, affect relative ``#include`` paths.

In short, the proper static analysis of a code requires a precise knowledge
of how the code is to be built, and thus necessarily requires equal complexity.
Build-system complexity is often hidden from end users, for simplicity's sake.
Our intention is to similarly hide analysis-system complexity, when possible.
Just be aware that the concepts of *building* a code, and of *analyzing* it,
go hand-in-hand. With codes like those in FleCSALE-MM, therefore, or even those
in FleCSI's tutorial examples, you can't simply feed the relevant ``.cc`` files
into ``flecstan`` and hope to get meaningful analysis. The analyzer must know
about compilation context, ``#include`` directories, ``#define``s, compiler
flags, etc., just as the build system does.

**Remark** |br|
``Flecstan`` itself is a C++ application, built using the Clang and
LLVM APIs. As such, it is, in some sense, a compiler. When you use ``flecstan``
to analyze a C++ code, however, it doesn't "compile" your code in the usual
sense of the term. Specifically, it doesn't take your C++ and produce either
object files, or an executable. While we could have designed ``flecstan`` to do
this - in effect, to act as a modified compiler - we decided that it was cleaner
and more flexible to design it as a separate, standalone analysis tool. You may,
after all, want to fully compile your code using a C++ compiler other than Clang
(``g++``, say, or the Intel compiler). Or, you may wish to run our analysis tool
periodically, while building your app, without incurring the extra overhead of
having it fully compile.



--------------------
Quick Tour
--------------------

Let's begin with a simple, do-nothing C++ code:

.. code-block: cpp

   // File: stub.cc
   int main()
   {
   }

The following ``flecstan`` command performs a simple analysis (not that there's
much, yet, to analyze):

.. code-block:: console

   $ flecstan stub.cc

Output is as follows:

.. code-block:: console

   [30;1m--------------------------------------------------------------------------------[0m
   [30;1mCommand[0m
   [30;1m--------------------------------------------------------------------------------[0m

   [32;21mNote:[0m
      [32;21mQueueing C++ file stub.cc.[0m

   [30;1m--------------------------------------------------------------------------------[0m
   [30;1mCompilation[0m
   [30;1m--------------------------------------------------------------------------------[0m

   [35;21mFile:[0m
      [35;21mstub.cc[0m

   [34;1mScanning for FleCSI macros...[0m

   [34;1mVisiting the C++ abstract syntax tree...[0m

   [30;1m--------------------------------------------------------------------------------[0m
   [30;1mAnalysis[0m
   [30;1m--------------------------------------------------------------------------------[0m

   [34;1mSynopsis:[0m
      [36;1mNo errors or warnings were detected.[0m

   [30;1m--------------------------------------------------------------------------------[0m
   [30;1mSummary[0m
   [30;1m--------------------------------------------------------------------------------[0m

   [32;21mNote:[0m
      [32;21mFleCSI static analysis completed.[0m

Martin, good luck with that. See what the damage is, especially with regards
to the ANSI color escape sequences.



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

   * **Informational**
      * ``-[-]version``
      * ``-[-]help``

   * **Format: general**
      * ``-[-]long``
      * ``-[-]short``

   * **Format: visual**
      * ``-[-]print``
      * ``-[-][no-]color[s]``
      * ``-[-][no-]formfeed[s]``

   * **Format: files**
      * ``-[-]file-long``
      * ``-[-]file-short``
      * ``-[-]file-shorter``
      * ``-[-]file-full``
      * ``-[-]file-strip``

   * **Content: combos**
      * ``-[-]quiet``
      * ``-[-]verbose``

   * **Content: sections**
      * ``-[-][no-]section-command``
      * ``-[-][no-]section-compilation``
      * ``-[-][no-]section-analysis``
      * ``-[-][no-]section-summary``

   * **Content: general**
      * ``-[-][no-]title[s]``
      * ``-[-][no-]file[s]``
      * ``-[-][no-]report[s]``
      * ``-[-][no-]scan[ning]``
      * ``-[-][no-]macro[s]``
      * ``-[-][no-]visit[ing]``
      * ``-[-][no-]link[s]``
      * ``-[-][no-]column[s]``

   * **Content: diagnostics**
      * ``-[-][no-]note[s]``
      * ``-[-][no-]warning[s]``
      * ``-[-][no-]error[s]``

   * **Content: auxiliary**
      * ``-[-][no-]trace[s]``
      * ``-[-][no-]ccdetail[s]``

   * **Content: debugging**
      * ``-[-]debug``

   * **Files: input**
      * ``-[-]json``
      * ``-[-]make``
      * ``-[-]yaml``

   * **Direct compilation**
      * ``-[-]dir[ectory], -[-]folder``
      * ``-[-]clang[++]``
      * ``-[-]flag[s]``
      * ``-[-]cc, -[-]cpp, -[-]cxx, -[-]c++, -[-]C``

   * **Files: output**
      * ``-[-]yout``



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

A console code block...

.. code-block:: console

   $ flecstan 01-task-good-register-execute-inside.json

This ``filename`` is in a fixed-width font.

*This is italics*

**This is bold**

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
