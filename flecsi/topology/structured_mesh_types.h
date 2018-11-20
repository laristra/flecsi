/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_structured_mesh_types_h
#define flecsi_topology_structured_mesh_types_h

#include <array>
#include <unordered_map>
#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>

#include "flecsi/coloring/box_types.h"
#include "flecsi/data/data_client.h"
#include "flecsi/topology/mesh_utils.h"
#include "flecsi/topology/structured_index_space.h"
#include "flecsi/topology/types.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation:
//----------------------------------------------------------------------------//

namespace flecsi {
namespace topology {  

//----------------------------------------------------------------------------//
//! The structured_mesh_entity_ type...
//!
//! @ingroup
//----------------------------------------------------------------------------//
//template<class>
//class structured_mesh_topology_base__;

class structured_mesh_entity_base_{
public:
 using sm_id_t = size_t;
};


template <size_t NUM_DOMAINS>
class structured_mesh_entity_base__ : public structured_mesh_entity_base_ 
{
 public:
  structured_mesh_entity_base__(){};

  virtual ~structured_mesh_entity_base__() {}
  
  template<size_t M>
  size_t id() const
  {
    return global_id_[M];
  }

  size_t id(size_t domain) const
  {
    return global_id_[domain];
  }

  template<size_t M>
  void set_id(const size_t &id)
  {
    global_id_[M] = id;
  }

  void set_id(const size_t &id, size_t domain)
  {
    global_id_[domain] = id;
  }


  template <class MESH_POLICY>
  friend class structured_mesh_topology__;

 private:
  std::array<sm_id_t,NUM_DOMAINS> global_id_;
}; // class structured_mesh_entity_base__


template <size_t DIM, size_t NUM_DOMAINS>
class structured_mesh_entity__ : public structured_mesh_entity_base__<
                                        NUM_DOMAINS>
{
 public:
  static const size_t dimension = DIM;

  structured_mesh_entity__() : structured_mesh_entity_base__<NUM_DOMAINS>(){};
  virtual ~structured_mesh_entity__() {}
}; // class structured_mesh_entity__

//----------------------------------------------------------------------------//
//! The structured_mesh_topology_base_t type...
//!
//! @ingroup
//----------------------------------------------------------------------------//
class structured_mesh_topology_base_t {};

template<class STORAGE_TYPE>
class structured_mesh_topology_base__ : public data::data_client_t,
                                        public structured_mesh_topology_base_t
{
  public:

  // Default constructor
  structured_mesh_topology_base__(STORAGE_TYPE * ms = nullptr) : ms_(ms) {}

  // Copy constructor
  structured_mesh_topology_base__(const structured_mesh_topology_base__ & m) : ms_(m.ms_){}

  //Don't allow copy assignment
  structured_mesh_topology_base__ & operator=(const structured_mesh_topology_base__ &) = delete;

  /// Allow move operations
  structured_mesh_topology_base__(structured_mesh_topology_base__ &&) = default;

  //! override default move assignement
  structured_mesh_topology_base__ & operator=(structured_mesh_topology_base__ && o)
  {
    // call base_t move operator
    data::data_client_t::operator=(std::move(o));
    // return a reference to the object
    return *this;
  };

  STORAGE_TYPE * set_storage(STORAGE_TYPE *ms)
  {
    ms_ = ms;
    return ms_; 
  } //set_storage

  STORAGE_TYPE * storage()
  {
    return ms_;
  } //storage

  void clear_storage()
  {
    ms_ = nullptr;
  } //clear_storage

  void delete_storage()
  {
    delete ms_;
  } //delete_storage


  /*!
    Return the number of entities in for a specific domain and topology dim.
   */
  virtual size_t num_entities(size_t dim, size_t domain) const = 0;

  protected:
  STORAGE_TYPE * ms_ = nullptr;
}; // structured_mesh_topology_base__

using box_t = flecsi::coloring::box_t; 
struct box_metadata
{
   box_metadata(box_t exclusive, std::vector<box_t> shared, 
                std::vector<box_t> ghost, std::vector<box_t> domain_halo,
                std::vector<box_t> overlay): exclusive_{exclusive}, 
                shared_{shared}, ghost_{ghost}, domain_halo_{domain_halo},
                overlay_{overlay} {}; 

   auto exclusive() 
   {
     return exclusive_;
   }   
   
   auto shared() 
   {
     return shared_;
   }   

   auto ghost() 
   {
     return ghost_;
   }
   
   auto domain_halo() 
   {
     return domain_halo_;
   }
  
   auto overlay() 
   {
     return overlay_;
   }  
 
private:
   box_t exclusive_;   
   std::vector<box_t> shared_;   
   std::vector<box_t> ghost_;
   std::vector<box_t> domain_halo_;
   std::vector<box_t> overlay_;

}; // struct box_metadata 

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_structured_mesh_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
