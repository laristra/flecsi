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

#ifndef flecsi_utils_bitfield_h
#define flecsi_utils_bitfield_h

/*!
 * \file 
 * \date Initial file creation: Oct 05, 2015
 */

#include <limits>
#include <cstddef>
#include <cstdint>
#include <climits>

namespace flecsi {
namespace utils {
  
/*!
  \class bitfield__ bitfield.h
  \brief bitfield__ provides...
 */

template <typename T>
class bitfield__
{
 public:
  using field_type_t = T;

  //! Default constructor
  bitfield__(const field_type_t bits = 0x0) : bits_(bits)
  {
    static_assert(std::numeric_limits<T>::is_integer &&
            !std::numeric_limits<T>::is_signed,
        "must be an unsigned integer type");
  }

  //! Copy constructor (disabled)
  bitfield__(const bitfield__ & bf) : bits_(bf.bits_) {}
  //! Assignment operator (disabled)
  bitfield__ & operator=(const bitfield__ & bf) { bits_ = bf.bits_; }
  //! Destructor
  ~bitfield__() {}
  /*!-------------------------------------------------------------------------*
   * Set bits in mask.
   *--------------------------------------------------------------------------*/

  field_type_t set(const field_type_t mask) { return bits_ |= mask; } // set
  /*!-------------------------------------------------------------------------*
   * Set individual bit.
   *--------------------------------------------------------------------------*/
#undef setbit
  field_type_t setbit(const size_t bit)
  {
    field_type_t tmp = 1 << bit;
    return bits_ |= tmp;
  } // setbit

  /*!-------------------------------------------------------------------------*
   * Clear all bits.
   *--------------------------------------------------------------------------*/

  field_type_t clear() { return bits_ = 0x0; } // set
  /*!-------------------------------------------------------------------------*
   * Clear bits in mask.
   *--------------------------------------------------------------------------*/

  field_type_t clear(const field_type_t mask) { return bits_ &= ~mask; } // set
  /*!-------------------------------------------------------------------------*
   * Clear individual bit.
   *--------------------------------------------------------------------------*/

  field_type_t clearbit(const size_t bit)
  {
    field_type_t tmp = 1 << bit;
    return bits_ &= ~tmp;
  } // setbit

  /*!-------------------------------------------------------------------------*
   * Test bits in mask.
   *--------------------------------------------------------------------------*/

  bool bitsset(const field_type_t mask) const
  {
    return mask & bits_;
  } // bitsset

  /*!-------------------------------------------------------------------------*
   * Test bit.
   *--------------------------------------------------------------------------*/

  bool bitset(const size_t bit) const
  {
    field_type_t tmp = 1 << bit;
    return tmp & bits_;
  } // bitset

  /*!-------------------------------------------------------------------------*
   * Test all bits.
   *--------------------------------------------------------------------------*/

  bool anybitset() const
  {
    return (bits_ != 0x0);
  } // bitset

  /*!-------------------------------------------------------------------------*
   * Test clear.
   *--------------------------------------------------------------------------*/

  bool bitclear(const size_t bit) const
  {
    field_type_t tmp = 1 << bit;
    return !(tmp & bits_);
  } // bitsset

  /*!-------------------------------------------------------------------------*
   * Test clear.
   *--------------------------------------------------------------------------*/

  bool bitsclear() const { return bits_ == 0x0; } // bitsset
 private:
  field_type_t bits_;

}; // class bitfield__

using bitfield_t = bitfield__<uint32_t>;

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_bitfield_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
