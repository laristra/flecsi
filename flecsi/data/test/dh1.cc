/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_handle_test_h
#define flecsi_data_handle_test_h

#define DH1 1

#include <iostream>
#include <vector>

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/data/data.h"
#include "flecsi/data/data_client.h"
#include "flecsi/data/legion/data_policy.h"
#include "flecsi/execution/legion/helper.h"

#define np(X)                                                            \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << (X) << std::endl

///
/// \file
/// \date Initial file creation: Jan 25, 2017
///

using namespace std;
using namespace flecsi;
using namespace execution;

using namespace LegionRuntime::HighLevel;
using namespace LegionRuntime::Accessor;
using namespace LegionRuntime::Arrays;

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t = 
  flecsi::data::legion::dense_handle_t<T, EP, SP, GP, flecsi::data::legion_meta_data_t<flecsi::default_user_meta_data_t> >;

namespace flecsi {
namespace execution {
  
void task1(handle_t<double, 0, 1, 2> x, float y) {
  np(y);
} // task1

flecsi_register_task(task1, loc, single);

const size_t NUM_CELLS = 16;

class data_client : public data::data_client_t{
public:

};

// THIS IS NEW
flecsi_new_register_data(data_client, test, var, double, dense, 0, 2);

void
driver(
  int argc, 
  char ** argv
)
{
  std::cout << "driver start" << std::endl;
/*
  context_t & context_ = context_t::instance();
  size_t task_key = utils::const_string_t{"driver"}.hash();
  auto runtime = context_.runtime(task_key);
  auto context = context_.context(task_key);
*/
  data_client dc;
  
  // data client
  // "hydro" namespace
  // "pressure" name
  // type double
  // dense storage type
  // versions
  // index space

  auto h1 = 
    flecsi_get_handle(dc, hydro, pressure, double, dense, 0);

  flecsi_execute_task(task1, loc, single, h1, 3.7);

} // driver

} // namespace execution
} // namespace flecsi

#undef DH1

#endif // flecsi_data_handle_test_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
