# FleCSI: C++ Language Extensions
<!--
  The above header is required for Doxygen to correctly name the
  auto-generated page. It is ignored in the FleCSI guide documentation.
-->

<!-- CINCHDOC DOCUMENT(user-guide) SECTION(flecsi-language-extensions) -->

# Kitsune Project

The FleCSI C++ language extensions are implemented as part of the
Kitsune project. Kitsune provides support for compiler-aided tools
development:

* **Static Analysis & Optimization**

  Kitsune provides an interface for defining AST visitors that recognize
  user-defined regular expressions and perform static analysis or
  supercede normal compilation to optimize specific code patterns.
  
* **Domain-Specific Language (DSL) Development**

  Kitsune also provides an interface for defining custom language
  extensions.

Access to Kitsune is currently restricted to yellow-network users at
LANL. If you need access, please contact Pat McCormick
([pat@lanl.gov](mailto:pat@lanl.gov)).

Once you have access to the repository, you can obtain the source and
build like:
```
$ git clone git@gitlab.lanl.gov:pat/kitsune.git
$ cd kitsune
$ mkdir build
$ cmake -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 -DCMAKE_INSTALL_PREFIX=/path/to/install ../llvm
$ make install
```
If you are a developer, you may want to specify *Debug* for the build
type.

*Kitsune* is the Japanese word for *fox*.

# FleCSI C++ Language Extensions

Currently, FleCSI provides only the *forall* keyword extension to C++
for defining fine-grained, data-parallel operations on logically shared
data regions.
```
void update(mesh<ro> m, field<rw, rw, ro> p) {
  forall(auto c: m.cells(owned)) {
    p(c) += 1.0;
  } // forall
}
```

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
