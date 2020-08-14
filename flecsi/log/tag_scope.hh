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
#pragma once

/*! @file */

#include <flecsi-config.h>

#if defined(FLECSI_ENABLE_FLOG)

#include "flecsi/log/state.hh"

namespace flecsi {
namespace log {

/*!
  This type sets the active tag id to the id passed to the constructor,
  stashing the current active tag. When the instance goes out of scope,
  the active tag is reset to the stashed value.
 */

struct tag_scope_t {
  tag_scope_t(size_t tag = 0) : stash_(state::instance().active_tag()) {
#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: activating tag " << tag
              << FLOG_COLOR_PLAIN << std::endl;
#endif

    // Warn users about externally-scoped messages
    if(!state::instance().initialized()) {
      std::cerr
        << FLOG_COLOR_YELLOW << "FLOG: !!!WARNING You cannot use "
        << "tag guards for externally scoped messages!!! "
        << "This message will be active if FLOG_ENABLE_EXTERNAL is defined!!!"
        << FLOG_COLOR_PLAIN << std::endl;
    } // if

    state::instance().active_tag() = tag;
  } // tag_scope_t

  ~tag_scope_t() {
    state::instance().active_tag() = stash_;
  } // ~tag_scope_t

private:
  size_t stash_;

}; // tag_scope_t

} // namespace log
} // namespace flecsi

#endif // FLECSI_ENABLE_FLOG
