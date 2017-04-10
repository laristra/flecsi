/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: Dec 7, 2016
///

#include "flecsi/data/data_client.h"
#include "flecsi/data/storage.h"

namespace flecsi {
namespace data {

// Move constructor.
data_client_t::data_client_t(
  data_client_t && dc
)
  : data_client_t()
{
  // Store old runtime id
  auto rid = dc.runtime_id();

  // Move the data now
  flecsi::data::storage_t::instance().move( rid, runtime_id() );
} // data_client_t::data_client_t

// Move assignment.
data_client_t &
data_client_t::operator = (
  data_client_t && dc
)
{
  // Store old runtime id
  auto rid = dc.runtime_id();

  // Move the data now
  flecsi::data::storage_t::instance().move( rid, runtime_id() );

  // Return a reference to the new object
  return *this;
} // data_client_t::operator =

// Reset.
void
data_client_t::reset() 
{
  flecsi::data::storage_t::instance().reset(runtime_id());
} // data_client_t::reset

} // namespace
} // namespace

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
