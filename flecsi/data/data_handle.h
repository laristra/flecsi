/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_handle_h
#define flecsi_data_handle_h

///
/// \file
/// \date Initial file creation: Aug 01, 2016
///

namespace flecsi {

///
/// \struct data_handle_t data_handle.h
/// \brief data_handle_t provides an empty type for compile-time identification
///                      of data handle objects.
///
struct data_handle_base
{

}; // struct data_handle_t

template<
  typename T,
  size_t PS,
  typename MD,
  typename ST
>
struct data_handle__ : public data_handle_base
{
  MD metadata;
};

} // namespace flecsi

#endif // flecsi_data_handle_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
