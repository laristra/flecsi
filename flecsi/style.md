<!-- CINCHDOC DOCUMENT(Developer Guide) SECTION(Style Guide) -->

# Style Guide

If not otherwise indicated, the FleCSI coding style follows the Google
C++ Style Guide.

Notable exceptions include:

* Type Names
* Function Names
* Structs vs. Classes

The exceptions are covered in the following sections.

## Guiding Principles

1. No line in a file shall exceed 80 characters!

2. If you are editing a file, maintain the original formatting unless it
violates our style guide.

3. For the most part, all names are lowercase and follow the conventions
of the C++ Standard Template Library.

4. All delimiters should be terminated with a C-style comment:

>     struct trivial_t {
>       double value;
>     }; // struct trivial_t <- This is the delimiter comment

5. Conditional and loop logic should use explicit delimiters:

>     for(size_t i(0); i<10; ++i) do_it(i); // WRONG!

> Correct way:

>     for(size_t i(0); i<10; ++i) {
>       do_it(i);
>     } // for

6. FleCSI header includes should use the full relative path from the
top-level FleCSI source directory, e.g.:

>     #include "../mesh_topology.h" // WRONG!

> Correct way:

>     #include "flecsi/topology/mesh_topology.h"

## Directory Structure 

The source code for the core FleCSI infrastructure is located in the
*top-level/flecsi* directory.

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

In general, FleCSI types should prefer *struct* to *class*. The only
real difference between the two definitions is the default access
permissions, i.e., *struct* defaults to public, and *class* defaults to
private. Using a *struct* makes it natural to put the public interface
at the beginning of the type.

## Variable Names

This is **not** an exception to the Google C++ Syle Guide! Please read the
guide and follow its conventions.

## Function & Method Formatting

Functions and methods should be formatted with the each template
parameter, return type, name, and each parameter on its own line:

    template<
      typename T1,
      typename T2,
      typename T3
    >
    return_t &
    name(
      argument1 arg1name,
      argument2 arg2name,
      argument3 arg3name
    )
    {
    } // name

Arguments should have one tab equivalent indentation.

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
aggregate data names, which use a single underscore.

### Template Parameter Names

Template parameters should use single letter, uppercase names unless
they are variadic, in which case, a lowercase *s* should be appended to
the name:

    /*!
      \tparam T The POD type.
      \tparam As A variadic list of arguments.
     */
    template<typename T, typename ... As>

In this example, *As* indicates that the parameter is plural. Developers
should avoid verbose parameter names, opting instead to use Doxygen to
document the parameter meaning as above.

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
      T two_times(T input)
      {
        return 2.0*value_;
      } // two_times

    private:

      // private member functions

      void initialize(T value)
      {
        return value + 5.0;
      } // initialize

      // private data members

      T value_;

    }; // struct my_interface_t

--------------------------------------------------------------------------------

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
