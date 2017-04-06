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

struct data_handle_base{};

///
/// \struct data_handle_t data_handle.h
/// \brief data_handle_t provides an empty type for compile-time identification
///                      of data handle objects.
/// \tparam DP Data policy.
///
template<
  typename T,
  size_t EP,
  size_t SP,
  size_t GP,
  typename DP
>
struct data_handle_base__ : public DP, public data_handle_base{};

} // namespace flecsi

#include "flecsi_runtime_data_handle_policy.h"

namespace flecsi {
  
  template<
    typename T,
    size_t EP,
    size_t SP,
    size_t GP
  >
  using data_handle__ =
    data_handle_base__<T, EP, SP, GP, flecsi_data_handle_policy_t>;

} // namespace flecsi

#endif // flecsi_data_handle_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
