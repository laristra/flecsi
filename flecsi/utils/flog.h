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

#include "flog/message.h"

#if defined(FLECSI_ENABLE_FLOG)

// Note that none of the tag interface is thread safe. This will have
// to be fixed in the future. One way to do this would be to use TLS
// for the active tag information.
//
// Another feature that would be nice is if the static size_t definition
// failed with helpful information if the user tries to create a tag
// scope for a tag that hasn't been registered.

/*!
  @def flog_register_tag

  Register a tag group with the runtime (flog_t). We need the static
  size_t so that tag scopes can be created quickly during execution.
  
  @ingroup logging 
 */

#define flog_register_tag(name)                                                \
  static size_t name ## _flog_tag_id =                                         \
  flecsi::utils::flog::flog_t::instance().register_tag(_flog_stringify(name))

/*!
  @def flog_tag_guard

  Create a new tag scope.
  
  @ingroup logging 
 */

#define flog_tag_guard(name)                                                   \
  flecsi::utils::flog::flog_tag_scope_t name ## _flog_tag_scope__(             \
    flog_tag_lookup(name))

/*!
  @def flog_tag_map

  Return a std::map of registered tags.
  
  @ingroup logging 
 */

#define flog_tag_map()                                                         \
  flecsi::utils::flog::flog_t::instance().tag_map()

/*!
  @def flog_init(active)

  This call initializes the flog runtime with the list of tags specified
  in \em active.

  @param active A const char * or std::string containing the list of
                active tags. Tags should be comma delimited.

  \b Usage
  \code
  int main(int argc, char ** argv) {

     // Fill a string object with the active tags.
     std::string tags{"init,advance,analysis"};

     // Initialize the flog runtime with active tags, 'init', 'advance',
     // and 'analysis'.
     flog_init(tags);

     return 0;
  } // main
  \endcode

  @ingroup logging
 */

#define flog_init(active)                                                      \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  flecsi::utils::flog::flog_t::instance().init(active)

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

  @ingroup logging
 */

#define flog(severity)                                                         \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  true && flecsi::utils::flog::severity ## _log_message_t(                     \
    __FILE__, __LINE__).stream()

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

  @ingroup logging
 */

#define flog_trace(message)                                                    \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  flecsi::utils::flog::trace_log_message_t(__FILE__, __LINE__).stream() <<     \
    message

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

  @ingroup logging
 */

#define flog_info(message)                                                     \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  flecsi::utils::flog::info_log_message_t(__FILE__, __LINE__).stream() <<      \
    message

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

  @ingroup logging
 */

#define flog_warn(message)                                                     \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  flecsi::utils::flog::warn_log_message_t(__FILE__, __LINE__).stream() <<      \
    message

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

  @ingroup logging
 */

#define flog_error(message)                                                    \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  flecsi::utils::flog::error_log_message_t(__FILE__, __LINE__).stream() <<     \
    message

/*!
  @def clog_fatal(message)

  Throw a runtime exception with the provided message.

  @param message The stream message to be printed.

  @note Fatal level severity log entires are not disabled by tags or
        by the ENABLE_CLOG or CLOG_STRIP_LEVEL build options, i.e.,
        they are always active.

  @b Usage
  @code
  int value{20};

  // Print the value and exit
  clog_fatal("Value: " << value);
  @endcode

  @ingroup clog
 */

#else

#define flog_register_tag(name)
#define flog_tag_lookup(name)
#define flog_tag_guard(name)

#define flog_init(active)
#define flog(severity) if(true) {} else std::cerr
#define flog_trace(message)
#define flog_info(message)
#define flog_warn(message)
#define flog_error(message)

#endif // FLECSI_ENABLE_FLOG

#define flog_fatal(message)                                                    \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  {                                                                            \
  std::stringstream _sstream;                                                  \
  _sstream << OUTPUT_LTRED("FATAL ERROR ") <<                                  \
    OUTPUT_YELLOW(flecsi::utils::flog::rstrip<'/'>(__FILE__) << ":" <<         \
      __LINE__ << " ") << OUTPUT_LTRED(message) << std::endl;                  \
  throw std::runtime_error(_sstream.str());                                    \
  } /* scope */

/*!
  @def clog_assert(test, message)

  Clog assertion interface. Assertions allow the developer to catch
  invalid program state. This call will invoke clog_fatal if the test
  condition is false.

  @param test    The test condition.
  @param message The stream message to be printed.

  @note Failed assertions are not disabled by tags or
        by the ENABLE_CLOG or CLOG_STRIP_LEVEL build options, i.e.,
        they are always active.

  @b Usage
  @code
  int value{20};

  // Print the value and exit
  clog_assert(value == 20, "invalid value");
  @endcode

  @ingroup clog
 */

#define clog_assert(test, message)                                             \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  if(!(test)) { clog_fatal(message); }

