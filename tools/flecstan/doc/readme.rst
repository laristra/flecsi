.. |br| raw:: html

   <br />

..

********************************************************************************
FLECSTAN
********************************************************************************

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FleCSI Static Analyzer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. See if there's a way to make the above appear as a centered title, and larger
.. than paragraph headings as we have below.

.. Good info: ``http://docutils.sourceforge.net/docs/user/rst/quickstart.html``



================================================================================
Introduction
================================================================================

++++++++++++++++++++++++++++++++++++++++
Overview
++++++++++++++++++++++++++++++++++++++++

The FleCSI Static Analyzer, or ``Flecstan``, is a code analysis tool that
provides FleCSI users with additional analysis of their codes, above and
beyond what a C++ compiler provides.

Our basic idea is that we can use our knowledge about specific FleCSI
constructs, in particular about their proper uses versus possible misuses,
to immediately detect certain problems that might not normally arise until
run-time.

For our initial version of the tool, we have focused on uses and misuses
of FleCSI's *macro interface*. As users know, FleCSI provides a rich set
of macros for performing various activities. For example, FleCSI at present
defines four macros (regular, simplified, MPI, and MPI simplified) for task
registration, and four similar macros for task execution. A common mistake
is to execute a function that hasn't been registered. This could happen due
to the simple oversight of never having invoked a registration macro, or of
invoking it with a typo in the task name or the namespace.

For technical reasons we'll omit here, such a mistake won't generally be
detected by the C++ compiler, and the error will therefore appear only at
run-time. In a particularly unfortunate scenario, the error may be triggered,
and cause program failure, only after much work has already been done, and
after the code takes a different path than it has before.

Although the macro interface has been our primary focus for analysis so far,
``flecstan`` is designed and written to be somewhat open-ended, so that it
can grow and evolve as FleCSI does. As more people use FleCSI and report
on their experiences, we'll learn what additional analysis capabilities
might be possible and beneficial for us to include.

The following may be of particular interest to some users:

   - Analysis can be done on *any* C++ code - it doesn't *need* to be
     FleCSI based. Naturally, however, if invoked on a non-FleCSI code,
     the analyzer won't find any FleCSI constructs to evaluate.

   - ``Flecstan`` intercepts the compiler diagnostics that the underlying
     ``clang++`` compiler produces, and displays them in its own format.
     C++ compilers are notorious for generating confusing and visually
     cluttered diagnostics. While automatically interpreting and rewriting
     such cryptic content would involve some manner of magic that we don't
     possess, we can and do re-format and colorize the compiler's native
     diagnostics in a manner that some users may find to be more clear.

The second point may lead to some users actually running our analyzer on
non-FleCSI codes - the first point above - just to see diagnostics that,
arguably, are presented more clearly than the compiler's. We'd like to hear
about it if you do choose to use ``flecstan`` this way, especially if you
have additional ideas on how we can improve on the compiler's reporting.



++++++++++++++++++++++++++++++++++++++++
Design
++++++++++++++++++++++++++++++++++++++++

----------------------------------------
Basic Use
----------------------------------------

``Flecstan`` is a command-line tool. You invoke it on the command line,

.. code-block:: console

   flecstan [options] [file(s)] ...

.. with any of various command-line options and (input) files. The ordering
.. of arguments generally doesn't matter, and affects output only cosmetically,
.. or in the presentation of certain diagnostics (as in "Y duplicates X"
.. replacing "X duplicates Y", which really mean the same thing).

The arguments - input files, in particular - must describe *precisely* how
your FleCSI-based code would be compiled; more on this soon. The results of
``flecstan``'s analysis are printed to standard output, i.e. to your terminal
window.

----------------------------------------
Options
----------------------------------------

Numerous command-line options allow you to control, in detail, the content and
formatting of ``flecstan``'s output.

There's the (default) ``-long`` output format, for instance, but ``-short`` form
if you prefer. Those affect the *form* - more visually squeezed, or less so - in
which information is printed; they don't change fundamental content.

For content, we have (the default) ``-verbose``, as well as a ``-quiet`` mode
that entirely switches off certain content that we think you're less likely to
care about.

You'll probably like our default ``-color`` if your terminal supports ANSI color
escape sequences. If it doesn't, then ``-no-color`` is your friend.

And we'll mention here our ``-debug`` mode, if only so that you'll try it out
once, and realize that you'll probably never want to think about it again unless
you become a ``flecstan`` developer someday.

The above is only a short sampling. In fact we provide for a great deal of even
finer-granularity control over precisely what ``flecstan`` produces, and will
discuss all that in due time.

But let's talk about *input* now. What files should ``flecstan`` analyze, and
how? That is, after all, why we're here....

----------------------------------------
Input
----------------------------------------

There are three basic methods for describing a FleCSI code's compilation. Any
number of any of these can be used together; ``flecstan`` is awesome that way.

   - You can pull information from ``make`` processes

   - You can provide Json-format "compilation databases"

   - You can directly specify compiler flags and C++ files

Each of these is described in detail in the *Input* chapter.

**Important**: |br|
We must emphasize the importance of ``flecstan`` being told *precisely* how
a FleCSI code is to be compiled.

Consider, for example, one of the FleCSALE-MM applications; say, ``hydro_2d``.
Building ``hydro_2d`` requires the compilation of several C++ source files,
spread out across several directories. Each such compilation involves the
use of numerous compilation flags that stipulate such things as ``#include``
directories (as with ``-I``) and ``#defines`` (as with ``-D``). Subtle details
of an individual C++ file's compilation can depend on context: a build system
will, for example, enter into a particular directory before compiling a file,
and this directory context will, or can, affect the meaning of relative
``#include`` paths.

In short, proper static analysis of a code requires a precise knowledge of
how the code is to be built, and thus necessarily requires equal complexity.
Build-system complexity is often hidden from users, for simplicity's sake.
Our intention is to similarly hide analysis-system complexity, when possible.
Just be aware that the concepts of *building* a code, and of *analyzing* it,
go hand-in-hand. With codes like those in FleCSALE-MM, therefore, or even those
in FleCSI's tutorial examples, you can't just feed the relevant source files
into ``flecstan`` and hope to get meaningful analysis. The analyzer must know
about compilation context, ``#include`` and ``#define`` specifications as might
be given by ``-I`` and ``-D`` on the command line, compiler flags, etc., just
as the build system does.

----------------------------------------
Remark
----------------------------------------

``Flecstan`` itself is a C++ application, built using the Clang and LLVM APIs.
As such, it is, in some sense, a compiler. When you use ``flecstan`` to analyze
a C++ code, however, it doesn't "compile" your code in the usual sense of the
term. Specifically, it doesn't take your C++ and produce either object files,
or an executable.

We could have designed ``flecstan`` to do that - in effect, to act as a compiler
replacement - but decided instead that it was cleaner and more flexible to make
it a standalone analysis tool. You may, after all, want to fully compile your
code something other than ``clang++``. Or, you may wish to run our analysis tool
periodically, while building your app, without incurring the extra overhead of
full compilation.



++++++++++++++++++++++++++++++++++++++++
Quick Tour
++++++++++++++++++++++++++++++++++++++++

.. zzz then maybe "same tour, more time"

----------------------------------------
Example: C++ File
----------------------------------------

Consider a simple, stub C++ code:

.. include:: stub.cc
   :code: cpp
   :number-lines:

Run this ``flecstan`` command to perform an analysis:

.. code-block:: console

   flecstan stub.cc

Output is as follows:

.. include:: stub.rst

We'll call this a "**report**."

In this simple scenario, ``Flecstan``'s output report is, dare we say,
disproportionately spectacular in relation to its rather mundane input. Later,
we'll describe how various elements of the analyzer's output can be compressed,
or removed entirely. For now, during this quick tour, just note the following.

   * Color! Yay! (Or if "Nay!" then use ``-no-color``.)

   * Clearly delineated sections: Command, Compilation, Analysis, Summary.

   * **Command**. How does ``flecstan`` interpret your command line. May
     be helpful for your own debugging or peace-of-mind.

   * **Compilation**. For each source file, ``flecstan`` finds your macro
     calls, finds the code the calls produced, and collates the information.

   * **Analysis**. Information collected above, across all files, is combined
     and examined.

   * **Summary**. ``Flecstan`` makes some closing remarks.

..
..
.. put this verbose detail later...

   * Color! Yay! (Or if "Nay!" then use ``-no-color``.)

   * Four clearly delineated sections: Command, Compilation, Analysis, Summary.

   * **Command**. Prints information about what ``flecstan`` finds on the
     command line. This began as our own debugging aid, but we decided to
     keep it. The messages may reassure you that ``flecstan`` is doing what
     you intend. If it's just mindless chatter, you can switch it off:|br|
     ``flecstan -no-section-command stub.cc``

   * **Compilation**. For every C++ code involved in the analysis, three
     actions take place:

      * The file name is printed. (This part isn't too exciting yet.)

      * **Preprocessing**. ``Flecstan`` hooks into the C++ macro preprocessor
        and collects information about your use of FleCSI's macros.

      * **AST Traversal**. ``Flecstan`` traverses the C++ Abstract Syntax
        Tree (AST), identifies C++ constructs that were inserted by macro
        calls, and collates the new information with what it found in the
        preprocessing stage.

   * **Analysis**. The information collected as described above, across all
     files, is combined and examined as a whole.  This process is analogous
     to the link stage of a build. If ``flecstan`` detects any instances of
     the problematic constructs knows about (say, the execution in any file
     of a task that was defined in no file), here is where the problem will
     be reported.

   * **Summary**. Here, ``flecstan`` makes some closing remarks.

The above simple invocation of ``flecstan`` works great if the input is, well,
simple. When you're analyzing a typical FleCSI-based code, however, realize
that the code's underlying build process probably involves several C++ source
files, and probably some complicated compiler invocations. When things get more
complicated, you may want to use Json files, as explained next.

----------------------------------------
Example: flecsale-mm
----------------------------------------

The ``flecsale-mm`` suite of applications has a build system based on ``cmake``
and ``make``, and serves as a good illustration of how to have the analyzer
evaluate real FleCSI-based codes. We must consider two issues.

First, ``flecsale-mm`` has several possible target executables: ``hydro_2d``,
``hydro_3d`` etc.  For proper code analysis, ``flecstan`` needs input that
reflects those (and only those) source files that are involved in any *one*
of the executables. Roughly speaking, that would mean one C++ source code with
a ``main()`` (or ``driver()``, for a FleCSI code), plus all supporting C++
code with which the first file will link. (But not more than that, and not
FleCSI's source code itself.) Provide few too files, and the analysis will be
incomplete - just as trying to link an incomplete set of object files into a
working executable is likely to fail. Provide too many files (say, the codes
for both ``hydro_2d`` and ``hydro_3d`` together), and ``flecstan`` doesn't know
how to divvy up the various elements - and you'll end up getting, among other
wrong results, the moral equivalent of multiply-defined symbol errors at link
time.

Second, the underlying compilation commands - the full ``g++`` or ``clang++``
commands that ``make`` would invoke - are more complex here than they'd be
when compiling a simple C++ code as shown above.

.. different suite of codes uses a ``cmake/make``-based build system,

.. zzz write this

----------------------------------------
Example: FleCSI Tutorial
----------------------------------------

.. zzz write this



================================================================================
Input
================================================================================

++++++++++++++++++++++++++++++++++++++++
Make Output
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Json Files
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Direct C++ Source
++++++++++++++++++++++++++++++++++++++++



================================================================================
Examples, Part I
================================================================================

++++++++++++++++++++++++++++++++++++++++
Example 01
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Example 02
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Example 03
++++++++++++++++++++++++++++++++++++++++



================================================================================
Analyzing FleCSI Tutorial Codes
================================================================================



================================================================================
Analyzing FleCSALE-MM Codes
================================================================================



================================================================================
Report Content
================================================================================

++++++++++++++++++++++++++++++++++++++++
Quiet, Verbose
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
General
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Sections
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Diagnostics
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Auxiliary
++++++++++++++++++++++++++++++++++++++++



================================================================================
Report Formatting
================================================================================

++++++++++++++++++++++++++++++++++++++++
General
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Visual Candy
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
File printing
++++++++++++++++++++++++++++++++++++++++



================================================================================
Examples, Part II
================================================================================

++++++++++++++++++++++++++++++++++++++++
Example 01
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Example 02
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Example 03
++++++++++++++++++++++++++++++++++++++++



================================================================================
Advanced Topics
================================================================================

++++++++++++++++++++++++++++++++++++++++
YAML Input
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
YAML Output
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Diagnostic Traces
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Echoing Compilation Commands
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Debug Mode
++++++++++++++++++++++++++++++++++++++++



================================================================================
Appendices
================================================================================

++++++++++++++++++++++++++++++++++++++++
Command-Line Options
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Variants
++++++++++++++++++++++++++++++++++++++++



++++++++++++++++++++++++++++++++++++++++
Categorized
++++++++++++++++++++++++++++++++++++++++

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

   * **Format: color markup**
      * ``-[-]markup-ansi``
      * ``-[-]markup-html``
      * ``-[-]markup-rst``
      * ``-[-]markup-tex``
      * ``-[-]markup-tex-listing``

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

   * **Direct compilation**
      * ``-[-]dir[ectory]``
      * ``-[-]folder``
      * ``-[-]clang[++]``
      * ``-[-]flag[s]``
      * ``-[-]cc``
      * ``-[-]cpp``
      * ``-[-]cxx``
      * ``-[-]c++``
      * ``-[-]C``

   * **Files: input**
      * ``-[-]make``
      * ``-[-]json``
      * ``-[-]yaml``

   * **Files: output**
      * ``-[-]yout``



++++++++++++++++++++++++++++++++++++++++
Alphabetical
++++++++++++++++++++++++++++++++++++++++



================================================================================
Remove the following later...
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

   flecstan 01-task-good-register-execute-inside.json

This ``filename`` is in a fixed-width font.

*This is italics*

**This is bold**

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
