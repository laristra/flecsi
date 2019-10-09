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

  This file defines the type identifier type \em reference_base.
 */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include <flecsi/runtime/types.hh>
#include <flecsi/utils/serialize.hh>

namespace flecsi {
namespace data {

#if 0
template<typename TOPOLOGY_TYPE>
struct topology_instance;
#endif

/*!
  The reference_base type is the base of all FleCSI data model types.
  It is used to identify FleCSI data model types, and to store basic handle
  information.
 */

struct reference_base {

  reference_base(size_t identifier) : identifier_(identifier) {}

  size_t identifier() const {
    return identifier_;
  } // identifier

protected:
  size_t identifier_;

}; // struct reference_base

/*!
  The field_reference_t type is used to reference fields. It adds a \em
  topology identifier field to the reference_base to track the
  associated topology instance.
 */

struct field_reference_t : public reference_base {

  field_reference_t(size_t identifier, size_t topology_identifier)
    : reference_base(identifier), topology_identifier_(topology_identifier) {}

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
