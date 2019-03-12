/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

#pragma once

#include <iostream>

#include <flecsi/data/data.h>
#include <flecsi/data/data_client.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>
#include <flecsi/utils/common.h>

#define np(X)                                                                  \
  std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__      \
            << ": " << #X << " = " << (X) << std::endl

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jan 25, 2017
//----------------------------------------------------------------------------//

template<typename T>
using accessor_t = flecsi::data::serial::dense_accessor_t<T,
  flecsi::data::serial_meta_data_t<flecsi::default_user_meta_data_t>>;

namespace flecsi {
namespace execution {

void
task1(accessor_t<double> x) {
  std::cout << "Executing task1" << std::endl;

  np(x[0]);
  np(x[1]);

  // np(x);
  // std::cout << "val = " << val << std::endl;
} // task1

register_task(task1, loc, single);

class data_client : public data::data_client_t
{
public:
  size_t indices(size_t index_space) const override {
    return 10;
  }
};

void
specialization_tlt_init(int argc, char ** argv) {
  std::cout << "driver start" << std::endl;

  context_t & context_ = context_t::instance();
  size_t task_key = utils::const_string_t{"specialization_driver"}.hash();

  data_client dc;

  // data client
  // "hydro" namespace
  // "pressure" name
  // type double
  // dense storage type
  // versions
  // index space

  register_data(dc, hydro, pressure, double, dense, 1, 0);

  auto ac = get_accessor(dc, hydro, pressure, double, dense, /* version */ 0);

  auto h1 = get_handle(dc, hydro, pressure, double, dense, 0, rw, rw, ro);

  ac[0] = 100.0;
  ac[1] = 200.0;

  EXECUTE_TASK(task1, loc, single, h1);

} // specialization_tlt_init

} // namespace execution
} // namespace flecsi

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
