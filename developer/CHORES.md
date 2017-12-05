# FleCSI Developer Chores

This document describes various chores that may be assigned to FleCSI
developers and how to complete them. For the most part, these chores are
mundane code maintenance tasks that need to be done from time to time to
keep the code clean.

## Task: Update CMakeLists

Update CMakeLists.txt files to reflect current header and source
files. This is important because it affects what gets installed and
which set of files are *active* for a given build.

**What you have to do:**<br>  
Go through the subdirectories of *top-level/flecsi* and update the
CMakeLists.txt files in each of the namespace subdirectories to reflect
the actual files that are there.

**Tools that help you:**<br>  
There is a script in *top-level/tools* that will locate all of the
header and source files under a subdirectory. The script is called
*find_sources*. **NOTE that the script ignores test subdirectories as
these are not part of the distribution.** Consider the following
example:

```
$ cd top-level/flecsi/execution
$ ../../find_sources
Searching for source files in /home/bergen/devel/laristra/flecsi/flecsi/execution...

Headers:

common/execution_state.h
common/function_handle.h
common/launch.h
common/processor.h
context.h
default_driver.h
execution.h
function.h
kernel.h
legion/context_policy.h
legion/execution_policy.h
legion/finalize_handles.h
legion/future.h
legion/helper.h
legion/init_args.h
legion/init_handles.h
legion/internal_field.h
legion/internal_index_space.h
legion/internal_task.h
legion/legion_tasks.h
legion/mapper.h
legion/registration_wrapper.h
legion/runtime_driver.h
legion/runtime_state.h
legion/task_epilog.h
legion/task_prolog.h
legion/task_wrapper.h
mpi/context_policy.h
mpi/execution_policy.h
mpi/finalize_handles.h
mpi/future.h
mpi/runtime_driver.h
mpi/task_epilog.h
mpi/task_prolog.h
mpi/task_wrapper.h
task.h

Sources:

driver_initialization.cc
legion/context_policy.cc
legion/legion_tasks.cc
legion/runtime_driver.cc
legion/runtime_main.cc
mpi/context_policy.cc
mpi/runtime_driver.cc
mpi/runtime_main.cc
```

Using this information, you can update the CMakeLists.txt file to
accurately reflect the actual files in the subdirectory.

**NOTE to take care that you follow the file conventions in
CMakeLists.txt with respect to formatting and runtime sections, e.g.,
you cannot just cut and paste the output into the CMakeLists.txt file:
you must add files under the correct runtime section.** 

**NOTE that you must also make decisions about which files should
actually be in the distribution. Some of the files in the subdirectory
may need to be removed or updated. If you don't know, ask the last
person who modified the file!**

## Task: Update FleCSI Branding & Copyright

Update distribution file *headers* and *footers* with the correct
branding and copyright information. The *header* contains the FleCSI
ASCII-art branding and copyright, while the *footer* contains formatting
options. Generally, new files that are added to the project may not have
the branding, although they must have the stubs for it.

The header and footer stubs are special comments that identify them as
such. For example, headers in a C++ header file have the form:

```
/*~------------------------ ... -------------------~*  // This begins a replaceable section with the id 1
 *~------------------------ ... -------------------~*/ // This ends a replacement section with the id 1

/*~------------------------ ... ------------------~-*  // This begins a replaceable section with the id 2
 *~------------------------ ... ------------------~-*/ // This ends a replacement section with the id 2

/*~------------------------ ... ------------------~~*  // This begins a replaceable section with the id 3
 *~------------------------ ... ------------------~~*/ // This ends a replacement section with the id 3

/*~------------------------ ... -----------------~--*  // This begins a replaceable section with the id 4
 *~------------------------ ... -----------------~--*/ // This ends a replacement section with the id 4
```

This is a C-style comment that includes a *tilde* after the beginning
comment characters, and a set of *tildes* at the end of the comment
line. The initial tilde identifies the line as the start of a
replaceable section, while the combination of *-* and *~* characters at
the end of the line are interpreted as binary characters, e.g., *--~~*
is interpreted as the binary number *0011* or *3* in decimal. These
identifiers correspond to the decimal-numbered replacement content in
*top-level/config/brand.py* that ccli uses to update the branding.

Currently, C/C++ files use *1* for a header file *header*, and *2* for a
header file *footer*, while source files use *3* for a *header* and *4*
for a *footer*. Other file types generally use *1* and *2* for the
*header* and *footer*, respectively.

**What you have to do:**<br>  
Check that the source and header files under *top-level/flecsi* have the
correct header and footer stubs (don't worry about what is inside of the
stubs, this will be replaced). Run *ccli* to update the branding as in
the example below.

**NOTE that the header and footer branding and copyright information are
in the file *top-level/config/brand.py*. This can be updated as needed.
However, the copyright date should never change: the copyright is from
the original year of the copyright assertion regardless of the current
year.**

**Tools that help you:**<br>  
Assuming that all of the files in *top-level/flecsi* have the correct
header and footer stubs, you can update them using the Cinch
command-line tool (ccli) from the Cinch Utils project **(this gets
installed with the FleCSI Third-Party Libraries project)**.

```
% cd top-level
% ccli brand -b config/brand.py cc flecsi
```

This will update the header and footers of all source and header files
under the *flecsi* subdirectory. The arguments to ccli indicate the
service (brand), the brand configuration file (-b config/brand.py), the
type of file to update (cc for C/C++ source and header files), and the
top-level subdirectory to recurse (flecsi).

**NOTE that branding may be applied to other file types, including CMake
and Python files. When you do this chore, you should also update these
file types as well using the appropriate modifications to the example
command.**



<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
