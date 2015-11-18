/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_state_h
#define flexi_state_h

#include "default_storage_policy.h"
#include "../utils/const_string.h"
#include "../utils/bitfield.h"

/*!
 * \file state.h
 * \authors bergen
 * \date Initial file creation: Oct 09, 2015
 */

namespace flexi {

/*!
  \brief state_name_space_t defines the various state namespaces that
    are available for registering and maintaining state.

  This type is provided as a convenience to avoid naming collisions.
 */
enum class state_name_space_t : size_t {
  user = 0,
  flexi = std::numeric_limits<size_t>::max()
}; // enum class state_name_space_t

/*!
  \brief state_attribute_t defines different state attributes.

  This type should probably be pushed up in the interface so that users
  can define their own attributes.
 */
enum class state_attribute_t : bitfield_t::field_type_t {
  persistent = 0x0001,
  temporary  = 0x0002
}; // enum class state_attribute_t

/*!
  \brief This exposes the persistent attribute so that it can be used
    without specifying the full type information.
 */
const bitfield_t::field_type_t persistent =
  static_cast<bitfield_t::field_type_t>(flexi::state_attribute_t::persistent);


/*!
  \brief This exposes the temporary attribute so that it can be used
    without specifying the full type information.
 */
const bitfield_t::field_type_t temporary =
  static_cast<bitfield_t::field_type_t>(flexi::state_attribute_t::temporary);

/*----------------------------------------------------------------------------*
 * struct default_state_meta_data_t
 *----------------------------------------------------------------------------*/

/*!
  \brief default_state_user_meta_data_t defines a default meta data type.

  This type should really never get used, i.e., the state specialization
  should provide a meta data type.  So far, this type has mostly been
  useful for testing.
 */
struct default_state_user_meta_data_t {

  void initialize(const size_t & site_id_,
    bitfield_t::field_type_t attributes_) {
    site_id = site_id_;
    attributes = attributes_;
  } // initialize

  size_t site_id;
  bitfield_t::field_type_t attributes;

}; // struct default_state_user_meta_data_t

/*----------------------------------------------------------------------------*
 * class state_t
 *----------------------------------------------------------------------------*/

/*!
  \class state_t state.h
  \brief state_t provides an interface for registering and accessing
    state data.

  This type can be statically configured to use various implementations.
 */

template<typename user_meta_data_t = default_state_user_meta_data_t,
  template<typename> typename storage_policy_t =
    default_state_storage_policy_t>
class state_t : public storage_policy_t<user_meta_data_t>
{
private:

  // This is just for convenience...
  using sp_t = storage_policy_t<user_meta_data_t>;

public:

  //! Use the accessor type defined by the storage policy.
  template<typename T>
  using accessor_t = typename sp_t::template accessor_t<T>;

  //! Default constructor
  state_t() : sp_t() {}

  //! Destructor
   ~state_t() {}

  /*!
    Register a state variable with the state manager.  This method
    logically allocates space for the variable of size \e indices which
    will be identified by \e key.

    \tparam T The type of the variable to be registered.  Any type that
      does not contain a pointer is supported.
    \tparam NS The namespace in which to register the state variable.  The
      default namespace is the \e user namespace as defined by
      \ref flexi::state_name_space_t.  Other namespaces may
      be defined to avoid naming collisions.
    \tparam Args A variadic list of arguments that are  passed to the
      inititalization method of the user-defined meta data type.

    \param key The name of the state variable to register. See the
      documentation for \ref const_string_t for more information on
      valid usage.  Normally, this parameter is just a
      \e const \e char \e *, e.g., "density".
    \param indices The size of the index space with which this variable
      is associated, i.e., the number of elements to allocate.
    \param args A variadic list of arguments to pass to the initialization
      function of the user-defined meta data type.
   */

  template<typename T,
    size_t NS = static_cast<size_t>(state_name_space_t::user),
    typename ... Args>
  decltype(auto) register_state(const const_string_t & key, size_t indices,
    Args && ... args) {
    return sp_t::template register_state<T,NS>(key, indices,
      std::forward<Args>(args) ...);
  } // register_state

  /*!
    Return an accessor_t instance that provides logical array-based access
    to the state data.

    \tparam T The type of the variable to be returned.  The type information
      for a state variable is not necessarily stored, meaning that the
      state can be interpreted however the user wants.  Needless to say,
      if the type requested is not consistent with the actual stored type,
      bad things will happen.
    \tparam NS The namespace of the requested state variable.  The
      default namespace is the \e user namespace as defined by
      \ref flexi::state_name_space_t.

    \param key The name of the state variable to return.

    \return An accessor to the requested state data.
   */

  template<typename T,
    size_t NS = static_cast<size_t>(state_name_space_t::user)>
  accessor_t<T> accessor(const const_string_t & key) {
    return sp_t::template accessor<T,NS>(key);
  } // accessor

  /*!
    Return a std::vector of accessors to the stored states with
    type \e T in namespace \e NS.

    \tparam T All state variables of this type will be returned.
    \tparam NS Namespace to use.

    \return A std::vector of accessors to the state variables that
      match the type and namespace criteria.
   */

  template<typename T,
    size_t NS = static_cast<size_t>(state_name_space_t::user)>
  std::vector<accessor_t<T>> accessors() {
    return sp_t::template accessors<T,NS>();
  } // accessors

  /*!
    Return a std::vector of accessors to the stored states with
    type \e T in namespace \e NS satisfying the predicate function
    \e predicate.

    \tparam T All state variables of this type will be returned.
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

  template<typename T,
    typename P,
    size_t NS = static_cast<size_t>(state_name_space_t::user)>
  std::vector<accessor_t<T>> accessors(P && predicate) {
    return sp_t::template accessors<T,NS,P>(std::forward<P>(predicate));
  } // accessors

  /*!
    Return a std::vector of raw accessors to the stored states with
    in namespace \e NS.  Raw accessors are of type uint8_t.

    \tparam NS Namespace to use.

    \return A std::vector of raw accessors to the state variables that
      match the namespace criteria.
   */

  template<size_t NS = static_cast<size_t>(state_name_space_t::user)>
  std::vector<accessor_t<uint8_t>> raw_accessors() {
    return sp_t::template raw_accessors<NS>();
  } // raw_accessors

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

  template<typename P,
    size_t NS = static_cast<size_t>(state_name_space_t::user)>
  std::vector<accessor_t<uint8_t>> raw_accessors(P && predicate) {
    return sp_t::template raw_accessors<NS>(std::forward<P>(predicate));
  } // raw_accessors

  /*!
    Return the user meta data for the state variable identified by \e key
    in namespace \e NS.

    \param key The name of the state variable for which to return
      the meta data.

    \return The meta data corresponding to the key and namespace.
   */

  template<size_t NS = static_cast<size_t>(state_name_space_t::user)>
  user_meta_data_t & meta_data(const const_string_t & key) {
    return sp_t::template meta_data<NS>(key);
  } // user_meta_data

  /*!
    Return a copy of the state variable as an array of type \e T.

    \attention Calls to this function are expensive because they
      require that data (that may not be physically stored as an array)
      be replicated and repackaged to satisfy this request.

    \tparam T The type to be used to interpret the state variable.
    \tparam NS The namespace of the desired state variable.

    \param key The name of the state variable for which to return
      the data.

    \return A std::shared_ptr<T> with a copy of the state data.
   */

  template<typename T,
    size_t NS = static_cast<size_t>(state_name_space_t::user)>
  std::shared_ptr<T> & data(const const_string_t & key) {
    return sp_t::template data<T,NS>(key);
  } // data

  //! Copy constructor (disabled)
  state_t(const state_t &) = delete;

  //! Assignment operator (disabled)
  state_t & operator = (const state_t &) = delete;

}; // class state_t

} // namespace flexi

#endif // flexi_state_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
