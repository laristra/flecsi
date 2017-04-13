/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef debruijn_h
#define debruijn_h

///
/// \file
/// \date Initial file creation: Apr 09, 2017
///

#include <cstdint>
#include <cstddef>

namespace flecsi {
namespace utils {

///
/// \class debruijn_t debruijn.h
/// \brief debruijn_t provides a mechanism for doing lookups of the set
///                   bits in a bitfield.
///
/// A de Bruijn sequence can be used to quickly find the index of the
/// least significant set bit ("right-most 1") or the most significant
/// set bit ("left-most 1") in a word using bitwise operations.
///
/// \note This implementation is based on the example given in
///       http://supertech.csail.mit.edu/papers/debruijn.pdf
///
struct debruijn32_t {

  // de Bruijn sequence 
  //            binary: 0000 0111 0111 1100 1011 0101 0011 0001
  //               hex:    0    7    7    c    b    5    3    1
  static constexpr uint32_t seq_ = 0x077cb531;

  // Lookup table. Note that this depends on the specific de Bruijn
  // sequence that is being used.
  static constexpr uint32_t index_[32] = {
    0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
  };

  ///
  /// Return the index of the right-most set bit in the given bit field.
  ///
  /// \note Successive bits can be found by subtracting the indexed bit.
  ///
  static
  uint32_t
  index(
    uint32_t b
  )
  {
    b &= -b;  // twos-complement to isolate the right-most set bit

    b *= seq_; // shift and truncate the sequence by the isolated bit index

    b >>= 27; // shift off all but the high log_2(32) bits to get the
              // lookup index.

    return index_[b]; // return the index from the lookup table
  } // index

}; // debruijn32_t

} // namespace utils
} // namespace flecsi

#endif // debruijn_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
