/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
/// \file
/// \date Initial file creation: June 26, 2017
///

#include <cinchlog.h>
#include <cinchtest.h>
#include <math.h>

#include "flecsi/data/data.h"
#include "flecsi/execution/execution.h"
#include "flecsi/io/simple_definition.h"
#include "flecsi/supplemental/coloring/add_colorings.h"
#include "flecsi/topology/closure_utils.h"

// FIXME: Must match add_colorings.cc
#define M 16
#define N 16

#define INDEX_ID 0
#define VERSIONS 1

#define DIFFUSIVITY 0.25
#define DH (8.0 / M)
#define DT (DH * DH * 0.25 / DIFFUSIVITY)
#define LENGTH (8.0 + DH)
#define EPS 1.0e-16
#define L_INF_ERROR 210.0

using namespace flecsi;
using namespace topology;

clog_register_tag(cell_to_cell_connectivity);

template<typename T, size_t EP, size_t SP, size_t GP>
using handle_t = flecsi::data::legion::dense_handle_t<T, EP, SP, GP>;

void check_values_task(
    handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dno> cell_ID,
    handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> T,
    const double time);
flecsi_register_task(check_values_task, flecsi::loc, flecsi::single);

void init_values_task(
    handle_t<size_t, flecsi::drw, flecsi::drw, flecsi::dno> cell_ID,
    handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> T);
flecsi_register_task(init_values_task, flecsi::loc, flecsi::single);

void fwd_euler_heat_task(
    handle_t<double, flecsi::drw, flecsi::drw, flecsi::dro> T,
    handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> T_temp);
flecsi_register_task(fwd_euler_heat_task, flecsi::loc, flecsi::single);

class client_type : public flecsi::data::data_client_t{};

flecsi_register_field(client_type, name_space, cell_ID, size_t, dense,
    INDEX_ID, VERSIONS);
flecsi_register_field(client_type, name_space, T, double, dense,
    INDEX_ID, VERSIONS);
flecsi_register_field(client_type, name_space, T_temp, double, dense,
    INDEX_ID, VERSIONS);

class vertex : public mesh_entity_t<0, 1>{
public:
  template<size_t M2>
  uint64_t precedence() const { return 0; }
  vertex() = default;
  
  template<typename ST>
  vertex(mesh_topology_base_t<ST> &) {}

};

class edge : public mesh_entity_t<1, 1>{
public:
  template<typename ST>
  edge(mesh_topology_base_t<ST> &) {}
};

class face : public mesh_entity_t<1, 1>{
public:
  template<typename ST>
  face(mesh_topology_base_t<ST> &) {}

};

class cell : public mesh_entity_t<2, 1>{
public:

  using id_t = utils::id_t;

  cell(){}

  void set_precedence(size_t dim, uint64_t precedence) {}

  std::vector<size_t>
  create_entities(id_t cell_id, size_t dim, domain_connectivity<2> & c, id_t * e){
    id_t* v = c.get_entities(cell_id, 0);

    e[0] = v[0];
    e[1] = v[2];
    
    e[2] = v[1];
    e[3] = v[3];
    
    e[4] = v[0];
    e[5] = v[1];
    
    e[6] = v[2];
    e[7] = v[3];

    return {2, 2, 2, 2};
  }

};

class mesh_types_t{
public:
  static constexpr size_t num_dimensions = 2;

  static constexpr size_t num_domains = 1;

  using entity_types = std::tuple<
    std::tuple<index_space_<0>, domain_<0>, vertex>,
    std::tuple<index_space_<1>, domain_<0>, cell>>;

  using connectivities = 
    std::tuple<std::tuple<index_space_<2>, domain_<0>, cell, cell>>;

  using bindings = std::tuple<>;

  template<size_t M2, size_t D2, typename ST>
  static mesh_entity_base_t<num_domains>*
  create_entity(mesh_topology_base_t<ST>* mesh, size_t num_vertices){
    switch(M){
      case 0:{
        switch(D2){
          default:
            assert(false && "invalid topological dimension");
        }
        break;
      }
      default:
        assert(false && "invalid domain");
    }
  }
};

using mesh_t = mesh_topology_t<mesh_types_t>;


namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Specialization driver.
//----------------------------------------------------------------------------//

void specialization_tlt_init(int argc, char ** argv) {
  flecsi_execute_mpi_task(add_colorings, 0);
} // specialization_tlt_init

void specialization_spmd_init(int argc, char ** argv) {
  auto runtime = Legion::Runtime::get_runtime();
  const int my_color = runtime->find_local_MPI_rank();
  clog(error) << "Rank " << my_color << " in specialization_spmd_init" << std::endl;

} // specialization_spmd_init

//----------------------------------------------------------------------------//
// User driver.
//----------------------------------------------------------------------------//

void fill_connectivity_buffers(
    const std::set<flecsi::coloring::entity_info_t>& entities,
    int my_color,
    const std::map<size_t,size_t>& gid_to_lid_map,
    LegionRuntime::Arrays::Point<2>* positions,
    uint64_t* indices,
    size_t* index)
{
  flecsi::io::simple_definition_t sd("simple2d-16x16.msh");

  for (auto entity_itr = entities.begin(); entity_itr != entities.end(); ++entity_itr) {
    flecsi::coloring::entity_info_t entity = *entity_itr;
    std::set<size_t> this_cell;
    this_cell.insert(entity.id);

    auto nearby_cells = flecsi::topology::entity_closure<2,2,0>(sd, this_cell);
    nearby_cells.erase(entity.id);
    auto these_verts = flecsi::topology::vertex_closure<2>(sd, this_cell);
    for(auto neighbor : nearby_cells) {
      std::set<size_t> this_neighbor;
      this_neighbor.insert(neighbor);
      auto neighbor_verts = flecsi::topology::vertex_closure<2>(sd, this_neighbor);
      size_t shared_vertices_count = 0;
      for(auto vertex: neighbor_verts) {
        if(these_verts.find(vertex) != these_verts.end()) {
          shared_vertices_count++;
          clog(trace) << " cell " << entity.id << " shares vertex " <<
            vertex << " with " << neighbor << " : " << shared_vertices_count << std::endl;
        }
      } // for vertex
      if(2==shared_vertices_count) {
        clog(trace)  << my_color << " cell " << gid_to_lid_map.at(entity.id) <<
          " nbr " << gid_to_lid_map.at(neighbor) << std::endl;
        indices[*index] = gid_to_lid_map.at(neighbor);
        *index = *index + 1;
      }
    } // for neighbor
    LegionRuntime::Arrays::Point<2> this_position;
    if(0==gid_to_lid_map.at(entity.id))
      this_position.x[0] = 0; // offset
    else
      this_position.x[0] = positions[gid_to_lid_map.at(entity.id)-1].x[0] +
      positions[gid_to_lid_map.at(entity.id)-1].x[1];
    this_position.x[1] = *index - this_position.x[0];  // range
    positions[gid_to_lid_map.at(entity.id)] = this_position;
  } // for entity_itr
}

void driver(int argc, char ** argv) {
  auto runtime = Legion::Runtime::get_runtime();
  const int my_color = runtime->find_local_MPI_rank();
  clog(error) << "Rank " << my_color << " in driver" << std::endl;

  // FIXME: Following code should move to spmd_init once control point works
  auto ctx = runtime->get_context();
  mesh_t mesh;
  context_t & context_ = context_t::instance();
  auto& ispace_dmap = context_.index_space_data_map();
  Legion::Domain exclusive_domain = runtime->get_index_space_domain(ctx,
      ispace_dmap[INDEX_ID].exclusive_lr.get_index_space());
  Legion::Domain shared_domain = runtime->get_index_space_domain(ctx,
      ispace_dmap[INDEX_ID].shared_lr.get_index_space());
  LegionRuntime::Arrays::Rect<2> exclusive_rect = exclusive_domain.get_rect<2>();
  LegionRuntime::Arrays::Rect<2> shared_rect = shared_domain.get_rect<2>();
  size_t num_cells = exclusive_rect.hi.x[1] - exclusive_rect.lo.x[1] + 1
      + shared_rect.hi.x[1] - shared_rect.lo.x[1] + 1;
  clog(trace) << my_color << " exclusive " << exclusive_rect.hi.x[1] - exclusive_rect.lo.x[1] + 1 << std::endl;
  clog(trace) << my_color << " shared " << shared_rect.hi.x[1] - shared_rect.lo.x[1] + 1 << std::endl;

  LegionRuntime::Arrays::Point<2>* positions = (LegionRuntime::Arrays::Point<2>*)
      malloc(sizeof(LegionRuntime::Arrays::Point<2>)*num_cells);  // FIXME leak

  uint64_t* indices = (uint64_t*)malloc(sizeof(uint64_t)*num_cells*8); // FIXME leak, 8 is hack

  flecsi::io::simple_definition_t simple_def("simple2d-16x16.msh");

  const std::map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();
  auto index_coloring = coloring_map.find(INDEX_ID);

  std::map<size_t,size_t> gid_to_lid_map;
  auto entries = index_coloring->second.exclusive;

  size_t lid = 0;
  for (auto entity_itr = entries.begin(); entity_itr != entries.end(); ++entity_itr) {
    flecsi::coloring::entity_info_t entity = *entity_itr;
    gid_to_lid_map[entity.id] = lid;
    lid++;
  }

  entries = index_coloring->second.shared;
  for (auto entity_itr = entries.begin(); entity_itr != entries.end(); ++entity_itr) {
    flecsi::coloring::entity_info_t entity = *entity_itr;
    gid_to_lid_map[entity.id] = lid;
    lid++;
  }

  entries = index_coloring->second.ghost;
  for (auto entity_itr = entries.begin(); entity_itr != entries.end(); ++entity_itr) {
    flecsi::coloring::entity_info_t entity = *entity_itr;
    gid_to_lid_map[entity.id] = lid;
    lid++;
  }

  size_t idx = 0;
  fill_connectivity_buffers(index_coloring->second.exclusive, my_color,
      gid_to_lid_map, positions, indices, &idx);
  fill_connectivity_buffers(index_coloring->second.shared, my_color,
      gid_to_lid_map, positions, indices, &idx);

  for(size_t cell=0; cell<num_cells; cell++) {
    size_t offset = positions[cell].x[0];
    size_t range = positions[cell].x[1];
    clog(trace) << "Rank " << my_color << " cell " << cell << " offset " <<
        offset << " range " << range << std::endl;
    for(size_t idx=offset; idx < (offset+range); idx++)
      clog(trace) << "Rank " << my_color << " cell " << cell << " nbr " <<
          indices[idx] << std::endl;

  }

  // setup connectivity

  auto mesh_storage_policy = mesh.storage();
  mesh_entity_base_* entities;
  size_t dimension = 2;
  entities = (mesh_entity_base_*)malloc(sizeof(mesh_entity_base_)*num_cells); // FIXME leak
  mesh_storage_policy->init_entities(INDEX_ID,dimension,entities,num_cells);
  cell* c = mesh.make<cell>();
  mesh_storage_policy->init_connectivity(0,0,2,2, positions, indices, num_cells);

  // start what really is driver, not broken spmd_init
  client_type client;

  auto cell_IDs = flecsi_get_handle(client, name_space, cell_ID, size_t, dense,
      INDEX_ID);
  auto Ts = flecsi_get_handle(client, name_space, T, double, dense,
      INDEX_ID);
  auto temp_Ts = flecsi_get_handle(client, name_space, T_temp, double, dense,
      INDEX_ID);

  flecsi_execute_task(init_values_task, single, cell_IDs, Ts);

  double time = 0.0;
  while(time < 4.0) {
    flecsi_execute_task(fwd_euler_heat_task, single, Ts, temp_Ts);
    time += DT;
  }

  flecsi_execute_task(check_values_task, single, cell_IDs, Ts, time);
} // driver

} // namespace execution
} // namespace flecsi

double analytic_heat_solution(size_t cell_ID, double time);

double initial_temperature(size_t cell_ID);

void init_values_task(
        handle_t<size_t, flecsi::drw, flecsi::drw, flecsi::dno> cell_ID,
        handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> T) {
  clog(trace) << "init_values_task" << std::endl;

  flecsi::execution::context_t & context_
    = flecsi::execution::context_t::instance();
  const std::map<size_t, flecsi::coloring::index_coloring_t> coloring_map
    = context_.coloring_map();
  auto index_coloring = coloring_map.find(INDEX_ID);

  size_t index = 0;
  for (auto exclusive_itr = index_coloring->second.exclusive.begin();
    exclusive_itr != index_coloring->second.exclusive.end(); ++exclusive_itr) {
    flecsi::coloring::entity_info_t exclusive = *exclusive_itr;
    cell_ID(index) = exclusive.id;
    T(index) = initial_temperature(cell_ID(index));
    clog(trace) << cell_ID(index) << " init'd as " << T(index) << std::endl;
    index++;
  } // exclusive_itr

  for (auto shared_itr = index_coloring->second.shared.begin(); shared_itr !=
      index_coloring->second.shared.end(); ++shared_itr) {
    flecsi::coloring::entity_info_t shared = *shared_itr;
    cell_ID(index) = shared.id;
    T(index) = initial_temperature(cell_ID(index));
    clog(trace) << cell_ID(index) << " init'd as " << T(index) << std::endl;
    index++;
  } // shared_itr
} // init_values_task

double x_coord(size_t cell_ID) {
  return (cell_ID % M + 1.0) * DH;
}

double y_coord(size_t cell_ID) {
  return (cell_ID / M + 1.0) * DH;
}

void check_values_task(
        handle_t<size_t, flecsi::dro, flecsi::dro, flecsi::dno> cell_ID,
        handle_t<double, flecsi::dro, flecsi::dro, flecsi::dno> T,
        const double time) {
  clog(trace) << "check_values_task, t = " << time << std::endl;

  for(size_t i=0; i<(cell_ID.exclusive_size()+cell_ID.shared_size()); i++) {
    double expected = analytic_heat_solution(cell_ID(i), time);
    double error = fabs(expected - T(i));
    if(error > (L_INF_ERROR/(M*M)))
        clog(error) << cell_ID(i) << " actual " << T(i) << " expected " <<
            expected << " error " << error << " > " << (L_INF_ERROR/(M*M)) <<
            std::endl;
    assert(error <= (L_INF_ERROR/(M*M)));
  }  // for i
} // check_values_task

void fwd_euler_heat_task(
    handle_t<double, flecsi::drw, flecsi::drw, flecsi::dro> T,
    handle_t<double, flecsi::drw, flecsi::drw, flecsi::dno> T_temp) {
  clog(trace) << "fwd_euler_heat_task" << std::endl;
  // T_temp[i,j] = -4.0 T[i,j]
  // T_temp[i,j] += T[i+/-1,j+/-j] : no diagonals
  // T_temp[i,j] *= DIFFUSIVITY * DT / (DH*DH)
  // T[i,j] += T_temp[i,j]
}

double approximation_order(double time) {
  double error = EPS / (M * M);
  double order = 1025.0;
  if (time > 0.0)
    order = sqrt(-log(error) * LENGTH * LENGTH / (2.0 * DIFFUSIVITY * time
        * M_PI * M_PI));
  return order;
}

double analytic_heat_solution(size_t cell_ID, double time) {
  double approx_order = approximation_order(time);
  double analytic = 0.0;
  double base_spatial_coef = 4.0 * LENGTH * LENGTH / (M_PI * M_PI);
  double const_t_factor = -DIFFUSIVITY * time * M_PI * M_PI / (LENGTH * LENGTH);
  for(double m = 1.0; m < approx_order; m++)
    for(double n = 1.0; n < approx_order; n++) {
      double coef = base_spatial_coef / (m * n);
      if(((int)m+(int)n)%2 == 1)
        coef *= -1.0;
      double spatial = coef * sin(m * M_PI * x_coord(cell_ID) / LENGTH)
        * sin(n * M_PI * y_coord(cell_ID) / LENGTH);
      analytic += spatial * exp(const_t_factor * (m*m + n*n));
    }
  return analytic;
}

double initial_temperature(size_t cell_ID) {
  return x_coord(cell_ID) * y_coord(cell_ID);
}

TEST(cell_to_cell_connectivity, testname) {

} // TEST

#undef M
#undef N

#undef INDEX_ID
#undef VERSIONS

#undef DIFFUSIVITY
#undef DH
#undef DT
#undef LENGTH
#undef EPS
#undef L_INF_ERROR

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
