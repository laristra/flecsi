/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_model_new_data_h
#define flecsi_data_model_new_data_h

#include "flecsi/data/default/default_storage_policy.h"
#include "flecsi/data/data_client.h"

/*!
 * \file new_data.h
 * \authors bergen
 * \date Initial file creation: Apr 17, 2016
 */

namespace flecsi {
namespace data_model {

struct default_user_meta_data_t {
  void initialize() {}
}; // struct default_user_meta_data_t

/*!
  \class new_data new_data.h
  \brief new_data provides an interface for data registration and access.
 */
template<
  typename user_meta_data_t = default_user_meta_data_t,
  template<typename> typename storage_policy_t = default_storage_policy_t
  >
struct new_data_t : public storage_policy_t<user_meta_data_t> {

  /*--------------------------------------------------------------------------*
   * Type definitions.
   *--------------------------------------------------------------------------*/

  using sp_t = storage_policy_t<user_meta_data_t>;

  template<size_t data_type_t>
  using st_t = typename sp_t::template storage_type_t<data_type_t>;

  /*--------------------------------------------------------------------------*
   * Class interface.
   *--------------------------------------------------------------------------*/

	// Constructor.
  new_data_t() {}

  // Hide these.
  new_data_t(const new_data_t &) = delete;
  new_data_t & operator = (const new_data_t &) = delete;

  // Allow move operations.
  new_data_t(new_data_t &&) = default;
  new_data_t & operator = (new_data_t &&) = default;

	/*!
		\brief Return a static instance of the data manager.
	 */
  static new_data_t & instance() {
    static new_data_t d;
    return d;
  } // instance

  /*--------------------------------------------------------------------------*
   * Data registration.
   *--------------------------------------------------------------------------*/

  /*!
		\brief Register data with the data manager.

    \tparam DT Data type...
    \tparam T Type...
    \tparam NS Namespace...
    \tparam Args Variadic arguments...

		\param[in] runtime_namespace
		\param[in] key
		\param[in] versions
		\param[in] args

		\return Returns a handle to the newly registered data.
   */
  template<size_t DT, typename T, size_t NS, typename ... Args>
  decltype(auto) register_data(data_client_t & data_client,
    const const_string_t & key, size_t versions=1, Args && ... args) {
    return st_t<DT>::template register_data<T, NS>(data_client,
      sp_t::data_store_, key, versions, 
      std::forward<Args>(args) ...);
  } // register_data

  /*--------------------------------------------------------------------------*
   * Data accessors.
   *--------------------------------------------------------------------------*/

  /*!
    \brief get an accessor to registered data.

    \tparam DT
    \tparam T
    \tparam NS

    \param[in] runtime_namespace
    \param[in] key
    \param[in] version
   */
  template<size_t DT, typename T, size_t NS>
  decltype(auto) get_accessor(data_client_t & data_client,
    const const_string_t & key, size_t version=0) {
    return st_t<DT>::template get_accessor<T, NS>(data_client,
      sp_t::data_store_, key, version);
  } // get_accessor

  /*!
    \brief get an accessor to registered data.

    \tparam DT
    \tparam T
    \tparam NS

    \param[in] runtime_namespace
    \param[in] key
    \param[in] version
   */
  template<size_t DT, typename T, size_t NS>
  decltype(auto) get_mutator(data_client_t & data_client,
    const const_string_t & key, size_t slots, size_t version=0) {
    return st_t<DT>::template get_mutator<T, NS>(data_client,
      sp_t::data_store_, key, slots, version);
  } // get_accessor

  /*!
    \brief Return a std::vector of accessors to the stored states with
      type \e T in namespace \e NS satisfying the predicate function
      \e predicate.

    \tparam DT
    \tparam T All state variables of this type will be returned.
    \tparam NS Namespace to use.
    \tparam P Predicate function type.

    \param predicate A predicate function (returns true or false) that
      will be used to select which state variables are included in the
      return vector.  Valid predicate funcitons must match the
      signature:
      \code
      bool predicate(const & user_meta_data_t)
      \endcode

    \return A std::vector of accessors to the state variables that
      match the namespace and predicate criteria.
   */
  template<size_t DT, typename T, size_t NS, typename P>
  decltype(auto) get_accessors(data_client_t & data_client, P && predicate) {
    return st_t<DT>::template get_accessors<T, NS, P>(data_client,
      sp_t::data_store_, std::forward<P>(predicate));
  } // get_accessors

  /*--------------------------------------------------------------------------*
   * Data handles.
   *--------------------------------------------------------------------------*/

  /*!
    \brief get a handle to registered data.

    \tparam DT
    \tparam T
    \tparam NS

    \param[in] runtime_namespace
    \param[in] key
    \param[in] version
   */
  template<size_t DT, typename T, size_t NS>
  decltype(auto) get_handle(data_client_t & data_client,
    const const_string_t & key, size_t version=0) {
    return st_t<DT>::template get_handle<T, NS>(data_client,
      sp_t::data_store_, key, version);
  } // get_accessor

  /*--------------------------------------------------------------------------*
   * Data management.
   *--------------------------------------------------------------------------*/

  void reset() {
    sp_t::reset();
  } // reset

  void reset(uintptr_t runtime_namespace) {
    sp_t::reset(runtime_namespace);
  } // reset

  template<typename T>
  void release(T && key, uintptr_t runtime_namespace) {
    sp_t::release(std::forward<T>(key), runtime_namespace);
  } // release

  void move(uintptr_t from_runtime_namespace,
    uintptr_t to_runtime_namespace ) {
    sp_t::move(from_runtime_namespace, to_runtime_namespace);
  } // move

}; // class new_data_t

} // namespace data_model
} // namespace flecsi

#endif // flecsi_data_model_new_data_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
