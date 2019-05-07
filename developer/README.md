![logo](../doc/flecsi.png)

# FleCSI Developer Documentation

# Branch Naming Conventions

FleCSI uses [semantic versioning](https://semver.org), and follows the
conventions established by the Cinch project. A full description of
versioning and branch management is
[here](https://github.com/laristra/cinch/blob/master/developer/VERSIONING.md)
**(If you are creating or managing branches, make sure that you read the
Cinch documentation!)**

# FleCSI Developer Chores

This section describes various chores that may be assigned to FleCSI
developers and how to complete them. For the most part, these chores are
mundane code maintenance tasks that need to be done from time to time to
keep the code clean.

## Task: Branch Cleanup

Remove old/unused/merged branches from the repository. In particular,
remove any branches that fail to comply with our branch naming
conventions.

**What you have to do:**<br>
Create a list of branches with the originator's email using:
```
$ git for-each-ref --format='%(committerdate) %09 %(authorname) %09 %(refname)' | sort -k5n -k2M -k3n -k4n > branches
```
Remove branches that you are sure about, e.g., your own. Contact
developers who have non-compliant branches. Non-compliant branches must
either be renamed or removed.

**Tools that help you:**<br>

Git command to rename the branch you are on:
```
$ git branch -m new-name
```

Git command to rename a branch you are not on:
```
$ git branch -m old-name new-name
```

Git command to push changes to origin
```
$ git push origin :old-name new-name
```

Git command to reset upstream for local branch
```
$ git push origin -u new-name
```

Git command to remove local branch:
```
$ git branch -d branch
```
or,
```
$ git branch -D branch
```
The *-D* flag is an alias for *--delete --force*, and allows remoaval of
branches that have not been merged in its upstream branch.

Git command to remove a remote branch:
```
$ git push <remote-name> --delete <branch-name>
```

**NOTE:** When renaming a branch, you will likely have to reset the
upstream:
```
$ git branch --set-upstream-to=origin/<new-branch-name>
```

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

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
