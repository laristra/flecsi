/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_model_new_data_h
#define flecsi_data_model_new_data_h

#include "flecsi/data/default/default_storage_policy.h"

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

  // Type definitions
  using sp_t = storage_policy_t<user_meta_data_t>;

  template<size_t data_type_t>
  using st_t = typename sp_t::template storage_type_t<data_type_t>;

  // Hide these
  new_data_t() = delete;
  new_data_t(const new_data_t &) = delete;
  new_data_t & operator = (const new_data_t &) = delete;

  // Allow move operations
  new_data_t(new_data_t &&) = default;
  new_data_t & operator = (new_data_t &&) = default;

  //! Meyer's singleton instance.
  static new_data_t & instance() {
    static new_data_t d;
    return d;
  } // instance

  /*!
    \tparam DT Data type...
    \tparam T Type...
    \tparam NS Namespace...
    \tparam Args Variadic arguments...
   */
  template<size_t DT, typename T, size_t NS, typename ... Args>
  decltype(auto) register_data(uintptr_t runtime_namespace,
    const const_string_t & key, Args && ... args) {
    return st_t<DT>::template register_data<T, NS>(sp_t::data_store_,
      runtime_namespace, key, std::forward<Args>(args) ...);
  } // register_data

  template<size_t DT, typename T, size_t NS>
  decltype(auto) get_accessor(uintptr_t runtime_namespace,
    const const_string_t & key) {
    return st_t<DT>::template get_accessor<T, NS>(sp_t::data_store_,
      runtime_namespace, key);
  } // get_accessor

  template<size_t DT, typename T, size_t NS>
  decltype(auto) get_handle(uintptr_t runtime_namespace,
    const const_string_t & key) {
    return st_t<DT>::template get_handle<T, NS>(sp_t::data_store_,
      runtime_namespace, key);
  } // get_accessor

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
