![logo](../doc/flecsi.png)

# FleCSI Developer Documentation

# Branch Naming Conventions

The FleCSI *master* branch is the primary development branch for the
proejct. Commits to master require that continuous integration tests
pass before they may be merged.

Please use the following naming conventions when creating branches:

* **release**\_*version*<br>
  The *release* prefix is reserved for supported FleCSI release
  branches.

* **release\_candidate**\_*version*<br>
  The *release\_candidate* prefix should be used to identify a branch that is
  being considered for release.

* **stable**\_*version*<br>
  A *stable* branch is a development or feature branch that is
  guarunteed to build and pass the FleCSI continuous integration test
  suite, but one which incorporates new features or capabilities that
  are not available in a release branch.

* **feature**\_*branch\_name*<br>
  A *feature* branch is where new development is done. However, feature
  branches are required to periodically be merged with the master
  branch.

* **bug**\_*reference*<br>
  Bug-fix branches should use the *bug* prefix, and should include
  either the name of the associated problem branch, or a reference to an
  issue number.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
