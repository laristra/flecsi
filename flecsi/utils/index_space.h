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

#ifndef flecsi_utils_index_space_h
#define flecsi_utils_index_space_h

//!
//! \file index_space.h
//! \date Initial file creation: Oct 09, 2015
//!

#include "flecsi/utils/iterator.h"

namespace flecsi {
namespace utils {

//!
//! \class index_space_t index_space.h
//! \brief index_space_t provides...
//!

class index_space_t
{
public:

  using iterator_t = iterator<index_space_t, std::size_t>;

  //! Default constructor
  index_space_t() = default;

  //! Constructor with size
  index_space_t(std::size_t size) : size_(size), index_(0) {}

  //! Copy constructor
  index_space_t(const index_space_t & is) {
    size_ = is.size_;
    index_ = is.index_;
  } // index_space_t

  //! Assignment operator
  index_space_t & operator=(const index_space_t & is) {
    size_ = is.size_;
    index_ = is.index_;
    return *this;
  } // operator =

  //! Destructor
  ~index_space_t() {}

  // operator []
  std::size_t operator[](std::size_t index) const { return index; }

  // FIXME: This is not thread-safe!!!
  std::size_t & operator[](std::size_t index) {
    index_ = index;
    return index_;
  } // operator []

  iterator_t begin() { return {*this, 0}; }
  iterator_t end() { return {*this, size_}; }

private:

  std::size_t size_ = 0;
  std::size_t index_ = 0;

}; // class index_space_t

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_index_space_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
