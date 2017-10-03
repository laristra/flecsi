/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 03, 2017
//----------------------------------------------------------------------------//

#ifndef flecsi_topology_set_types_h
#define flecsi_topology_set_types_h

#include "flecsi/data/data_client.h"
#include "flecsi/topology/types.h"

namespace flecsi{
namespace topology{

class set_entity_t{
  using id_t = flecsi::utils::id_t;

  id_t global_id() const
  {
    return id_;
  }

  size_t id() const
  {
    return id_.entity();
  }

  void set_global_id(id_t id){
    id_ = id;
  }

private:
  id_t id_;
};

template<class STORAGE_TYPE>
class set_topology_base_t : public data::data_client_t
{
public:

  // Default constructor
  set_topology_base_t(STORAGE_TYPE * ss = nullptr)
    : ss_(ss) {}

  // Don't allow the set to be copied or copy constructed
  set_topology_base_t(const set_topology_base_t & s)
    : ss_(s.ss_) {}

  set_topology_base_t & operator=(const set_topology_base_t &) = delete;

  /// Allow move operations
  set_topology_base_t(set_topology_base_t &&) = default;

  //! override default move assignement
  set_topology_base_t & operator=(set_topology_base_t && o)
  {
    // call base_t move operator
    data::data_client_t::operator=(std::move(o));
    // return a reference to the object
    return *this;
  };

  STORAGE_TYPE *
  set_storage(
    STORAGE_TYPE * ss
  )
  {
    ss_ = ss;
    return ss_;
  } // set_storage

  STORAGE_TYPE *
  storage()
  {
    return ss_;
  } // set_storage

  void
  clear_storage()
  {
    ss_ = nullptr;
  } // clear_storage

  void
  delete_storage()
  {
    delete ss_;
  } // delete_storage

  /*!
    This method should be called to construct and entity rather than
    calling the constructor directly. This way, the ability to have
    extra initialization behavior is reserved.
  */
  template <class T, class... S>
  T * make(S &&... args)
  {
    return ss_->template make<T>(std::forward<S>(args)...);
  } // make

protected:
  STORAGE_TYPE * ss_ = nullptr;
};

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_set_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
