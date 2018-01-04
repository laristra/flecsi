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


#include <cstddef>
#include <cstdint>

namespace flecsi {
namespace utils {

//----------------------------------------------------------------------------//
//!
//! \class debruijn_t debruijn.h
//! \brief debruijn_t provides a mechanism for doing lookups of the set
//!                   bits in a bitfield.
//!
//! A de Bruijn sequence can be used to quickly find the index of the
//! least significant set bit ("right-most 1") or the most significant
//! set bit ("left-most 1") in a word using bitwise operations.
//!
//! \note This implementation is based on the example given in
//!       http://supertech.csail.mit.edu/papers/debruijn.pdf
//!
//----------------------------------------------------------------------------//

class debruijn32_t {

  // de Bruijn sequence
  //            binary: 0000 0111 0111 1100 1011 0101 0011 0001
  //               hex:    0    7    7    c    b    5    3    1
  static constexpr uint32_t seq_ = 0x077cb531;

  // Lookup table. Note that this depends on the specific de Bruijn
  // sequence that is being used.
  static constexpr uint32_t index_[32] = {
      0,  1,  28, 2,  29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4,  8,
      31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6,  11, 5,  10, 9};

public:
  //--------------------------------------------------------------------------//
  //! Return the index of the right-most set bit in the given bit field.
  //!
  //! \note Successive bits can be found by subtracting the indexed bit.
  //--------------------------------------------------------------------------//

  static constexpr uint32_t index(const uint32_t b) {
    // 1. &  Two's-complement to isolate the right-most set bit
    // 2. *  Shift and truncate the sequence by the isolated bit index
    // 3. >> Shift off all but the high log_2(32) bits to get the lookup index
    // 4. [] return the index from the lookup table
    return index_[(b & -b) * seq_ >> 27];
  } // index

}; // debruijn32_t

} // namespace utils
} // namespace flecsi
