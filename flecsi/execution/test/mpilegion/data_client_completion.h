/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_client_completion_h
#define flecsi_data_client_completion_h

#include "flecsi/data/data_client.h"

//FIXME ndm - what is this for and where does it belong?

namespace flecsi {

class data_client : public data::data_client_t{
public:
  size_t indices(size_t index_space) const override{
    flecsi::execution::context_t::partitioned_index_space& space = data::data_client_t::get_index_space(index_space);
    return space.size;
  }

};

} // namespace flecsi

#endif //flecsi_data_client_completion_h
