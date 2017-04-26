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

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
