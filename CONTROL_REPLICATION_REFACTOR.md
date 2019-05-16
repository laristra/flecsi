# Refactor Steps

1. Swap *index* and *single*

   **Note:** This is only an interface change.

   (Until we actually have control replication, this will mean that the
    behavior of single and index will be swapped)

   - For MPI, index will be a function call, and single will use
     std::async
   - For Legion, the new assignments are consistent with the control
     replication meanings of these terms

2. Update code to move region requirements from regions to partitions.
   This will probably require new logic for representing partitions in
   FleCSI that are consistent with the current Legion interface.

3. Change runtime_driver.cc so that the information that we need in the
   context is available in the right place.
   
   - What context information needs to be in top-level task, and what
     needs to be in the index sub-tasks.

   - Partition information will be needed on the index task instances.

4. At some point, we will replace single launches with index launches
   (internally).

5. Need to replace current copy launchers with index copy launchers
   (shared to ghosts).

Assignments: Look at dependent partitioning (get Wei's example)


<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
