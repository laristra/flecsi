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

#include "flecsi/log/state.hh"
#include "flecsi/log/types.hh"
#include "flecsi/log/utils.hh"

#include <iostream>

namespace flecsi {
namespace log {

/*!
  The message type provides a basic log message type that is customized
  with a formatting policy.
 */

template<typename Policy>
struct message {

  message(const char * file, int line, bool devel = false)
    : file_(file), line_(line), devel_(devel) {
#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: log_message_t constructor " << file
              << " " << line << FLOG_COLOR_PLAIN << std::endl;
#endif
  }

  ~message() {
    if(Policy::strip() or not state::instance().tag_enabled()) {
      return;
    } // if

#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: log_message_t destructor "
              << FLOG_COLOR_PLAIN << std::endl;
#endif

    if(clean_) {
      auto str = ss_.str();
      if(str.back() == '\n') {
        str = str.substr(0, str.size() - 1);
        str += FLOG_COLOR_PLAIN;
        str += '\n';
        ss_.str(std::string{});
        ss_ << str;
      }
      else {
        ss_ << FLOG_COLOR_PLAIN;
      }
    } // if

#if defined(FLOG_ENABLE_MPI)
    if(state::instance().initialized()) {
      state::instance().buffer_output(ss_.str());
    }
    else {
      std::cout << ss_.str();
    } // if
#else
    std::cout << ss_.str();
#endif // FLOG_ENABLE_MPI
  }

  /*
    Make this type work like std::ostream.
   */

  template<typename T>
  message & operator<<(T const & value) {
    ss_ << value;
    return *this;
  }

  /*
    This handles basic manipulators like std::endl.
   */

  message & operator<<(
    ::std::ostream & (*basic_manipulator)(::std::ostream & stream)) {
    ss_ << basic_manipulator;
    return *this;
  }

  /*
    Invoke the policy's formatting method and return this message.
   */

  message & format() {
    clean_ = Policy::format(ss_, file_, line_, devel_);
    return *this;
  }

  /*
    Conversion to bool for ternary usage.
   */

  operator bool() const {
    return true;
  }

private:
  const char * file_;
  int line_;
  bool devel_;
  bool clean_{false};
  std::stringstream ss_;
}; // message

} // namespace log
} // namespace flecsi
