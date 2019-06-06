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
/*! @file */

#include <cstddef>
#include <cstdint>

#ifdef ENABLE_CALIPER
#include <caliper/cali.h>
#endif

#include <flecsi/coloring/mpi_utils.h>
#include <flecsi/data/data.h>

clog_register_tag(runtime_driver);

namespace flecsi {
namespace execution {

//----------------------------------------------------------------------------//
// Implementation of FleCSI runtime driver task.
//----------------------------------------------------------------------------//

void
remap_shared_entities() {
  // TODO: Is this superseded by index_map/reverse_index_map?
  auto & context_ = context_t::instance();
  const auto & my_color = context_.color();
  const auto & num_colors = context_.colors();

  const auto mpi_size_t = coloring::mpi_typetraits_u<size_t>::type();

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
        auto & buf = recv_buffers[i];
        buf.resize(n);
        requests.resize(requests.size() + 1);
        auto & my_request = requests.back();
        auto ret = MPI_Irecv(
          buf.data(), n, mpi_size_t, rank, tag, MPI_COMM_WORLD, &my_request);
      }
    }

    // send the data
    for(const auto & comm_pair : send_buffers) {
      const auto & rank = comm_pair.first;
      const auto & buf = comm_pair.second;
      requests.resize(requests.size() + 1);
      auto & my_request = requests.back();
      auto ret = MPI_Isend(buf.data(), buf.size(), mpi_size_t, rank, tag,
        MPI_COMM_WORLD, &my_request);
    }

    // wait for everything to complete
    std::vector<MPI_Status> status(requests.size());
    MPI_Waitall(requests.size(), requests.data(), status.data());

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

void
runtime_driver(int argc, char ** argv) {
  {
    clog_tag_guard(runtime_driver);
    clog(info) << "In MPI runtime driver" << std::endl;
  }

  auto & context_ = context_t::instance();

  //--------------------------------------------------------------------------//
  // Invoke callbacks for entries in the reduction operation registry.
  //--------------------------------------------------------------------------//

  auto & reduction_registry = context_.reduction_registry();

  for(auto & c : reduction_registry) {
    c.second();
  } // for

  //--------------------------------------------------------------------------//
  // Invoke callbacks for entries in the client registry.
  //
  // NOTE: This needs to be called before the field registry below because
  //       The client callbacks register field callbacks with the field
  //       registry.
  //--------------------------------------------------------------------------//

  auto & client_registry =
    flecsi::data::storage_t::instance().client_registry();

  for(auto & c : client_registry) {
    for(auto & d : c.second) {
      d.second.second(d.second.first);
    } // for
  } // for

  //--------------------------------------------------------------------------//
  // Invoke callbacks for entries in the field registry.
  //--------------------------------------------------------------------------//

  auto & field_registry = flecsi::data::storage_t::instance().field_registry();

  for(auto & c : field_registry) {
    for(auto & f : c.second) {
      f.second.second(f.first, f.second.first);
    } // for
  } // for

  for(auto fi : context_.registered_fields()) {
    context_.put_field_info(fi);
  }

#if defined(FLECSI_ENABLE_SPECIALIZATION_TLT_INIT)
  {
    clog_tag_guard(runtime_driver);
    clog(info) << "Executing specialization tlt task" << std::endl;
  }

  // Execute the specialization driver.
  CALI_MARK_BEGIN("tlt_init");
  specialization_tlt_init(argc, argv);
  CALI_MARK_END("tlt_init");

#endif // FLECSI_ENABLE_SPECIALIZATION_TLT_INIT

  remap_shared_entities();

  // Setup maps from mesh to compacted (local) index space and vice versa
  //
  // This depends on the ordering of the BLIS data structure setup.
  // Currently, this is Exclusive - Shared - Ghost.

  for(auto is : context_.coloring_map()) {
    std::map<size_t, size_t> _map;
    size_t counter(0);

    for(auto index : is.second.exclusive) {
      _map[counter++] = index.id;
    } // for

    for(auto index : is.second.shared) {
      _map[counter++] = index.id;
    } // for

    for(auto index : is.second.ghost) {
      _map[counter++] = index.id;
    } // for

    context_.add_index_map(is.first, _map);
  } // for

#if defined(FLECSI_ENABLE_DYNAMIC_CONTROL_MODEL)

  // Execute control
  if(context_.top_level_driver()) {
    context_.top_level_driver()(argc, argv);
  }

#else

  context_.advance_state();

  // Call the specialization color initialization function.
#if defined(FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT)
  CALI_MARK_BEGIN("spmd_init");
  specialization_spmd_init(argc, argv);
  CALI_MARK_END("spmd_init");
#endif // FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT

  context_.advance_state();

  // Execute the user driver.
  driver(argc, argv);

#endif // FLECSI_ENABLE_DYNAMIC_CONTROL_MODEL

} // runtime_driver

} // namespace execution
} // namespace flecsi
