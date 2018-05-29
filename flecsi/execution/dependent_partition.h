#pragma once

#include <flecsi-config.h>

#include <flecsi/execution/context.h>
#include <flecsi/topology/mesh_definition.h>

namespace flecsi {
namespace execution {

template<class DEPENDENT_PARTITION_POLICY>
struct dependent_partition__ : public DEPENDENT_PARTITION_POLICY {
  using entity_t = typename DEPENDENT_PARTITION_POLICY::entity_t;
  using adjacency_t = typename DEPENDENT_PARTITION_POLICY::adjacency_t;
  using partition_t = typename DEPENDENT_PARTITION_POLICY::partition_t;
  
  entity_t load_entity(int entities_size, int entity_id, int entity_map_id, std::vector<int> &entity_vector, flecsi::topology::mesh_definition_base__ &md)
  {
    return DEPENDENT_PARTITION_POLICY::load_entity(entities_size, entity_id, entity_map_id, entity_vector, md);
  }
  
  adjacency_t load_cell_to_entity(entity_t &cell_region, entity_t &entity_region, flecsi::topology::mesh_definition_base__ &md)
  {
    return DEPENDENT_PARTITION_POLICY::load_cell_to_entity(cell_region, entity_region, md);
  }
  
  partition_t partition_by_color(entity_t &entity)
  {
    return DEPENDENT_PARTITION_POLICY::partition_by_color(entity);
  }
  
  partition_t partition_by_image(entity_t &from_entity, entity_t &to_entity, adjacency_t &adjacency, partition_t &from)
  {
    return DEPENDENT_PARTITION_POLICY::partition_by_image(from_entity, to_entity, adjacency, from);
  }
  
  partition_t partition_by_difference(entity_t &entity, partition_t &par1, partition_t &par2)
  {
    return DEPENDENT_PARTITION_POLICY::partition_by_difference(entity, par1, par2);
  }
  
  partition_t partition_by_intersection(entity_t &entity, partition_t &par1, partition_t &par2)
  {
    return DEPENDENT_PARTITION_POLICY::partition_by_intersection(entity, par1, par2);
  }
  
  void output_partition(entity_t &entity, partition_t &primary, partition_t &ghost, partition_t &shared, partition_t &exclusive)
  {
     DEPENDENT_PARTITION_POLICY::output_partition(entity, primary, ghost, shared, exclusive);
  }
  
  void min_reduction_by_color(entity_t &entity, partition_t &alias_partition)
  {
    DEPENDENT_PARTITION_POLICY::min_reduction_by_color(entity, alias_partition);
  }
};

} // namespace execution
} // namespace flecsi

// This include file defines the FLECSI_DEPENDENT_PARTITION_POLICY used below.

#include <flecsi/execution/dependent_partition_policy.h>

namespace flecsi {
namespace execution {

/*!
  The dependent_partition_t type is the high-level interface to the FleCSI
  dependent partition.
  @ingroup execution
 */

using dependent_partition_t = dependent_partition__<FLECSI_DEPENDENT_PARTITION_POLICY>;
using entity_t = dependent_partition__<FLECSI_DEPENDENT_PARTITION_POLICY>::entity_t;
using adjacency_t = dependent_partition__<FLECSI_DEPENDENT_PARTITION_POLICY>::adjacency_t;
using partition_t = dependent_partition__<FLECSI_DEPENDENT_PARTITION_POLICY>::partition_t;

} // namespace execution
} // namespace flecsi