/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2018, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_class.h!!!
// Using this approach allows us to have only one storage_class_u
// definition that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE hpx
#include <flecsi/data/storage_class.h>
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include <flecsi/data/common/privilege.h>
#include <flecsi/data/data_client.h>
#include <flecsi/data/global_data_handle.h>
#include <flecsi/data/storage.h>
#include <flecsi/execution/context.h>
#include <flecsi/utils/const_string.h>

#include <algorithm>

namespace flecsi {
namespace data {
namespace hpx {

//----------------------------------------------------------------------------//
// Global handle.
//----------------------------------------------------------------------------//

/*!
 The global_handle_u provide an access to global variables that have
 been registered in data model

 \tparam T The type of the data variable. If this type is not
           consistent with the type used to register the data, bad things
           can happen. However, it can be useful to reinterpret the type,
           e.g., when writing raw bytes. This class is part of the
           low-level \e flecsi interface, so it is assumed that you
           know what you are doing...

 @tparam PERMISSIONS The permissions to the handle.
 */

template<typename T, size_t PERMISSIONS>
struct global_handle_u : public global_data_handle_u<T, PERMISSIONS> {

  /*!
    Type definitions.
   */

  using base_t = global_data_handle_u<T, PERMISSIONS>;

  /*!
    Constructor.
   */

  global_handle_u() {
    base_t::global = true;
  }

  /*!
   Destructor.
   */

  ~global_handle_u() {}

  /*
    Copy constructor.
   */

  template<size_t P2>
  global_handle_u(const global_handle_u<T, P2> & a)
    : base_t(reinterpret_cast<const base_t &>(a)), label_(a.label()),
      size_(a.size()) {
    static_assert(P2 == 0, "passing mapped handle to task args");
  }

  //--------------------------------------------------------------------------//
  // Member data interface.
  //--------------------------------------------------------------------------//

  /*!
   \brief Return a std::string containing the label of the data variable
          reference by this handle.
   */

  const std::string & label() const {
    return label_;
  } // label

  /*!
   \brief Return the index space size of the data variable
          referenced by this handle.
   */

  size_t size() const {
    return size_;
  } // size

  /*!
   \brief Test to see if this handle is empty

   \return true if registered.
   */

  operator bool() const {
    return base_t::combined_data != nullptr;
  } // operator bool

private:
  std::string label_ = "";
  size_t size_ = 1;
}; // struct global_handle_u

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Global storage type.
//----------------------------------------------------------------------------//

/*!
 FIXME: Global storage type.
 */
template<>
struct storage_class_u<global> {

  /*!
   Type definitions.
   */

  template<typename T, size_t PERMISSIONS>
  using handle_t = global_handle_u<T, PERMISSIONS>;

  /*!
   Data handles.
   */

  template<typename DATA_CLIENT_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION,
    size_t PERMISSIONS>
  static handle_t<DATA_TYPE, 0> get_handle(
    const data_client_handle<DATA_CLIENT_TYPE, PERMISSIONS> & client_handle) {
    handle_t<DATA_TYPE, 0> h;
    auto & context = execution::context_t::instance();

    auto & field_info = context.get_field_info_from_name(
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code(),
      utils::hash::field_hash<NAMESPACE, NAME>(VERSION));

    auto & registered_field_data = context.registered_field_data();
    auto fieldDataIter = registered_field_data.find(field_info.fid);
    if(fieldDataIter == registered_field_data.end()) {
      // TODO: deal with VERSION
      context.register_field_data(field_info.fid, field_info.size);
    }

    auto data = registered_field_data[field_info.fid].data();

    h.fid = field_info.fid;
    h.index_space = field_info.index_space;
    h.data_client_hash = field_info.data_client_hash;

    h.combined_data = reinterpret_cast<DATA_TYPE *>(data);

    h.global = true;
    h.state = context.execution_state();

    auto& registered_field_futures = context.registered_field_futures();
    h.future = &registered_field_futures[field_info.fid];

    h.ghost_is_readable = nullptr;

    return h;
  }

}; // struct storage_class_u

} // namespace hpx
} // namespace data
} // namespace flecsi
