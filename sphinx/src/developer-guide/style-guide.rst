Style Guide
===========

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

.. code-block:: cpp

  struct trivial_t {
    double value;
  }; // struct trivial_t <- This is the delimiter comment


* Conditional and loop logic should use explicit delimiters:

.. code-block:: cpp

  for(size_t i{0}; i<10; ++i) do_it(i); // WRONG!

> Correct way:

.. code-block:: cpp

  for(size_t i{0}; i<10; ++i) {
    do_it(i);
  } // for

* FleCSI header includes should use the full relative path from the
  top-level FleCSI source directory, e.g.:

.. code-block:: cpp

  #include "../mesh_topology.h" // WRONG!

> Correct way:

.. code-block:: cpp

  #include <flecsi/topology/mesh_topology.h>

* FleCSI header guard names should use the partial relative path. They
  should be lower case, and they should be appended with an underscore
  h (\_h). The endif statement should be appended with a C++-style
  comment repeating the guard name. As an example, if the header file is
  in **'flecsi/partition/dcrs.h'**, its guard entry should look like:

.. code-block:: cpp

  #ifndef HEADER_HH // WRONG!

> Correct way:

.. code-block:: cpp

  #ifndef partition_dcrs_h
  #define partition_dcrs_h

  // Code...

  #endif // partition_dcrs_h

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

.. code-block:: cpp

  for ( size_t i{0}; i<N; ++i ) { // WRONG!
  } // for

  if ( condition ) {} // WRONG!

Correct way:

.. code-block:: cpp

  for(size_t i{0}; i<N; ++i) {
  } // for

  if(condition) {
  } // if

### Braced Initialization

Braced initialization *should* use spaces:

.. code-block:: cpp

  std::vector<size_t> vec = {1,2,3}; // WRONG!

Correct way:

.. code-block:: cpp

  std::vector<size_t> vec = { 1, 2, 3 };

### Function & Method Formatting

Function and method invocations should not insert spaces:

.. code-block:: cpp

  my_function ( argument1, "argument 2" ); // WRONG!

Correct way:

.. code-block:: cpp

  my_function(argument1, "argument 2");

### More on spaces...

Never put empty characters at the end of a line!

### Function & Method Formatting (prototypes)

Functions and methods should be formatted with each template parameter,
the scope (static, inline), the return type, the name, and each
signature parameter on its own line:

.. code-block:: cpp

  template<
    typename TYPENAME1,
    typename TYPENAME2,
    typename TYPENAME3
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

.. code-block:: cpp

  // Trivial template and signature
  template<typename TYPENAME>
  return_t &
  name()
  {
  } // name

  // Trivial template
  template<typename TYPENAME>
  return_t &
  name(
    argument1 arg1name,
    argument2 arg2name
  )
  {
  } // name

  // Trivial signature
  template<
    typename TYPENAME1,
    typename TYPENAME2
  >
  return_t &
  name()
  {
  } // name

## Type Names

FleCSI follows a C-style naming convention of all lower-case letters
with underscores. Fully-qualified types should also append an
underscore lower-case *t*, i.e., \_t to the end of the type name:

.. code-block:: cpp

  struct my_type_t
  {
    double value;
  }; // struct my_type_t

Type definitions should be terminated with a C-style comment indicating
the type name.

### Template Type Naming

For templated types, use a double underscore for the unqualified type:

.. code-block:: cpp

  my_template_type_u


This allows the type to be fully qualified using the normal type naming
convention listed above, e.g.:

.. code-block:: cpp

  // Unqualified type definition
  template<typename TYPENAME>
  struct my_template_type_u
  {
    TYPENAME value;
  }; // struct my_template_type_u

  // Fully qualified type
  using my_template_type_t = my_template_type_u<double>;

The double underscore was chosen so that it does not conflict with
member variable names, which use a single underscore.

### Template Parameter Names

Template parameters should use descriptive, uppercase names:

.. code-block:: cpp

  //------------------------------------------------------------------------//
  //! @tparam TYPENAME The POD type.
  //! @tparam ARGUMENTS A variadic list of arguments.
  //------------------------------------------------------------------------//

  template<typename TYPENAME, typename ... ARGUMENTS>

## Error & Exception Handling

Use assertions and static assertions to assert things that must be true:

.. code-block:: cpp

  template<size_t FROM_DIMENSION>
  connectivity &
  get(
    size_t to_dim
  )
  {
    static_assert(FROM_DIMENSION <= DIMENSION, "invalid from dimension");
    assert(to_dim <= DIMENSION && "invalid to dimension");
    return conns_[FROM_DIMENSION][to_dim];
  } // get

In this case, we can verify that the from dimension (template parameter
FROM_DIMENSION) is in bounds using a static assertion. We need to use a
dynamic assertion to check the to dimension (to_dim) since it is passed
as an argument to the method. In both cases, the assertions must be true
for the code to not be broken, i.e., if the assertion is not true, there
is a bug! Fix it!

Use exception handling to catch exceptional situations, i.e., when a
condition for the correct functioning of the code is not met. An
exception may be caught and the program can recover from it:

.. code-block:: cpp

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

Appendix A: Style Examples
==========================

.. code-block:: cpp

  //------------------------------------------------------------------------//
  //! The my_interface_t type provides an example of a correctly formatted
  //! and documented type.
  //------------------------------------------------------------------------//

  template<
    typename TYPENAME
  >
  struct my_interface_t
  {

    //----------------------------------------------------------------------//
    //! Construct a my_interface_t with value.
    //----------------------------------------------------------------------//

    my_interface_t(
      TYPENAME value
    )
    :
      value_(initialize(value))
    {}

    //----------------------------------------------------------------------//
    //! This method provides an example of a member function.
    //!
    //! @param input The input value to the method.
    //!
    //! @return A modified value of type TYPENAME.
    //----------------------------------------------------------------------//

    TYPENAME
    two_times(
      TYPENAME input
    )
    {
      return 2.0*value_;
    } // two_times

  private:

    // private member functions

    void
    initialize(
      TYPENAME value
    )
    {
      return value + 5.0;
    } // initialize

    // private data members

    TYPENAME value_;

  }; // struct my_interface_t

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
