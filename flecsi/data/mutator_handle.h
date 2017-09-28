/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_mutator_handle_h
#define flecsi_mutator_handle_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Sep 28, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

template<typename MUTATOR_POLICY>
struct mutator_handle_base__ : public MUTATOR_POLICY{

  //--------------------------------------------------------------------------//
  //! Default constructor.
  //--------------------------------------------------------------------------//

  mutator_handle_base__(){}

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //--------------------------------------------------------------------------//

  mutator_handle_base__(const mutator_handle_base__& b)
  : MUTATOR_POLICY(b){

  }

};

} // namespace flecsi

#include "flecsi/runtime/flecsi_runtime_mutator_handle_policy.h"

namespace flecsi {
  
using mutator_handle_t = mutator_handle_base__<
  FLECSI_RUNTIME_MUTATOR_HANDLE_POLICY
>;

} // namespace flecsi

#endif // flecsi_mutator_handle_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
