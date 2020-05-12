#include "control-overview.hh"

#include "flecsi/execution.hh"
#include "flecsi/flog.hh"

using namespace example;

// Control Point 1

int
actionX() {
  flog(info) << __FUNCTION__ << std::endl;
  return 0;
}

control::action<actionX, cp::one> action_x;

// Control Point 2

int
actionA() {
  flog(info) << __FUNCTION__ << std::endl;
  return 0;
}

control::action<actionA, cp::two> action_a;

int
actionB() {
  flog(info) << __FUNCTION__ << std::endl;
  return 0;
}

control::action<actionB, cp::two> action_b;

int
actionC() {
  flog(info) << __FUNCTION__ << std::endl;
  return 0;
}

control::action<actionC, cp::two> action_c;

int
actionD() {
  flog(info) << __FUNCTION__ << std::endl;
  return 0;
}

control::action<actionD, cp::two> action_d;

int
actionE() {
  flog(info) << __FUNCTION__ << std::endl;
  return 0;
}

control::action<actionE, cp::two> action_e;

int
actionF() {
  flog(info) << __FUNCTION__ << std::endl;
  return 0;
}

control::action<actionF, cp::three> action_f;

const auto dep_ba = action_b.add(action_a);
const auto dep_ca = action_c.add(action_a);
const auto dep_db = action_d.add(action_b);
const auto dep_ec = action_e.add(action_c);
const auto dep_ed = action_e.add(action_d);

// Control Point 3

int
actionG() {
  flog(info) << __FUNCTION__ << std::endl;
  return 0;
}

control::action<actionG, cp::three> action_g;

int
actionH() {
  flog(info) << __FUNCTION__ << std::endl;
  return 0;
}

control::action<actionH, cp::three> action_h;

int
actionI() {
  flog(info) << __FUNCTION__ << std::endl;
  return 0;
}

control::action<actionI, cp::three> action_i;

const auto dep_gf = action_g.add(action_f);
const auto dep_hf = action_h.add(action_f);
const auto dep_ig = action_i.add(action_g);
const auto dep_ih = action_i.add(action_h);

// Control Point 3

int
actionY() {
  flog(info) << __FUNCTION__ << std::endl;
  return 0;
}

control::action<actionY, cp::four> action_y;

#if 1
int
actionN() {
  flog(info) << __FUNCTION__ << std::endl;
  return 0;
}

control::action<actionN, cp::two> action_n;
const auto dep_na = action_n.add(action_a);
const auto dep_bn = action_b.add(action_n);
#endif

// Main

int
main(int argc, char ** argv) {
  auto status = flecsi::initialize(argc, argv);

  if(status != flecsi::run::status::success) {
    return status == flecsi::run::status::help ? 0 : status;
  }

  status = control::check_options();

  if(status != flecsi::run::status::success) {
    flecsi::finalize();
    return status == flecsi::run::status::control ? 0 : status;
  } // if

  flecsi::log::add_output_stream("clog", std::clog, true);

  status = flecsi::start(control::execute);

  flecsi::finalize();

  return status;
} // main
