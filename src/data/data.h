/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_data_h
#define flecsi_data_h

#include <limits>

#include "default_storage_policy.h"
#include "data_constants.h"
#include "default_meta_data.h"

/*!
 * \file data.h
 * \authors bergen
 * \date Initial file creation: Oct 09, 2015
 */

namespace flecsi
{

namespace data_model
{

/*----------------------------------------------------------------------------*
 * class data_t
 *----------------------------------------------------------------------------*/

/*!
  \class data_t data.h
  \brief data_t provides an interface for registering and accessing
    data data.

  This type can be statically configured to use various implementations.
 */
template<
  typename user_meta_data_t = default_state_user_meta_data_t,
  template <typename> typename storage_policy_t = default_data_storage_policy_t
  >
class data_t : public storage_policy_t<user_meta_data_t>
{
private:

  // This is just for convenience...
  using sp_t = storage_policy_t<user_meta_data_t>;

public:

  /*!
    Return an instance of the data manager.
   */
  static data_t & instance()
  {
    static data_t d;
    std::cout << "state address: " << &d << std::endl;
    return d;
  } // instance

  void reset() {
    sp_t::reset();
  } // reset

  //! Use the accessor type defined by the storage policy.
  template <typename T>
  using dense_accessor_t = typename sp_t::template dense_accessor_t<T>;

  //! Use the accessor type defined by the storage policy.
  template <typename T>
  using global_accessor_t = typename sp_t::template global_accessor_t<T>;

  //! Default constructor
  data_t() : sp_t() {}

  //! Destructor
  ~data_t() {}

  /*--------------------------------------------------------------------------*
   * Dense Registration / Access
   *--------------------------------------------------------------------------*/
  /*!
    \brief Register a state variable with the state manager.  This method
      logically allocates space for the variable of size \e indices which
      will be identified by \e key.

    \tparam T The type of the variable to be registered.  Any type that
      does not contain a pointer is supported.
    \tparam NS The namespace in which to register the state variable.  The
      default namespace is the \e user namespace as defined by
      \ref flecsi::state_name_space_t.  Other namespaces may
      be defined to avoid naming collisions.
    \tparam Args A variadic list of arguments that are  passed to the
      inititalization method of the user-defined meta data type.

    \param[in] key The name of the state variable to register. See the
      documentation for \ref const_string_t for more information on
      valid usage.  Normally, this parameter is just a
      \e const \e char \e *, e.g., "density".
    \param indices The size of the index space with which this variable
      is associated, i.e., the number of elements to allocate.
    \param args A variadic list of arguments to pass to the initialization
      function of the user-defined meta data type.

    \return FIXME
   */
  template <typename T, size_t NS = flecsi_user_space, typename... Args>
  decltype(auto) register_state(
      const const_string_t & key, size_t indices, uintptr_t runtime_namespace,
      Args &&... args)
  {
    return sp_t::template register_state<T, NS>(
        key, indices, runtime_namespace, std::forward<Args>(args)...);
  } // register_state

  /*!
    Return a dense_accessor_t instance that provides logical
    array-based access to the state data.

    \tparam T The type of the variable to be returned.  The type information
      for a state variable is not necessarily stored, meaning that the
      state can be interpreted however the user wants.  Needless to say,
      if the type requested is not consistent with the actual stored type,
      bad things will happen.
    \tparam NS The namespace of the requested state variable.  The
      default namespace is the \e user namespace as defined by
      \ref flecsi::state_name_space_t.

    \param key The name of the state variable to return.

    \return An accessor to the requested state data.
   */

  template <typename T, size_t NS = flecsi_user_space>
  dense_accessor_t<T> dense_accessor(const const_string_t & key,
    uintptr_t runtime_namespace = flecsi_runtime_user_space)
  {
    return sp_t::template dense_accessor<T, NS>(key, runtime_namespace);
  } // dense_accessor

  /*!
    \brief Return a std::vector of accessors to the stored states with
      type \e T in namespace \e NS.

    \tparam T All state variables of this type will be returned.
    \tparam NS Namespace to use.

    \return A std::vector of accessors to the state variables that
      match the type and namespace criteria.
   */
  template <typename T, size_t NS = flecsi_user_space>
  std::vector<dense_accessor_t<T>> dense_accessors(
    uintptr_t runtime_namespace = flecsi_runtime_user_space)
  {
    return sp_t::template dense_accessors<T, NS>(runtime_namespace);
  } // dense_accessors

  /*!
    \brief Return a std::vector of accessors to the stored states with
      type \e T in namespace \e NS satisfying the predicate function
      \e predicate.

    \tparam T All state variables of this type will be returned.
    \tparam P Predicate function type.
    \tparam NS Namespace to use.

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

  template <typename T, typename P, size_t NS = flecsi_user_space>
  std::vector<dense_accessor_t<T>> dense_accessors(P && predicate,
    uintptr_t runtime_namespace = flecsi_runtime_user_space)
  {
    return sp_t::template dense_accessors<T, NS, P>(
      std::forward<P>(predicate), runtime_namespace);
  } // dense_accessors

  /*!
    Return a std::vector of raw accessors to the stored states with
    in namespace \e NS.  Raw accessors are of type uint8_t.

    \tparam NS Namespace to use.

    \return A std::vector of raw accessors to the state variables that
      match the namespace criteria.
   */

  template <size_t NS = flecsi_user_space>
  std::vector<dense_accessor_t<uint8_t>> raw_dense_accessors(
    uintptr_t runtime_namespace = flecsi_runtime_user_space)
  {
    return sp_t::template raw_dense_accessors<NS>(runtime_namespace);
  } // raw_dense_accessors

  /*!
    Return a std::vector of raw accessors to the stored states with
    in namespace \e NS satisfying the predicate function \e predicate.

    \tparam NS Namespace to use.

    \param predicate A predicate function (returns true or false) that
      will be used to select which state variables are included in the
      return vector.  Valid predicate funcitons must match the
      signature:
      \code
      bool predicate(const & user_meta_data_t)
      \endcode

    \return A std::vector of raw accessors to the state variables that
      match the namespace and predicate criteria.
   */

  template <typename P, size_t NS = flecsi_user_space>
  std::vector<dense_accessor_t<uint8_t>> raw_dense_accessors(P && predicate,
    uintptr_t runtime_namespace = flecsi_runtime_user_space)
  {
    return sp_t::template raw_dense_accessors<NS>(
      std::forward<P>(predicate), runtime_namespace);
  } // raw_dense_accessors

  /*--------------------------------------------------------------------------*
   * Global Registration / Access
   *--------------------------------------------------------------------------*/

  /*!
    \brief Register a state variable with the state manager.  This method
      logically allocates space for the variable which
      will be identified by \e key.

    \tparam T The type of the variable to be registered.  Any type that
      does not contain a pointer is supported.
    \tparam NS The namespace in which to register the state variable.  The
      default namespace is the \e user namespace as defined by
      \ref flecsi::state_name_space_t.  Other namespaces may
      be defined to avoid naming collisions.
    \tparam Args A variadic list of arguments that are  passed to the
      inititalization method of the user-defined meta data type.

    \param[in] key The name of the state variable to register. See the
      documentation for \ref const_string_t for more information on
      valid usage.  Normally, this parameter is just a
      \e const \e char \e *, e.g., "density".
    \param args A variadic list of arguments to pass to the initialization
      function of the user-defined meta data type.

    \return FIXME
   */
  template <typename T, size_t NS = flecsi_user_space, typename... Args>
  decltype(auto) register_global_state(
    const const_string_t & key, uintptr_t runtime_namespace, Args &&... args)
  {
    return sp_t::template register_global_state<T, NS>(
        key, runtime_namespace, std::forward<Args>(args)...);
  } // register_state

  /*!
    Return a global_accessor_t instance that provides logical
    array-based access to the state data.

    \tparam T The type of the variable to be returned.  The type information
      for a state variable is not necessarily stored, meaning that the
      state can be interpreted however the user wants.  Needless to say,
      if the type requested is not consistent with the actual stored type,
      bad things will happen.
    \tparam NS The namespace of the requested state variable.  The
      default namespace is the \e user namespace as defined by
      \ref flecsi::state_name_space_t.

    \param key The name of the state variable to return.

    \return An accessor to the requested state data.
   */

  template <typename T, size_t NS = flecsi_user_space>
  global_accessor_t<T> global_accessor(const const_string_t & key,
    uintptr_t runtime_namespace = flecsi_runtime_user_space)
  {
    return sp_t::template global_accessor<T, NS>(key, runtime_namespace);
  } // global_accessor

  /*!
    \brief Return a std::vector of accessors to the stored states with
      type \e T in namespace \e NS.

    \tparam T All state variables of this type will be returned.
    \tparam NS Namespace to use.

    \return A std::vector of accessors to the state variables that
      match the type and namespace criteria.
   */
  template <typename T, size_t NS = flecsi_user_space>
  std::vector<global_accessor_t<T>> global_accessors(
    uintptr_t runtime_namespace = flecsi_runtime_user_space)
  {
    return sp_t::template global_accessors<T, NS>(runtime_namespace);
  } // global_accessors

  /*!
    \brief Return a std::vector of accessors to the stored states with
      type \e T in namespace \e NS satisfying the predicate function
      \e predicate.

    \tparam T All state variables of this type will be returned.
    \tparam P Predicate function type.
    \tparam NS Namespace to use.

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

  template <typename T, typename P, size_t NS = flecsi_user_space>
  std::vector<global_accessor_t<T>> global_accessors(P && predicate,
    uintptr_t runtime_namespace = flecsi_runtime_user_space)
  {
    return sp_t::template global_accessors<T, NS, P>(
      std::forward<P>(predicate), runtime_namespace);
  } // global_accessors

  /*!
    Return a std::vector of raw accessors to the stored states with
    in namespace \e NS.  Raw accessors are of type uint8_t.

    \tparam NS Namespace to use.

    \return A std::vector of raw accessors to the state variables that
      match the namespace criteria.
   */

  template <size_t NS = flecsi_user_space>
  std::vector<global_accessor_t<uint8_t>> raw_global_accessors(
    uintptr_t runtime_namespace = flecsi_runtime_user_space)
  {
    return sp_t::template raw_global_accessors<NS>(runtime_namespace);
  } // raw_global_accessors

  /*!
    Return a std::vector of raw accessors to the stored states with
    in namespace \e NS satisfying the predicate function \e predicate.

    \tparam NS Namespace to use.

    \param predicate A predicate function (returns true or false) that
      will be used to select which state variables are included in the
      return vector.  Valid predicate funcitons must match the
      signature:
      \code
      bool predicate(const & user_meta_data_t)
      \endcode

    \return A std::vector of raw accessors to the state variables that
      match the namespace and predicate criteria.
   */

  template <typename P, size_t NS = flecsi_user_space>
  std::vector<global_accessor_t<uint8_t>> raw_global_accessors(P && predicate,
    uintptr_t runtime_namespace = flecsi_runtime_user_space)
  {
    return sp_t::template raw_global_accessors<NS>(
      std::forward<P>(predicate), runtime_namespace);
  } // raw_global_accessors

  /*--------------------------------------------------------------------------*
   * General Registration / Access
   *--------------------------------------------------------------------------*/

  /*!
    \brief Return the user meta data for the state variable identified
      by \e key in namespace \e NS.

    \param key The name of the state variable for which to return
      the meta data.

    \return The meta data corresponding to the key and namespace.
   */
  template <size_t NS = flecsi_user_space>
  user_meta_data_t & meta_data(const const_string_t & key,
    uintptr_t runtime_namespace = flecsi_runtime_user_space)
  {
    return sp_t::template meta_data<NS>(key, runtime_namespace);
  } // user_meta_data

  /*!
    \brief Return a copy of the state variable as an array of type \e T.

    \attention Calls to this function are expensive because they
      require that data (that may not be physically stored as an array)
      be replicated and repackaged to satisfy this request.

    \tparam T The type to be used to interpret the state variable.
    \tparam NS The namespace of the desired state variable.

    \param key The name of the state variable for which to return
      the data.

    \return A std::shared_ptr<T> with a copy of the state data.
   */
  template <typename T, size_t NS = flecsi_user_space>
  std::shared_ptr<T> & data(const const_string_t & key,
    uintptr_t runtime_namespace = flecsi_runtime_user_space)
  {
    return sp_t::template data<T, NS>(key, runtime_namespace);
  } // data

  //! Copy constructor (disabled)
  data_t(const data_t &) = delete;

  //! Assignment operator (disabled)
  data_t & operator=(const data_t &) = delete;

  // Allow move operations
  data_t(data_t &&) = default;
  data_t & operator=(data_t &&) = default;

}; // class data_t

} // namespace data_model
} // namespace flecsi

#endif // flecsi_data_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
