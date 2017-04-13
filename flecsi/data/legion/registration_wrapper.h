/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_registration_wrapper_h
#define flecsi_data_registration_wrapper_h

///
/// \file
/// \date Initial file creation: Apr 10, 2017
///

#include <cinchlog.h>

#include "flecsi/execution/context.h"

namespace flecsi {
namespace data {

///
/// \class registration_wrapper_t registration_wrapper.h
/// \brief registration_wrapper_t provides...
///
template<
  typename DATA_CLIENT_TYPE,
  size_t STORAGE_TYPE,
  typename DATA_TYPE,
  size_t NAMESPACE_HASH,
  size_t NAME_HASH,
  size_t INDEX_SPACE,
  size_t VERSIONS
>
struct registration_wrapper_t
{
  using field_id_t = LegionRuntime::HighLevel::FieldID;

  static
  void
  register_callback(
    field_id_t fid
  )
  {
    clog(info) << "In register_callback" << std::endl;
    // Do stuff
  } // register_callback

}; // class registration_wrapper_t

} // namespace data
} // namespace flecsi

#endif // flecsi_data_registration_wrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
