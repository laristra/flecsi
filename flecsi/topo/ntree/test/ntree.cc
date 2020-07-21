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

#include "physics.hh"
#include "txt_definition.hh"

using namespace flecsi;

struct sph_ntree_t : topo::specialization<topo::ntree, sph_ntree_t> {
  static constexpr unsigned int dimension = 3;
  using key_t = morton_curve<dimension, int64_t>;

  using index_space = flecsi::topo::ntree_base::index_space;
  using index_spaces = flecsi::topo::ntree_base::index_spaces;

  // using entity_types = std::tuple<util::constant<base::entities>>;

  using ent_t = flecsi::topo::sort_entity<dimension, double, key_t>;
  using node_t = flecsi::topo::node<dimension, double, key_t>;

  static coloring color(const std::string & name, std::vector<ent_t> & ents) {
    txt_definition<key_t, dimension> hd(name);
    int size, rank;
    rank = process();
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Move to the coloring constructor
    // coloring c(hd);
    coloring c;

    // For now the number of partitions is the number of processes
    c.nparts_ = size;

    c.local_entities_ = hd.local_num_entities();
    c.global_entities_ = hd.global_num_entities();
    c.entities_distribution_.resize(size);
    for(int i = 0; i < size; ++i)
      c.entities_distribution_[i] = hd.distribution(i);
    c.entities_offset_.resize(size);
    if(rank == 0)
      std::cout << "Ents Offset: ";

    ents.resize(hd.local_num_entities());
    for(int i = 0; i < hd.local_num_entities(); ++i) {
      ents[i] = hd.entities(i);
    }

    for(int i = 0; i < size; ++i) {
      c.entities_offset_[i] = hd.offset(i);
      if(rank == 0)
        std::cout << c.entities_offset_[i].first << "-"
                  << c.entities_offset_[i].second << " ; ";
    }
    if(rank == 0)
      std::cout << std::endl;

    // Nodes, hmap and tdata information
    // \TODO move these inside the topology ntree
    // The user should not have control on their allocation
    c.local_nodes_ = c.local_entities_;
    c.global_nodes_ = c.global_entities_;
    c.nodes_offset_ = c.entities_offset_;

    c.global_hmap_ = c.nparts_ * c.local_hmap_;
    c.hmap_offset_.resize(c.nparts_);
    for(int i = 0; i < c.nparts_; ++i) {
      c.hmap_offset_[i] =
        std::make_pair(i * c.local_hmap_, (i + 1) * c.local_hmap_);
    }
    if(rank == 0)
      std::cout << "hmap offset: ";
    for(int i = 0; i < size; ++i) {
      if(rank == 0)
        std::cout << c.hmap_offset_[i].first << "-" << c.hmap_offset_[i].second
                  << " ; ";
    }
    if(rank == 0)
      std::cout << std::endl;

    c.tdata_offset_.resize(c.nparts_);
    for(int i = 0; i < c.nparts_; ++i) {
      c.tdata_offset_[i] = std::make_pair(i, i + 1);
    }
    if(rank == 0)
      std::cout << "tdata offset: ";
    for(int i = 0; i < size; ++i) {
      if(rank == 0)
        std::cout << c.tdata_offset_[i].first << "-"
                  << c.tdata_offset_[i].second << " ; ";
    }
    if(rank == 0)
      std::cout << std::endl;

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
auto pressure = entity_field(sph_ntree);

std::vector<sph_ntree_t::ent_t> tmp_ents;

int
init(sph_ntree_t::accessor<wo> t,
  const std::vector<sph_ntree_t::ent_t> & ents) {
  UNIT {
    for(int i = 0; i < ents.size(); ++i) {
      t.e_coordinates(i) = ents[i].coordinates();
      t.e_radius(i) = ents[i].radius();
      t.e_keys(i) = ents[i].key();
    }
  };
} // init

int
make_tree(sph_ntree_t::accessor<rw> t) {
  UNIT { t.make_tree(); };
}

int
ntree_driver() {
  UNIT {

    // Use txt_coloring in two MPI calls:
    // coloring creating
    // initialization
    // Keep same object: MPI task allows pointer or reference

    // Call process function
    size_t proc = process();
    size_t size = processes();
    int tmp;
    coloring.allocate("coordinates.blessed", tmp_ents);
    if(!proc)
      std::cout << "Coloring allocate DONE" << std::endl;
    sph_ntree.allocate(coloring.get());
    if(!proc)
      std::cout << "sph_ntree allocate DONE" << std::endl;

    flecsi::execute<init, flecsi::mpi>(sph_ntree, tmp_ents);
    tmp_ents.clear();

    // Construct the tree data structure
    flecsi::execute<make_tree>(sph_ntree);
  };
} // ntree_driver

flecsi::unit::driver<ntree_driver> driver;
