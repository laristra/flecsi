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

/*!
  @file

  This file contains the \em field_registration_u type. This type
  is currently used for all field registrations regardless of runtime or
  client type.
 */

#include <flecsi/runtime/types.h>

flog_register_tag(field_registration);

namespace flecsi {
namespace data {

/*!
  The field_registration_u type uses a form of type erasure
  (by defining a generic callback type) that can be used to capture
  static type information at runtime.

  @tparam DATA_CLIENT_TYPE The data client type on which the data
                           attribute should be registered.
  @tparam STORAGE_CLASS    The storage type for the data attribute.
  @tparam DATA_TYPE        The data type, e.g., double. This may be
                           P.O.D. or a user-defined type that is
                           trivially-copyable.
  @tparam NAMESPACE        The namespace key. Namespaces allow separation
                           of attribute names to avoid collisions.
  @tparam NAME             The attribute name.
  @tparam VERSIONS         The number of versions that shall be associated
                           with this attribute.
  @tparam INDEX_SPACE      The index space identifier.
*/

template<typename DATA_CLIENT_TYPE,
  size_t STORAGE_CLASS,
  typename DATA_TYPE,
  size_t NAMESPACE,
  size_t NAME,
  size_t VERSIONS,
  size_t INDEX_SPACE>
struct field_registration_u {

  /*!
    Callback method for capturing static type information during
    runtime execution.
   */

  static void register_callback(size_t key, field_id_t fid) {
#if 0    
    execution::context_t::field_info_t fi;

    fi.data_client_hash =
      typeid(typename DATA_CLIENT_TYPE::type_identifier_t).hash_code();

    fi.storage_class = STORAGE_CLASS;
    fi.size = sizeof(DATA_TYPE);
    fi.namespace_hash = NAMESPACE;
    fi.name_hash = NAME;
    fi.versions = VERSIONS;
    fi.index_space = INDEX_SPACE;
    fi.fid = fid;
    fi.key = key;

    execution::context_t::instance().register_field_info(fi);
#endif
  } // register_callback

}; // class field_registration_u

} // namespace data
} // namespace flecsi
