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

#ifndef flecsi_mpi_meta_data_h
#define flecsi_mpi_meta_data_h

//----------------------------------------------------------------------------//
//! \file
//! \date Initial file creation: Apr 15, 2016
//----------------------------------------------------------------------------//

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
// struct mpi_meta_data_t
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//! The mpi_meta_data_t type provides storage for extra information that is
//! used to interpret data variable information at different points in the
//! low-level runtime.
//! 
//! @tparam USER_META_DATA A user-defined data type that will be carried
//!                          with the meta data.
//----------------------------------------------------------------------------//

template<typename USER_META_DATA>
struct mpi_meta_data_t
{

  //--------------------------------------------------------------------------//
  //! The user_meta_data_t is used to carry user-defined meta data.
  //--------------------------------------------------------------------------//

  using user_meta_data_t = USER_META_DATA;

}; // struct mpi_meta_data_t

} // namespace data
} // namespace flecsi

#endif // flecsi_mpi_meta_data_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
