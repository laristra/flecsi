Type Erasure
============

Type erasure allows the creation of a gneneric container, i.e., one that
contains instances of different types. This contravenes C++'s normal
requirements of strict and static typing. This pattern is attributed to
Kevlin Henney, *C++ Report 12(7)*, July/August 2000, entitled,
`Valued Conversions
<http://www.two-sdg.demon.co.uk/curbralan/papers/ValuedConversions.pdf>`_.
His definition of the *any* type follows:

.. code-block:: cpp

  class any
  {
  public:

    any() : content(0) {}

    ~any() { delete content; }

    const std::type_info & type_info() const {
      return content ? content->type_info() : typeid(void);
    } // type_info

  private:

    struct placeholder
    {
      virtual ~placeholder() {}

      virtual const std::type_info &
      type_info() const = 0;

      virtual placeholder * clone() const = 0;

    }; // struct placeholder

    // This is the main 'trick': We generate a type that derives from the
    // placeholder that wraps the user type. This type is used internally
    // to store the instances of the user types. This could be done
    // externally by defining a common base type. However, this approach is
    // automatic and more convenient...
    template<typename value_type>
    struct holder : public placeholder
    {
      holder(const value_type & value) : held(value) {}

      virtual const std::type_info & type_info() const {
        return typeid(value_type);
      } // type_info

      virtual placeholder * clone() const {
        return new holder(held);
      } // clone

      const value_type held;
    }; // struct holder

    placeholder * content;

  }; // class any

Using this basic definition, we can add inward conversion interface from
arbitrary types:

.. code-block:: cpp

  class any
  {
  public:

    // Copy constructor from another any instance.
    any(const any & other)
    : content(other.content ? other.content->clone() : 0)
    {
    }

    // Copy constructor from another instance of the same type.
    template<typename value_type>
    any(const value_type & value)
    : content(new holder<value_type>(value)) {}

    // Swap the contents
    any &swap(any & rhs)
    {
      std::swap(content, rhs.content);
      return *this;
    } // swap

    // Assignment to another any instance.
    any &operator=(const any & rhs)
    {
      return swap(any(rhs));
    } // operator =

    // Assignment to another instance of the same type.
    template<typename value_type>
    any &operator=(const value_type & rhs)
    {
      return swap(any(rhs));
    }
  }; // class any

Recovering the typed value can be handled like:

.. code-block:: cpp

  class any
  {
  public:

    // Conversion to void *.
    operator const void *() const
    {
      return content;
    } // operator const void *

    // Conversion back to stored type.
    template<typename value_type>
    bool copy_to(value_type &value) const
    {
      const value_type * copyable =
      to_ptr<value_type>();
      if(copyable)
      value = * copyable;
      return copyable;
    } // copy_to

    // Conversion to pointer to stored type.
    template<typename value_type>
    const value_type * to_ptr() const
    {
      return type_info() == typeid(value_type)
        ? &static_cast<
          holder<value_type> *>(content)->held
        : 0;
    } // to_ptr

  }; // class any

  // Convenience cast function.
  template<typename value_type>
  value_type any_cast(const any &operand)
  {
    const value_type * result =
    operand.to_ptr<value_type>();
    return result ? * result : throw std::bad_cast();
  }

Working off of this model, FleCSI uses a simpler, less-explicit form of
type erasure through the definition of a common method interface for a
set of parameterized types. For example, if several types define a
method *erasure_method* like

.. code-block:: cpp

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

we can recover the type information at runtime by invoking the common
method. References to each type's *erasure_method* function can be
stored in a standard container because they are all of the same type.
This pattern is used in several places in FleCSI. A specific example is
in *flecsi/execution/legion/task_wrapper.h*. In particular,
*task_wrapper_u::registration_callback* and
*task_wrapper_u::execute_user_task* use this design pattern.

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
