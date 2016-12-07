//
// Created by ollie on 12/6/16.
//

#ifndef FLECSI_WEAVER_H_H
#define FLECSI_WEAVER_H_H

class weaver
{
private:
  flecsi::io::mesh_definition &sd;

public:
  weaver(flecsi::io::mesh_definition &mesh) : sd(mesh) {}
  {
    using entry_info_t = flecsi::dmp::entry_info_t;

    int size;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Create a mesh definition from file.
    //flecsi::io::simple_definition_t sd("simple2d-4x4.msh");

    // Create the dCRS representation for the distributed
    // partitioner.
    auto dcrs = flecsi::dmp::make_dcrs(sd);

    // Create a partitioner instance to generate the primary partition.
    auto partitioner = std::make_shared<flecsi::dmp::parmetis_partitioner_t>();

    // Create the primary partition.
    auto primary = partitioner->partition(dcrs);

    // Compute the dependency closure of the primary cell partition
    // through vertex intersections (specified by last argument "1").
    // To specify edge or face intersections, use 2 (edges) or 3 (faces).
    // FIXME: We may need to replace this with a predicate function.
    auto closure = flecsi::io::cell_closure(sd, primary, 1);

    // Subtracting out the initial set leaves just the nearest
    // neighbors. This is similar to the image of the adjacency
    // graph of the initial indices.
    auto nearest_neighbors = flecsi::utils::set_difference(closure, primary);

    // The closure of the nearest neighbors intersected with
    // the initial indeces gives the shared indices. This is similar to
    // the preimage of the nearest neighbors.
    auto nearest_neighbor_closure =
      flecsi::io::cell_closure(sd, nearest_neighbors, 1);

    // We can iteratively add halos of nearest neighbors, e.g.,
    // here we add the next nearest neighbors. For most mesh types
    // we actually need information about the ownership of these indices
    // so that we can deterministically assign rank ownership to vertices.
    auto next_nearest_neighbors =
      flecsi::utils::set_difference(nearest_neighbor_closure, closure);

    // The union of the nearest and next-nearest neighbors gives us all
    // of the cells that might reference a vertex that we need.
    auto all_neighbors = flecsi::utils::set_union(nearest_neighbors,
      next_nearest_neighbors);

    // Create a communicator instance to get neighbor information.
    auto communicator = std::make_shared<flecsi::dmp::mpi_communicator_t>();

    // Get the rank and offset information for our nearest neighbor
    // dependencies. This also gives information about the ranks
    // that access our shared cells.
    auto cell_nn_info = communicator->get_cell_info(primary, nearest_neighbors);

    //
    auto cell_all_info = communicator->get_cell_info(primary, all_neighbors);

    // Create a map version of the local info for lookups below.
    std::unordered_map<size_t, size_t> primary_indices_map;
    {
      size_t offset(0);
      for(auto i: primary) {
        primary_indices_map[offset++] = i;
      } // for
    } // scope

    // Create a map version of the remote info for lookups below.
    std::unordered_map<size_t, entry_info_t> remote_info_map;
    for(auto i: std::get<1>(cell_all_info)) {
      remote_info_map[i.id] = i;
    } // for

    std::set<entry_info_t> exclusive_cells;
    std::set<entry_info_t> shared_cells;
    std::set<entry_info_t> ghost_cells;

    // Populate exclusive and shared cell information.
    {
      size_t offset(0);
      for(auto i: std::get<0>(cell_nn_info)) {
        if(i.size()) {
          shared_cells.insert(entry_info_t(primary_indices_map[offset],
            rank, offset, i));
        }
        else {
          exclusive_cells.insert(entry_info_t(primary_indices_map[offset],
            rank, offset, i));
        } // if
        ++offset;
      } // for
    } // scope

    // Populate ghost cell information.
    {
      size_t offset(0);
      for(auto i: std::get<1>(cell_nn_info)) {
        ghost_cells.insert(i);
      } // for
    } // scope

  }


};
#endif //FLECSI_WEAVER_H_H
