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

#include <sstream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>

#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

namespace flecsi {
namespace utils {

/*!
  The binary_serializer_t type provides dynamic serialization of arbitrary
  types through an insertion-style operator.

  Attribution:
  https://stackoverflow.com/questions/3015582/direct-boost-serialization-to-char-array/5604782
 */

struct binary_serializer_t {

  binary_serializer_t()
    : inserter_(serial_data_), stream_(inserter_), oa_(stream_) {}

  /*!
    Insertion operator to write a value to the underlying string device.
   */

  template<typename T>
  binary_serializer_t & operator<<(T const & value) {
    oa_ << value;
    return *this;
  } // operator<<

  /*!
    Synchronize with the underlying storage device.
   */

  void flush() {
    stream_.flush();
  }

  /*!
    Reset the string device.
   */

  void clear() {
    serial_data_.clear();
  }

  /*!
    Return the bumber of bytes stored in the string device.
   */

  size_t size() {
    return serial_data_.size();
  } // size

  /*!
    Return a pointer to the string device data.
   */

  char const * data() const {
    return serial_data_.data();
  } // data

private:
  std::string serial_data_;
  boost::iostreams::back_insert_device<std::string> inserter_;
  boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>>
    stream_;
  boost::archive::binary_oarchive oa_;

}; // struct binary_serializer_t

/*!
  The binary_deserializer_t type provides dynamic de-serialization of arbitrary
  types through an extraction-style operator.

  Attribution:
  https://stackoverflow.com/questions/3015582/direct-boost-serialization-to-char-array/5604782
 */

struct binary_deserializer_t {

  /*!
    Construct a deserializer from the given data pointer and byte size. Note
    that the buffer must have been created with an instance of
    binary_serializer_t.
   */

  binary_deserializer_t(char const * data, size_t bytes)
    : device_(data, bytes), stream_(device_), ia_(stream_) {}

  /*!
    Extraction operator to write a value to the underlying string device.
   */

  template<typename T>
  binary_deserializer_t & operator>>(T & value) {
    ia_ >> value;
    return *this;
  } // operator>>

private:
  boost::iostreams::basic_array_source<char> device_;
  boost::iostreams::stream<boost::iostreams::basic_array_source<char>> stream_;
  boost::archive::binary_iarchive ia_;

}; // struct binary_deserializer_t

} // namespace utils
} // namespace flecsi
