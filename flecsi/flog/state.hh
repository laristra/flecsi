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

#include <flecsi/flog/packet.hh>
#include <flecsi/flog/types.hh>
#include <flecsi/flog/utils.hh>

#include <bitset>
#include <cassert>
#include <sstream>
#include <string>
#include <unordered_map>

namespace flecsi {
namespace log {

// Forward
void flush_packets();

/*!
  The flog_t type provides access to logging parameters and configuration.

  This type provides access to the underlying logging parameters for
  configuration and information. The FleCSI logging functions provide
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

  int initialize(std::string active = "none",
    int verbose = 0,
    std::size_t one_process = -1) {
#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: initializing runtime"
              << FLOG_COLOR_PLAIN << std::endl;
#endif

    verbose_ = verbose;

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
    tag_reverse_map_[0] = "all";

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

    {
      int p, np;
      MPI_Comm_rank(MPI_COMM_WORLD, &p);
      MPI_Comm_size(MPI_COMM_WORLD, &np);
      process_ = p;
      processes_ = np;
    }

    if(one_process + 1 && one_process >= processes_) {
      if(process_ == 0) {
        std::cerr << "flog process " << one_process << " out-of-bounds ("
                  << processes_ << " processes)" << std::endl;
      } // if

      return !0;
    } // if

    one_process_ = one_process;

    if(process_ == 0) {
      std::thread flusher(flush_packets);
      instance().flusher_thread().swap(flusher);
    } // if
#endif // FLOG_ENABLE_MPI

    initialized_ = true;

    return 0;
  } // initialize

  void finalize() {
#if defined(FLOG_ENABLE_MPI)
    if(initialized_) {
      send_to_one();

      if(process_ == 0) {
        end_flusher();
        flusher_thread_.join();
      } // if
    } // if
#endif // FLOG_ENABLE_MPI
  } // finalize

  int verbose() const {
    return verbose_;
  }

  /*!
    Return the tag map.
   */

  const std::unordered_map<std::string, size_t> & tag_map() {
    return tag_map_;
  }

  /*!
    Return the buffered log stream.
   */

  std::stringstream & buffer_stream() {
    return buffer_stream_;
  }

  /*!
    Return the log stream.
   */

  std::ostream & stream() {
    return *stream_;
  }

  /*!
    Return the log stream predicated on a boolean.
    This method interface will allow us to select between
    the actual stream and a null stream.
   */

  std::ostream & severity_stream(bool active = true) {
    return active ? buffer_stream_ : null_stream_;
  }

  /*!
    Return a null stream to disable output.
   */

  std::ostream & null_stream() {
    return null_stream_;
  }

  /*!
    Return the tee stream to allow the user to set configuration options.
    FIXME: Need a better interface for this...
   */

  tee_stream_t & config_stream() {
    return *stream_;
  }

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
    tag_reverse_map_[id] = tag;
    return id;
  } // next_tag

  /*!
    Return a reference to the active tag (const version).
   */

  const size_t & active_tag() const {
    return active_tag_;
  }

  /*!
    Return a reference to the active tag (mutable version).
   */

  size_t & active_tag() {
    return active_tag_;
  }

  /*!
    Return the tag name associated with a tag id.
   */

  std::string tag_name(size_t id) {
    assert(tag_reverse_map_.find(id) != tag_reverse_map_.end());
    return tag_reverse_map_[id];
  }

  /*!
    Return the tag name associated with the active tag.
   */

  std::string active_tag_name() {
    assert(tag_reverse_map_.find(active_tag_) != tag_reverse_map_.end());
    return tag_reverse_map_[active_tag_];
  }

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
  }

  bool initialized() {
    return initialized_;
  }

#if defined(FLOG_ENABLE_MPI)
  bool one_process() const {
    return one_process_ < processes_;
  }

  size_t output_process() const {
    return one_process_;
  }

  std::size_t process() {
    return process_;
  }

  std::size_t processes() {
    return processes_;
  }

  std::thread & flusher_thread() {
    return flusher_thread_;
  }

  std::mutex & packets_mutex() {
    return packets_mutex_;
  }

  void buffer_output() {
    std::string tmp = buffer_stream().str();

    // Make sure that the string fits within the packet size.
    if(tmp.size() > FLOG_MAX_MESSAGE_SIZE) {
      tmp.resize(FLOG_MAX_MESSAGE_SIZE - 100);
      std::stringstream stream;
      stream << tmp << FLOG_COLOR_LTRED << " OUTPUT BUFFER TRUNCATED "
             << FLOG_MAX_MESSAGE_SIZE << "(" << buffer_stream().str().size()
             << ")" << FLOG_COLOR_PLAIN << std::endl;
      tmp = stream.str();
    } // if

    packets_.push_back({tmp.c_str()});
  }

  std::vector<packet_t> & packets() {
    return packets_;
  }

  bool run_flusher() {
    return run_flusher_;
  }

  void end_flusher() {
    run_flusher_ = false;
  }

  void set_serialized() {
    serialized_ = true;
  }

  bool serialized() {
    return serialized_;
  }
#endif

private:
  /*!
    Constructor. This method is hidden because we are a singleton.
   */

  flog_t() : null_stream_(0), tag_id_(0), active_tag_(0) {}

  ~flog_t() {
#if defined(FLOG_ENABLE_DEBUG)
    std::cerr << FLOG_COLOR_LTGRAY << "FLOG: flog_t destructor" << std::endl;
#endif
  }

  bool initialized_ = false;
  int verbose_ = 0;

  tee_stream_t stream_;
  std::stringstream buffer_stream_;
  std::ostream null_stream_;

  size_t tag_id_;
  size_t active_tag_;
  std::bitset<FLOG_TAG_BITS> tag_bitset_;
  std::unordered_map<std::string, size_t> tag_map_;
  std::unordered_map<size_t, std::string> tag_reverse_map_;

#if defined(FLOG_ENABLE_MPI)
  std::size_t one_process_, process_, processes_;
  std::thread flusher_thread_;
  std::mutex packets_mutex_;
  std::vector<packet_t> packets_;
  bool run_flusher_ = true;
  bool serialized_ = false;
#endif

}; // class flog_t

} // namespace log
} // namespace flecsi

#endif // FLECSI_ENABLE_FLOG
