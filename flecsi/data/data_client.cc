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

data_client_t::~data_client_t() 
{
  flecsi::data::storage_t::instance().reset( runtime_id() );
}

} // namespace
} // namespace