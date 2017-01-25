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
namespace execution {

//----------------------------------------------------------------------------//
// Fake mesh definition.
//----------------------------------------------------------------------------//

enum mesh_index_spaces_t : size_t {
  vertices,
  edges,
  faces,
  cells
}; // enum mesh_index_spaces_t

enum mesh_data_attribute_t : size_t {
  type_a,
  type_b
}; // enum mesh_attribute_t

enum class privileges : size_t {
  read,
  read_write,
  write_discard
}; // enum data_access_type_t

struct mesh_t : public data::data_client_t {

  size_t indices(size_t index_space_id) const override {

    switch(index_space_id) {
      case cells:
        return 10;
        break;
      default:
        // FIXME: lookup user-defined index space
        assert(false && "unknown index space");
    } // switch

  } // indices

}; // struct mesh_t

// FIXME: Need to try to hide this
using namespace flecsi::data;
template<typename T>
using dense_field_t = storage_t::st_t<dense>::handle_t<double>;

//----------------------------------------------------------------------------//
// Task registration.
//----------------------------------------------------------------------------//

double task1(double dval, int ival) {
  std::cout << "Executing task1" << std::endl;
  std::cout << "Value(double): " << dval << std::endl;
  std::cout << "Value(int): " << ival << std::endl;
  return dval;
} // task1

//#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion
//  register_task(task1, mpi, index, void, double, int);
//#else
  register_task(task1, loc, single);
//#endif

#if 1
double task2(double x, double y, dense_field_t<double> p) {
  std::cout << "Executing task2" << std::endl;
  std::cout << "(x,y): (" << x << "," << y << ")" << std::endl;
  std::cout << "Return: " << x*y << std::endl;
  return x*y;
} // task2

register_task(task2, loc, single);

double task3(double val) {
   std::cout << "Executing task3 with index launcher" << std::endl;
   return 1.0;
}

register_task(task3, loc, index);
#endif


// This is here to make sure that references can be used in the serial version
#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_serial

double task4(double x, double & y, dense_field_t<double> p) {
  std::cout << "Executing task4" << std::endl;
  std::cout << "(x,y): (" << x << "," << y << ")" << std::endl;
  std::cout << "Return: " << x*y << std::endl;
  return x*y;
} // task4

register_task(task4, loc, single);

#endif

//----------------------------------------------------------------------------//
// Function registration.
//----------------------------------------------------------------------------//

double eos_gruneisen(double r, double e) {
  std::cout << "Executing gruneisen" << std::endl;
  std::cout << "(r,e): (" << r << "," << e << ")" << std::endl;
  return r*e;
} // function1

register_function(eos_gruneisen);

double eos_gamma(double r, double e) {
  std::cout << "Executing gamma" << std::endl;
  std::cout << "(r,e): (" << r << "," << e << ")" << std::endl;
  return 2*r*e;
} // function1

register_function(eos_gamma);

/*
  Templated function
 */
struct eos_param_t {
  static constexpr double m = 4.0;
}; // struct eos_params_t

template<typename P>
double eos_other__(double r, double e) {
  std::cout << "Executing other" << std::endl;
  std::cout << "(r,e): (" << r << "," << e << ")" << std::endl;
  return P::m*r*e;
} // function1

double eos_other(double r, double e) {
  return eos_other__<eos_param_t>(r, e);
} // eos_other

register_function(eos_other);

//----------------------------------------------------------------------------//
// User type.
//----------------------------------------------------------------------------//

define_function_type(eos_function_t, double, double, double);

struct material_t {
  eos_function_t eos_function;
  double r;
  double e;

  double eos() {
    return execute_function(eos_function, r, e);
  } // eos

  material_t(eos_function_t eos_function_, double r_, double e_)
    : eos_function(eos_function_), r(r_), e(e_) {}
}; // struct material_t

struct copper_t : material_t {
  copper_t(double r_, double e_)
    : material_t(function_handle(eos_gruneisen), r_, e_) {}
};

struct steel_t : material_t {
  steel_t(double r_, double e_)
    : material_t(function_handle(eos_gamma), r_, e_) {}
};

struct silver_t : material_t {
  silver_t(double r_, double e_)
    : material_t(function_handle(eos_other), r_, e_) {}
};

//----------------------------------------------------------------------------//
// Driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {

  mesh_t m;

  // FIXME: need this to come from get_handle
  auto p = register_data(m, hydro, pressure, double, dense, 1, cells);
  double alpha(10.0);

//#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_mpilegion
//  execute_task(task1, mpi, index, alpha, 5);
//#else

  auto f1 = execute_task(task1, loc, single, alpha, 5);

  f1.wait();

  auto f2 = execute_task(task2, loc, single, alpha, 5.0, p);

  execute_task(task3, loc, index, 5.0);

#if FLECSI_RUNTIME_MODEL == FLECSI_RUNTIME_MODEL_serial
  auto f4 = execute_task(task4, loc, single, alpha, 5.0, p);
#endif

  register_data(m, hydro, materials, material_t, dense, 1, cells);

  auto mats1 = get_accessor(m, hydro, materials, material_t, dense, 0);

  for(size_t i(0); i<4; ++i) {
    mats1(i) = copper_t(2.0, 2.0);
  } // for

  for(size_t i(4); i<8; ++i) {
    mats1(i) = steel_t(2.0, 2.0);
  } // for

  for(size_t i(8); i<10; ++i) {
    mats1(i) = silver_t(2.0, 2.0);
  } // for

  for(size_t i(0); i<10; ++i) {
    std::cout << mats1(i).eos() << std::endl;
  } // for

#if 1
  std::cout << "f2.get(): " << f2.get() << std::endl;
  std::cout << "f1.get(): " << f1.get() << std::endl;
#endif

} // driver

} // namespace execution
} // namespace flecsi

#endif // flecsi_default_driver_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
