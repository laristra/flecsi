/*----------------------------------------------------------------------------*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *----------------------------------------------------------------------------*/

#include <cinchdevel.h>

#include <flecsi/execution/context.h>
#include <flecsi/execution/reduction.h>

#include <flecsi/execution/test/harness.h>

clog_register_tag(reduction_interface);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Variable registration
//----------------------------------------------------------------------------//

flecsi_register_field(mesh_t, data, double_values, double, dense, 1,
  index_spaces::cells);

//----------------------------------------------------------------------------//
// Double
//----------------------------------------------------------------------------//

void double_init(mesh<ro> m, field<rw, rw, ro> v) {
  for(auto c: m.cells(owned)) {
    v(c) = 1.0;
  } // for
} // double_init

flecsi_register_task(double_init, flecsi::execution, loc, single);

double double_task(mesh<ro> m, field<rw, rw, ro> v) {
  double sum{ 0.0 };

  for(auto c: m.cells(owned)) {
    sum += v(c);
  } // for

  return sum;
} // double_task

flecsi_register_task(double_task, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {

  clog_tag_guard(reduction_interface);

  auto mh = flecsi_get_client_handle(mesh_t, clients, m);
  auto vh = flecsi_get_handle(mh, data, double_values, double, dense, 0);

  flecsi_execute_task(double_init, flecsi::execution, single, mh, vh);

  {
  auto f = flecsi_execute_reduction_task(double_task, flecsi::execution,
    single, min, double, mh, vh);

  clog(info) << "reduction min: " << f.get() << std::endl;
  } // scope

  {
  auto f = flecsi_execute_reduction_task(double_task, flecsi::execution,
    single, max, double, mh, vh);

  clog(info) << "reduction max: " << f.get() << std::endl;
  } // scope

  {
  auto f = flecsi_execute_reduction_task(double_task, flecsi::execution,
    single, sum, double, mh, vh);

  clog(info) << "reduction sum: " << f.get() << std::endl;
  } // scope

  {
  auto f = flecsi_execute_reduction_task(double_task, flecsi::execution,
    single, product, double, mh, vh);

  clog(info) << "reduction product: " << f.get() << std::endl;
  } // scope
} // driver

} // namespace execution
} // namespace flecsi

DEVEL(registration_interface) {}

/*----------------------------------------------------------------------------*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *----------------------------------------------------------------------------*/
