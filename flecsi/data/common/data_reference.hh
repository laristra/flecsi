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

/*!
  @file

  This file defines the type identifier type \em data_reference_base_t.
 */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/runtime/types.hh>
#endif

namespace flecsi {
namespace data {

/*!
  The data_reference_base_t type is the base of all FleCSI data model types.
  It is used to identify FleCSI data model types, and to store basic handle
  information.
 */

struct data_reference_base_t {

  data_reference_base_t(size_t identifier) : identifier_(identifier) {}

  size_t identifier() const {
    return identifier_;
  } // identifier

private:
  size_t identifier_;

}; // struct data_reference_base_t

/*!
  The topology_reference_u captures the topology type that is required for
  dependent field references.
 */

template<typename TOPOLOGY_TYPE>
struct topology_reference_u : public data_reference_base_t {
  using topology_t = TOPOLOGY_TYPE;

  topology_reference_u(size_t identifier) : data_reference_base_t(identifier) {}
}; // struct topology_reference_u

/*!
  The field_reference_t type is used to reference fields. It adds a \em
  topology identifier field to the data_reference_base_t to track the
  associated topology instance.
 */

struct field_reference_t : public data_reference_base_t {

  field_reference_t(size_t identifier, size_t topology_identifier)
    : data_reference_base_t(identifier),
      topology_identifier_(topology_identifier) {}

  size_t topology_identifier() const {
    return topology_identifier_;
  } // topology_identifier

private:
  size_t topology_identifier_;

}; // struct field_reference_t

/// A \c field_reference is a \c field_reference_t tagged with a data type.
/// \tparam T data type (merely for type safety)
template<class T>
struct field_reference : field_reference_t {
  using field_reference_t::field_reference_t;
};

} // namespace data
} // namespace flecsi
