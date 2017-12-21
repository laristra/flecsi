/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */


#include <array>
#include <cmath>
#include <ostream>

#include <flecsi/utils/common.h>

namespace flecsi {
namespace utils {

template<typename... CONDITIONS>
struct and_ : std::true_type {};

template<typename CONDITION, typename... CONDITIONS>
struct and_<CONDITION, CONDITIONS...>
    : std::conditional<CONDITION::value, and_<CONDITIONS...>, std::false_type>::
          type {}; // struct and_

template<typename TARGET, typename... TARGETS>
using are_type__ = and_<std::is_same<TARGETS, TARGET>...>;

//----------------------------------------------------------------------------//
//! Enumeration for axes.
//----------------------------------------------------------------------------//

enum class axis : size_t { x = 0, y = 1, z = 2 };

//----------------------------------------------------------------------------//
//! The dimensioned_array__ type provides a general base for defining
//! contiguous array types that have a specific dimension.  Please look at
//! the \ref point__ and \ref vector__ types for an example of its use.
//!
//! @tparam TYPE      The type of the array, e.g., P.O.D. type.
//! @tparam DIMENSION The dimension of the array, i.e., the number of elements
//!                   to be stored in the array.
//! @tparam NAMESPACE The namespace of the array.  This is a dummy parameter
//!                   that is useful for creating distinct types that alias
//!                   dimensioned_array__.
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION, size_t NAMESPACE>
class dimensioned_array__ {
public:
  //! Default constructor.
  dimensioned_array__() = default;

  //! Default copy constructor.
  dimensioned_array__(dimensioned_array__ const &) = default;

  //--------------------------------------------------------------------------//
  //! Initializer list constructor.
  //--------------------------------------------------------------------------//

  dimensioned_array__(std::initializer_list<TYPE> list) {
    assert(list.size() == DIMENSION && "dimension size mismatch");
    std::copy(list.begin(), list.end(), data_.begin());
  } // dimensioned_array__

  //--------------------------------------------------------------------------//
  //! Variadic constructor.
  //--------------------------------------------------------------------------//

  template<
      typename... ARGS,
      typename = typename std::enable_if<
          sizeof...(ARGS) == DIMENSION &&
          are_type__<TYPE, ARGS...>::value>::type>
  dimensioned_array__(ARGS... args) {
    data_ = {args...};
  } // dimensioned_array__

  //--------------------------------------------------------------------------//
  //! Constructor (fill with given value).
  //--------------------------------------------------------------------------//

  dimensioned_array__(TYPE const & val) {
    data_.fill(val);
  } // dimensioned_array__

  //--------------------------------------------------------------------------//
  //! Return the size of the array.
  //--------------------------------------------------------------------------//

  static constexpr size_t size() {
    return DIMENSION;
  }; // size

  //--------------------------------------------------------------------------//
  //! Support for enumerated type access, e.g., da[x], for accessing the
  //! x axis.
  //--------------------------------------------------------------------------//

  template<typename ENUM_TYPE>
  TYPE & operator[](ENUM_TYPE e) {
    return data_[static_cast<size_t>(e)];
  } // operator []

  //--------------------------------------------------------------------------//
  //! Support for enumerated type access, e.g., da[x], for accessing the
  //! x axis.
  //--------------------------------------------------------------------------//

  template<typename ENUM_TYPE>
  TYPE const & operator[](ENUM_TYPE e) const {
    return data_[static_cast<size_t>(e)];
  } // operator []

  //--------------------------------------------------------------------------//
  //! Assignment operator.
  //--------------------------------------------------------------------------//

  dimensioned_array__ & operator=(dimensioned_array__ const & rhs) {
    if (this != &rhs) {
      data_ = rhs.data_;
    } // if

    return *this;
  } // operator =

  //--------------------------------------------------------------------------//
  //! Assignment operator.
  //--------------------------------------------------------------------------//

  dimensioned_array__ & operator=(const TYPE & val) {
    for (size_t i = 0; i < DIMENSION; i++) {
      data_[i] = val;
    } // for

    return *this;
  } // operator =

    //--------------------------------------------------------------------------//
    // Macro to avoid code replication.
    //--------------------------------------------------------------------------//

#define define_operator(op)                                                    \
  dimensioned_array__ & operator op(dimensioned_array__ const & rhs) {         \
    if (this != &rhs) {                                                        \
      for (size_t i{0}; i < DIMENSION; i++) {                                  \
        data_[i] op rhs[i];                                                    \
      } /* for */                                                              \
    } /* if */                                                                 \
                                                                               \
    return *this;                                                              \
  }

    //--------------------------------------------------------------------------//
    // Macro to avoid code replication.
    //--------------------------------------------------------------------------//

#define define_operator_type(op)                                               \
  dimensioned_array__ & operator op(TYPE val) {                                \
    for (size_t i{0}; i < DIMENSION; i++) {                                    \
      data_[i] op val;                                                         \
    } /* for */                                                                \
                                                                               \
    return *this;                                                              \
  }

  //--------------------------------------------------------------------------//
  //! Addition/Assignment operator.
  //--------------------------------------------------------------------------//

  define_operator(+=);

  //--------------------------------------------------------------------------//
  //! Addition/Assignment operator.
  //--------------------------------------------------------------------------//

  define_operator_type(+=);

  //--------------------------------------------------------------------------//
  //! Subtraction/Assignment operator.
  //--------------------------------------------------------------------------//

  define_operator(-=);

  //--------------------------------------------------------------------------//
  //! Subtraction/Assignment operator.
  //--------------------------------------------------------------------------//

  define_operator_type(-=);

  //--------------------------------------------------------------------------//
  //! Multiplication/Assignment operator.
  //--------------------------------------------------------------------------//

  define_operator(*=);

  //--------------------------------------------------------------------------//
  //! Multiplication/Assignment operator.
  //--------------------------------------------------------------------------//

  define_operator_type(*=);

  //--------------------------------------------------------------------------//
  //! Division/Assignment operator.
  //--------------------------------------------------------------------------//

  define_operator(/=);

  //--------------------------------------------------------------------------//
  //! Division/Assignment operator.
  //--------------------------------------------------------------------------//

  define_operator_type(/=);

  //! \brief Division operator involving a constant.
  //! \param[in] val The constant on the right hand side of the operator.
  //! \return A reference to the current object.
  dimensioned_array__ operator/(TYPE val) {
    dimensioned_array__ tmp(*this);
    tmp /= val;

    return tmp;
  } // operator /

private:
  std::array<TYPE, DIMENSION> data_;

}; // class dimensioned_array__

//----------------------------------------------------------------------------//
//! Addition operator.
//!
//! @tparam TYPE      The type of the array, e.g., P.O.D. type.
//! @tparam DIMENSION The dimension of the array, i.e., the number of elements
//!                   to be stored in the array.
//! @tparam NAMESPACE The namespace of the array.  This is a dummy parameter
//!                   that is useful for creating distinct types that alias
//!                   dimensioned_array__.
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION, size_t NAMESPACE>
dimensioned_array__<TYPE, DIMENSION, NAMESPACE>
operator+(
    const dimensioned_array__<TYPE, DIMENSION, NAMESPACE> & lhs,
    const dimensioned_array__<TYPE, DIMENSION, NAMESPACE> & rhs) {
  dimensioned_array__<TYPE, DIMENSION, NAMESPACE> tmp(lhs);
  tmp += rhs;
  return tmp;
} // operator +

//----------------------------------------------------------------------------//
//! Addition operator.
//!
//! @tparam TYPE      The type of the array, e.g., P.O.D. type.
//! @tparam DIMENSION The dimension of the array, i.e., the number of elements
//!                   to be stored in the array.
//! @tparam NAMESPACE The namespace of the array.  This is a dummy parameter
//!                   that is useful for creating distinct types that alias
//!                   dimensioned_array__.
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION, size_t NAMESPACE>
dimensioned_array__<TYPE, DIMENSION, NAMESPACE>
operator-(
    const dimensioned_array__<TYPE, DIMENSION, NAMESPACE> & lhs,
    const dimensioned_array__<TYPE, DIMENSION, NAMESPACE> & rhs) {
  dimensioned_array__<TYPE, DIMENSION, NAMESPACE> tmp(lhs);
  tmp -= rhs;
  return tmp;
} // operator -

//----------------------------------------------------------------------------//
//! Addition operator.
//!
//! @tparam TYPE      The type of the array, e.g., P.O.D. type.
//! @tparam DIMENSION The dimension of the array, i.e., the number of elements
//!                   to be stored in the array.
//! @tparam NAMESPACE The namespace of the array.  This is a dummy parameter
//!                   that is useful for creating distinct types that alias
//!                   dimensioned_array__.
//!
//! @param stream The output stream.
//! @param a      The dimensioned array.
//----------------------------------------------------------------------------//

template<typename TYPE, size_t DIMENSION, size_t NAMESPACE>
std::ostream &
operator<<(
    std::ostream & stream,
    dimensioned_array__<TYPE, DIMENSION, NAMESPACE> const & a) {
  stream << "[";

  for (size_t i = 0; i < DIMENSION; i++) {
    stream << " " << a[i];
  } // for

  stream << " ]";

  return stream;
} // operator <<

} // namespace utils
} // namespace flecsi
