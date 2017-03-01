/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_utils_any_h
#define flecsi_utils_any_h

#include <iostream>
#include <typeinfo>

///
/// \file any.h
/// \date Initial file creation: Aug 29, 2016
///

namespace flecsi {
namespace utils {

/// 
/// \class any_t any.h
/// \brief any_t class can store arbitrary types of information.
///
class any_t
{
public:
  
  ///
  /// default constructor
  ///
  any_t()
  {
    holder_ = nullptr;
  } // any_t

  ///
  /// copy constructor
  ///
  template<
    class T
  >
  any_t(const T & value)
  {
    holder_ = new holder_t<T>(value);
  } // any_t

  ///
  /// destructor
  ///
  ~any_t()
  {
    delete holder_;
    holder_ = nullptr;
  } // ~any_t

  ///
  /// copy operator
  ///
  any_t(const any_t & rhs)
  {
    holder_=rhs.holder_ ? rhs.holder_->copy() : nullptr;
  } // any_t

  ///
  /// assignment operator
  ///
  any_t & operator = (const any_t & rhs)
  {
    delete holder_;
    holder_ = rhs.holder_ ? rhs.holder_->copy() : nullptr;
    return *this;
  } // operator =

  ///
  /// cast to the type T
  ///
  template<
    class T
  >
  operator T()
  const
  {
    return dynamic_cast<holder_t<T>&>(*holder_).value;
  } // operator T()

  template<class T>
  friend const T & any_cast(any_t & rhs);

  ///
  /// returns Type of the element
  ///
  const
  std::type_info &
  get_type()
  const
  {
    return holder_ ? holder_->get_type() : typeid(void);
  } // get_type

private:

  ///
  /// helper class 
  ///
  class i_holder_t
  {
  public:

    virtual ~i_holder_t() {}
    virtual i_holder_t * copy() const = 0;
    virtual const std::type_info & get_type() const = 0;

  }; //class i_holder_t


  ///
  /// helper class
  ///
  template<
    typename T
  >
  class holder_t : public i_holder_t
  {
  public:

    holder_t(const T & value) : value(value) {}

    holder_t * copy() const
    {
      return new holder_t(value);
    } // copy

    const
    std::type_info &
    get_type()
    const
    {
      return typeid(T);
    } // get_type

  public:

    T value;

  }; // class Holder

  i_holder_t * holder_; 

}; //class any_t

///
/// cast to the type
///
template<
  class T
>
inline
const 
T & any_cast(
  any_t & rhs
)
{
  return dynamic_cast<any_t::holder_t<T>&>(*(rhs.holder_)).value;
} // any_cast

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_any_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
