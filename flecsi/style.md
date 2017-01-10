<!-- CINCHDOC DOCUMENT(Developer Guide) SECTION(Style Guide) -->

# Style Guide

If not otherwise indicated, the FleCSI coding style follows the Google
C++ Style Guide.

Notable exceptions include:

* C++ Exception Handling
* Type Names
* Function Names
* Variable Names (in some cases)
* Structs vs. Classes
* Formatting

The exceptions are covered in the following sections.

## Guiding Principles

* No line in a file shall exceed 80 characters!

* If you are editing a file, maintain the original formatting unless it
violates our style guide. If it does, fix it!

* For the most part, all names are lowercase and follow the conventions
of the C++ Standard Template Library.

* All delimiters should be terminated with a C++-style comment:

>     struct trivial_t {
>       double value;
>     }; // struct trivial_t <- This is the delimiter comment

* Conditional and loop logic should use explicit delimiters:

>     for(size_t i(0); i<10; ++i) do_it(i); // WRONG!

> Correct way:

>     for(size_t i(0); i<10; ++i) {
>       do_it(i);
>     } // for

* FleCSI header includes should use the full relative path from the
top-level FleCSI source directory, e.g.:

>     #include "../mesh_topology.h" // WRONG!

> Correct way:

>     #include "flecsi/topology/mesh_topology.h"

* FleCSI header guard names should use the partial relative path. They
  should be lower case, and they should be appended with an underscore
  h (\_h). The endif statement should be appended with a C++-style
  comment repeating the guard name. As an example, if the header file is
  in **'flecsi/partition/dcrs.h'**, its guard entry should look like:

>     #ifndef HEADER_HH // WRONG!

> Correct way:

>     #ifndef partition_dcrs_h
>     #define partition_dcrs_h
>
>     // Code...
>
>     #endif // partition_dcrs_h

## Directory Structure 

The source code for the core FleCSI infrastructure is located in the
*top-level/flecsi* directory. For the most part, the subdirectories of
this directory correspond to the different namespaces in the core
infrastructure. Each of these subdirectories must contain a valid
CMakeLists.txt file. However, none of their children should have a
CMakeLists.txt file, i.e., the build system will not recurse beyond the
first level of subdirectories. Developers should use relative paths
within a CMakeLists.txt file to identify source in subdirectories.

Unit test files should be placed in the *test* subdirectory of each
namespace subdirectory. By convention, developers should not create
subdirectories within the test subdirectory.

## Names and Order of Includes

This is **not** an exception to the Google C++ Syle Guide! Please read the
guide and follow its conventions.

## Struct & Class Conventions

This section describes the basic conventions used in defining structs
and classes. For some examples of correctly formatted type definitions,
please look at Appendix A.

### Structs vs. Classes

The public interface should appear at the top of the type definition
when possible.

According to many sources, developers should prefer *class* to *struct*.
The only real difference between the two definitions is the default
access permissions, i.e., *struct* defaults to public, and *class*
defaults to private.

For FleCSI, we mostly follow the Google C++ Style Guide, which prefers
*class* over *struct* unless the type is intended to offer direct access to
its data members. An exception to this rule is for metaprogramming
types. Many of the types used in metaprogramming do not have any data
members (they only provide type definitions). In this case, developers
should prefer *struct* over *class*.

Like the Google C++ Style Guide convention, developers should always use
a struct for type definitions that do not have restricted access
permissions.

## Variable Names

This is *mostly* not an exception to the Google C++ Style Guide, so you
should read the guide and understand its conventions for variable names.
In FleCSI, we follow those conventions for classes, and for structs that
do not have restricted access permissions. For structs that **do** have
access permissions, we follow the Google C++ Style Guide convention for
classes.

## Formatting

### Control Flow

Control flow operations should not insert spaces:

    for ( size_t i(0); i<N; ++i ) { // WRONG!
    } // for

    if ( condition ) {} // WRONG!
    

Correct way:

    for(size_t i(0); i<N; ++i) {
    } // for

    if(condition) {
    } // if

### Braced Initialization

Braced initialization *should* use spaces:

   std::vector<size_t> vec = {1,2,3}; // WRONG!

Correct way:

   std::vector<size_t> vec = { 1, 2, 3 };

### Function & Method Formatting

Function and method invocations should not insert spaces:

    my_function ( argument1, "argument 2" ); // WRONG!

Correct way:

    my_function(argument1, "argument 2");

### More on spaces...

Never put empty characters at the end of a line!

### Function & Method Formatting (prototypes)

Functions and methods should be formatted with each template parameter,
the scope (static, inline), the return type, the name, and each
signature parameter on its own line:

    template<
      typename T1,
      typename T2,
      typename T3
    >
    static
    return_t &
    name(
      argument1 arg1name,
      argument2 arg2name,
      argument3 arg3name
    )
    {
    } // name

Parameters should have one tab equivalent indentation. The convention is
to define a tab as two spaces. FleCSI source files have formatting hints
for Vim and Emacs to expand tabs to this number of spaces.

**NOTE:** If the parameters to a function or method definition are
trivial, i.e., there is only a single template parameter, **or** there
are no signature parameters, it is not necessary to break up the
arguments:

    // Trivial template and signature
    template<typename T>
    return_t &
    name()
    {
    } // name

    // Trivial template
    template<typename T>
    return_t &
    name(
      argument1 arg1name,
      argument2 arg2name
    )
    {
    } // name

    // Trivial signature
    template<
      typename T1,
      typename T2
    >
    return_t &
    name()
    {
    } // name

## Type Names

FleCSI follows a C-style naming convention of all lower-case letters
with underscores. Fully-qualified types should also append an
underscore lower-case *t*, i.e., \_t to the end of the type name:

    struct my_type_t
    {
      double value;
    }; // struct my_type_t

Type definitions should be terminated with a C-style comment indicating
the type name.

### Template Type Naming

For templated types, use a double underscore for the unqualified type:

    my_template_type__

This allows the type to be fully qualified using the normal type naming
convention listed above, e.g.:

    // Unqualified type definition
    template<typename T>
    struct my_template_type__
    {
      T value;
    }; // struct my_template_type__

    // Fully qualified type
    using my_template_type_t = my_template_type__<double>;

The double underscore was chosen so that it does not conflict with
member variable names, which use a single underscore.

### Template Parameter Names

Template parameters should use single letter, uppercase names unless
they are variadic, in which case, a lowercase *s* should be appended to
the name:

    /*!
      \tparam T The POD type.
      \tparam As A variadic list of arguments.
     */
    template<typename T, typename ... As>

In this example, the *s* in *As* indicates that the parameter is plural.
Developers should avoid verbose parameter names, opting instead to use
Doxygen to document the parameter meaning (as shown above).

## Error & Exception Handling

Use assertions and static assertions to assert things that must be true:

    template<size_t FD>
    connectivity &
    get(
      size_t to_dim
    )
    {
      static_assert(FD <= D, "invalid from dimension");
      assert(to_dim <= D && "invalid to dimension");
      return conns_[FD][to_dim];
    } // get

In this case, we can verify that the from dimension (template parameter
FD) is in bounds using a static assertion. We need to use a dynamic
assertion to check the to dimension (to_dim) since it is passed as an
argument to the method. In both cases, the assertions must be true for
the code to not be broken, i.e., if the assertion is not true, there is
a bug! Fix it!

Use exception handling to catch exceptional situations, i.e., when a
condition for the correct functioning of the code is not met. An
exception may be caught and the program can recover from it:

    try {
      type_t * t = new type_t;
    }
    catch(std::bad_alloc & e) {
      // do something because the allocation failed...
    }
    catch(...) {
      // default exception
    } // try

In many cases, exception handling should be reserved for interfaces that
can be called by a developer. Internal interfaces should use assertions
to identify bugs.

## Summary

Failure to respect the FleCSI style guidelines will lead to public
ritualized torture and eventual sacrifice...

--------------------------------------------------------------------------------

<!-- CINCHDOC DOCUMENT(Developer Guide) SECTION(Appendix A) -->

\pagebreak

# Appendix A: Style Examples

    /*!
      \struct my_interface_t my_interface.h
      \brief my_interface_t provides an example of a correctly formatted
        and documented type.
     */
    template<typename T>
    struct my_interface_t
    {

      /*!
        Constructor.

        \param value Initialization value...
       */
      my_interface_t(T value) : value_(initialize(value)) {}

      /*----------------------------------------------------------------------*
       * Section delimiter.
       *----------------------------------------------------------------------*/

      /*!
        This method provides an example of a member function.

        \param input The input value to the method.

        \return A modified value of type T.
       */
      T
      two_times(
        T input
      )
      {
        return 2.0*value_;
      } // two_times

    private:

      // private member functions

      void
      initialize(
        T value
      )
      {
        return value + 5.0;
      } // initialize

      // private data members

      T value_;

    }; // struct my_interface_t

--------------------------------------------------------------------------------

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
