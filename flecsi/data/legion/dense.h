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

#ifndef flecsi_legion_dense_h
#define flecsi_legion_dense_h

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_type.h!!!
// Using this approach allows us to have only one storage_type__
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE legion
#include "flecsi/data/storage_type.h"
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include "flecsi/data/data_client.h"
#include "flecsi/data/data_handle.h"
#include "flecsi/data/storage.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/index_space.h"
#include "flecsi/execution/context.h"
#include "flecsi/data/common/privilege.h"

///
// \file legion/dense.h
// \authors bergen
// \date Initial file creation: Apr 7, 2016
///

namespace flecsi {
namespace data {
namespace legion {

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Helper type definitions.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Dense handle.
//----------------------------------------------------------------------------//

///
// \brief dense_handle_t provides logically array-based access to data
//        variables that have been registered in the data model.
//
// \tparam T The type of the data variable. If this type is not
//           consistent with the type used to register the data, bad things
//           can happen. However, it can be useful to reinterpret the type,
//           e.g., when writing raw bytes. This class is part of the
//           low-level \e flecsi interface, so it is assumed that you
//           know what you are doing...
///
template<
  typename T,
  size_t EP,
  size_t SP,
  size_t GP
>
struct dense_handle_t : public data_handle__<T, EP, SP, GP>
{
  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using base_t = data_handle__<T, EP, SP, GP>;

  //--------------------------------------------------------------------------//
  // Constructors.
  //--------------------------------------------------------------------------//

  dense_handle_t() {}

  //--------------------------------------------------------------------------//
  // Destructor.
  //--------------------------------------------------------------------------//

  ~dense_handle_t(){
    Legion::Runtime* runtime = base_t::runtime;
    Legion::Context& context = base_t::context;

    // Unmap physical regions and copy back out ex/sh/gh regions if we
    // have write permissions 

    if(base_t::exclusive_data){
      if(base_t::exclusive_priv > privilege_t::dro){
        std::memcpy(base_t::exclusive_buf, base_t::exclusive_data,
                    base_t::exclusive_size * sizeof(T));
      }

    }

    if(base_t::shared_data){
      if(base_t::shared_priv > privilege_t::dro){
        std::memcpy(base_t::shared_buf, base_t::shared_data,
                    base_t::shared_size * sizeof(T));
      }

    }

    // ghost is never mapped with write permissions

    if(base_t::master && base_t::combined_data){
#ifndef MAPPER_COMPACTION
      delete[] base_t::combined_data;
#endif
    }
  }
  
  ///
  // Copy constructor.
  ///
  template<size_t EP2, size_t SP2, size_t GP2>
  dense_handle_t(const dense_handle_t<T, EP2, SP2, GP2> & a)
    : base_t(reinterpret_cast<const base_t&>(a)),
      label_(a.label_)
    {
      static_assert(EP2 == 0 && SP2 == 0 && GP2 == 0,
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
    return base_t::combined_size;
  } // size

  ///
  // \brief Return the index space size of the data variable
  //        referenced by this handle.
  ///
  size_t
  exclusive_size() const
  {
    return base_t::exclusive_size;
  } // size

  ///
  // \brief Return the index space size of the data variable
  //        referenced by this handle.
  ///
  size_t
  shared_size() const
  {
    return base_t::shared_size;
  } // size

  ///
  // \brief Return the index space size of the data variable
  //        referenced by this handle.
  ///
  size_t
  ghost_size() const
  {
    return base_t::ghost_size;
  } // size

  //--------------------------------------------------------------------------//
  // Operators.
  //--------------------------------------------------------------------------//

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T &
  exclusive (
    size_t index
  ) const
  {
    assert(index < base_t::exclusive_size && "index out of range");
    return base_t::exclusive_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T &
  exclusive (
    size_t index
  )
  {
    assert(index < base_t::exclusive_size && "index out of range");
    return base_t::exclusive_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T &
  shared (
    size_t index
  ) const
  {
    assert(index < base_t::shared_size && "index out of range");
    return base_t::shared_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T &
  shared (
    size_t index
  )
  {
    assert(index < base_t::shared_size && "index out of range");
    return base_t::shared_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  const T &
  ghost (
    size_t index
  ) const
  {
    assert(index < base_t::ghost_size && "index out of range");
    return base_t::ghost_data[index];
  } // operator []

  ///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
  ///
  T &
  ghost (
    size_t index
  )
  {
    assert(index < base_t::ghost_size && "index out of range");
    return base_t::ghost_data[index];
  } // operator []

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
    assert(index < base_t::combined_size && "index out of range");
#ifndef MAPPER_COMPACTION
    return base_t::combined_data[index];
#else
    return *(base_t::combined_data+index);
#endif
  } // operator ()

  ///
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
#ifndef MAPPER_COMPACTION
    return base_t::combined_data[index];
#else
    return *(base_t::combined_data+index);
#endif
  } // operator ()

  ///
  // \brief Test to see if this handle is empty
  //
  // \return true if registered.
  ///
  operator bool() const
  {
    return base_t::combined_data != nullptr;
  } // operator bool

  template<typename, size_t, size_t, size_t>
  friend class dense_handle_t;

private:

  std::string label_ = "";

}; // struct dense_handle_t

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Dense storage type.
//----------------------------------------------------------------------------//

///
// FIXME: Dense storage type.
///
template<>
struct storage_type__<dense>
{
  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  template<
    typename T,
    size_t EP,
    size_t SP,
    size_t GP
  >
  using handle_t = dense_handle_t<T, EP, SP, GP>;

  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

  ///
  //
  ///
  template<
    typename T,
    size_t NAMESPACE,
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

  template<
    typename T,
    size_t NAMESPACE
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

  ///
  //
  ///
  template<
    typename DATA_CLIENT_TYPE,
    typename DATA_TYPE,
    size_t NAMESPACE,
    size_t NAME,
    size_t VERSION,
    size_t PERMISSIONS
  >
  static
  handle_t<DATA_TYPE, 0, 0, 0>
  get_handle(
    const data_client_handle__<DATA_CLIENT_TYPE, PERMISSIONS>& client_handle
  )
  {
    handle_t<DATA_TYPE, 0, 0, 0> h;

    auto& context = execution::context_t::instance();
    
    auto& field_info = 
      context.get_field_info(typeid(DATA_CLIENT_TYPE).hash_code(),
      NAMESPACE ^ NAME);

    size_t index_space = field_info.index_space;
    auto& ism = context.index_space_data_map();

    h.data_client_hash = field_info.data_client_hash;
    h.exclusive_lr = ism[index_space].exclusive_lr;
    h.shared_lr = ism[index_space].shared_lr;
    h.ghost_lr = ism[index_space].ghost_lr;
    h.pbarrier_as_owner_ptr =
      &ism[index_space].pbarriers_as_owner[field_info.fid];
    h.ghost_owners_pbarriers_ptrs.resize(0);
    for(size_t i=0;
      i < ism[index_space].ghost_owners_pbarriers[field_info.fid].size() ;
      i++)
        h.ghost_owners_pbarriers_ptrs.push_back(&(ism[index_space]
                            .ghost_owners_pbarriers[field_info.fid][i]));
    h.ghost_owners_lregions = ism[index_space].ghost_owners_lregions;
    h.color_region = ism[index_space].color_region;
    h.global_to_local_color_map_ptr =
      &ism[index_space].global_to_local_color_map;
    h.fid = field_info.fid;
    h.index_space = field_info.index_space;

    return h;
  } // get_handle

}; // struct storage_type__

} // namespace legion
} // namespace data
} // namespace flecsi

#endif // flecsi_legion_dense_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
