/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi-config.h>

#include "colors.h"

#include <iostream>

#include "flog_types.h"

namespace flecsi {
namespace utils {
namespace flog {

/*!
  Function always returning true. Used for defaults.
 */

inline
bool
true_state()
{
  return true;
} // true_state

/*!
  Create a timestamp.
 */

inline
std::string
timestamp(bool underscores = false)
{
  char stamp[14];
  time_t t = time(0);
  std::string format = underscores ? "%m%d_%H%M%S" : "%m%d %H:%M:%S";
  strftime(stamp, sizeof(stamp), format.c_str(), localtime(&t));
  return std::string(stamp);
} // timestamp

/*!
  Strip path from string up to last character C.

  @tparam C The character to strip.
 */

template<char C>
std::string rstrip(const char *file) {
  std::string tmp(file);
  return tmp.substr(tmp.rfind(C)+1);
} // rstrip

/*!
  The log_message_t type provides a base class for implementing
  formatted logging utilities.
 */

template<typename P>
struct log_message_t
{ /*!
    Constructor.

    @tparam P Predicate function type.

    @param file            The current file (where the log message was
                           created).  In general, this will always use the
                           __FILE__ parameter from the calling macro.
    @param line            The current line (where the log message was called).
                           In general, this will always use the __LINE__
                           parameter from the calling macro.
    @param predicate       The predicate function to determine whether or not
                           the calling runtime should produce output.
    @param can_send_to_one A boolean indicating whether the calling scope
                           can route messages through one rank.
   */

  log_message_t(
    const char * file,
    int line,
    P && predicate,
    bool can_send_to_one = true
  )
  :
    file_(file), line_(line), predicate_(predicate),
    can_send_to_one_(can_send_to_one), clean_color_(false)
  {
#if defined(FLOG_DEBUG)
    std::cerr << COLOR_LTGRAY << "FLOG: log_message_t constructor " <<
      file << " " << line << COLOR_PLAIN << std::endl;
#endif
  } // log_message_t

  virtual
  ~log_message_t()
  {
#if defined(FLOG_DEBUG)
    std::cerr << COLOR_LTGRAY << "FLOG: log_message_t destructor " <<
      COLOR_PLAIN << std::endl;
#endif

#if !defined(SERIAL) && defined(FLOG_ENABLE_MPI)
    if(can_send_to_one_) {
      send_to_one(flog_t::instance().buffer_stream().str().c_str());
    }
    else {
      flog_t::instance().stream() << flog_t::instance().buffer_stream().str();
    } // if
#else
    flog_t::instance().stream() << flog_t::instance().buffer_stream().str();
#endif

    flog_t::instance().buffer_stream().str(std::string{});
  } // ~log_message_t

  /*!
    Return the output stream. Override this method to add additional
    formatting to a particular severity output.
   */

  virtual
  std::ostream &
  stream()
  {
    return flog_t::instance().severity_stream(predicate_());
  } // stream

protected:

  const char * file_;
  int line_;
  P & predicate_;
  bool can_send_to_one_;
  bool clean_color_;

}; // struct log_message_t

//----------------------------------------------------------------------------//
// Convenience macro to define severity levels.
//
// Log message types defined using this macro always use the default
// predicate function, true_state().
//----------------------------------------------------------------------------//

#define severity_message_t(severity, P, format)                                \
struct severity ## _log_message_t                                              \
  : public log_message_t<P>                                                    \
{                                                                              \
  severity ## _log_message_t(                                                  \
    const char * file,                                                         \
    int line,                                                                  \
    P && predicate = true_state,                                               \
    bool can_send_to_one = true                                                \
  )                                                                            \
    : log_message_t<P>(file, line, predicate, can_send_to_one) {}              \
                                                                               \
  ~severity ## _log_message_t()                                                \
  {                                                                            \
    /* Clean colors from the stream */                                         \
    if(clean_color_) {                                                         \
      flog_t::instance().buffer_stream() << COLOR_PLAIN;                       \
    }                                                                          \
  }                                                                            \
                                                                               \
  std::ostream &                                                               \
  stream() override                                                            \
    /* This is replaced by the scoped logic */                                 \
    format                                                                     \
}

#define message_stamp                                                          \
  timestamp() << " " << rstrip<'/'>(file_) << ":" << line_

#if !defined(SERIAL) && defined(FLOG_ENABLE_MPI)
  #define mpi_stamp " r" << mpi_state_t::instance().rank()
#else
  #define mpi_stamp ""
#endif

// Trace
severity_message_t(trace, decltype(flecsi::utils::flog::true_state),
  {
    std::ostream & stream =
      flog_t::instance().severity_stream(FLOG_STRIP_LEVEL < 1 &&
        predicate_() && flog_t::instance().tag_enabled());

    {
    stream << OUTPUT_CYAN("[T") << OUTPUT_LTGRAY(message_stamp);
    stream << OUTPUT_DKGRAY(mpi_stamp);
    stream << OUTPUT_CYAN("] ");
    } // scope

    return stream;
  });

// Info
severity_message_t(info, decltype(flecsi::utils::flog::true_state),
  {
    std::ostream & stream =
      flog_t::instance().severity_stream(FLOG_STRIP_LEVEL < 2 &&
        predicate_() && flog_t::instance().tag_enabled());

    {
    stream << OUTPUT_GREEN("[I") << OUTPUT_LTGRAY(message_stamp);
    stream << OUTPUT_DKGRAY(mpi_stamp);
    stream << OUTPUT_GREEN("] ");
    } // scope

    return stream;
  });

// Warn
severity_message_t(warn, decltype(flecsi::utils::flog::true_state),
  {
    std::ostream & stream =
      flog_t::instance().severity_stream(FLOG_STRIP_LEVEL < 3 &&
        predicate_() && flog_t::instance().tag_enabled());

    {
    stream << OUTPUT_BROWN("[W") << OUTPUT_LTGRAY(message_stamp);
    stream << OUTPUT_DKGRAY(mpi_stamp);
    stream << OUTPUT_BROWN("] ") << COLOR_YELLOW;
    } // scope

    clean_color_ = true;
    return stream;
  });

// Error
severity_message_t(error, decltype(flecsi::utils::flog::true_state),
  {
    std::ostream & stream = std::cerr;

    {
    stream << OUTPUT_RED("[E") << OUTPUT_LTGRAY(message_stamp);
    stream << OUTPUT_DKGRAY(mpi_stamp);
    stream << OUTPUT_RED("] ") << COLOR_LTRED;
    } // scope

    clean_color_ = true;
    return stream;
  });

#undef severity_message_t

} // namespace flog
} // namespace utils
} // namespace flecsi
