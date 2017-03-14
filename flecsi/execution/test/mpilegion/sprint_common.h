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
    return size_;
  }

  void set_size(size_t size){
    size_ = size;
  }

private:
  size_t size_;
};

} // namespace flecsi

#endif flecsi_sprint_common_h
