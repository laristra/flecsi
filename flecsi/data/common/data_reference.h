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
#include <flecsi/runtime/types.h>
#endif

namespace flecsi {
namespace data {

/*!
  The data_reference_base_t type is the base of all FleCSI data model types.
  It is used to identify FleCSI data model types, and to store basic handle
  information.
 */

struct data_reference_base_t {

  data_reference_base_t(field_id_t fid, size_t identifier)
    : fid_(fid), identifier_(identifier) {}

  data_reference_base_t(data_reference_base_t const & ref)
    : fid_(ref.fid_), identifier_(ref.identifier_) {}

  field_id_t fid() const {
    return fid_;
  } // fid

  size_t identifier() const {
    return identifier_;
  } // identifier

private:
  field_id_t fid_;
  size_t identifier_;

}; // struct data_reference_base_t

/*!
  The topology_reference_u captures the topology type that is required for
  dependent field references.
 */

template<typename TOPOLOGY_TYPE>
struct topology_reference_u : public data_reference_base_t {
  using topology_t = TOPOLOGY_TYPE;

  topology_reference_u(field_id_t fid, size_t identifier)
    : data_reference_base_t(fid, identifier) {}

  topology_reference_u(topology_reference_u const & ref)
    : data_reference_base_t(ref) {}

}; // struct topology_reference_u

/*!
  The field_reference_t type is used to reference fields. It adds a \em
  topology identifier field to the data_reference_base_t to track the
  associated topology instance.
 */

struct field_reference_t : public data_reference_base_t {

  field_reference_t(field_id_t fid,
    size_t identifier,
    size_t topology_identifier)
    : data_reference_base_t(fid, identifier),
      topology_identifier_(topology_identifier) {}

  size_t topology_identifier() const {
    return topology_identifier_;
  } // topology_identifier

private:
  size_t topology_identifier_;

}; // struct field_reference_t

} // namespace data
} // namespace flecsi
