/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2019, Triad National Security, LLC
   All rights reserved.
                                                                              */
/*! @file */
#include "flecsi/utils/mpi_type_traits.h"

namespace flecsi {
namespace execution {

inline void
remap_shared_entities() {
  // TODO: Is this superseded by index_map/reverse_index_map?
  auto & context_ = context_t::instance();
  const auto & my_color = context_.color();
  const auto & num_colors = context_.colors();

  const auto mpi_size_t = utils::mpi_type<std::size_t>();

  for(auto & coloring_info_pair : context_.coloring_info_map()) {
    auto index_space = coloring_info_pair.first;
    auto & coloring_info = coloring_info_pair.second;

    auto & my_coloring_info = context_.coloring_info(index_space).at(my_color);
    auto & index_coloring = context_.coloring(index_space);

    //    for (auto& shared : index_coloring.shared) {
    //      clog_rank(warn, 0) << "myrank: " << my_color
    //                         << " shared id: " << shared.id
    //                         << ", rank: " << shared.rank
    //                         << ", offset: " << shared.offset
    //                         << ", index: " << index << std::endl;
    //     }

    // we are renumbering the entities such that the shared will be
    // gather the data to send into one buffer per rank
    size_t index = 0;
    std::unordered_map<size_t, std::vector<size_t>> send_buffers;
    std::set<flecsi::coloring::entity_info_t> new_shared;

    for(auto & shared : index_coloring.shared) {
      for(auto peer : shared.shared) {
        send_buffers[peer].emplace_back(index);
      }
      new_shared.insert(flecsi::coloring::entity_info_t(
        shared.id, shared.rank, index, shared.shared));
      index++;
    }
    context_t::instance().coloring(index_space).shared.swap(new_shared);

    // create storage for the requests
    std::vector<MPI_Request> requests;
    requests.reserve(2 * num_colors);

    // figure out who i am receiving from
    std::vector<size_t> counts(num_colors, 0);
    for(auto ghost : index_coloring.ghost)
      counts[ghost.rank]++;

    auto tag = 0;

    // post receives
    std::unordered_map<size_t, std::vector<size_t>> recv_buffers;
    for(size_t i = 0; i < num_colors; ++i) {
      auto n = counts[i];
      if(n > 0) {
        auto rank = i;
        clog_assert(
          rank != my_color, "Why would I be receiving data from myself?");
        auto & buf = recv_buffers[i];
        buf.resize(n);
        requests.resize(requests.size() + 1);
        auto & my_request = requests.back();
        auto ret = MPI_Irecv(buf.data(), static_cast<int>(n), mpi_size_t,
          static_cast<int>(rank), tag, MPI_COMM_WORLD, &my_request);
      }
    }

    // send the data
    for(const auto & comm_pair : send_buffers) {
      const auto & rank = comm_pair.first;
      clog_assert(rank != my_color, "Why would I be sending data to myself?");
      const auto & buf = comm_pair.second;
      requests.resize(requests.size() + 1);
      auto & my_request = requests.back();
      auto ret = MPI_Isend(buf.data(), static_cast<int>(buf.size()), mpi_size_t,
        static_cast<int>(rank), tag, MPI_COMM_WORLD, &my_request);
    }

    // wait for everything to complete
    std::vector<MPI_Status> status(requests.size());
    MPI_Waitall(
      static_cast<int>(requests.size()), requests.data(), status.data());

    // now we can unpack the messages and reconstruct the ghost entities
    std::set<flecsi::coloring::entity_info_t> new_ghost;
    std::fill(counts.begin(), counts.end(), 0);

    for(auto ghost : index_coloring.ghost) {
      auto & offset = counts[ghost.rank];
      auto index = recv_buffers.at(ghost.rank).at(offset);
      new_ghost.insert(
        flecsi::coloring::entity_info_t(ghost.id, ghost.rank, index, {}));
      offset++;
    }
    //    for (auto ghost : index_coloring.ghost) {
    //      clog_rank(warn, 1) << "myrank: " << my_color
    //                         << " old ghost id: " << ghost.id
    //                         << ", rank: " << ghost.rank
    //                         << ", offset: " << ghost.offset
    //                         << std::endl;
    //    }
    //    for (auto ghost : new_ghost) {
    //      clog_rank(warn, 1) << "myrank: " << my_color
    //                         << " new ghost id: " << ghost.id
    //                         << ", rank: " << ghost.rank
    //                         << ", offset: " << ghost.offset
    //                         << std::endl;
    //    }
    context_t::instance().coloring(index_space).ghost.swap(new_ghost);
  }
}

} // namespace execution
} // namespace flecsi
