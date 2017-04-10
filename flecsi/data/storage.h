/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_storage_h
#define flecsi_data_storage_h

#include "flecsi/utils/const_string.h"
#include "flecsi/data/data_client.h"
#include "flecsi/data/data_handle.h"

///
/// \file
/// \date Initial file creation: Apr 17, 2016
///

namespace flecsi {
namespace data {

///
/// \class storage__ storage.h
/// \brief storage__ provides an interface for data registration and access.
///
template<typename user_meta_data_t,
  template<typename> class storage_policy_t>
struct storage__ : public storage_policy_t<user_meta_data_t> {

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using sp_t = storage_policy_t<user_meta_data_t>;

  template<size_t data_type_t>
  using st_t = typename sp_t::template storage_type_t<data_type_t>;

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

	///
  /// \brief Return a static instance of the data manager.
	///
  static storage__ &
  instance()
  {
    static storage__ d;
    return d;
  } // instance

  //--------------------------------------------------------------------------//
  // Data registration.
  //--------------------------------------------------------------------------//

  ///
  /// \brief Register data with the data manager.
  ///
  /// \tparam ST Storage type...
  /// \tparam T Type...
  /// \tparam NS Namespace...
  /// \tparam Args Variadic arguments...
  ///
  /// \param[in] runtime_namespace
  /// \param[in] key
  /// \param[in] versions
  /// \param[in] args
  ///
  /// \return Returns a handle to the newly registered data.
  ///
  template<
    size_t ST,
    typename T,
    size_t NS,
    typename ... Args
  >
  decltype(auto)
  register_data(
    const data_client_t & data_client,
    const utils::const_string_t & key,
    size_t versions,
    Args && ... args
  )
  {
    return st_t<ST>::template register_data<T, NS>(data_client,
      sp_t::data_store_, key, versions,
      std::forward<Args>(args) ...);
  } // register_data

  ///
  /// \brief Register data with the data manager.
  ///
  /// \tparam DT Data client type
  /// \tparam ST Storage type
  /// \tparam T Data type
  /// \tparam NS Namespace
  /// \tparam Args Variadic arguments
  ///
  /// \return Returns a boolean with success or failure of registration.
  ///

#if 0
  template<
    typename DC,
    size_t ST,
    typename T,
    size_t NS,
    size_t N,
    size_t V
  >
  void register_data(size_t fid) {
      st_t<ST>::template register_data<T, NS>(DC, sp_t::data_store_, N, V);
  } // register_data
#endif

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
  new_register_data()
  {
    return sp_t::template new_register_data<
      DATA_CLIENT_TYPE,
      STORAGE_TYPE,
      DATA_TYPE,
      NAMESPACE_HASH,
      NAME_HASH,
      INDEX_SPACE,
      VERSIONS
    >();
  } // new_register_data

  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

  ///
  /// \brief get an handle to registered data.
  ///
  /// \tparam ST
  /// \tparam T
  /// \tparam NS
  ///
  /// \param[in] runtime_namespace
  /// \param[in] key
  /// \param[in] version
  ///
  template<
    size_t ST,
    typename T,
    size_t NS
  >
  decltype(auto)
  get_mutator(
    const data_client_t & data_client,
    const utils::const_string_t & key,
    size_t slots, size_t version=0
  )
  {
    return st_t<ST>::template get_mutator<T, NS>(data_client,
      sp_t::data_store_, key, slots, version);
  } // get_mutator

  ///
  /// \brief Return a std::vector of handles to the stored states with
  ///        type \e T in namespace \e NS satisfying the predicate function
  ///        \e predicate.
  ///
  /// \tparam ST
  /// \tparam T All state variables of this type will be returned.
  /// \tparam NS Namespace to use.
  /// \tparam P Predicate function type.
  ///
  /// \param predicate A predicate function (returns true or false) that
  ///                  will be used to select which state variables are
  ///                  included in the return vector. Valid predicate
  ///                  funcitons must match the
  ///                  signature:
  ///                  \code
  ///                  bool predicate(const & user_meta_data_t)
  ///                  \endcode
  ///
  /// \param [in]  sorted  Sort the returned list by label lexographically.
  /// 
  /// \return A std::vector of handles to the state variables that
  ///         match the namespace and predicate criteria.
  ///
  template<
    size_t ST,
    typename T,
    size_t NS,
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
    return st_t<ST>::template get_handles<T, NS, P>(data_client,
      sp_t::data_store_, version, std::forward<P>(predicate), sorted);
  } // get_handles

  /// \brief Return a std::vector of handles to the stored states with
  ///        type \e T satisfying the predicate function
  ///        \e predicate.
  ///
  /// \tparam ST
  /// \tparam T All state variables of this type will be returned.
  /// \tparam P Predicate function type.
  ///
  /// \param predicate A predicate function (returns true or false) that
  ///                  will be used to select which state variables are
  ///                  included in the return vector. Valid predicate
  ///                  funcitons must match the
  ///                  signature:
  ///                  \code
  ///                  bool predicate(const & user_meta_data_t)
  ///                  \endcode
  ///
  /// \param [in]  sorted  Sort the returned list by label lexographically.
  /// 
  /// \return A std::vector of handles to the state variables that
  ///         match the namespace and predicate criteria.
  ///
  template<
    size_t ST,
    typename T,
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
    return st_t<ST>::template get_handles<T, P>(data_client,
      sp_t::data_store_, version, std::forward<P>(predicate), sorted);
  } // get_handles

  ///
  /// \brief Return a std::vector of handles to the stored states with
  ///        type \e T in namespace \e NS.
  ///
  /// \tparam ST
  /// \tparam T All state variables of this type will be returned.
  /// \tparam NS Namespace to use.
  /// \tparam P Predicate function type.
  ///
  /// \param [in]  sorted  Sort the returned list by label lexographically.
  /// 
  /// \return A std::vector of handles to the state variables that
  ///         match the namespace and predicate criteria.
  ///
  template<
    size_t ST,
    typename T,
    size_t NS
  >
  decltype(auto)
  get_handles(
    const data_client_t & data_client,
    size_t version,    
    bool sorted = true
  )
  {
    return st_t<ST>::template get_handles<T, NS>(data_client,
      sp_t::data_store_, version, sorted);
  } // get_handles

  /// \brief Return a std::vector of handles to the stored states with
  ///        type \e T.
  ///
  /// \tparam ST
  /// \tparam T All state variables of this type will be returned.
  /// \tparam P Predicate function type.
  ///
  /// \param [in]  sorted  Sort the returned list by label lexographically.
  /// 
  /// \return A std::vector of handles to the state variables that
  ///         match the namespace and predicate criteria.
  ///
  template<
    size_t ST,
    typename T
  >
  decltype(auto)
  get_handles(
    const data_client_t & data_client,
    size_t version,    
    bool sorted = true
  )
  {
    return st_t<ST>::template get_handles<T>(data_client,
      sp_t::data_store_, version, sorted);
  } // get_handles

  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

  ///
  /// \brief get a handle to registered data.
  ///
  /// \tparam ST
  /// \tparam T
  /// \tparam NS
  ///
  /// \param[in] runtime_namespace
  /// \param[in] key
  /// \param[in] version
  ///
  template<
    size_t ST,
    typename T,
    size_t NS
  >
  decltype(auto)
  get_handle(
    const data_client_t & data_client,
    const utils::const_string_t & key,
    size_t version=0
  )
  {
    return st_t<ST>::template get_handle<T, NS>(data_client,
      sp_t::data_store_, key, version);
  } // get_handle
/*
  template<
    size_t ST>
  void
  get_all_handles(
    const data_client_t & data_client,
    std::vector<data_handle_t<void, 0, 0, 0>>& handles,
    std::vector<size_t>& hashes,
    std::vector<size_t>& namespaces,
    std::vector<size_t>& versions)
  {
    return st_t<ST>::get_all_handles(data_client,
      sp_t::data_store_, handles, hashes, namespaces, versions);
  }

  template<
    size_t ST>
  void
  put_all_handles(
    const data_client_t & data_client,
    size_t num_handles,
    data_handle_t<void, 0, 0, 0>* handles,
    size_t* hashes,
    size_t* namespaces,
    size_t* versions)
  {
    return st_t<ST>::put_all_handles(data_client,
      sp_t::data_store_, num_handles, handles, hashes, namespaces, versions);
  }
  */

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
  template<typename T>
  void
  release(
    T && key,
    uintptr_t runtime_namespace
  )
  {
    sp_t::release(std::forward<T>(key), runtime_namespace);
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

}; // class storage__

} // namespace data
} // namespace flecsi

#include "flecsi_runtime_data_policy.h"

namespace flecsi {
namespace data {

using storage_t = storage__<flecsi_user_meta_data_policy_t,
  flecsi_storage_policy_t>;

} // namespace data
} // namespace flecsi

#endif // flecsi_data_storage_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
