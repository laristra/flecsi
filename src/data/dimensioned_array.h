/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_dimensioned_array_h
#define flexi_dimensioned_array_h

#include <array>
#include <cmath>

#include "../utils/common.h"

/*!
 * \file dimensioned_array.h
 * \authors bergen
 * \date Initial file creation: Sep 23, 2015
 */

namespace flexi {

enum class axis : size_t { x = 0, y = 1, z = 2 };

/*!
  \class dimensioned_array dimensioned_array.h
  \brief The dimensioned_array type provides a general base for defining
  contiguous array types that have a specific dimension.  Please look at
  the \ref point and \ref vector types for an example of its use.
 */
template <typename T, size_t D> class dimensioned_array {
public:
  dimensioned_array(const dimensioned_array &a) : data_(a.data_) {}

  //! Default constructor
  dimensioned_array(std::initializer_list<T> list) {
    assert(list.size() == D && "dimension size mismatch");
    std::copy(list.begin(), list.end(), data_.begin());
  } // dimensioned_array

  //!
  template <typename... A> dimensioned_array(A... args) {
    // add check for dimension
    data_ = {args...};
  }

  dimensioned_array &operator=(const dimensioned_array &a) {
    data_ = a.data_;
    return *this;
  }

  //! Destructor
  ~dimensioned_array() {}

  /*!
    \brief

    ADD COMMENT ABOUT SUPPORT FOR ENUMS
   */
  template <typename E> T &operator[](E e) {
    return data_[static_cast<size_t>(e)];
  } // operator ()

  template <typename E> const T &operator[](E e) const {
    return data_[static_cast<size_t>(e)];
  } // operator ()

private:
  std::array<T, D> data_;

}; // class dimensioned_array

} // namespace flexi

#endif // flexi_dimensioned_array_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
