/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_dmp_index_partition_h
#define flecsi_dmp_index_partition_h

#include <vector>
#include <cassert>
#include <cereal/types/vector.hpp>

///
// \file index_partition.h
// \authors bergen
// \date Initial file creation: Aug 17, 2016
///

namespace flecsi {
namespace dmp {

///
// \class partition__ index_partition.h
// \brief partition__ provides...
///
template<typename T>
struct index_partition__
{
  using identifier_t = T;

  struct ghost_info_t
  {
    identifier_t mesh_id;
    identifier_t global_id;
    identifier_t rank;   
 
    bool 
    operator==(
      const ghost_info_t& a
    ) 
    const
    {
      return (rank == a.rank &&
              mesh_id == a.mesh_id &&
              global_id == a.global_id);
    }
    
    template<typename A>
    void serialize(A & archive) {
       archive(rank, mesh_id, global_id);
    } // serialize

  }; // struct ghost_info_t

  struct shared_info_t
  {
    identifier_t mesh_id;
    identifier_t global_id;
    std::vector<identifier_t> dependent_ranks;

    bool 
    operator==(
      const shared_info_t& a
    ) 
    const
    {
      return (dependent_ranks == a.dependent_ranks && 
              mesh_id == a.mesh_id &&
              global_id == a.global_id);
    }

    template<typename A>
    void serialize(A & archive) {
       archive(mesh_id, global_id, dependent_ranks);
    } 

  }; // struct shared_info

  //--------------------------------------------------------------------------//
  // Data members.
  //--------------------------------------------------------------------------//

  // Vector of mesh ids of the primary partition
  std::vector<identifier_t> primary;

  // Vector of mesh ids of the exclusive indices
  std::vector<identifier_t> exclusive;

  std::vector<shared_info_t> shared;

  std::vector<ghost_info_t> ghost;

  ///
  // Equality operator.
  //
  // \param ip The index_partition_t to compare with \e this.
  //
  // \return True if \e ip is equivalent to \e this, false otherwise.
  ///
  bool
  operator == (
    const index_partition__ & ip
  ) const
  {
    return (this->exclusive == ip.exclusive &&
      this->shared ==  ip.shared &&
      this->ghost == ip.ghost);
  } // operator ==

  ///
  // Cereal serialization method.
  ///
  template<typename A>
  void serialize(A & archive) {
    archive(exclusive, shared, ghost);
  } // serialize

  template <typename Indx_t>
  identifier_t 
  shared_id(
    Indx_t &indx
  )
  {
    assert (indx<=shared.size());
    return shared[indx].mesh_id;
  } 

  template <typename Indx_t>
  identifier_t 
  ghost_id(
    Indx_t &indx 
  )
  {
    assert (indx<=shared.size());
    return ghost[indx].mesh_id;
  }


}; // class partition__

} // namespace dmp
} // namespace flecsi

#endif // flecsi_dmp_index_partition_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
