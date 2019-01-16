.. |br| raw:: html

   <br />

Branch Naming Conventions
=========================

The *master* branch is the primary development branch for the
proejct. Commits to master require that continuous integration tests
pass before they may be merged.

Please use the following naming conventions when creating branches:

* **release**/*major.minor* |br|
  The *release* prefix is reserved for supported release
  branches. Actual releases and pre-releases will be identified using
  tags.
  
  Tagging conventions for the release branch:

  * Patch numbers for a release will be specified in the tag, such
    that a given release will have the form *major.minor.patch*.  

  * Alpha, Beta, or Release Candidates |br|
    Any pre-release version should include the name of the pre-release,
    e.g., alpha, beta or rc, and the version of the pre-release at the
    end of the normal release version. Here are some examples:

    * Release candidate 2 of release 1.0.0 would be *1.0.0-rc.2*.
    * Alpha release 1 of release 1.0.0 would be *1.0.0-alpha.1*.

  * Must branch from: **master**

* **stable**/*branch\_name* |br|
  A *stable* branch is a development or feature branch that is
  guaranteed to build and pass the FleCSI continuous integration test
  suite, but one which incorporates new features or capabilities that
  are not available in a release branch. This label should not be used
  for a branch that is intended to become a release candidate (Use
  *release* instead.)

* **feature/username**/*branch\_name* or **feature**/*branch\_name* |br|
  A *feature* branch is where new development is done. However, master
  should be merged periodically into a feature branch. If the branch is
  to primarily be developed by an individual, it should include the
  *username* as part of the branch.

  * Must branch from: **master**
  * Must merge to: **master**

* **fix**/*reference* |br|
  Bug-fix branches should use the *fix* prefix, and should include
  a reference number or name, e.g., issue number or the related release
  tag.

  * Must branch from: **release branch**
  * Must merge to: **master *and* release branch**

# Managing Branches
Please use the following sections as a guide to creating and managing
branches.

Feature Branches
****************

All new development is done on a feature branch.

Creating
++++++++

To create a new feature branch:

.. code-block:: console

  # Make sure that you are up-to-date on the master branch
  $ git checkout master
  $ git pull

  # If the project uses submodules you should also make sure
  # that they are up-to-date
  $ git submodule update --recursive

  # Create a new feature branch
  $ git checkout -b feature/name

Once you make a change to your feature branch, you can push it to the
remote--**after considering classification and export control
implications**--using the following:

.. code-block:: console

  git push --set-upstream origin feature/name

Merging
+++++++

When you are done developing a feature, you should merge your feature
branch with master using the *no fast-forward (--no-ff)* flag to git:

.. code-block:: console

  # Make sure that you are up-to-date on the master branch
  $ git checkout master
  $ git pull

  # Merge your branch into master
  $ git merge --no-ff feature/name

The *--no-ff* flag preserves information about the existence of the
feature branch after it is removed.

After you have merged your changes into **master**, you should submit a
pull request. This can be done on the [github
website](https://github.com/laristra).

Once the pull request has been accepted, you can safetly remove the
feature branch:

.. code-block:: console

  # Remove the local branch
  $ git branch -d feature/name

  # Remove the remote branch
  $ git push origin --delete feature/name

After the feature branch is removed, it will no longer appear in the
commit messages for the project. However, because we used *--no-ff*, the
structure of the branch will be retained, e.g., try *git log --graph*.
The name of the branch will also appear in the message at the merge
point, provided that you used the default message provided by git.

Release Branches
****************

A release branch is used to prepare and maintain supported releases.

Creating
++++++++

Creating a release branch is similar to creating a feature branch,
except that the naming convention for release branches is more
restrictive, i.e., the branch name must have the form
*release/major.minor*:

.. code-block:: console

  # Make sure that you are up-to-date on the master branch
  $ git checkout master
  $ git pull

  # If the project uses submodules you should also make sure
  # that they are up-to-date
  $ git submodule update --recursive

  # Create a new release branch
  $ git checkout -b release/major.minor

Finishing
+++++++++

**Properly completing work on a release branch in preparation for a
supported release requires a combination of command-line and server-side
operations. These instructions assume that the server is GitHub. These
directions also assume that you have completed a *CHANGELOG* of the
changes for the new release.**

1. When you are ready to create an actual release on a release branch, you
need to create an annotated tag with the release version:

.. code-block:: console

  $ git tag -a -m "Release major.minor.patch" major.minor.patch

The *major.minor* part of the version must be consistent with the branch
name. The patch level should be sequential, and should use the next
integer value from the last tag on the branch, e.g.:

* 1.0: Initial release tag
* 1.0.1: First patch tag
* 1.0.2: Second patch tag

*Tags should be consistent with the branch name, but they are unrelated
to the number of commits on the branch.*

2. Once you have created the tag, you should go to the GitHub project site,
and click on the *releases* link. This will take you to the release page
for the project.

3. Next, you should click on the *Draft a new release* button. This will
bring up a page with several edit fields.

4. In the *Tag version* edit field, enter the tag that you created in the
previous steps.

5. Create a release title of the form *Release major.minor.patch* (*patch*
should only be used when appropriate.)

6. Cut and paste the information from your *CHANGELOG* file into the
*Describe this release* field, and make any other changes or additions
that are appropriate.

7. Once you have completed filling out the information, press the *Publish
release* button.

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
