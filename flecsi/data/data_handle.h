/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_handle_h
#define flecsi_data_handle_h

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_legion || \
      FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion
#include "flecsi/data/legion/handle_policy.h"
#define flecsi_handle_policy_t data::legion_handle_policy_t
#else
#include "flecsi/data/serial/handle_policy.h"
#define flecsi_handle_policy_t data::serial_handle_policy_t
#endif

///
/// \file
/// \date Initial file creation: Aug 01, 2016
/// \authors bergen, nickm

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
