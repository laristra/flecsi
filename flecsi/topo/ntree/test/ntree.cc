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
    std::size_t operator()(const key_int_t & k) const noexcept {
      return static_cast<std::size_t>(k & ((1 << 22) - 1));
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
    sph_ntree.allocate(coloring.get());
    flecsi::execute<init, flecsi::mpi>(sph_ntree, tmp_ents);
    tmp_ents.clear();

    // Construct the tree data structure
    flecsi::execute<make_tree>(sph_ntree);
  };
} // ntree_driver

flecsi::unit::driver<ntree_driver> driver;
