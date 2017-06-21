/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_storage_h
#define flecsi_data_storage_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Apr 17, 2016
//----------------------------------------------------------------------------//

#include <unordered_map>

#include "flecsi/data/common/data_hash.h"
#include "flecsi/data/data_client.h"
#include "flecsi/data/data_handle.h"
#include "flecsi/utils/const_string.h"

namespace flecsi {
namespace data {

//----------------------------------------------------------------------------//
//! The storage__ type provides a high-level data model context interface that
//! is implemented by the given storage policy.
//!
//! @tparam USER_META_DATA A user-defined meta data type.
//! @tparam STORAGE_POLICY The backend storage policy.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

template<
  typename USER_META_DATA,
  template<typename> class STORAGE_POLICY
>
struct storage__ : public STORAGE_POLICY<USER_META_DATA> {

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using sp_t = STORAGE_POLICY<USER_META_DATA>;

  template<size_t data_type_t>
  using st_t = typename sp_t::template storage_type_t<data_type_t>;

  using registration_function_t = std::function<void(size_t)>;

  //--------------------------------------------------------------------------//
  // These types depend on the backend.
  //--------------------------------------------------------------------------//

  using field_id_t = typename sp_t::field_id_t;
  using unique_fid_t = utils::unique_id_t<field_id_t, FLECSI_GENERATED_ID_MAX>;
  using data_value_t = std::pair<field_id_t, registration_function_t>;

  using client_value_t =
    std::unordered_map<
      data_hash_t::key_t, // key
      data_value_t,       // value
      data_hash_t,        // hash function
      data_hash_t         // equivalence operator
    >;

  template<
    typename DATA_CLIENT_TYPE,
    size_t STORAGE_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH,
    size_t INDEX_SPACE,
    size_t VERSIONS
  >
  using registration_wrapper__ =
      typename sp_t::template registration_wrapper__<
        DATA_CLIENT_TYPE,
        STORAGE_TYPE,
        DATA_TYPE,
        NAMESPACE_HASH,
        NAME_HASH,
        INDEX_SPACE,
        VERSIONS>;    

  template<
    typename DATA_CLIENT_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH
  >
  using client_registration_wrapper__ =
    typename sp_t::template client_registration_wrapper__<
      DATA_CLIENT_TYPE,
      NAMESPACE_HASH,
      NAME_HASH>;

  //--------------------------------------------------------------------------//
  // Class interface.
  //--------------------------------------------------------------------------//

	/// Constructor.
  storage__() {}

  /// Hide these.
  storage__(const storage__ &) = delete;
  storage__ & operator = (const storage__ &) = delete;

  /// Allow move operations.
  storage__(storage__ &&) = default;
  storage__ & operator = (storage__ &&) = default;

	//--------------------------------------------------------------------------//
  //! Myer's singleton instance.
  //!
  //! @return The single instance of this type.
	//--------------------------------------------------------------------------//

  static storage__ &
  instance()
  {
    static storage__ d;
    return d;
  } // instance

  //--------------------------------------------------------------------------//
  //! Register data with the FleCSI runtime. This method should be thought
  //! of as registering a data attribute on the given data client type.
  //! All instances of the client type will have this attribute. However,
  //! this does not mean that each data client instance will have an
  //! an instance of the attribute. Attribute instances will be created
  //! only when they are actually mapped into a task.
  //!
  //! @tparam DATA_CLIENT_TYPE The data client type on which the data
  //!                          attribute should be registered.
  //! @tparam STORAGE_TYPE     The storage type for the data attribute.
  //! @tparam DATA_TYPE        The data type, e.g., double. This may be
  //!                          P.O.D. or a user-defined type that is
  //!                          trivially-copyable.
  //! @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
  //!                          of attribute names to avoid collisions.
  //! @tparam NAME_HASH        The attribute name.
  //! @tparam INDEX_SPACE      The index space identifier.
  //! @tparam VERSIONS         The number of versions that shall be associated
  //!                          with this attribute.
  //!
  //! @ingroup data
  //--------------------------------------------------------------------------//

  template<
    typename DATA_CLIENT_TYPE,
    size_t STORAGE_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH,
    size_t INDEX_SPACE,
    size_t VERSIONS
  >
  bool
  register_data()
  {
    using wrapper_t =
      registration_wrapper__<
        DATA_CLIENT_TYPE,
        STORAGE_TYPE,
        DATA_TYPE,
        NAMESPACE_HASH,
        NAME_HASH,
        INDEX_SPACE,
        VERSIONS
      >;

    for(size_t i(0); i<VERSIONS; ++i) {
      data_registry_[typeid(DATA_CLIENT_TYPE).hash_code()]
        [data_hash_t::make_key(NAMESPACE_HASH, NAME_HASH)] =
        { unique_fid_t::instance().next(), wrapper_t::register_callback };
    } // for

    return true;
  } // register_data

  void register_all()
  {
    for(auto & c: data_registry_) {
      for(auto & d: c.second) {
        d.second.second(d.second.first);
      } // for
    } // for
  } // register_all

  //--------------------------------------------------------------------------//
  //! Register a data client with the FleCSI runtime.
  //!
  //! @tparam DATA_CLIENT_TYPE The data client type.
  //! @tparam NAMESPACE_HASH   The namespace key. Namespaces allow separation
  //!                          of attribute names to avoid collisions.
  //! @tparam NAME_HASH        The attribute name.
  //!
  //! @ingroup data
  //--------------------------------------------------------------------------//

  template<
    typename DATA_CLIENT_TYPE,
    size_t NAMESPACE_HASH,
    size_t NAME_HASH
  >
  bool
  register_data_client()
  {
    using wrapper_t =
      client_registration_wrapper__<
        DATA_CLIENT_TYPE,
        NAMESPACE_HASH,
        NAME_HASH
      >;

    data_client_registry_[typeid(DATA_CLIENT_TYPE).hash_code()]
      [data_hash_t::make_key(NAMESPACE_HASH, NAME_HASH)] =
      { unique_fid_t::instance().next(), wrapper_t::register_callback };

    return true;
  } // register_data_client

  //--------------------------------------------------------------------------//
  //! Return the data registry.
  //!
  //! @ingroup data
  //--------------------------------------------------------------------------//

  const auto &
  data_registry()
  const
  {
    return data_registry_;
  } // data_registry

  //--------------------------------------------------------------------------//
  //! Return the data client registry.
  //!
  //! @ingroup data
  //--------------------------------------------------------------------------//

  const auto &
  data_client_registry()
  const
  {
    return data_client_registry_;
  } // data_client_registry

  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

  ///
  /// \brief get an handle to registered data.
  ///
  /// \tparam STORAGE_TYPE
  /// \tparam TYPE
  /// \tparam NAMESPACE
  ///
  /// \param[in] runtime_namespace
  /// \param[in] key
  /// \param[in] version
  ///
  template<
    size_t STORAGE_TYPE,
    typename TYPE,
    size_t NAMESPACE
  >
  decltype(auto)
  get_mutator(
    const data_client_t & data_client,
    const utils::const_string_t & key,
    size_t slots, size_t version=0
  )
  {
    return st_t<STORAGE_TYPE>::template get_mutator<TYPE, NAMESPACE>(data_client,
      sp_t::data_store_, key, slots, version);
  } // get_mutator

  ///
  /// \brief Return a std::vector of handles to the stored states with
  ///        type \e TYPE in namespace \e NAMESPACE satisfying the predicate function
  ///        \e predicate.
  ///
  /// \tparam STORAGE_TYPE
  /// \tparam TYPE All state variables of this type will be returned.
  /// \tparam NAMESPACE Namespace to use.
  /// \tparam P Predicate function type.
  ///
  /// \param predicate A predicate function (returns true or false) that
  ///                  will be used to select which state variables are
  ///                  included in the return vector. Valid predicate
  ///                  funcitons must match the
  ///                  signature:
  ///                  \code
  ///                  bool predicate(const & USER_META_DATA)
  ///                  \endcode
  ///
  /// \param [in]  sorted  Sort the returned list by label lexographically.
  /// 
  /// \return A std::vector of handles to the state variables that
  ///         match the namespace and predicate criteria.
  ///
  template<
    size_t STORAGE_TYPE,
    typename TYPE,
    size_t NAMESPACE,
    typename P
  >
  decltype(auto)
  get_handles(
    const data_client_t & data_client,
    size_t version,    
    P && predicate,
    bool sorted = true
  )
  {
    return st_t<STORAGE_TYPE>::template get_handles<TYPE, NAMESPACE, P>(data_client,
      sp_t::data_store_, version, std::forward<P>(predicate), sorted);
  } // get_handles

  /// \brief Return a std::vector of handles to the stored states with
  ///        type \e TYPE satisfying the predicate function
  ///        \e predicate.
  ///
  /// \tparam STORAGE_TYPE
  /// \tparam TYPE All state variables of this type will be returned.
  /// \tparam P Predicate function type.
  ///
  /// \param predicate A predicate function (returns true or false) that
  ///                  will be used to select which state variables are
  ///                  included in the return vector. Valid predicate
  ///                  funcitons must match the
  ///                  signature:
  ///                  \code
  ///                  bool predicate(const & USER_META_DATA)
  ///                  \endcode
  ///
  /// \param [in]  sorted  Sort the returned list by label lexographically.
  /// 
  /// \return A std::vector of handles to the state variables that
  ///         match the namespace and predicate criteria.
  ///
  template<
    size_t STORAGE_TYPE,
    typename TYPE,
    typename P
  >
  decltype(auto)
  get_handles(
    const data_client_t & data_client,
    size_t version,    
    P && predicate,
    bool sorted = true
  )
  {
    return st_t<STORAGE_TYPE>::template get_handles<TYPE, P>(data_client,
      sp_t::data_store_, version, std::forward<P>(predicate), sorted);
  } // get_handles

  ///
  /// \brief Return a std::vector of handles to the stored states with
  ///        type \e TYPE in namespace \e NAMESPACE.
  ///
  /// \tparam STORAGE_TYPE
  /// \tparam TYPE All state variables of this type will be returned.
  /// \tparam NAMESPACE Namespace to use.
  /// \tparam P Predicate function type.
  ///
  /// \param [in]  sorted  Sort the returned list by label lexographically.
  /// 
  /// \return A std::vector of handles to the state variables that
  ///         match the namespace and predicate criteria.
  ///
  template<
    size_t STORAGE_TYPE,
    typename TYPE,
    size_t NAMESPACE
  >
  decltype(auto)
  get_handles(
    const data_client_t & data_client,
    size_t version,    
    bool sorted = true
  )
  {
    return st_t<STORAGE_TYPE>::template get_handles<TYPE, NAMESPACE>(data_client,
      sp_t::data_store_, version, sorted);
  } // get_handles

  /// \brief Return a std::vector of handles to the stored states with
  ///        type \e TYPE.
  ///
  /// \tparam STORAGE_TYPE
  /// \tparam TYPE All state variables of this type will be returned.
  /// \tparam P Predicate function type.
  ///
  /// \param [in]  sorted  Sort the returned list by label lexographically.
  /// 
  /// \return A std::vector of handles to the state variables that
  ///         match the namespace and predicate criteria.
  ///
  template<
    size_t STORAGE_TYPE,
    typename TYPE
  >
  decltype(auto)
  get_handles(
    const data_client_t & data_client,
    size_t version,    
    bool sorted = true
  )
  {
    return st_t<STORAGE_TYPE>::template get_handles<TYPE>(data_client,
      sp_t::data_store_, version, sorted);
  } // get_handles

  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

  ///
  /// \brief get a handle to registered data.
  ///
  /// \tparam STORAGE_TYPE
  /// \tparam TYPE
  /// \tparam NAMESPACE
  ///
  /// \param[in] runtime_namespace
  /// \param[in] key
  /// \param[in] version
  ///
  template<
    size_t STORAGE_TYPE,
    typename TYPE,
    size_t NAMESPACE,
    typename DATA_CLIENT_TYPE
  >
  decltype(auto)
  get_handle(
    const data_client_t & data_client,
    const utils::const_string_t & key,
    size_t version=0
  )
  {
    return st_t<STORAGE_TYPE>::template get_handle<
      TYPE, NAMESPACE, DATA_CLIENT_TYPE
      >(data_client, sp_t::data_store_, key, version);
  } // get_handle

  //--------------------------------------------------------------------------//
  // Data management.
  //--------------------------------------------------------------------------//

  ///
  ///
  ///
  void
  reset()
  {
    sp_t::reset();
  } // reset

  ///
  ///
  ///
  void
  reset(
    uintptr_t runtime_namespace
  )
  {
    sp_t::reset(runtime_namespace);
  } // reset

  ///
  ///
  ///
  template<typename TYPE>
  void
  release(
    TYPE && key,
    uintptr_t runtime_namespace
  )
  {
    sp_t::release(std::forward<TYPE>(key), runtime_namespace);
  } // release

  ///
  ///
  ///
  void
  move(
    uintptr_t from_runtime_namespace,
    uintptr_t to_runtime_namespace
  )
  {
    sp_t::move(from_runtime_namespace, to_runtime_namespace);
  } // move

private:

  // Data registration map
  std::unordered_map<size_t, client_value_t> data_registry_;

  // Data client registration map
  std::unordered_map<size_t, client_value_t> data_client_registry_;

}; // class storage__

} // namespace data
} // namespace flecsi

#include "flecsi_runtime_data_policy.h"

namespace flecsi {
namespace data {

using storage_t = storage__<FLECSI_RUNTIME_USER_META_DATA_POLICY,
  FLECSI_RUNTIME_STORAGE_POLICY>;

} // namespace data
} // namespace flecsi

#endif // flecsi_data_storage_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
