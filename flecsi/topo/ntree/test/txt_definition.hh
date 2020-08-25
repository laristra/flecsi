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
#pragma once

/*! @file */

#include <fstream>
#include <set>
#include <vector>

#include "colorer.hh"

/*!
  The definition type...

  @ingroup ntree-topology
 */

template<typename KEY, int DIM>
class txt_definition
{
public:
  const int dim = DIM;
  using key_t = KEY;
  using point_t = flecsi::util::point<double, DIM>;
  using ent_t = flecsi::topo::sort_entity<DIM, double, key_t>;
  using colorer_t = tree_colorer::colorer<ent_t, key_t, DIM>;

  txt_definition(const std::string & filename) {
    int size, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    read_entities_(filename);
    // Compute the range
    colorer_t::mpi_compute_range(entities_, range_);
    // Generate the keys
    for(size_t i = 0; i < entities_.size(); ++i) {
      entities_[i].set_key(key_t(range_, entities_[i].coordinates()));
    }

    nlocal_entities_ = entities_.size();
    // Distribute the particles among the ranks
    colorer_t::mpi_qsort(entities_, nglobal_entities_);

    if(rank == 0)
      flog(info) << rank << ": Range: " << range_[0] << ";" << range_[1]
                 << std::endl;
  }

  size_t global_num_entities() const {
    return nglobal_entities_;
  }

  size_t distribution(const int & i) const {
    return distribution_[i];
  }

  std::pair<size_t, size_t> offset(const int & i) const {
    return std::pair(offset_[i], offset_[i + 1]);
  }

  std::vector<ent_t> & entities() {
    return entities_;
  }

  ent_t & entities(const int & i) {
    return entities_[i];
  }

private:
  void read_entities_(const std::string & filename) {
    int size, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    // For now read all particles?
    std::ifstream myfile(filename);
    nglobal_entities_ = 0;
    myfile >> nglobal_entities_;

    offset_.resize(size + 1, 0);
    distribution_.resize(size, 0);
    // Entities per ranks
    int nlocal_entities = nglobal_entities_ / size;
    int lm = nglobal_entities_ % size;
    for(int i = 0; i < size; ++i) {
      distribution_[i] = nlocal_entities;
      if(i < lm)
        ++distribution_[i];
    }

    for(int i = 1; i < size + 1; ++i) {
      offset_[i] = distribution_[i - 1] + offset_[i - 1];
    }

    if(rank == 0) {
      flog(info) << "Global entities: " << nglobal_entities_ << std::endl;
      std::ostringstream oss;
      oss << "Distribution:";
      for(int i = 0; i < size; ++i) {
        oss << " " << i << ":" << distribution_[i];
      }
      flog(info) << oss.str() << std::endl;
      oss.str("");
      oss.clear();
      oss << "Offset:";
      for(int i = 0; i < size + 1; ++i) {
        oss << " " << i << ":" << offset_[i];
      }
      flog(info) << oss.str() << std::endl;
    }

    nlocal_entities_ = distribution_[rank];

    entities_.resize(nlocal_entities_);

    // Coordinates, ignore the other ranks
    int k = 0;
    for(size_t i = 0; i < nglobal_entities_; ++i) {
      point_t p;
      for(int j = 0; j < dim; ++j) {
        myfile >> p[j];
      }
      if(i >= offset_[rank] && i < offset_[rank + 1])
        entities_[k++].set_coordinates(p);
    }

    // Radius
    k = 0;
    for(size_t i = 0; i < nglobal_entities_; ++i) {
      double r;
      myfile >> r;
      if(i >= offset_[rank] && i < offset_[rank + 1])
        entities_[k++].set_radius(r);
    }

    // Mass
    k = 0;
    for(size_t i = 0; i < nglobal_entities_; ++i) {
      double m;
      myfile >> m;
      if(i >= offset_[rank] && i < offset_[rank + 1])
        entities_[k++].set_mass(m);
    }

    k = 0;
    for(size_t i = 0; i < nglobal_entities_; ++i) {
      if(i >= offset_[rank] && i < offset_[rank + 1])
        entities_[k++].set_id(i);
    }

    myfile.close();
  }

  std::array<point_t, 2> range_;
  std::vector<ent_t> entities_;
  size_t nglobal_entities_;
  size_t nlocal_entities_;
  std::vector<size_t> distribution_;
  std::vector<size_t> offset_;

}; // class txt_definition
