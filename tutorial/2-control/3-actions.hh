/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */

#include "3-dependencies.hh"

#include "flecsi/execution.hh"
#include "flecsi/flog.hh"

using namespace dependencies;

/*
  Register several actions under control point one.
 */

int
package_a() {
  flog(info) << "package_a" << std::endl;
  return 0;
}
control::action<package_a, cp::cp1> package_a_action;

int
package_b() {
  flog(info) << "package_b" << std::endl;
  return 0;
}
control::action<package_b, cp::cp1> package_b_action;

int
package_c() {
  flog(info) << "package_c" << std::endl;
  return 0;
}
control::action<package_c, cp::cp1> package_c_action;

int
package_d() {
  flog(info) << "package_d" << std::endl;
  return 0;
}
control::action<package_d, cp::cp1> package_d_action;

/*
  Register several actions under control point one.
 */

int
package_e() {
  flog(info) << "package_e" << std::endl;
  return 0;
}
control::action<package_e, cp::cp2> package_e_action;

int
package_f() {
  flog(info) << "package_f" << std::endl;
  return 0;
}
control::action<package_f, cp::cp2> package_f_action;

int
package_g() {
  flog(info) << "package_g" << std::endl;
  return 0;
}
control::action<package_g, cp::cp2> package_g_action;

/*
  Add dependencies a -> b, b -> d, and a -> d, i.e.,
  b depends on a, d depends on b, and d depends on a.
 */

const auto dep_ba = package_b_action.add(package_a_action);
const auto dep_db = package_d_action.add(package_b_action);
const auto dep_da = package_d_action.add(package_a_action);

/*
  Add dependencies e -> f, e -> g, and f -> g, i.e., f depends on e,
  g depends on e, and g depends on f.
 */

const auto dep_fe = package_f_action.add(package_e_action);
const auto dep_ge = package_g_action.add(package_e_action);
const auto dep_gf = package_g_action.add(package_f_action);
