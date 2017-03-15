/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_sprint_common_h
#define flecsi_sprint_common_h

#include "flecsi/data/data_client.h"

namespace flecsi {

class data_client : public data::data_client_t{
public:
  size_t indices(size_t index_space) const override{
    flecsi::execution::context_t::partitioned_index_space& space = data::data_client_t::get_index_space(index_space);
    return space.size;
  }

};

} // namespace flecsi

#endif //flecsi_sprint_common_h
