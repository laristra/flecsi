/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#define __FLECSI_PRIVATE__
#include <flecsi/data.hh>

#include "flecsi/util/geometry/filling_curve.hh"
#include "flecsi/util/geometry/point.hh"
#include "flecsi/util/unit.hh"

#include "flecsi/topo/ntree/interface.hh"
#include "flecsi/topo/ntree/types.hh"

#include "txt_definition.hh"

using namespace flecsi;

struct sph_ntree_t : topo::specialization<topo::ntree, sph_ntree_t> {
  static constexpr unsigned int dimension = 3;
  using key_int_t = uint64_t;
  using key_t = morton_curve<dimension, key_int_t>;

  using index_space = flecsi::topo::ntree_base::index_space;
  using index_spaces = flecsi::topo::ntree_base::index_spaces;

  struct key_t_hasher {
    std::size_t operator()(const key_t & k) const noexcept {
      return static_cast<std::size_t>(k.value() & ((1 << 22) - 1));
    }
  };

  using hash_f = key_t_hasher;

  using ent_t = flecsi::topo::sort_entity<dimension, double, key_t>;
  using node_t = flecsi::topo::node<dimension, double, key_t>;

  static coloring color(const std::string & name, std::vector<ent_t> & ents) {
    txt_definition<key_t, dimension> hd(name);
    int size, rank;
    rank = process();
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    coloring c(size);

    c.global_entities_ = hd.global_num_entities();
    c.entities_distribution_.resize(size);
    for(int i = 0; i < size; ++i)
      c.entities_distribution_[i] = hd.distribution(i);
    c.entities_offset_.resize(size);

    ents = hd.entities();

    std::ostringstream oss;
    if(rank == 0)
      oss << "Ents Offset: ";

    for(int i = 0; i < size; ++i) {
      c.entities_offset_[i] =
        c.entities_distribution_[i]; // hd.offset(i).second;
      if(rank == 0)
        oss << c.entities_offset_[i] << " ; ";
    }

    if(rank == 0)
      flog(info) << oss.str() << std::endl;

    // Nodes, hmap and tdata information
    // \TODO move these inside the topology ntree
    // The user should not have control on their allocation
    c.local_nodes_ = c.local_entities_;
    c.global_nodes_ = c.global_entities_;
    c.nodes_offset_ = c.entities_offset_;

    c.global_sizes_.resize(4);
    c.global_sizes_[0] = c.global_entities_;
    c.global_sizes_[1] = c.global_nodes_;
    c.global_sizes_[2] = c.global_hmap_;
    c.global_sizes_[3] = c.nparts_;

    return c;
  } // color
};

using point_t = typename topo::ntree<sph_ntree_t>::point_t;

sph_ntree_t::slot sph_ntree;
sph_ntree_t::cslot coloring;

const field<double>::definition<sph_ntree_t, sph_ntree_t::base::entities>
  entity_field;

int
init_task(sph_ntree_t::accessor<wo> t,
  const std::vector<sph_ntree_t::ent_t> & ents) {
  UNIT {
    for(size_t i = 0; i < ents.size(); ++i) {
      t.e_coordinates(i) = ents[i].coordinates();
      t.e_radius(i) = ents[i].radius();
      t.e_keys(i) = ents[i].key();
    }
    t.exchange_boundaries();
  };
} // init_task

int
make_tree(sph_ntree_t::accessor<rw> t) {
  UNIT {
    t.make_tree();
    t.graphviz_draw(0);
  };
} // make_tree

// The initialization part of the execution model
// Create the coloring
// Create the ntree memory space
// Feed the basic fields of the ntree
// Create the ntree structure
int
init() {
  std::vector<sph_ntree_t::ent_t> ents;
  coloring.allocate("coordinates.blessed", ents);
  sph_ntree.allocate(coloring.get());
  flecsi::execute<init_task, flecsi::mpi>(sph_ntree, ents);
  sph_ntree.get().exch();
  flecsi::execute<make_tree>(sph_ntree);

  std::vector<data::partition::row> rv(coloring.get().nparts_);
  std::vector<std::vector<data::partition::point>> pv(coloring.get().nparts_);
  return 0;
} // init
flecsi::unit::initialization<init> initialization;

int
ntree_driver() {

  return 0;
} // ntree_driver
flecsi::unit::driver<ntree_driver> driver;
