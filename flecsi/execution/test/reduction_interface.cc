/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <cinchdevel.h>

#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>

#include <flecsi/execution/test/harness.h>

clog_register_tag(reduction_interface);

//----------------------------------------------------------------------------//
// Generic sum reduction type.
//----------------------------------------------------------------------------//

template<typename T> T reduction_init_zero__() { return T{0}; }

template<typename T, T (*INITIAL)() = reduction_init_zero__>
struct reduction_sum__ {

  using LHS = T;
  using RHS = T;
  static constexpr T identity{};

  template<bool EXCLUSIVE = true>
  static void apply(LHS & lsh, RHS rhs) {
    lsh += rhs;
  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(LHS & lsh, RHS rhs) {
    lsh += rhs;
  } // apply

  static T initial() {
    return INITIAL();
  } // initial

}; // struct reduction_sum__

//----------------------------------------------------------------------------//
// Generic product reduction type.
//----------------------------------------------------------------------------//

template<typename T, T (*INITIAL)() = reduction_init_zero__>
struct reduction_product__ {

  using LHS = T;
  using RHS = T;
  static constexpr T identity{};

  template<bool EXCLUSIVE = true>
  static void apply(LHS & lsh, RHS rhs) {
    lsh *= rhs;
  } // apply

  template<bool EXCLUSIVE = true>
  static void fold(LHS & lsh, RHS rhs) {
    lsh *= rhs;
  } // apply

  static T initial() {
    return INITIAL();
  } // initial

}; // reduction_product__

//----------------------------------------------------------------------------//
// Complex number type for test example.
//----------------------------------------------------------------------------//

struct complex_t {
  double real;
  double imag;

  constexpr complex_t()
    : real(0.0), imag(0.0) {}

  complex_t(double real_, double imag_)
    : real(real_), imag(imag_) {}

  complex_t(double value_)
    : complex_t(value_, value_) {}

  complex_t & operator *= (complex_t const & c) {
    complex_t tmp{ this->real*c.real - this->imag*c.imag,
      this->real*c.imag + this->imag*c.real };
    *this = tmp;
    return *this;
  } // operator *=

}; // struct complex_t

using sum_double = reduction_sum__<double>;
using prod_complex = reduction_product__<complex_t>;

flecsi_register_reduction_operation(sum, sum_double);
flecsi_register_reduction_operation(product, prod_complex);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Variable registration
//----------------------------------------------------------------------------//

flecsi_register_field(mesh_t, data, values, double, dense, 1,
  index_spaces::cells);

//----------------------------------------------------------------------------//
// Initialize values
//----------------------------------------------------------------------------//

size_t initialize_values(mesh<ro> m, field<rw, rw, ro> v) {
  size_t sum{0};

  for(auto c: m.cells(owned)) {
    ++sum;
  } // for

  return sum;
} // initialize_values

flecsi_register_task(initialize_values, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// Print task
//----------------------------------------------------------------------------//

void print_values(mesh<ro> m, field<ro, ro, ro> v) {
} // print_values

flecsi_register_task(print_values, flecsi::execution, loc, single);

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void driver(int argc, char ** argv) {

  auto mh = flecsi_get_client_handle(mesh_t, clients, m);
  auto vh = flecsi_get_handle(mh, data, values, double, dense, 0);

  auto f = flecsi_execute_reduction_task(initialize_values, flecsi::execution,
    single, sum, mh, vh);

  std::cout << "reduction: " << f.get() << std::endl;
} // driver

} // namespace execution
} // namespace flecsi

DEVEL(registration_interface) {}

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
