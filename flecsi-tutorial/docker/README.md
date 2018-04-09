# FLeCSI Tutorial Docker Container
<!--
  The above header is required for Doxygen to correctly name the
  auto-generated page. It is ignored in the FleCSI guide documentation.
-->

<!-- CINCHDOC DOCUMENT(user-guide) SECTION(tutorial) -->

# Introduction

This documentation is intended for **FleCSI developers** who need to update
or maintain the tutorial container. It is not intended for normal end
users of FleCSI. Instructions for using the FleCSI tutorial docker
container are included in the tutorial documentation.

# Installing Docker

The Docker daemon is available for a variety of platforms from the
[Docker Website](https://www.docker.com).

# Building the Docker Container

From this directory, you can build a new docker image with:
```
$ docker build -t larsitra/flecsi-tutorial:latest .
```
If you want to push images to the docker hub repository, you will need
to log in:
```
$ docker login
```
This will prompt you for a username and password. You will need to be a
member of the *laristra* organization to successfully push images to the
docker hub.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
