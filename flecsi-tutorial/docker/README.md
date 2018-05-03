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

# Building the Docker Container Locally

**Note: These instructions are only relevant if you want to modify or
rename the automatically generated tutorial images. The docker files in
this directory are automatically built on Docker Hub, whenever they get
modified on GitHub.  These instructions are primarily intended for
developers who are modifying the image scripts and need to test them.**

From this directory, you can build a new docker image with:
```
$ docker build -t larsitra/flecsi-tutorial:TAG -f RUNTIME .
```
where *TAG* and *RUNTIME* are replaced with a tag and a specific runtime
backend, e.g., mpi or legion. The TAG argument may be any name that you
would like to give to the image. The default is *latest*.

If you want to push images to the docker hub repository, you will need
to log in:
```
$ docker login
```
This will prompt you for a username and password. You will need to be a
member of the *laristra* organization to successfully push images to the
docker hub. Once you are logged in, you can push an image to Docker Hub
with:
```
$ docker push laristra/flecsi-tutorial:TAG
```
where TAG is the tag that you specified during the build.

In general, pushing the image manually is unnecessary because it will
automatically be rebuilt when the Docker file is updated on GitHub.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
