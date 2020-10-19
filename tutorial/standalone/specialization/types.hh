/*
   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

#include <flecsi/data.hh>
#include <flecsi/flog.hh>

namespace standalone {

template<typename T>
using single = flecsi::field<T, flecsi::data::single>;

} // namespace standalone
