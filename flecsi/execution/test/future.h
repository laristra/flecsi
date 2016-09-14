/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_future_h
#define flecsi_future_h

#include <iostream>

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"

/*!
 * \file future.h
 * \authors demeshko
 * \date Initial file creation: Jul 24, 2016
 */

/// this test check future_t class in FLeCSI

namespace flecsi {
namespace execution {


/*----------------------------------------------------------------------------*
 * Task registration.
 *----------------------------------------------------------------------------*/

void task1(double dval, int ival) {
  std::cout << "Executing task1" << std::endl;
  std::cout << "Value(double): " << dval << std::endl;
  std::cout << "Value(int): " << ival << std::endl;
} // task1

#if FLECSI_RUNTIME_MODEL_mpilegion
register_task(task1, mpi, index, void, double, int);
#else
register_task(task1, loc, single, void, double, int);
#endif

double task2(double x, double y, double p) {
  std::cout << "Executing task2" << std::endl;
  std::cout << "(x,y): (" << x << "," << y << ")" << std::endl;
  std::cout << "Return: " << x*y << std::endl;
//  return x*y;
} // task2

register_task(task2, loc, single, void, double, double, double);

void task3(double val) {
   std::cout << "Executing task3 with index launcher" << std::endl;
}

register_task(task3, loc, index, void, double);

/*----------------------------------------------------------------------------*
 * Driver.
 *----------------------------------------------------------------------------*/

void driver(int argc, char ** argv) {


#if FLECSI_RUNTIME_MODEL_mpilegion
  execute_task(task1, mpi, index, 1.55, 5);
  //f1.wait();
#else
  //auto f1 = 
  execute_task(task1, loc, single, 1.55, 5);
  //f1.wait();
#endif
  //auto f2 =  
  execute_task(task2, loc, single,  1.55, 5.0, 3.14);
 // f2.wait();

 //  auto f3= 
  execute_task(task3, loc, index, 5.0);
  //f3.wait();
} // driver

} // namespace execution
} // namespace flecsi

#endif // flecsi_future_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
