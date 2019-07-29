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

  This file contains the C++ data model interface \em field_interface_t
  for FleCSI field data structures. The \em field_interface_t type is a
  specialization of the \em field_interface_u type on the backend runtimne
  policy that is selected at compile time.
 */

#include <flecsi/runtime/data_policy.hh>

// Needed for storage_class.hh and for FLECSI_RUNTIME_DATA_POLICY:
#include <flecsi/data/common/data_reference.hh>
#include <flecsi/data/common/storage_class.hh>
#include <flecsi/execution/context.hh>
#include <flecsi/runtime/types.hh>
#include <flecsi/utils/common.hh>
#include <flecsi/utils/hash.hh>

namespace flecsi {
namespace data {

/*!
  The field_interface_u type provides a high-level field interface that
  is implemented by the given data policy.

  @tparam DATA_POLICY The backend data policy.

  @ingroup data
 */

template<typename DATA_POLICY>
struct field_interface_u {
  /// Each \c field object registers (various versions of) a field.
  template<class T,
    class Topo = topology::index_topology_t,
    storage_label_t S = index>
  struct field {
    using version = std::size_t;
    using topology_reference_t = topology_reference_u<Topo>;

    field(version v = 1) : versions(v), fid(reg()) {}
    /// Get a field reference.
    /// \param t topology reference
    /// \param v field version
    field_reference_t operator()(const topology_reference_t & t,
      version v = 0) const {
      flog_assert(v < versions,
        "no such version #" << v << " in " << versions << " versions");
      // NB: This mimics the primary storage_class_u template only.
      return {fid + v, t.identifier()};
    }

  private:
    field_id_t reg() const {

      constexpr auto max = utils::hash::field_max_versions;

      flog_assert(versions <= max,
        "can't have " << versions << '>' << max << " versions");

      field_id_t ret;

      for(version v = 0; v < versions; ++v) {
        const auto fid = unique_fid_t::instance().next();
        if(v) {
          assert(fid == ret + v);
        }
        else {
          ret = fid;
        } // if

        execution::context_t::instance().add_field_info(
          Topo::type_identifier_hash, S, {fid, sizeof(T)}, fid);
      }
      return ret;
    }
    version versions;
    field_id_t fid;
  };

  template<class T>
  using global_field = field<T, topology::global_topology_t, dense>;

  /*!
    Add a field to the given topology type. This method should be thought of as
    registering a field attribute on the topology.  All instances of the
    topology type will have this attribute. However, this does not mean that
    each topology instance will allocate memory for this attribute.  Attribute
    instances will be created only when they are actually mapped into a task.

    @tparam TOPOLOGY_TYPE The data client type on which the data
                          attribute should be registered.
    @tparam STORAGE_CLASS The storage type for the data attribute.
    @tparam DATA_TYPE     The data type, e.g., double. This may be
                          P.O.D. or a user-defined type that is
                          trivially-copyable.
    @tparam NAMESPACE     The namespace key. Namespaces allow separation
                          of attribute names to avoid collisions.
    @tparam NAME          The attribute name.
    @tparam VERSIONS      The number of versions that shall be associated
                          with this attribute.

    @param name The string version of the field name.

    @ingroup data
   */

  template<typename TOPOLOGY_TYPE,
    size_t STORAGE_CLASS,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSIONS>
  static bool add_field(std::string const & name) {
    static_assert(VERSIONS <= utils::hash::field_max_versions,
      "max field versions exceeded");

    field_info_t fi;

    fi.type_size = sizeof(DATA_TYPE);

    flog_devel(info) << "Registering field" << std::endl
                     << "\tname: " << name << std::endl
                     << "\ttype: " << utils::demangle(typeid(DATA_TYPE).name())
                     << std::endl;

    for(size_t version(0); version < VERSIONS; ++version) {
      fi.fid = unique_fid_t::instance().next();

      execution::context_t::instance().add_field_info(
        TOPOLOGY_TYPE::type_identifier_hash,
        STORAGE_CLASS,
        fi,
        utils::hash::field_hash<NAMESPACE, NAME>(version));
    } // for

    return true;
  } // register_field

  /*!
    Return a reference to the field associated with the given parameters and
    topology instance.

    @tparam TOPOLOGY_TYPE The data client type on which the data
                          attribute should be registered.
    @tparam STORAGE_CLASS The storage type for the data attribute.
    @tparam DATA_TYPE     The data type, e.g., double. This may be
                          P.O.D. or a user-defined type that is
                          trivially-copyable.
    @tparam NAMESPACE     The namespace key. Namespaces allow separation
                          of attribute names to avoid collisions.
    @tparam NAME          The attribute name.
    @tparam VERSION       The data version.

    @ingroup data
   */

  template<typename TOPOLOGY_TYPE,
    size_t STORAGE_CLASS,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION = 0>
  static decltype(auto) field_instance(
    topology_reference_u<TOPOLOGY_TYPE> const & topology_reference) {

    static_assert(
      VERSION < utils::hash::field_max_versions, "max field version exceeded");

    using storage_class_t =
      typename DATA_POLICY::template storage_class_u<STORAGE_CLASS,
        TOPOLOGY_TYPE>;

    return storage_class_t::template get_reference<NAMESPACE, NAME, VERSION>(
      topology_reference);
  } // field_instance

#if 0
  /*!
    Return the mutator associated with the given parameters and data client.

    @tparam TOPOLOGY_TYPE The data client type on which the data
                          attribute should be registered.
    @tparam STORAGE_CLASS The storage type for the data attribute.
    @tparam DATA_TYPE     The data type, e.g., double. This may be
                          P.O.D. or a user-defined type that is
                          trivially-copyable.
    @tparam NAMESPACE     The namespace key. Namespaces allow separation
                          of attribute names to avoid collisions.
    @tparam NAME          The attribute name.
    @tparam VERSION       The data version.

    @ingroup data
   */

  template<typename TOPOLOGY_TYPE,
    size_t STORAGE_CLASS,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION = 0,
    size_t PERMISSIONS>
  static decltype(auto) get_mutator(
    const client_handle_u<TOPOLOGY_TYPE, PERMISSIONS> & client_handle,
    size_t slots) {
    static_assert(
      VERSION < utils::hash::field_max_versions, "max field version exceeded");

    using storage_class_t =
      typename DATA_POLICY::template storage_class_u<STORAGE_CLASS>;

    return storage_class_t::template get_mutator<TOPOLOGY_TYPE, DATA_TYPE,
      NAMESPACE, NAME, VERSION>(client_handle, slots);
  } // get_mutator
#endif

}; // struct field_interface_u

} // namespace data
} // namespace flecsi

namespace flecsi {
namespace data {

/*!
  The field_interface_t type is the high-level interface to the FleCSI
  field data model.

  @ingroup data
 */

using field_interface_t = field_interface_u<FLECSI_RUNTIME_DATA_POLICY>;

/*!
  The global_accessor_u type is the high-level accessor type for FleCSI
  field data on the global topology.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
using global_accessor_u =
  typename FLECSI_RUNTIME_DATA_POLICY::global_accessor_u<DATA_TYPE, PRIVILEGES>;

/*!
  The color_accessor_u type is the high-level accessor type for FleCSI
  field data on the color topology.
 */

template<typename DATA_TYPE, size_t PRIVILEGES>
using index_accessor_u =
  typename FLECSI_RUNTIME_DATA_POLICY::index_accessor_u<DATA_TYPE, PRIVILEGES>;

} // namespace data
} // namespace flecsi
