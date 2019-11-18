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

#include <flecsi/utils/flog/state.hh>
#include <flecsi/utils/flog/types.hh>
#include <flecsi/utils/flog/utils.hh>

#include <iostream>

namespace flecsi {
namespace utils {
namespace flog {

/*!
  Function always returning true. Used for defaults.
 */

inline bool
true_state() {
  return true;
} // true_state

/*!
  The log_message_t type provides a base class for implementing
  formatted logging utilities.
 */

struct log_message_t {

  /*!
    Constructor.

    @param file            The current file (where the log message was
                            created). In general, this will always use the
                            __FILE__ parameter from the calling macro.
    @param line            The current line (where the log message was
                           called). In general, this will always use the
                           __LINE__ parameter from the calling macro.
*/

  log_message_t(const char * file, int line)
    : file_(file), line_(line), clean_color_(false) {
#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: log_message_t constructor " << file
              << " " << line << FLOG_COLOR_PLAIN << std::endl;
#endif
  } // log_message_t

  virtual ~log_message_t() {
#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: log_message_t destructor "
              << FLOG_COLOR_PLAIN << std::endl;
#endif

#if defined(FLOG_ENABLE_MPI)
    if(flog_t::instance().initialized()) {
      flog_t::instance().buffer_output();
    }
    else {
      if(!flog_t::instance().initialized()) {
#if defined(FLOG_ENABLE_EXTERNAL)
        std::cout << flog_t::instance().buffer_stream().str();
#endif
      }
      else {
        flog_t::instance().stream() << flog_t::instance().buffer_stream().str();
      } // if
    } // if
#else
    flog_t::instance().stream() << flog_t::instance().buffer_stream().str();
#endif // FLOG_ENABLE_MPI

    flog_t::instance().buffer_stream().str(std::string{});
  } // ~log_message_t

  /*!
    Return the output stream. Override this method to add additional
    formatting to a particular severity output.
   */

  virtual std::ostream & stream() {
    return flog_t::instance().severity_stream();
  } // stream

protected:
  const char * file_;
  int line_;
  bool clean_color_;

}; // struct log_message_t

/*----------------------------------------------------------------------------*
  Convenience macro to define severity levels.
 *----------------------------------------------------------------------------*/

#define severity_message_t(severity, format)                                   \
  struct severity##_log_message_t : public log_message_t {                     \
    severity##_log_message_t(const char * file, int line, bool devel = false)  \
      : log_message_t(file, line), devel_(devel) {}                            \
                                                                               \
    ~severity##_log_message_t() {                                              \
      /* Clean colors from the stream */                                       \
      if(clean_color_) {                                                       \
        auto str = flog_t::instance().buffer_stream().str();                   \
        if(str.back() == '\n') {                                               \
          str = str.substr(0, str.size() - 1);                                 \
          str += FLOG_COLOR_PLAIN;                                             \
          str += '\n';                                                         \
          flog_t::instance().buffer_stream().str(std::string{});               \
          flog_t::instance().buffer_stream() << str;                           \
        }                                                                      \
        else {                                                                 \
          flog_t::instance().buffer_stream() << FLOG_COLOR_PLAIN;              \
        }                                                                      \
      }                                                                        \
    }                                                                          \
                                                                               \
    std::ostream &                                                             \
    stream() override /* This is replaced by the scoped logic */               \
      format                                                                   \
                                                                               \
      private : bool devel_;                                                   \
  }

#define verbose_stamp                                                          \
  timestamp() << " " << rstrip<'/'>(file_) << ":" << line_ << " "

#if defined(FLOG_ENABLE_MPI)
#define process_stamp " p" << flog_t::instance().process()
#else
#define process_stamp ""
#endif

//----------------------------------------------------------------------------//
// utility
//
// This is currently only used by ftest.
//----------------------------------------------------------------------------//

severity_message_t(utility, {
  (void)devel_;
  std::ostream & stream =
    flog_t::instance().severity_stream(flog_t::instance().tag_enabled());
  return stream;
});

//----------------------------------------------------------------------------//
// trace
//----------------------------------------------------------------------------//

severity_message_t(trace, {
  std::ostream & stream = flog_t::instance().severity_stream(
    FLOG_STRIP_LEVEL < 1 && flog_t::instance().tag_enabled());

  std::string devel = devel_ ? "(devel)" : "";
  std::string active_tag = flog_t::instance().initialized()
                             ? flog_t::instance().active_tag_name()
                             : "external";

  switch(flog_t::instance().verbose()) {
    case 1: {
      stream << FLOG_OUTPUT_CYAN("[trace ") << FLOG_OUTPUT_PURPLE(devel);
      stream << FLOG_OUTPUT_LTGRAY(verbose_stamp);
      stream << FLOG_OUTPUT_CYAN(active_tag);
      stream << FLOG_OUTPUT_GREEN(process_stamp);
      stream << FLOG_OUTPUT_CYAN("] ");
    } // scope
    break;

    case 0: {
      stream << FLOG_OUTPUT_CYAN("[trace ") << FLOG_OUTPUT_PURPLE(devel);
      stream << FLOG_OUTPUT_CYAN(active_tag);
      stream << FLOG_OUTPUT_GREEN(process_stamp);
      stream << FLOG_OUTPUT_CYAN("] ");
    } // scope
    break;

    default:
      break;
  } // switch

  return stream;
});

//----------------------------------------------------------------------------//
// info
//----------------------------------------------------------------------------//

severity_message_t(info, {
  std::ostream & stream = flog_t::instance().severity_stream(
    FLOG_STRIP_LEVEL < 2 && flog_t::instance().tag_enabled());

  std::string devel = devel_ ? "devel " : "";
  std::string active_tag = flog_t::instance().initialized()
                             ? flog_t::instance().active_tag_name()
                             : "external";

  switch(flog_t::instance().verbose()) {
    case 1: {
      stream << FLOG_OUTPUT_GREEN("[info ") << FLOG_OUTPUT_PURPLE(devel);
      stream << FLOG_OUTPUT_LTGRAY(verbose_stamp);
      stream << FLOG_OUTPUT_CYAN(active_tag);
      stream << FLOG_OUTPUT_GREEN(process_stamp);
      stream << FLOG_OUTPUT_GREEN("] ");
    } // scope
    break;

    case 0: {
      stream << FLOG_OUTPUT_GREEN("[info ") << FLOG_OUTPUT_PURPLE(devel);
      stream << FLOG_OUTPUT_CYAN(active_tag);
      stream << FLOG_OUTPUT_GREEN(process_stamp);
      stream << FLOG_OUTPUT_GREEN("] ");
    } // scope
    break;

    default:
      break;
  } // switch

  return stream;
});

//----------------------------------------------------------------------------//
// warn
//----------------------------------------------------------------------------//

severity_message_t(warn, {
  std::ostream & stream = flog_t::instance().severity_stream(
    FLOG_STRIP_LEVEL < 3 && flog_t::instance().tag_enabled());

  std::string devel = devel_ ? "(devel)" : "";
  std::string active_tag = flog_t::instance().initialized()
                             ? flog_t::instance().active_tag_name()
                             : "external";

  switch(flog_t::instance().verbose()) {
    case 1: {
      stream << FLOG_OUTPUT_BROWN("[Warn ") << FLOG_OUTPUT_PURPLE(devel);
      stream << FLOG_OUTPUT_LTGRAY(verbose_stamp);
      stream << FLOG_OUTPUT_CYAN(active_tag);
      stream << FLOG_OUTPUT_GREEN(process_stamp);
      stream << FLOG_OUTPUT_BROWN("] ") << FLOG_COLOR_YELLOW;
    } // scope
    break;

    case 0: {
      stream << FLOG_OUTPUT_BROWN("[Warn ") << FLOG_OUTPUT_PURPLE(devel);
      stream << FLOG_OUTPUT_CYAN(active_tag);
      stream << FLOG_OUTPUT_GREEN(process_stamp);
      stream << FLOG_OUTPUT_BROWN("] ") << FLOG_COLOR_YELLOW;
    } // scope
    break;

    default:
      break;
  } // switch

  clean_color_ = true;
  return stream;
});

//----------------------------------------------------------------------------//
// error
//----------------------------------------------------------------------------//

severity_message_t(error, {
  std::ostream & stream = flog_t::instance().severity_stream(
    FLOG_STRIP_LEVEL < 4 && flog_t::instance().tag_enabled());

  std::string devel = devel_ ? "(devel)" : "";
  std::string active_tag = flog_t::instance().initialized()
                             ? flog_t::instance().active_tag_name()
                             : "external";

  switch(flog_t::instance().verbose()) {
    case 1: {
      stream << FLOG_OUTPUT_RED("[ERROR ") << FLOG_OUTPUT_PURPLE(devel);
      stream << FLOG_OUTPUT_LTGRAY(verbose_stamp);
      stream << FLOG_OUTPUT_CYAN(active_tag);
      stream << FLOG_OUTPUT_GREEN(process_stamp);
      stream << FLOG_OUTPUT_RED("] ") << FLOG_COLOR_LTRED;
    } // scope
    break;

    case 0: {
      stream << FLOG_OUTPUT_RED("[ERROR ") << FLOG_OUTPUT_PURPLE(devel);
      stream << FLOG_OUTPUT_CYAN(active_tag);
      stream << FLOG_OUTPUT_GREEN(process_stamp);
      stream << FLOG_OUTPUT_RED("] ") << FLOG_COLOR_LTRED;
    } // scope
    break;

    default:
      break;
  } // switch

  clean_color_ = true;
  return stream;
});

#undef severity_message_t

} // namespace flog
} // namespace utils
} // namespace flecsi
