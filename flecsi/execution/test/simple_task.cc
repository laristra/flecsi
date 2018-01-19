/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_task_driver_h
#define flecsi_task_driver_h

#include <iostream>
#include <cinchtest.h>

#include <flecsi/utils/common.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>

/*!
 * \file default_driver.h
 * \authors bergen
 * \date Initial file creation: Jul 24, 2016
 */

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Task registration.
//----------------------------------------------------------------------------//

double task(double dval, int ival) {
  clog(info) << "Executing task" << std::endl;
  clog(info) << "Value(double): " << dval << std::endl;
  clog(info) << "Value(int): " << ival << std::endl;
  return dval;
} // task1

void single_task(void) {
  clog(info) << "This is a single task" << std::endl;
}

void index_task(void) {
  clog(info) << "This is an index task" << std::endl;
}

void taskvoid(void) {
  clog(info) << "This is a void(void) task" << std::endl;
}

void mpi_task(void){
 clog(info) << "This is an mpi task" << std::endl;
}

flecsi_register_task(task, flecsi::execution, loc, single | index);
flecsi_register_task(single_task, flecsi::execution, loc, single);
flecsi_register_task(index_task, flecsi::execution, loc, index);
flecsi_register_task(taskvoid, flecsi::execution, loc, single | index);
flecsi_register_task(mpi_task, flecsi::execution, mpi, single);

//----------------------------------------------------------------------------//
// Driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {
  clog(info) << "Inside user driver" << std::endl;

  const double alpha{10.0};

  auto f = flecsi_execute_task(task, flecsi::execution, single, alpha, 5);

  f.wait();

  auto f2 = flecsi_execute_task(task, flecsi::execution, index, alpha, 3);

  f2.wait();

  clog(info) << "Task return: " << f.get() << std::endl;

  auto f3 = flecsi_execute_task(taskvoid, flecsi::execution, index);

  f3.wait();

  auto f4 = flecsi_execute_task(single_task, flecsi::execution, single);

  f4.wait();

  auto f5 = flecsi_execute_task(index_task, flecsi::execution, index);

  f5.wait();

  auto f6 = flecsi_execute_task(mpi_task, flecsi::execution, single);

  //f6.wait();
} // driver

//----------------------------------------------------------------------------//
// TEST.
//----------------------------------------------------------------------------//

TEST(simple_task, testname) {

} // TEST

} // namespace execution
} // namespace flecsi

#endif // flecsi_task_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
