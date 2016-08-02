/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_default_driver_h
#define flecsi_default_driver_h

#include <iostream>

#include "flecsi/utils/common.h"
#include "flecsi/execution/context.h"
#include "flecsi/execution/execution.h"
#include "flecsi/data/data.h"

/*!
 * \file default_driver.h
 * \authors bergen
 * \date Initial file creation: Jul 24, 2016
 */

namespace flecsi {

/*----------------------------------------------------------------------------*
 * Fake mesh definition.
 *----------------------------------------------------------------------------*/

enum mesh_index_spaces_t : size_t {
  vertices,
  edges,
  faces,
  cells
}; // enum mesh_index_spaces_t

enum class privileges : size_t {
  read,
  read_write,
  write_discard
}; // enum data_access_type_t

struct mesh_t : public data_client_t {

  size_t indices(size_t index_space_id) override {

    switch(index_space_id) {
      case cells:
        return 100;
        break;
      default:
        // FIXME: lookup user-defined index space
        assert(false && "unknown index space");
    } // switch
  }

}; // struct mesh_t

// FIXME: Need to try to hide this
template<typename T>
using dense_field_t = storage_t::st_t<dense>::handle_t<double>;

/*----------------------------------------------------------------------------*
 * Task registration.
 *----------------------------------------------------------------------------*/

void task1(double dval, int ival) {
  std::cout << "Executing task1" << std::endl;
  std::cout << "Value(double): " << dval << std::endl;
  std::cout << "Value(int): " << ival << std::endl;
} // task1

register_task(task1, void, double, int);

double task2(double x, double y, dense_field_t<double> p) {
  std::cout << "Executing task2" << std::endl;
  std::cout << "(x,y): (" << x << "," << y << ")" << std::endl;
  std::cout << "Return: " << x*y << std::endl;
//  return x*y;
} // task2

register_task(task2, void, double, double, dense_field_t<double>);

void driver(int argc, char ** argv) {

	mesh_t m;

	// FIXME: need this to come from get_handle
	auto p = register_data(m, "pressure", 1, double, dense, cells);
  double alpha(10.0);

  execute_task(task1, alpha, 5);
  execute_task(task2, alpha, 5.0, p);

} // driver

} // namespace flecsi

#endif // flecsi_default_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
