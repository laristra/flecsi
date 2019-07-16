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

#include <flecsi/utils/flog/packet.hh>
#include <flecsi/utils/flog/types.hh>
#include <flecsi/utils/flog/utils.hh>

#include <bitset>
#include <cassert>
#include <unordered_map>

namespace flecsi {
namespace utils {
namespace flog {

// Forward
void flush_packets();

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

  void initialize(std::string active = "none",
    size_t one_process = std::numeric_limits<size_t>::max()) {
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

    one_process_ = one_process;

    MPI_Comm_rank(MPI_COMM_WORLD, &process_);
    MPI_Comm_size(MPI_COMM_WORLD, &processes_);

    if(process_ == 0) {
      std::thread flusher(flush_packets);
      instance().flusher_thread().swap(flusher);
    } // if
#endif // FLOG_ENABLE_MPI

    initialized_ = true;
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
  bool one_process() const {
    return one_process_ < processes_;
  }

  size_t output_process() const {
    return one_process_;
  }

  int process() {
    return process_;
  }

  int processes() {
    return processes_;
  }

  std::thread & flusher_thread() {
    return flusher_thread_;
  }

  std::mutex & packets_mutex() {
    return packets_mutex_;
  }

  void buffer_output() {
    packets_.push_back({buffer_stream().str().c_str()});
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

  tee_stream_t stream_;
  std::stringstream buffer_stream_;
  std::ostream null_stream_;

  size_t tag_id_;
  size_t active_tag_;
  std::bitset<FLOG_TAG_BITS> tag_bitset_;
  std::unordered_map<std::string, size_t> tag_map_;

#if defined(FLOG_ENABLE_MPI)
  size_t one_process_;
  int process_;
  int processes_;
  std::thread flusher_thread_;
  std::mutex packets_mutex_;
  std::vector<packet_t> packets_;
  bool run_flusher_ = true;
  bool serialized_ = false;
#endif

}; // class flog_t

} // namespace flog
} // namespace utils
} // namespace flecsi

#endif // FLECSI_ENABLE_FLOG
