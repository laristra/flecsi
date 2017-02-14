/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_handle_test_h
#define flecsi_data_handle_test_h

#include <iostream>

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/data/data.h"
#include "flecsi/data/data_client.h"

#define np(X)                                                            \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << (X) << std::endl

///
// \file data-handle.h
// \authors nickm
// \date Initial file creation: Jan 25, 2017
///

template<typename T>
using accessor_t = flecsi::data::serial::dense_accessor_t<T, flecsi::data::serial_meta_data_t<flecsi::default_user_meta_data_t> >;

namespace flecsi {
namespace execution {
  
void task1(accessor_t<double> x) {
  std::cout << "Executing task1" << std::endl;
  
  np(x[0]);
  np(x[1]);

  //np(x);
  //std::cout << "val = " << val << std::endl;
} // task1

register_task(task1, loc, single);

class data_client : public data::data_client_t{
public:
  size_t indices(size_t index_space) const override{
    return 10;
  }
};

void
driver(
  int argc, 
  char ** argv
)
{
  std::cout << "driver start" << std::endl;

  context_t & context_ = context_t::instance();
  size_t task_key = utils::const_string_t{"driver"}.hash();

  data_client dc;

  // data client
  // "hydro" namespace
  // "pressure" name
  // type double
  // dense storage type
  // versions
  // index space

  register_data(dc, hydro, pressure, double, dense, 1, 0);

  auto ac = 
    get_accessor(dc, hydro, pressure, double, dense, /* version */ 0);

  auto h1 = get_handle(dc, hydro, pressure, double, dense, 0, ro);

  ac[0] = 100.0;
  ac[1] = 200.0;

  EXECUTE_TASK(task1, loc, single, h1);

} // driver

} // namespace execution
} // namespace flecsi

#endif // flecsi_data_handle_test_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
