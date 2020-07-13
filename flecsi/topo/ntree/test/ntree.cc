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
  using key_t = morton_curve<dimension, int64_t>;

  using index_space = base::index_space;
  using index_spaces = base::index_spaces;

  // using entity_types = std::tuple<util::constant<base::entities>>;

  using ent_t = flecsi::topo::sort_entity<dimension, double, key_t>;

  static coloring color(const std::string & name, std::vector<ent_t> & ents) {
    txt_definition<key_t, dimension> hd(name);
    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Move to the coloring constructor
    // coloring c(hd);
    coloring c;
    c.nparts_ = size;
    c.local_entities_ = hd.local_num_entities();
    c.global_entities_ = hd.global_num_entities();
    c.entities_distribution_.resize(size);
    for(int i = 0; i < size; ++i)
      c.entities_distribution_[i] = hd.distribution(i);
    c.offset_.resize(size);
    if(rank == 0)
      std::cout << "Offset: ";

    ents.resize(hd.local_num_entities());
    for(int i = 0; i < hd.local_num_entities(); ++i) {
      ents[i] = hd.entities(i);
    }

    for(int i = 0; i < size; ++i) {
      c.offset_[i] = hd.offset(i);
      if(rank == 0)
        std::cout << rank << ": " << c.offset_[i].first << " - "
                  << c.offset_[i].second << std::endl;
    }

    if(rank == 0)
      std::cout << std::endl;

    return c;
  } // color
};

using point_t = typename topo::ntree<sph_ntree_t>::point_t;

sph_ntree_t::slot sph_ntree;
sph_ntree_t::cslot coloring;

const field<double>::definition<sph_ntree_t, sph_ntree_t::base::entities>
  entity_field;
auto pressure = entity_field(sph_ntree);

int
init(sph_ntree_t::accessor<wo> t,
  const std::vector<sph_ntree_t::ent_t> & ents) {
  UNIT {
    for(int i = 0; i < ents.size(); ++i) {
      t.get_coordinates(i) = ents[i].coordinates();
      t.get_radius(i) = ents[i].radius();
      t.get_key(i) = ents[i].key();
    }
  };
} // init

std::vector<sph_ntree_t::ent_t> tmp_ents;

int
ntree_driver() {
  UNIT {
    // Use txt_coloring in two MPI calls:
    // coloring creating
    // initialization
    // Keep same object: MPI task allows pointer or reference

    // Call process function
    int proc = process();
    MPI_Comm_rank(MPI_COMM_WORLD, &proc);
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
  };
} // ntree_driver

flecsi::unit::driver<ntree_driver> driver;
