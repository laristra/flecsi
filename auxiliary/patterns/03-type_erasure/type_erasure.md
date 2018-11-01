# Design Pattern: Type Erasure

Type erasure allows the creation of a gneneric container, i.e., one that
contains instances of different types. This contravenes C++'s normal
requirements of strict and static typing. This pattern is attributed to
Kevlin Henney, *C++ Report 12(7)*, July/August 2000, entitled,
[Valued Conversions](http://www.two-sdg.demon.co.uk/curbralan/papers/ValuedConversions.pdf). His definition of the *any* type follows:
```
class any
{
public:

  any() : content(0) {}

  ~any() { delete content; }

  const std::type_info & type_info() const {
    return content ? content->type_info() : typeid(void);
  } // type_info

private:

  class placeholder
  {
  public:

    virtual ~placeholder() {}

    virtual const std::type_info &
    type_info() const = 0;

    virtual placeholder *clone() const = 0;

  }; // placeholder

  template<typename value_type>
  class holder : public placeholder
  {
  public:

    holder(const value_type &value) : held(value) {}

    virtual const std::type_info & type_info() const {
      return typeid(value_type);
    } // type_info

    virtual placeholder * clone() const {
      return new holder(held);
    } // clone

    const value_type held;
  };

  placeholder *content;

}; // class any
```

Working off of this model, FleCSI uses a simpler, less-explicit form of
type erasure through the definition of a common method interface for a
set of parameterized types. For example, if several types define a
method *erasure_method* like
```
template<
  typename ... PARAMS
>
struct type_t
{

  // This method can be used to capture static parameters that were
  // passed to define the type. Runtime invocation of this method
  // allows the recovery of this type information.
  static
  void
  erasure_method()
  {
    // use PARAMS to do type-specific operations...
  } // erasure_method

}; // struct type_t
```
we can recover the type information at runtime by invoking the common
method. References to each type's *erasure_method* function can be
stored in a standard container because they are all of the same type.
This pattern is used in several places in FleCSI. A specific example is
in *flecsi/execution/legion/task_wrapper.h*. In particular,
*task_wrapper_u::registration_callback* and
*task_wrapper_u::execute_user_task* use this design pattern.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
