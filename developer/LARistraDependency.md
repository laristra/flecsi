```![logo](../doc/flecsi.png)

# LA Ristra Repo Summary

* `flecsi-buildenv`: FleCSI's build environment using docker for Travis-CI
* `flecsi-third-party`: Third party libraries for FleCSI and other project
* `flecsi`: A set of computational science infrastructure tools
  designed to aid in the implementation of multi-physics application development
* `flecsph`: A distributed parallel implementation of SPH problem using FleCSI framework
* `libristra`: A set of support utilities for other LA Ristra projects
* `flecsi-sp`: A set of various specializations for use with the FleCSI
  core programming system
* `flecsale`: A project for studying multi-phase continuum dynamics problems with
  different runtime environments using FleCSI's framework
* ` portage-buildenv`: Portage's build environment using docker for Travis-CI
* `tangram`: A framework for interface reconstruction in computational physics applications
* `portage`:

# LA Ristra Dependency Graph

1. `flecsi-buildenv`->`flecsi-third-party`->`flecsi`->`libristra`->`flecsi-sp`->`flecsale`

2. `portage-buildenv`->`tangram`->`portage`

