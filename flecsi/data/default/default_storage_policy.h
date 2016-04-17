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

#ifndef flecsi_default_storage_policy_h
#define flecsi_default_storage_policy_h

#include <unordered_map>
#include <vector>
#include <memory>
#include <typeinfo>
#include <cassert>

#include "flecsi/data/data_constants.h"

#include "flecsi/data/default/default_meta_data.h"

// Include partial specializations
#include "flecsi/data/default/default_dense.h"
#include "flecsi/data/default/default_scalar.h"

/*!
 * \file default_storage_policy.h
 * \authors bergen
 * \date Initial file creation: Oct 27, 2015
 */

namespace flecsi
{
namespace data_model
{

template<typename user_meta_data_t>
struct default_storage_policy_t {

  using meta_data_t = default_meta_data_t<user_meta_data_t>;

  using data_store_t = std::unordered_map<size_t,
    std::unordered_map<const_string_t::hash_type_t, meta_data_t>>;

  // Define the storage type
  template<size_t data_type_t>
  using storage_type_t =
    default_storage_policy::storage_type_t<data_type_t, data_store_t>;

protected:

  // Storage container instance
  data_store_t data_store_;

}; // struct default_storage_policy_t

#if 0
/*----------------------------------------------------------------------------*
 * class default_data_storage_policy_t
 *----------------------------------------------------------------------------*/

/*!
  \class default_data_storage_policy_t default_storage_policy.h
  \brief default_data_storage_policy_t provides a serial/local storage
    policy for the \e flecsi data model.

  This storage policy is probably adequate for serial or MPI-based
  runtimes.  This implementation should not be used as a template
  for creating new storage policies.  If you are developing a new
  policy, look at the interface in \ref data.h.  This is the interface
  that needs to be implemented by new storage policies.
 */
template <typename user_meta_data_t>
class default_data_storage_policy_t
{
 protected:

  //! Constructor
  default_data_storage_policy_t() {}

  //! Desctructor
  virtual ~default_data_storage_policy_t() {}

  //! \brief delete ALL data
  void reset() {
    meta_.clear();
  } // reset

  /*! 
   * \brief delete ALL data associated with this runtime namespace
   * \param [in] runtime_namespace the namespace to search
   */
  void reset( uintptr_t runtime_namespace ) {

    // check each namespace
    for ( auto & sub_map : meta_ ) {

      // the namespace data
      auto & namespace_key = sub_map.first;
      auto & meta_data = sub_map.second;
      
      // loop over each element in the namespace
      auto itr = meta_data.begin();
      while ( itr != meta_data.end() ) {
        // get the meta data key and label
        auto & meta_data_key = itr->first;
        auto & label = itr->second.label;
        // now build the hash for this label
        auto key_hash = hash<const_string_t::hash_type_t>( label, label.size() );
        auto hash = key_hash ^ runtime_namespace;
        // test if it should be deleted
        if ( meta_data_key == hash )
          itr = meta_data.erase(itr);
        else 
          ++itr;
      } // while
    } // for

  } // reset

  /*! 
   * \brief delete specific data associated with this runtime namespace
   * \param [in] key the key to delete
   * \param [in] runtime_namespace the namespace to search
   */
  void release( const const_string_t & key, uintptr_t runtime_namespace ) {

    auto hash = key.hash() ^ runtime_namespace;

    // check each namespace
    for ( auto & sub_map : meta_ ) {

      // the namespace data
      auto & namespace_key = sub_map.first;
      auto & meta_data = sub_map.second;

      // erase the data
      auto it = meta_data.erase( hash );

    } // for

  } // reset

  /*--------------------------------------------------------------------------*
   * struct meta_data_t
   *--------------------------------------------------------------------------*/

  /*!
    \brief meta_data_t provides storage for extra information that is
      used to interpret data variable information at different points
      in the low-level runtime.
   */
  struct meta_data_t {
    std::string label;
    user_meta_data_t user_data;
    size_t size;
    size_t type_size;

    /*!
      \brief type_info_t allows creation of reference information
        to the user-specified type of the data data.

      The std::type_info type requires dynamic initialization.  The
        type_info_t type is designed to allow construction without
        needing a non-trivial default constructor for the
        meta_data_t type.
     */
    struct type_info_t {
      type_info_t(const std::type_info & type_info_) : type_info(type_info_) {}
      const std::type_info & type_info;
    }; // struct type_info_t

    std::shared_ptr<type_info_t> rtti;

    std::vector<uint8_t> data;
  }; // struct meta_data_t

private:

  std::unordered_map<size_t,
    std::unordered_map<const_string_t::hash_type_t, meta_data_t>> meta_;

}; // class default_data_storage_policy_t
#endif

} // namespace data_model
} // namespace flecsi

#endif // flecsi_default_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
