# Documentation Developer Guide

This section attempts to describe the FleCSI documentation system and
setup to make it easier for developers to navigate.

## Doxygen

The primary Doxygen configuration file is in the *top-level/doc*
subdirectory in the main FleCSI project [here](../doc/doxygen.conf.in).
There are several additional files that are used by doxygen in this
directory:

[doxygen.conf.in](../doc/doxygen.conf.in)  
Repeated from above, this is the priamry Doxygen configuration file for
FleCSI. **Pleaes note that this is a *.in* file, which will be processed
by CMake to generate the actual doxygen.conf file. Keep this in mind
when making changes to the configuration.**

**More notes about doxygen.conf.in**  
* The project name and version are generated automatically using our
  CMake configuration. It is possible to specify a static version number
  in the CMake config.

* Extra input directories are specified under *INPUTS*. These are
  important because they identify additional sources that are not part
  of the main FleCSI code base, e.g., *top-level/auxilliary* and
  *top-level/cinch/logging* identify paths to markdown content that
  should be included as part of the Doxygen build. **Many of the files
  contained in these directories are used to generate content for the
  Doxygen *Related Pages* section.**

* Markdown files are excluded from the main FleCSI code base. This is
  because much of the user and developer guide documentation for FleCSI
  is maintained as markdown files. These are not intended to be included
  in the Doxygen documentation. Exclusions are defined by the
  *EXCLUDE_PATTERNS* keyword.

* The images directory for Doxygen is located in
  *top-level/doc/doxygen/images*.

* We use the *USE_MDFILE_AS_MAINPAGE* keyword to set the mainpage source
  to *top-level/doc/doxygen-mainpage.md*. Similarly, *HTML_HEADER* and
  *HTML_EXTRA_FILES* are used to set the
  *top-level/doc/doxygen-header.html* and
  *top-level/doc/flecsi-favicon.ico* files.

* Several preprocessor values are pre-defined so that documentation is
  correctly generated for macro interfaces. These are defined under the
  *PREDEFINED* keyword.

[doxygen-header.html](../doc/doxygen-header.html)  
This file is used to customize the look of the FleCSI logo and favicon.

**Notes about doxygen-header.html**  
* This was difficult to get right. Please don't change it unless you
  have a very good reason, and you are willing to take the time to
  understand what it is doing.

[doxygen-mainpage.md](../doc/doxygen-mainpage.md)  
This file is used to control the content and layout of the main Doxygen
page for FleCSI.

**Notes about doxygen-mainpage.md**  
* This file is easy to modify, and people should feel free to update it
  and add content as needed. Please *do* look at the output after you
  modify something to make sure that it still looks nice!

[doxygen-setup.h](../doc/doxygen-setup.h)  
This file is used to configure the primary Doxygen groups for the FleCSI
project. Groups are used to populate the modules page of Doxygen, and
offer quick access to the high-level components of FleCSI, which
basically correspond to the FleCSI namespaces.

**Notes about doxygen-setup.h**  
* Please follow the existing naming conventions and style, but feel free
  to update the groups as namespaces are added or removed from FleCSI.

## User & Developer Guides

The FleCSI user and developer guides use a combination of markdown and
latex to generate content. This content can be compiled using the Cinch
command-line utility and [Pandoc](http://johnmacfarlane.net/pandoc). The
Cinch command-line utility is automatically built as part of the FleCSI
third-party library package.

The configuration files for this type of documentation are split between
the *top-level/config* and *top-level/doc* directories. Build
configuration and document targets are defined in
*top-level/config/documentation.cmake*, while style and auxiliary files
are in *top-level/doc*. The actual documentation content is distributed
throughout the FleCSI source tree under *top-level/flecsi*. The
following files are important to the configuration:

[documentation.cmake](../config/documentation.cmake)
This file defines options and targets for various documentation targets
through the Cinch/CMake build system.

**Notes about documentation.cmake**
* Pandoc options are passed through ccli to Pandoc, and allow
  customization of the output. Documentation for the various Pandoc
  options can be obtained at their
  [website](http://johnmacfarlane.net/pandoc).

* Documentation targets are defined by calls to *cinch_add_doc*. Cinch
  documentation is available on the Cinch github
  [repository](https://github.com/laristra/cinch).

[flecsi_dg.py](../doc/flecsi_dg.py)
This file specifies layout and configuration options for the FleCSI
Develoepr Guide. In particular, the *sections* dict entry determines the
ordering of the sections of the resulting document. Sections that are
detected in the documentation source that are not listed will have an
arbitrary ordering.

**Notes about documentation.cmake**
* The *document* dictionary entry specifies the identifier that will be
  matched to look for source for this target.

[flecsi_dg_header.tex.in](../doc/flecsi_dg_header.tex.in)
This file allows customization of the FleCSI Developer Guide.

**Notes about flecsi_dg_header.tex.in**
* This is an input file that will be processed by CMake to include the
  project name and version.

[flecsi_dg_title.tex](../doc/flecsi_dg_title.tex)
This file allows customization of the FleCSI Developer Guide. In
particular, this file lists the project contributors and
acknowledgements. New contributors should be added as necessary.

[flecsi_ug.py](../doc/flecsi_ug.py)
This file is analogous to flecsi_dg.py, but for the FleCSI User Guide.

[flecsi_ug_header.tex.in](../doc/flecsi_ug_header.tex.in)
This file is analogous to flecsi_ug_header.tex.in, but for the FleCSI
User Guide.

[flecsi_ug_title.tex](../doc/flecsi_ug_title.tex)
This file is analogous to flecsi_dg_title.tex, but for the FleCSI User
Guide.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
