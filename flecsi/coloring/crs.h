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

namespace flecsi {
namespace coloring {

//----------------------------------------------------------------------------//
// Convenience macro to avoid having to reimplement this for each member.
//----------------------------------------------------------------------------//

#define define_as(member)                                                      \
  template<typename T>                                                         \
  std::vector<T> member##_as() const {                                         \
    std::vector<T> asvec(member.begin(), member.end());                        \
    return asvec;                                                              \
  }

/*
 This type is a container for compressed-storage of sparse data.

 @var offsets The offset at which each index begins and ends in the
              list of indices.
 @var indices The indices of the sparse structure of the data resolved
              by this storage member.

 @ingroup coloring
*/

struct crs_t {
  std::vector<size_t> offsets;
  std::vector<size_t> indices;

  define_as(offsets) define_as(indices)

      size_t size() const {
    return offsets.size() - 1;
  } // size

}; // struct crs_t

/*!
 Helper function to print a crs_t instance.
 */

inline std::ostream &
operator<<(std::ostream & stream, const crs_t & crs) {
  stream << "offsets: ";
  for (auto i : crs.offsets) {
    stream << i << " ";
  } // for
  stream << std::endl;

  stream << "indices: ";
  for (auto i : crs.indices) {
    stream << i << " ";
  } // for

  return stream;
} // operator <<

/*!
 This type is a container for distrinuted compressed-storage of sparse data.

 @var distribution The index ranges for each color.

 @ingroup coloring
 */

struct dcrs_t : public crs_t {
  define_as(distribution)

      std::vector<size_t> distribution;
}; // struct dcrs_t

/*!
 Helper function to print a crs_t instance.
 */

inline std::ostream &
operator<<(std::ostream & stream, const dcrs_t & dcrs) {
  stream << static_cast<const crs_t &>(dcrs) << std::endl;

  stream << "distribution: ";
  for (auto i : dcrs.distribution) {
    stream << i << " ";
  } // for

  return stream;
} // operator <<

#undef define_as

} // coloring
} // namespace flecsi
