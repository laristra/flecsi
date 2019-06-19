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

#include "packet.h"
#include "utils.h"

#include <bitset>
#include <cassert>
#include <sstream>
#include <unordered_map>

namespace flecsi {
namespace utils {
namespace flog {

/*
  Stream buffer type to allow output to multiple targets
  a la the tee function.
 */

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
    // Allow users to turn std::flog output on and off from
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

  void add_buffer(std::string key, std::ostream & s, bool colorized = false) {
    tee_.add_buffer(key, s.rdbuf(), colorized);
  } // add_buffer

  /*!
    Enable an existing buffer.

    \param key The string identifier of the streambuf.
   */

  bool enable_buffer(std::string key) {
    tee_.enable_buffer(key);
    return true;
  } // enable_buffer

  /*!
    Disable an existing buffer.

    \param key The string identifier of the streambuf.
   */

  bool disable_buffer(std::string key) {
    tee_.disable_buffer(key);
    return false;
  } // disable_buffer

private:
  tee_buffer_t tee_;

}; // struct tee_stream_t

/*!
  The flog_t type provides access to logging parameters and configuration.

  This type provides access to the underlying logging parameters for
  configuration and information. The cinch logging functions provide
  basic logging with an interface that is similar to Google's GLOG
  and the Boost logging utilities.

  @note We may want to consider adopting one of these packages
  in the future.

  @ingroup logging
 */

class flog_t
{
public:
  /*!
    Copy constructor (disabled)
   */

  flog_t(const flog_t &) = delete;

  /*!
    Assignment operator (disabled)
   */

  flog_t & operator=(const flog_t &) = delete;

  /*!
    Meyer's singleton instance.

    \return The singleton instance of this type.
   */

  static flog_t & instance() {
    static flog_t c;
    return c;
  } // instance

  void initialize(std::string active = "none") {
#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: initializing runtime"
              << FLOG_COLOR_PLAIN << std::endl;
#endif

#if defined(FLOG_ENABLE_TAGS)
    // Because active tags are specified at runtime, it is
    // necessary to maintain a map of the compile-time registered
    // tag names to the id that they get assigned after the flog_t
    // initialization (register_tag). This map will be used to populate
    // the tag_bitset_ for fast runtime comparisons of enabled tag groups.

    // Note: For the time being, the map uses actual strings rather than
    // hashes. We should consider creating a const_string_t type for
    // constexpr string creation.

    // Initialize everything to false. This is the default, i.e., "none".
    tag_bitset_.reset();

    // The default group is always active (unscoped). To avoid
    // output for this tag, make sure to scope all FLOG output.
    tag_bitset_.set(0);

    if(active == "all") {
      // Turn on all of the bits for "all".
      tag_bitset_.set();
    }
    else if(active != "none") {
      // Turn on the bits for the selected groups.
      std::istringstream is(active);
      std::string tag;
      while(std::getline(is, tag, ',')) {
        if(tag_map_.find(tag) != tag_map_.end()) {
          tag_bitset_.set(tag_map_[tag]);
        }
        else {
          std::cerr << "FLOG WARNING: tag " << tag
                    << " has not been registered. Ignoring this group..."
                    << std::endl;
        } // if
      } // while
    } // if

#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: active tags (" << active << ")"
              << FLOG_COLOR_PLAIN << std::endl;
#endif

#endif // FLOG_ENABLE_TAGS

#if defined(FLOG_ENABLE_MPI)

#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: initializing mpi state"
              << FLOG_COLOR_PLAIN << std::endl;
#endif

    mpi_state_t::instance().initialize();
#endif // FLOG_ENABLE_MPI

    initialized_ = true;
  } // initialize

  void finalize() {
#if defined(FLOG_ENABLE_MPI)
    mpi_state_t::instance().finalize();
#endif // FLOG_ENABLE_MPI
  } // finalize

  /*!
    Return the tag map.
   */

  const std::unordered_map<std::string, size_t> & tag_map() {
    return tag_map_;
  } // tag_map

  /*!
    Return the buffered log stream.
   */

  std::stringstream & buffer_stream() {
    return buffer_stream_;
  } // stream

  /*!
    Return the log stream.
   */
  std::ostream & stream() {
    return *stream_;
  } // stream

  /*!
    Return the log stream predicated on a boolean.
    This method interface will allow us to select between
    the actual stream and a null stream.
   */
  std::ostream & severity_stream(bool active = true) {
    return active ? buffer_stream_ : null_stream_;
  } // stream

  /*!
    Return a null stream to disable output.
   */

  std::ostream & null_stream() {
    return null_stream_;
  } // null_stream

  /*!
    Return the tee stream to allow the user to set configuration options.
    FIXME: Need a better interface for this...
   */

  tee_stream_t & config_stream() {
    return *stream_;
  } // stream

  /*!
    Return the next tag id.
   */

  size_t register_tag(const char * tag) {
    // If the tag is already registered, just return the previously
    // assigned id. This allows tags to be registered in headers.
    if(tag_map_.find(tag) != tag_map_.end()) {
      return tag_map_[tag];
    } // if

    const size_t id = ++tag_id_;
    assert(id < FLOG_TAG_BITS && "Tag bits overflow! Increase FLOG_TAG_BITS");
#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: registering tag " << tag << ": "
              << id << FLOG_COLOR_PLAIN << std::endl;
#endif
    tag_map_[tag] = id;
    return id;
  } // next_tag

  /*!
    Return a reference to the active tag (const version).
   */

  const size_t & active_tag() const {
    return active_tag_;
  } // active_tag

  /*!
    Return a reference to the active tag (mutable version).
   */

  size_t & active_tag() {
    return active_tag_;
  } // active_tag

  bool tag_enabled() {
#if defined(FLOG_ENABLE_TAGS)

#if defined(FLOG_ENABLE_DEBUG)
    auto active_set = tag_bitset_.test(active_tag_) == 1 ? "true" : "false";
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: tag " << active_tag_ << " is "
              << active_set << FLOG_COLOR_PLAIN << std::endl;
#endif

    // If the runtime context hasn't been initialized, return true only
    // if the user has enabled externally-scoped messages.
    if(!initialized_) {
#if defined(FLOG_ENABLE_EXTERNAL)
      return true;
#else
      return false;
#endif
    } // if

    return tag_bitset_.test(active_tag_);
#else
    return true;
#endif // FLOG_ENABLE_TAGS
  } // tag_enabled

  size_t lookup_tag(const char * tag) {
    if(tag_map_.find(tag) == tag_map_.end()) {
      std::cerr << FLOG_COLOR_YELLOW << "FLOG: !!!WARNING " << tag
                << " has not been registered. Ignoring this group..."
                << FLOG_COLOR_PLAIN << std::endl;
      return 0;
    } // if

    return tag_map_[tag];
  } // lookup_tag

  bool initialized() {
    return initialized_;
  } // initialized

#if defined(FLOG_ENABLE_MPI)
  int rank() {
    return mpi_state_t::instance().rank();
  } // rank

  int size() {
    return mpi_state_t::instance().size();
  } // rank
#endif

private:
  /*!
    Constructor. This method is hidden because we are a singleton.
   */
  flog_t()

    : null_stream_(0), tag_id_(0), active_tag_(0) {} // flog_t

  ~flog_t() {
#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: flog_t destructor" << std::endl;
#endif
  }

  bool initialized_ = false;

  tee_stream_t stream_;
  std::stringstream buffer_stream_;
  std::ostream null_stream_;

  size_t tag_id_;
  size_t active_tag_;
  std::bitset<FLOG_TAG_BITS> tag_bitset_;
  std::unordered_map<std::string, size_t> tag_map_;

}; // class flog_t

/*!
  \class flog_tag_scope_t
  \brief flog_tag_scope_t provides an execution scope for which a given
         tag id is active.

  This type sets the active tag id to the id passed to the constructor,
  stashing the current active tag. When the instance goes out of scope,
  the active tag is reset to the stashed value.
 */

struct flog_tag_scope_t {
  flog_tag_scope_t(size_t tag = 0) : stash_(flog_t::instance().active_tag()) {
#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: activating tag " << tag
              << FLOG_COLOR_PLAIN << std::endl;
#endif

    // Warn users about externally-scoped messages
    if(!flog_t::instance().initialized()) {
      std::cerr
        << FLOG_COLOR_YELLOW << "FLOG: !!!WARNING You cannot use "
        << "tag guards for externally scoped messages!!! "
        << "This message will be active if FLOG_ENABLE_EXTERNAL is defined!!!"
        << FLOG_COLOR_PLAIN << std::endl;
    } // if

    flog_t::instance().active_tag() = tag;
  } // flog_tag_scope_t

  ~flog_tag_scope_t() {
    flog_t::instance().active_tag() = stash_;
  } // ~flog_tag_scope_t

private:
  size_t stash_;

}; // flog_tag_scope_t

} // namespace flog
} // namespace utils
} // namespace flecsi

#define buffer_message(message)                                                \
                                                                               \
  if(mpi_state_t::instance().initialized()) {                                  \
    packet_t pkt(message);                                                     \
    mpi_state_t::instance().packets().push_back(pkt);                          \
  } /* if */

#define send_to_one(message)                                                   \
                                                                               \
  if(mpi_state_t::instance().initialized()) {                                  \
    packet_t pkt(message);                                                     \
                                                                               \
    packet_t * pkts = mpi_state_t::instance().rank() == 0                      \
                        ? new packet_t[mpi_state_t::instance().size()]         \
                        : nullptr;                                             \
                                                                               \
    MPI_Gather(pkt.data(),                                                     \
      pkt.bytes(),                                                             \
      MPI_BYTE,                                                                \
      pkts,                                                                    \
      pkt.bytes(),                                                             \
      MPI_BYTE,                                                                \
      0,                                                                       \
      MPI_COMM_WORLD);                                                         \
                                                                               \
    if(mpi_state_t::instance().rank() == 0) {                                  \
                                                                               \
      std::lock_guard<std::mutex> guard(                                       \
        mpi_state_t::instance().packets_mutex());                              \
                                                                               \
      for(size_t i{0}; i < mpi_state_t::instance().size(); ++i) {              \
        mpi_state_t::instance().packets().push_back(pkts[i]);                  \
      } /* for */                                                              \
                                                                               \
      delete[] pkts;                                                           \
                                                                               \
    } /* if */                                                                 \
  } /* if */

#endif // FLECSI_ENABLE_FLOG
