# Git Cheat Sheet

# Branch Creation

This section shows how to create a branch from your current local
changes, push it to the origin, and set the upstream branch location.

```
$ git checkout -b new_branch_name
$ git push -u origin new_branch_name
```

# Prune Directories

This section shows how to move a subdirectory from one project to
another, while preserving commit history.

Isolate the subdirectory you want to move:
```
$ git clone project_has_subdirectory
$ cd project_has_subdirectory
$ git remote rm origin
$ git filter-branch --subdirectory-filter subdirectory -- --all
$ mkdir subdirectory
$ mv * subdirectory
$ git add .
$ git commit 
```

Merge isolated subdirectory into the new project:
```
$ git clone project_gets_subdirectory
$ cd project_gets_subdirectory
$ git remote add has_subdirectory_branch project_has_subdirectory 
$ git pull has_subdirectory_branch master --allow-unrelated-histories
$ git remote rm has_subdirectory_branch
```

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
