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

enum class state_name_space_t : size_t {
  user = 0,
  flexi = std::numeric_limits<size_t>::max()
}; // enum class state_name_space_t

enum class state_attribute : bitfield_t::field_type_t {
  persistent = 0
}; // enum class state_attribute

const bitfield_t::field_type_t persistent =
  static_cast<bitfield_t::field_type_t>(flexi::state_attribute::persistent);

/*----------------------------------------------------------------------------*
 * struct default_state_meta_data_t
 *----------------------------------------------------------------------------*/

struct default_state_user_meta_data_t {

  void initialize(const size_t & site_id_) {
    site_id = site_id_;
  } // initialize

  size_t site_id;
}; // struct default_state_user_meta_data_t

/*----------------------------------------------------------------------------*
 * class state_t
 *----------------------------------------------------------------------------*/

/*!
  \class state state.h
  \brief state provides...
 */

template<typename user_meta_data_t = default_state_user_meta_data_t,
  template<typename> typename storage_policy_t =
    default_state_storage_policy_t>
class state_t : public storage_policy_t<user_meta_data_t>
{
public:

  using sp_t = storage_policy_t<user_meta_data_t>;

  template<typename T>
  using accessor_t = typename sp_t::template accessor_t<T>;

  enum class attribute {
    persistent = 0
  }; // enum class attribute

  //! Default constructor
  state_t() : sp_t() {}

  //! Destructor
   ~state_t() {}

  template<typename T,
    size_t NS = static_cast<size_t>(state_name_space_t::user),
    typename ... Args>
  void register_state(const const_string_t & key, size_t indices,
    Args && ... args) {
    sp_t::template register_state<T,NS>(key, indices,
      std::forward<Args>(args) ...);
  } // register_state

  template<typename T,
    size_t NS = static_cast<size_t>(state_name_space_t::user)>
  accessor_t<T> accessor(const_string_t key) {
    return sp_t::template accessor<T,NS>(key);
  } // accessor

  template<size_t NS = static_cast<size_t>(state_name_space_t::user)>
  user_meta_data_t & meta_data(const_string_t key) {
    return sp_t::template meta_data<NS>(key);
  } // user_meta_data

  template<typename T,
    size_t NS = static_cast<size_t>(state_name_space_t::user)>
  std::shared_ptr<T> & data(const_string_t key) {
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
