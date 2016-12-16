/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

///
/// \file
///

#include "flecsi/data/storage.h"
#include "flecsi/data/data_client.h"

namespace flecsi {
namespace data {

data_client_t::data_client_t(data_client_t && o) : data_client_t()
{
  // store old runtime id
  auto rid = o.runtime_id();
  // move the data now
  flecsi::data::storage_t::instance().move( rid, runtime_id() );
}


data_client_t & data_client_t::operator=(data_client_t && o)
{
  // store old runtime id
  auto rid = o.runtime_id();
  // move the data now
  flecsi::data::storage_t::instance().move( rid, runtime_id() );
  // return a reference to the new object
  return *this;
}

data_client_t::~data_client_t() 
{
  flecsi::data::storage_t::instance().reset( runtime_id() );
}

} // namespace
} // namespace
