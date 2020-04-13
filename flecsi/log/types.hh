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

#include "flecsi/log/packet.hh"
#include "flecsi/log/utils.hh"

#include <cassert>
#include <unordered_map>

namespace flecsi {
namespace log {

/*!
  The tee_buffer_t type provides a stream buffer that allows output to
  multiple targets.

  @ingroup logging
 */

class tee_buffer_t : public std::streambuf
{
public:
  /*!
    The buffer_data_t type is used to hold state and the actual low-level
    stream buffer pointer.
   */

  struct buffer_data_t {
    bool enabled;
    bool colorized;
    std::streambuf * buffer;
  }; // struct buffer_data_t

  /*!
    Add a buffer to which output should be written. This also enables
    the buffer,i.e., output will be written to it. For a given key,
    only the first call to this method will have an effect.
   */

  void add_buffer(std::string key, std::streambuf * sb, bool colorized) {
    if(buffers_.find(key) == buffers_.end()) {
      buffers_[key].enabled = true;
      buffers_[key].buffer = sb;
      buffers_[key].colorized = colorized;
    } // if
  } // add_buffer

  /*!
    Enable a buffer so that output is written to it. This is mainly
    for buffers that have been disabled and need to be re-enabled.
   */

  bool enable_buffer(std::string key) {
    assert(buffers_.find(key) != buffers_.end());
    buffers_[key].enabled = true;
    return buffers_[key].enabled;
  } // enable_buffer

  /*!
    Disable a buffer so that output is not written to it.
   */

  bool disable_buffer(std::string key) {
    assert(buffers_.find(key) != buffers_.end());
    buffers_[key].enabled = false;
    return buffers_[key].enabled;
  } // disable_buffer

protected:
  /*!
    Override the overflow method. This streambuf has no buffer, so overflow
    happens for every character that is written to the string, allowing
    us to write to multiple output streams. This method also detects
    colorization strings embedded in the character stream and removes
    them from output that is going to non-colorized buffers.

    \param c The character to write. This is passed in as an int so that
             non-characters like EOF can be written to the stream.
   */

  virtual int overflow(int c) {
    if(c == EOF) {
      return !EOF;
    }
    else {
      // Get the size before we add the current character
      const size_t tbsize = test_buffer_.size();

      // Buffer the output for now...
      test_buffer_.append(1, char(c)); // takes char

      switch(tbsize) {

        case 0:
          if(c == '\033') {
            // This could be a color string, start buffering
            return c;
          }
          else {
            // No match, go ahead and write the character
            return flush_buffer(all_buffers);
          } // if

        case 1:
          if(c == '[') {
            // This still looks like a color string, keep buffering
            return c;
          }
          else {
            // This is some other kind of escape. Write the
            // buffered output to all buffers.
            return flush_buffer(all_buffers);
          } // if

        case 2:
          if(c == '0' || c == '1') {
            // This still looks like a color string, keep buffering
            return c;
          }
          else {
            // This is some other kind of escape. Write the
            // buffered output to all buffers.
            return flush_buffer(all_buffers);
          } // if

        case 3:
          if(c == ';') {
            // This still looks like a color string, keep buffering
            return c;
          }
          else if(c == 'm') {
            // This is a plain color termination. Write the
            // buffered output to the color buffers.
            return flush_buffer(color_buffers);
          }
          else {
            // This is some other kind of escape. Write the
            // buffered output to all buffers.
            return flush_buffer(all_buffers);
          } // if

        case 4:
          if(c == '3') {
            // This still looks like a color string, keep buffering
            return c;
          }
          else {
            // This is some other kind of escape. Write the
            // buffered output to all buffers.
            return flush_buffer(all_buffers);
          } // if

        case 5:
          if(isdigit(c) && (c - '0') < 8) {
            // This still looks like a color string, keep buffering
            return c;
          }
          else {
            // This is some other kind of escape. Write the
            // buffered output to all buffers.
            return flush_buffer(all_buffers);
          } // if

        case 6:
          if(c == 'm') {
            // This is a color string termination. Write the
            // buffered output to the color buffers.
            return flush_buffer(color_buffers);
          }
          else {
            // This is some other kind of escape. Write the
            // buffered output to all buffers.
            return flush_buffer(all_buffers);
          } // if
      } // switch

      return c;
    } // if
  } // overflow

  /*!
    Override the sync method so that we sync all of the output buffers.
   */

  virtual int sync() {
    int state = 0;

    for(auto b : buffers_) {
      const int s = b.second.buffer->pubsync();
      state = (state != 0) ? state : s;
    } // for

    // Return -1 if one of the buffers had an error
    return (state == 0) ? 0 : -1;
  } // sync

private:
  // Predicate to select all buffers.
  static bool all_buffers(const buffer_data_t & bd) {
    return bd.enabled;
  } // any_buffer

  // Predicate to select color buffers.
  static bool color_buffers(const buffer_data_t & bd) {
    return bd.enabled && bd.colorized;
  } // any_buffer

  // Flush buffered output to buffers that satisfy the predicate function.
  template<typename P>
  int flush_buffer(P && predicate = all_buffers) {
    int eof = !EOF;

    // Put test buffer characters to each buffer
    for(auto b : buffers_) {
      if(predicate(b.second)) {
        for(auto bc : test_buffer_) {
          const int w = b.second.buffer->sputc(bc);
          eof = (eof == EOF) ? eof : w;
        } // for
      } // if
    } // for

    // Clear the test buffer
    test_buffer_.clear();

    // Return EOF if one of the buffers hit the end
    return eof == EOF ? EOF : !EOF;
  } // flush_buffer

  std::unordered_map<std::string, buffer_data_t> buffers_;
  std::string test_buffer_;

}; // class tee_buffer_t

/*!
  The tee_stream_t type provides a stream class that writes to multiple
  output buffers.
 */

struct tee_stream_t : public std::ostream {

  tee_stream_t() : std::ostream(&tee_) {
    // Allow users to turn std::clog output on and off from
    // their environment.
    if(std::getenv("FLOG_ENABLE_STDLOG")) {
      tee_.add_buffer("flog", std::clog.rdbuf(), true);
    } // if
  } // tee_stream_t

  tee_stream_t & operator*() {
    return *this;
  } // operator *

  /*!
    Add a new buffer to the output.
   */

  void add_buffer(std::string const & key,
    std::ostream & s,
    bool colorized = false) {
    tee_.add_buffer(key, s.rdbuf(), colorized);
  } // add_buffer

  /*!
    Enable an existing buffer.

    \param key The string identifier of the streambuf.
   */

  bool enable_buffer(std::string const & key) {
    tee_.enable_buffer(key);
    return true;
  } // enable_buffer

  /*!
    Disable an existing buffer.

    \param key The string identifier of the streambuf.
   */

  bool disable_buffer(std::string const & key) {
    tee_.disable_buffer(key);
    return false;
  } // disable_buffer

private:
  tee_buffer_t tee_;

}; // struct tee_stream_t

} // namespace log
} // namespace flecsi

#endif // FLECSI_ENABLE_FLOG
