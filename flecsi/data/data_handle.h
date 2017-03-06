/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_handle_h
#define flecsi_data_handle_h

#include "flecsi_runtime_data_client_policy.h"

///
/// \file
/// \date Initial file creation: Aug 01, 2016
/// \authors bergen, nickm

namespace flecsi {

  namespace data {

    enum class privilege : size_t {
      none = 0b00,
      ro =   0b01,
      wd =   0b10,
      rw =   0b11
    };

    enum class ghost_privilege : size_t {
      none = 0b00,
      ro =   0b01
    };

  } // namespace data

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
  typename P
>
struct data_handle__ : public data_handle_base, public P
{

};

template<
  typename T,
  size_t PS
>
using data_handle_t = data_handle__<T, PS, flecsi_handle_policy_t>;

} // namespace flecsi

#endif // flecsi_data_handle_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
