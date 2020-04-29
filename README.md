# FleCSI Papers

This is the central repository for FleCSI papers and technical reports.

# Prequisites

* CMake
* Latex
* ImageMagick

## Fedora

Installing *texlive-scheme-full* works:

```
$ dnf install texlive-scheme-full
```

## MacOS

Installing *cmake*, *texlive*, *texlive-pstricks*, and *ImageMagick* with
**MacPorts** works:
```
$ port install cmake
$ port install texlive
$ port install texlive-pstricks
$ port install ImageMagick
```

# Building a Paper

The build system for this project uses standard CMake.
In general, you can build like:
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```
If you want to avoid building all of the content in the project, you can
list the available targets by invoking:
```
$ make help
```
You can then build specific targets like:
```
$ make target
```

# Adding a New Document

To add a new paper, create a directory, e.g., *FleCSI-3.0* and create a
*CMakeLists.txt* with the following content:

```
add_latex_document(
  NAME_OF_MAIN.tex
)
```
where *NAME_OF_MAIN.tex* is the name of your master tex document.
The *add_latex_document* command takes other arguments.
For a good example, take a look at the structure of the *FleCSI-2.0*
directory.

Add a *add_subdirectory* entry to the top-level *CMakeLists.txt* file.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
