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
/*
 * future.h
 *
 *  Created on: Aug 31, 2017
 *      Author: jgraham
 */

#ifndef execution_future_h_
#define execution_future_h_


namespace flecsi {
namespace execution {

template<
  typename R
>
class flecsi_future__
{
public:
  ///
  /// get() method
  ///
  virtual R get(size_t index = 0, bool silence_warnings = false)=0;

}; // struct flecsi_future__

} // namespace execution
} // namespace flecsi

#endif /* execution_future_h_ */
