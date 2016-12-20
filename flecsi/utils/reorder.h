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

#ifndef flecsi_utils_reorder_h
#define flecsi_utils_reorder_h

#include<iterator>
#include<utility>

/*!
 * \file
 */

namespace flecsi {
namespace utils {

/*!
  \brief Reorders an array in place
  \remark this version maintains the order array
  \param [in] order_begin The begin iterator or the order array
  \param [in] order_end   The end iterator or the order array
  \param [in,out] v The begin iterator or the value array
 */
template< typename order_iterator, typename value_iterator >
void reorder( order_iterator order_begin, order_iterator order_end, value_iterator v )  {   

  typedef typename std::iterator_traits< order_iterator >::value_type index_t;

  auto remaining = order_end - 1 - order_begin;
  for ( index_t s = index_t(), d; remaining > 0; ++ s ) {
    for ( d = order_begin[s]; d > s; d = order_begin[d] ) ;
    if ( d == s ) {
      -- remaining;
      auto temp = v[s];
      while ( d = order_begin[d], d != s ) {
        std::swap( temp, v[d] );
        -- remaining;
      }
      v[s] = temp;
    }
  }
}


/*!
  \brief Reorders an array in place
  \remark this version destroys the order array for performance gains
  \param [in,out] order_begin The begin iterator or the order array
  \param [in,out] order_end   The end iterator or the order array
  \param [in,out] v The begin iterator or the value array
 */
template< typename order_iterator, typename value_iterator >
void reorder_destructive( order_iterator order_begin, order_iterator order_end, value_iterator v )  {
  typedef typename std::iterator_traits< order_iterator >::value_type index_t;
  typedef typename std::iterator_traits< order_iterator >::difference_type diff_t;

  auto remaining = order_end - 1 - order_begin;
  for ( auto s = index_t(); remaining > 0; ++ s ) {
    auto d = order_begin[s];
    if ( d == (diff_t) -1 ) continue;
    -- remaining;
    auto temp = v[s];
    for ( index_t d2; d != s; d = d2 ) {
      std::swap( temp, v[d] );
      std::swap( order_begin[d], d2 = (diff_t) -1 );
      -- remaining;
    }
    v[s] = temp;
  }
}


} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_reorder_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
