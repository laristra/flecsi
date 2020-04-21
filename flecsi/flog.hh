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

#include "flecsi/log/utils.hh"

#if defined(FLECSI_ENABLE_FLOG)
#include "flecsi/log/message.hh"
#include "flecsi/log/tag_scope.hh"
#endif

#include <iostream>
#include <sstream>

#include <unistd.h>

#if defined(FLECSI_ENABLE_FLOG)

namespace flecsi {
namespace log {

struct guard;

/*!
  Create a tag group to enable/disable output using guards.

  @param label The name of the tag.

  @ingroup flog
 */

struct tag {
  friend guard;

  tag(const char * label) : label_(label) {
    flog_t::instance().register_tag(label);
  }

private:
  std::string label_;
}; // struct tag

/*!
  Create a guard to control output of flog output within the scope of the
  guard.

  @param t The tag group that should enable/disable output.

  @ingroup flog
 */

struct guard {
  guard(tag const & t)
    : scope_(flog_t::instance().lookup_tag(t.label_.c_str())) {}

private:
  tag_scope_t scope_;
}; // struct guard

#if defined(FLOG_ENABLE_DEVELOPER_MODE)
using devel_tag = tag;
using devel_guard = guard;
#else
struct devel_tag {
  devel_tag(const char *) {}
};
struct devel_guard {
  devel_guard(devel_tag const &) {}
};
#endif

/*!
  Add an output stream to FLOG.

  @param label    An identifier for the stream. This can be used to access or
                  update an output stream after it has been added.
  @param stream   The output stream to add.
  @param colorize Indicates whether the output to this stream should be
                  colorized. It is useful to turn colorization off for
                  non-interactive output (default).

  @ingroup flog
 */

inline void
add_output_stream(std::string const & label,
  std::ostream & stream,
  bool colorize = false) {
  flog_t::instance().config_stream().add_buffer(label, stream, colorize);
} // add_output_stream

} // namespace log
} // namespace flecsi

/*!
  @def flog(severity)

  This handles all of the different logging modes for the insertion
  style logging interface.

  @param severity The severity level of the log entry.

  @note The form "true && ..." is necessary for tertiary argument
        evaluation so that the std::ostream & returned by the stream()
        function can be implicitly converted to an int.

  @b Usage
  @code
  int value{20};

  // Print the value at info severity level
  flog(info) << "Value: " << value << std::endl;

  // Print the value at warn severity level
  flog(warn) << "Value: " << value << std::endl;
  @endcode

  @ingroup flog
 */

#define flog(severity)                                                         \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  true && ::flecsi::log::severity##_log_message_t(__FILE__, __LINE__, false)   \
            .stream()

#if defined(FLOG_ENABLE_DEVELOPER_MODE)

#define flog_devel(severity)                                                   \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  true &&                                                                      \
    ::flecsi::log::severity##_log_message_t(__FILE__, __LINE__, true).stream()

#else

#define flog_devel(severity)                                                   \
  if(true) {                                                                   \
  }                                                                            \
  else                                                                         \
    std::cerr

#endif // FLOG_ENABLE_DEVELOPER_MODE

/*!
  @def flog_trace(message)

  Method style interface for trace level severity log entries.

  @param message The stream message to be printed.

  @b Usage
  @code
  int value{20};

  // Print the value at trace severity level
  flog_trace("Value: " << value);
  @endcode

  @ingroup flog
 */

#define flog_trace(message)                                                    \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  ::flecsi::log::trace_log_message_t(__FILE__, __LINE__).stream() << message

/*!
  @def flog_info(message)

  Method style interface for info level severity log entries.

  @param message The stream message to be printed.

  @b Usage
  @code
  int value{20};

  // Print the value at info severity level
  flog_info("Value: " << value);
  @endcode

  @ingroup flog
 */

#define flog_info(message)                                                     \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  ::flecsi::log::info_log_message_t(__FILE__, __LINE__).stream() << message

/*!
  @def flog_warn(message)

  Method style interface for warn level severity log entries.

  @param message The stream message to be printed.

  @b Usage
  @code
  int value{20};

  // Print the value at warn severity level
  flog_warn("Value: " << value);
  @endcode

  @ingroup flog
 */

#define flog_warn(message)                                                     \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  ::flecsi::log::warn_log_message_t(__FILE__, __LINE__).stream() << message

/*!
  @def flog_error(message)

  Method style interface for error level severity log entries.

  @param message The stream message to be printed.

  @b Usage
  @code
  int value{20};

  // Print the value at error severity level
  flog_error("Value: " << value);
  @endcode

  @ingroup flog
 */

#define flog_error(message)                                                    \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  ::flecsi::log::error_log_message_t(__FILE__, __LINE__).stream() << message

#define __flog_internal_wait_on_flusher() usleep(FLOG_PACKET_FLUSH_INTERVAL)

#else // FLECSI_ENABLE_FLOG

namespace flecsi {
namespace log {

struct tag {
  tag(const char *) {}
};
struct guard {
  guard(const char *) {}
};
struct devel_tag {
  devel_tag(const char *) {}
};
struct devel_guard {
  devel_guard(devel_tag const &) {}
};

inline void
add_output_stream(std::string const & label,
  std::ostream & stream,
  bool colorize = false) {}

} // namespace log
} // namespace flecsi

#define flog_initialize(active)
#define flog_finalize()

#define flog(severity)                                                         \
  if(true) {                                                                   \
  }                                                                            \
  else                                                                         \
    std::cerr

#define flog_devel(severity)                                                   \
  if(true) {                                                                   \
  }                                                                            \
  else                                                                         \
    std::cerr

#define flog_trace(message)
#define flog_info(message)
#define flog_warn(message)
#define flog_error(message)

#define __flog_internal_wait_on_flusher()

#endif // FLECSI_ENABLE_FLOG

/*!
  @def fixme

  Alias for severity level warn.

  @ingroup flog
 */

#define fixme() flog(warn)

#include <boost/stacktrace.hpp>

namespace flecsi {
namespace log {

inline void
dumpstack() {
#if !defined(NDEBUG)
  std::cerr << FLOG_OUTPUT_RED("FleCSI Runtime: std::abort called.")
            << std::endl
            << FLOG_OUTPUT_GREEN("Dumping stacktrace...") << std::endl;
  std::cerr << boost::stacktrace::stacktrace() << std::endl;
#else
  std::cerr << FLOG_OUTPUT_RED("FleCSI Runtime: std::abort called.")
            << std::endl
            << FLOG_OUTPUT_BROWN("Build with '-DCMAKE_BUILD_TYPE=Debug'"
                                 << " to enable FleCSI runtime stacktrace.")
            << std::endl;
#endif
} // dumpstack

} // namespace log
} // namespace flecsi

/*!
  @def flog_fatal(message)

  Throw a runtime exception with the provided message.

  @param message The stream message to be printed.

  @note Fatal level severity log entires are not disabled by tags or
        by the ENABLE_FLOG or FLOG_STRIP_LEVEL build options, i.e.,
        they are always active.

  @b Usage
  @code
  int value{20};

  // Print the value and exit
  flog_fatal("Value: " << value);
  @endcode

  @ingroup flog
 */

#define flog_fatal(message)                                                    \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  {                                                                            \
    std::stringstream _sstream;                                                \
    _sstream << FLOG_OUTPUT_LTRED("FATAL ERROR ")                              \
             << FLOG_OUTPUT_YELLOW(::flecsi::log::rstrip<'/'>(__FILE__)        \
                                   << ":" << __LINE__ << " ")                  \
             << FLOG_OUTPUT_LTRED(message) << std::endl;                       \
    __flog_internal_wait_on_flusher();                                         \
    std::cerr << _sstream.str() << std::endl;                                  \
    ::flecsi::log::dumpstack();                                                \
    std::abort();                                                              \
  } /* scope */

/*!
  @def flog_assert(test, message)

  Clog assertion interface. Assertions allow the developer to catch
  invalid program state. This call will invoke flog_fatal if the test
  condition is false.

  @param test    The test condition.
  @param message The stream message to be printed.

  @note Failed assertions are not disabled by tags or
        by the ENABLE_FLOG or FLOG_STRIP_LEVEL build options, i.e.,
        they are always active.

  @b Usage
  @code
  int value{20};

  // Print the value and exit
  flog_assert(value == 20, "invalid value");
  @endcode

  @ingroup flog
 */

/*
  This implementation avoids unused variables error
  Attribution: https://stackoverflow.com/questions/777261/
  avoiding-unused-variables-warnings-when-using-assert-in-a-release-build
 */
#ifdef NDEBUG
#define flog_assert(test, message)                                             \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  do {                                                                         \
    (void)sizeof(test);                                                        \
  } while(0)
#else
#define flog_assert(test, message)                                             \
  /* MACRO IMPLEMENTATION */                                                   \
                                                                               \
  do                                                                           \
    if(!(test)) {                                                              \
      flog_fatal(message);                                                     \
    }                                                                          \
  while(0)
#endif // NDEBUG
