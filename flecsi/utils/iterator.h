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

#ifndef flecsi_utils_iterator_h
#define flecsi_utils_iterator_h

/*!
 * \file
 * \date Initial file creation: Oct 09, 2015
 */

namespace flecsi {
namespace utils {

/*!
  \class iterator iterator.h
  \brief iterator provides...
 */
template <typename C, typename T>
class iterator
{
 public:

  using container_t = C;
  using type_t = T;

  //! Default constructor
  iterator(container_t & values, size_t index)
    : values_(values), index_(index) {}

  //! Destructor
  ~iterator() {}

  //! Copy constructor
  iterator(const iterator & it) : values_(it.values_), index_(it.index_) {}

  //! Assignment operator
  iterator & operator=(const iterator & it) {
    index_ = it.index_;
    values_ = it.values_;
    return *this;
  } // operator =

  //! Increment operator
  iterator & operator ++ () {
    ++index_;
    return *this;
  } // operator ++

  //! Dereference operator
  type_t & operator * () {
    return values_[index_];
  } // operator *

  //! Dereference operator
  type_t & operator -> () {
    return values_[index_];
  } // operator ->

  //! Equivalence operator
  bool operator == (const iterator & it) const {
    return index_ == it.index_;
  } // operator ==

  //! Comparision operator
  bool operator != (const iterator & it) const {
    return index_ != it.index_;
  } // operator !=

 private:

  container_t & values_;
  size_t index_;

}; // class iterator

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_iterator_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
