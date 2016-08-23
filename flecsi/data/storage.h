/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_storage_h
#define flecsi_data_storage_h

#include "flecsi/utils/const_string.h"
#include "flecsi/data/data_client.h"

///
// \file storage.h
// \authors bergen
// \date Initial file creation: Apr 17, 2016
///

namespace flecsi {
namespace data {

///
// \class storage__ storage.h
// \brief storage__ provides an interface for data registration and access.
///
template<typename user_meta_data_t,
  template<typename> typename storage_policy_t>
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
  // \brief Return a static instance of the data manager.
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
  // \brief Register data with the data manager.
  //
  // \tparam DT Data type...
  // \tparam T Type...
  // \tparam NS Namespace...
  // \tparam Args Variadic arguments...
  //
  // \param[in] runtime_namespace
  // \param[in] key
  // \param[in] versions
  // \param[in] args
  //
  // \return Returns a handle to the newly registered data.
  ///
  template<
    size_t DT,
    typename T,
    size_t NS,
    typename ... Args
  >
  decltype(auto)
  register_data(
    data_client_t & data_client,
    const const_string_t & key,
    size_t versions,
    Args && ... args
  )
  {
    return st_t<DT>::template register_data<T, NS>(data_client,
      sp_t::data_store_, key, versions,
      std::forward<Args>(args) ...);
  } // register_data

  //--------------------------------------------------------------------------//
  // Data accessors.
  //--------------------------------------------------------------------------//

  ///
  // \brief get an accessor to registered data.
  //
  // \tparam DT
  // \tparam T
  // \tparam NS
  //
  // \param[in] runtime_namespace
  // \param[in] key
  // \param[in] version
  ///
  template<
    size_t DT,
    typename T,
    size_t NS
  >
  decltype(auto)
  get_accessor(
    data_client_t & data_client,
    const const_string_t & key,
    size_t version=0
  )
  {
    return st_t<DT>::template get_accessor<T, NS>(data_client,
      sp_t::data_store_, key, version);
  } // get_accessor

  ///
  // \brief get an accessor to registered data.
  //
  // \tparam DT
  // \tparam T
  // \tparam NS
  //
  // \param[in] runtime_namespace
  // \param[in] key
  // \param[in] version
  ///
  template<
    size_t DT,
    typename T,
    size_t NS
  >
  decltype(auto)
  get_mutator(
    data_client_t & data_client,
    const const_string_t & key,
    size_t slots, size_t version=0
  )
  {
    return st_t<DT>::template get_mutator<T, NS>(data_client,
      sp_t::data_store_, key, slots, version);
  } // get_mutator

  ///
  // \brief Return a std::vector of accessors to the stored states with
  //        type \e T in namespace \e NS satisfying the predicate function
  //        \e predicate.
  //
  // \tparam DT
  // \tparam T All state variables of this type will be returned.
  // \tparam NS Namespace to use.
  // \tparam P Predicate function type.
  //
  // \param predicate A predicate function (returns true or false) that
  //                  will be used to select which state variables are
  //                  included in the return vector. Valid predicate
  //                  funcitons must match the
  //                  signature:
  //                  \code
  //                  bool predicate(const & user_meta_data_t)
  //                  \endcode
  // 
  // \return A std::vector of accessors to the state variables that
  //         match the namespace and predicate criteria.
  ///
  template<
    size_t DT,
    typename T,
    size_t NS,
    typename P
  >
  decltype(auto)
  get_accessors(
    data_client_t & data_client,
    P && predicate
  )
  {
    return st_t<DT>::template get_accessors<T, NS, P>(data_client,
      sp_t::data_store_, std::forward<P>(predicate));
  } // get_accessors

  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

  ///
  // \brief get a handle to registered data.
  //
  // \tparam DT
  // \tparam T
  // \tparam NS
  //
  // \param[in] runtime_namespace
  // \param[in] key
  // \param[in] version
  ///
  template<
    size_t DT,
    typename T,
    size_t NS
  >
  decltype(auto)
  get_handle(
    data_client_t & data_client,
    const const_string_t & key,
    size_t version=0
  )
  {
    return st_t<DT>::template get_handle<T, NS>(data_client,
      sp_t::data_store_, key, version);
  } // get_accessor

  //--------------------------------------------------------------------------//
  // Data management.
  //--------------------------------------------------------------------------//

  ///
  //
  ///
  void
  reset()
  {
    sp_t::reset();
  } // reset

  ///
  //
  ///
  void
  reset(
    uintptr_t runtime_namespace
  )
  {
    sp_t::reset(runtime_namespace);
  } // reset

  ///
  //
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
  //
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
