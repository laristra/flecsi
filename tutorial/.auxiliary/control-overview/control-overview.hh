#pragma once

#include "flecsi/control.hh"
#include "flecsi/flog.hh"

namespace example {

enum class cp { one, two, three, four };

inline const char *
operator*(cp control_point) {
  switch(control_point) {
    case cp::one:
      return "Control Point 1";
    case cp::two:
      return "Control Point 2";
    case cp::three:
      return "Control Point 3";
    case cp::four:
      return "Control Point 4";
  }
  flog_fatal("invalid control point");
}

struct control_policy {
  using control_points_enum = cp;
  struct node_policy {};

  using control = flecsi::control<control_policy>;

  size_t & step() {
    return step_;
  }

  static bool step_control() {
    return control::instance().step()++ < 5;
  }

  template<auto CP>
  using control_point = flecsi::control_point<CP>;

  using cycle = flecsi::
    cycle<step_control, control_point<cp::two>, control_point<cp::three>>;

  using control_points =
    std::tuple<control_point<cp::one>, cycle, control_point<cp::four>>;

private:
  size_t step_{0};
};

using control = flecsi::control<control_policy>;

} // namespace example
