//
// Created by ollie on 3/17/17.
//

#ifndef FLECSI_SIMPLE_ENTITY_TYPES_H
#define FLECSI_SIMPLE_ENTITY_TYPES_H

#include <flecsi/topology/mesh_types.h>
#include <flecsi/geometry/point.h>

struct simple_vertex_t : public flecsi::topology::mesh_entity_t<0, 1>
{
  using point_t = flecsi::point<double, 2>;

  template <typename ST>
  simple_vertex_t(flecsi::topology::mesh_topology_base_t<ST>& mesh,
                  const point_t& coords) : _coords(_coords) {}

  const point_t&
  coordinates() const
  {
    return _coords;
  }

  //flecsi::topology::mesh_topology_base_t& _mesh;
  point_t _coords;
};


struct simple_cell_t : public flecsi::topology::mesh_entity_t<2, 1>
{
  template <typename ST>
  simple_cell_t(flecsi::topology::mesh_topology_base_t<ST>& mesh, size_t _type) : type(_type){}

  std::vector<size_t>
  create_entities(
    id_t cell_id,
    size_t dim,
    flecsi::topology::domain_connectivity<2> & c,
    id_t * e
  )
  {
    // FIXME
    return {};
  } // create_entities

  size_t type;
};

#endif //FLECSI_SIMPLE_ENTITY_TYPES_H
