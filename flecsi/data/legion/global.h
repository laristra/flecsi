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

#ifndef flecsi_legion_global_h
#define flecsi_legion_global_h

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_type.h!!!
// Using this approach allows us to have only one storage_type__
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE legion
#include "flecsi/data/storage_type.h"
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include "flecsi/utils/const_string.h"
#include "flecsi/data/data_client.h"
#include "flecsi/data/data_handle.h"
#include "flecsi/data/storage.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/index_space.h"
#include "flecsi/execution/context.h"
#include "flecsi/data/common/privilege.h"

///
// \file legion/global.h
// \authors bergen
// \date Initial file creation: Apr 17, 2016
///

namespace flecsi {
namespace data {
namespace legion {

//----------------------------------------------------------------------------//
// Global handle.
//----------------------------------------------------------------------------//

///-------------------------------------------------------------------------//
//! global_handle_t provide an access to global variables that have
//! been registered in data model
//!
//! \tparam T The type of the data variable. If this type is not
//!           consistent with the type used to register the data, bad things
//!           can happen. However, it can be useful to reinterpret the type,
//!           e.g., when writing raw bytes. This class is part of the
//!           low-level \e flecsi interface, so it is assumed that you
//!           know what you are doing...
///--------------------------------------------------------------------------//

template<
  typename T,
  size_t P
>
struct global_handle_t : public data_handle__<T, P, 0, 0> {

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using base_t = data_handle__<T, P, 0, 0>;

  //--------------------------------------------------------------------------//
  // Constructors.
  //--------------------------------------------------------------------------//

   global_handle_t()
   {
    base_t::global=true;
   }

  //--------------------------------------------------------------------------//
  // Destructor.
  //--------------------------------------------------------------------------//

  ~global_handle_t(){}


  ///
  // Copy constructor.
  ///
  template<size_t P2>
  global_handle_t(const global_handle_t<T, P2> & a)
    : base_t(reinterpret_cast<const base_t&>(a)),
      label_(a.label()),
      size_(a.size())
    {
      static_assert(P2 == 0, 
        "passing mapped handle to task args");
    }


  //--------------------------------------------------------------------------//
  // Member data interface.
  //--------------------------------------------------------------------------//

  ///
  // \brief Return a std::string containing the label of the data variable
  //        reference by this handle.
  ///
  const std::string &
  label() const
  { 
    return label_;
  } // label

  ///
  // \brief Return the index space size of the data variable
  //        referenced by this handle.
  ///
  size_t
  size() const
  { 
    base_t::combined_size;
  } // size
 
  //--------------------------------------------------------------------------//
  // Operators.
  //--------------------------------------------------------------------------//

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \tparam E A complex index type.
  //
  // This version of the operator is provided to support use with
  // \e flecsi mesh entity types \ref mesh_entity_base_t.
  ///
  template<typename E>
  const T &
  operator () (
    E * e
  ) const
  {
    return this->operator()(e->template id<0>());
  } // operator ()

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \tparam E A complex index type.
  //
  // This version of the operator is provided to support use with
  // \e flecsi mesh entity types \ref mesh_entity_base_t.
  ///
  template<typename E>
  T &
  operator () (
    E * e
  )
  {
    return this->operator()(e->template id<0>());
  } // operator ()

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T &
  operator () (
    size_t index
  ) const
  {
    assert(index <  base_t::combined_size && "index out of range");
 //   return *(data_+index);
    return *(base_t::combined_data+index);
  } // operator ()


  //
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T &
  operator () (
    size_t index
  )
  {
    assert(index < base_t::combined_size && "index out of range");
    return *(base_t::combined_data+index);
  } // operator ()

  ///
  // \brief Test to see if this handle is empty
  //
  // \return true if registered.
  ///
  operator bool() const
  {
   return base_t::combined_data !=nullptr;
   // return data_ != nullptr;
  } // operator bool

  private:
    std::string label_ = "";
    size_t size_=0;
//    T* data_ = nullptr;
//     T* buffer = nullptr;
}; // struct global_handle_t

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Global storage type.
//----------------------------------------------------------------------------//

///
// FIXME: Global storage type.
///
template<>
struct storage_type__<global> {

  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  template<
    typename T,
    size_t P
  >
  using handle_t = global_handle_t<T, P>;

  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

  ///
  //
  ///
  template<
    typename T,
    size_t NS
  >
  static
  decltype(auto)
  get_handle(
    const data_client_t & data_client,
    const utils::const_string_t & key
  )
  {
    return {};
  } // get_handle


  ///
  /// FIXME documentation
  ///
  template<
    typename T,
    size_t NS,
    typename Predicate
  >
  static
  decltype(auto)
  get_handles(
    const data_client_t & data_client,
    size_t version,
    Predicate && predicate,
    bool sorted
  )
  {

  }

  ///
  /// FIXME documentation
  ///
  template<
    typename T,
    typename Predicate
  >
  static
  decltype(auto)
  get_handles(
    const data_client_t & data_client,
    size_t version,
    Predicate && predicate,
    bool sorted
  )
  {

  }

  ///
  /// FIXME documentation
  ///
  template<
    typename T,
    size_t NS
  >
  static
  decltype(auto)
  get_handles(
    const data_client_t & data_client,
    size_t version,
    bool sorted
  )
  {

  }

  ///
  /// FIXME documentation
  ///
  template<
    typename T
  >
  static
  decltype(auto)
  get_handles(
    const data_client_t & data_client,
    size_t version,
    bool sorted
  )
  {

  }

  template<
    typename DATA_CLIENT_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION
  >
  static
  handle_t<DATA_TYPE, 0>
  get_handle(
    const data_client_t & data_client
  )
  {
    handle_t<DATA_TYPE, 0> h;
    auto& context = execution::context_t::instance();

    auto& field_info =
      context.get_field_info(typeid(DATA_CLIENT_TYPE).hash_code(),
      NAMESPACE ^ NAME);

    size_t index_space = field_info.index_space;
    auto& ism = context.index_space_data_map();
    h.data_client_hash = field_info.data_client_hash;
    h.color_region = ism[index_space].color_region;
    h.fid = field_info.fid;
    h.index_space = field_info.index_space;
    h.global = true;

    return h;
  }


}; // struct storage_type__

} // namespace legion
} // namespace data
} // namespace flecsi

#endif // flecsi_legion_global_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
