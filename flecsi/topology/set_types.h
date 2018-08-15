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

#include <flecsi/data/data_client.h>
#include <flecsi/topology/types.h>
#include <flecsi/utils/id.h>

namespace flecsi {
namespace topology {

class set_entity_t {
public:
  using id_t = simple_id;

  size_t index_space_id() const {
    return 0;
  }
};

class set_topology_base_t {};

template<class STORAGE_TYPE>
class set_topology_base__ : public data::data_client_t,
                            public set_topology_base_t {
public:
  // Default constructor
  set_topology_base__(STORAGE_TYPE * ss = nullptr) : ss_(ss) {}

  // Don't allow the set to be copied or copy constructed
  set_topology_base__(const set_topology_base__ & s) : ss_(s.ss_) {}

  set_topology_base__ & operator=(const set_topology_base__ &) = delete;

  /// Allow move operations
  set_topology_base__(set_topology_base__ &&) = default;

  //! override default move assignement
  set_topology_base__ & operator=(set_topology_base__ && o) {
    // call base_t move operator
    data::data_client_t::operator=(std::move(o));
    // return a reference to the object
    return *this;
  };

  STORAGE_TYPE * set_storage(STORAGE_TYPE * ss) {
    ss_ = ss;
    return ss_;
  } // set_storage

  STORAGE_TYPE * storage() {
    return ss_;
  } // set_storage

  void clear_storage() {
    ss_ = nullptr;
  } // clear_storage

  void delete_storage() {
    delete ss_;
  } // delete_storage

  /*!
    This method should be called to construct and entity rather than
    calling the constructor directly. This way, the ability to have
    extra initialization behavior is reserved.
  */
  template<class T, class... ARG_TYPES>
  T * make(ARG_TYPES &&... args) {
    return ss_->template make<T>(std::forward<ARG_TYPES>(args)...);
  } // make

protected:
  STORAGE_TYPE * ss_ = nullptr;
};

} // namespace topology
} // namespace flecsi
