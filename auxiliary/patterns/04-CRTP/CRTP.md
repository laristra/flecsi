# Design Pattern: Curiously Recurring Template Pattern (CRTP)

More generally known as F-bound polymorphism, this idiom was formalized
in the 1980s. The name *CRTP* was coined by Jim Coplien in 1995. The
wikipedia entry is
[here](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern).

The general form of the pattern involves a templated base class and a
derived class that specializes the base class with its own type:

```
    template<
      typename CRTP_TYPE
    >
    struct base_u {
    }; // struct base_u

    struct derived_u : public base_u<derived_u> // This is the important bit.
    {
    }; // struct derived_u
```

One use of this pattern in FleCSI is for static polymorphism. In
particular, by statically casting the base type to the derived type, we
can allow the derived type to specialize the behavior of the base class
interface. In the following code example, *tuple_walker_u* is the base
class for a derived type identified by *CRTP_TYPE*:

```
    template<
      typename CRTP_TYPE
    >
    struct tuple_walker_u
    {
      // In this example, we employ a helper type to walk a tuple and
      // apply the handler method defined by the derived type.
      template<
        typename TUPLE_TYPE
      >
      void walk(
        TUPLE_TYPE & t
      )
      {
        using HELPER_TYPE =
          tuple_walker_helper_u<
            std::tuple_size<TUPLE_TYPE>::value,
            TUPLE_TYPE,
            CRTP_TYPE
          >;
      
        // CRTP is needed here to recover the derived type's type
        // for this static cast.
        HELPER_TYPE::walk(*static_cast<CRTP_TYPE*>(this), t);
      } // walk
    }; // struct tuple_walker_u
```

The helper class calls the handle method:

```
    template<
      std::size_t INDEX,
      typename TUPLE_TYPE,
      typename CRTP_TYPE
    >
    struct tuple_walker_helper_u
    {
      // The walk method of the helper class calls the CRTP type's
      // handle method.
      static
      std::size_t
      walk(
        CRTP_TYPE & p,
        TUPLE_TYPE & t
      )
      {
        p.handle(std::get<CURRENT>(t));
        return HELPER_TYPE::walk(p, t);
      } // walk
    }; // tuple_walker_helper_u
```

The *p.handle* method, defined by the *CRTP_TYPE*, allows the derived
type to apply specialized logic to each element type of the *TUPLE_TYPE*.

Several other uses of CRTP are documented on
[widipedia](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern).

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
