#pragma once

#include <flecsi-config.h>

#include <flecsi/execution/context.h>
#include <flecsi/topology/mesh_definition.h>

namespace flecsi {
namespace execution {

template<class DEPENDENT_PARTITION_POLICY>
struct dependent_partition__ : public DEPENDENT_PARTITION_POLICY {
  using space_t = typename DEPENDENT_PARTITION_POLICY::space_t;
  using map_t = typename DEPENDENT_PARTITION_POLICY::map_t;
  using set_t = typename DEPENDENT_PARTITION_POLICY::set_t;
  
  space_t load_entity(int entities_size, int entity_id, int entity_map_id, std::vector<int> &entity_vector, flecsi::topology::mesh_definition_base__ &md)
  {
    return DEPENDENT_PARTITION_POLICY::load_entity(entities_size, entity_id, entity_map_id, entity_vector, md);
  }
  
  map_t load_cell_to_entity(space_t &cell_region, space_t &entity_region, flecsi::topology::mesh_definition_base__ &md)
  {
    return DEPENDENT_PARTITION_POLICY::load_cell_to_entity(cell_region, entity_region, md);
  }
  
  set_t partition_by_color(space_t &entity)
  {
    return DEPENDENT_PARTITION_POLICY::partition_by_color(entity);
  }
  
  set_t partition_by_image(space_t &from_entity, space_t &to_entity, map_t &adjacency, set_t &from)
  {
    return DEPENDENT_PARTITION_POLICY::partition_by_image(from_entity, to_entity, adjacency, from);
  }
  
  set_t partition_by_difference(space_t &entity, set_t &par1, set_t &par2)
  {
    return DEPENDENT_PARTITION_POLICY::partition_by_difference(entity, par1, par2);
  }
  
  set_t partition_by_intersection(space_t &entity, set_t &par1, set_t &par2)
  {
    return DEPENDENT_PARTITION_POLICY::partition_by_intersection(entity, par1, par2);
  }
  
  void output_partition(space_t &entity, set_t &primary, set_t &ghost, set_t &shared, set_t &exclusive)
  {
     DEPENDENT_PARTITION_POLICY::output_partition(entity, primary, ghost, shared, exclusive);
  }
  
  void print_partition(space_t &entity, set_t &primary, set_t &ghost, set_t &shared, set_t &exclusive, int print_flag)
  {
     DEPENDENT_PARTITION_POLICY::print_partition(entity, primary, ghost, shared, exclusive, print_flag);
  }
  
  void min_reduction_by_color(space_t &entity, set_t &alias_partition)
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
using space_t = dependent_partition__<FLECSI_DEPENDENT_PARTITION_POLICY>::space_t;
using map_t = dependent_partition__<FLECSI_DEPENDENT_PARTITION_POLICY>::map_t;
using set_t = dependent_partition__<FLECSI_DEPENDENT_PARTITION_POLICY>::set_t;

} // namespace execution
} // namespace flecsi