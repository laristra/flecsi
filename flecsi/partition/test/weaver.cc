//
// Created by ollie on 12/6/16.
//

#include <cinchtest.h>

#include <mpi.h>

#include "flecsi/partition/weaver.h"

int output_rank = 0;

TEST(weaver, construct) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  flecsi::io::simple_definition_t sd("simple2d-8x8.msh");
  flecsi::dmp::weaver weaver(sd);

  using entry_info_t = flecsi::dmp::entry_info_t;

  std::set<entry_info_t> exclusive_cells = weaver.get_exclusive_cells();
  std::set<entry_info_t> shared_cells = weaver.get_shared_cells();
  std::set<entry_info_t> ghost_cells  = weaver.get_ghost_cells();

  std::unordered_map<size_t, size_t> num_cells_per_rank=
		weaver.get_n_cells_per_rank();

  if (rank == output_rank) {
    std::cout << "exclusive cells: " << std::endl;
    for(auto i: exclusive_cells) {
      std::cout << i << std::endl;
    } // for

    std::cout << "shared cells: " << std::endl;
    for(auto i: shared_cells) {
      std::cout << i << std::endl;
    } // for


    std::cout << "ghost cells: " << std::endl;
    for(auto i: ghost_cells) {
      std::cout << i << std::endl;
    } // for
  }

  std::set<entry_info_t> exclusive_vertices = weaver.get_exclusive_vertices();
  std::set<entry_info_t> shared_vertices = weaver.get_shared_vertices();
  std::set<entry_info_t> ghost_vertices  = weaver.get_ghost_vertices();

  std::unordered_map<size_t, size_t> num_vertices_per_rank=
                weaver.get_n_vertices_per_rank();

  if (rank == output_rank) {
    std::cout << "exclusive vertices: " << std::endl;
    for(auto i: exclusive_vertices) {
      std::cout << i << std::endl;
    } // for

    std::cout << "shared vertices: " << std::endl;
    for(auto i: shared_vertices) {
      std::cout << i << std::endl;
    } // for


    std::cout << "ghost vertices: " << std::endl;
    for(auto i: ghost_vertices) {
      std::cout << i << std::endl;
    } // for
  }

}
