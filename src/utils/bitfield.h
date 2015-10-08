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

#ifndef flexi_bitfield_h
#define flexi_bitfield_h

/*!
 * \file bitfield.h
 * \authors bergen
 * \date Initial file creation: Oct 05, 2015
 */

namespace flexi {

/*!
  \class bitfield bitfield.h
  \brief bitfield provides...
 */

template<typename T>
class bitfield
{
public:

  using field_type_t = T;

  //! Default constructor
  bitfield(const field_type_t bits = 0x0) : bits_(bits) {}

  //! Copy constructor (disabled)
  bitfield(const bitfield & bf) : bits_(bf.bits_) {}

  //! Assignment operator (disabled)
  bitfield & operator = (const bitfield & bf) { bits_ = bf.bits_; }

  //! Destructor
   ~bitfield() {}

  /*!-------------------------------------------------------------------------*
   * Set bits in mask.
   *--------------------------------------------------------------------------*/

  field_type_t set(const field_type_t mask) {
    return bits_ |= mask;
  } // set

  /*!-------------------------------------------------------------------------*
   * Set individual bit.
   *--------------------------------------------------------------------------*/

  field_type_t setbit(const size_t bit) {
    field_type_t tmp = 1<<bit;
    return bits_ |= tmp;
  } // setbit

  /*!-------------------------------------------------------------------------*
   * Clear all bits.
   *--------------------------------------------------------------------------*/

  field_type_t clear() {
    return bits_ = 0x0;
  } // set

  /*!-------------------------------------------------------------------------*
   * Clear bits in mask.
   *--------------------------------------------------------------------------*/

  field_type_t clear(const field_type_t mask) {
    return bits_ &= ~mask;
  } // set

  /*!-------------------------------------------------------------------------*
   * Clear individual bit.
   *--------------------------------------------------------------------------*/

  field_type_t clearbit(const size_t bit) {
    field_type_t tmp = 1<<bit;
    return bits_ &= ~tmp;
  } // setbit

  /*!-------------------------------------------------------------------------*
   * Test bits in mask.
   *--------------------------------------------------------------------------*/

  bool bitsset(const field_type_t mask) const {
    return mask & bits_;
  } // bitsset

  /*!-------------------------------------------------------------------------*
   * Test bit.
   *--------------------------------------------------------------------------*/

  bool bitset(const size_t bit) const {
    field_type_t tmp = 1<<bit;
    return tmp & bits_;
  } // bitset

  /*!-------------------------------------------------------------------------*
   * Test clear.
   *--------------------------------------------------------------------------*/

  bool bitclear(const size_t bit) const {
    field_type_t tmp = 1<<bit;
    return !(tmp & bits_);
  } // bitsset

  /*!-------------------------------------------------------------------------*
   * Test clear.
   *--------------------------------------------------------------------------*/

  bool bitsclear() const {
    return bits_ == 0x0;
  } // bitsset

private:

  field_type_t bits_;

}; // class bitfield

} // namespace flexi

#endif // flexi_bitfield_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
