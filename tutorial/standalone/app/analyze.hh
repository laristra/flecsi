/*
   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

#include "specialization/control.hh"

namespace standalone {
namespace action {

int
analyze() {
  return 0;
}
control::action<analyze, cp::analyze> analyze_action;

} // namespace action
} // namespace standalone
