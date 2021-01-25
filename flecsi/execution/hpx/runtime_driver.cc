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

#include <hpx/modules/execution_base.hpp> // yield_while
#include <hpx/modules/runtime_local.hpp> // get_thread_manager

#include <cstddef>
#include <cstdint>

#include <flecsi/data/data.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/hpx/runtime_driver.h>
#include <flecsi/execution/remap_shared.h>
#include <flecsi/utils/annotation.h>

clog_register_tag(runtime_driver);

namespace flecsi {
namespace execution {

void
termination_detection() {
  auto & tm = hpx::threads::get_thread_manager();
  hpx::util::yield_while(
    [&tm]() -> bool {
      return tm.get_thread_count() >
             std::int64_t(1) + tm.get_background_thread_count();
    },
    "termination_detection");
}

//----------------------------------------------------------------------------//
// Implementation of FleCSI runtime driver task.
//----------------------------------------------------------------------------//

void
hpx_runtime_driver(int argc, char ** argv) {
  {
    clog_tag_guard(runtime_driver);
    clog(info) << "In HPX runtime driver" << std::endl;
  }

  auto & context_ = context_t::instance();
  using annotation = flecsi::utils::annotation;

  annotation::begin<annotation::runtime_setup>();
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

  annotation::end<annotation::runtime_setup>();

#if defined(FLECSI_ENABLE_SPECIALIZATION_TLT_INIT)
  {
    clog_tag_guard(runtime_driver);
    clog(info) << "Executing specialization tlt task" << std::endl;
  }

  annotation::begin<annotation::spl_tlt_init>();
  // Execute the specialization driver.
  specialization_tlt_init(argc, argv);
  annotation::end<annotation::spl_tlt_init>();

  // make sure all activity has ceased
  termination_detection();
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

#if defined(FLECSI_USE_AGGCOMM)
  auto & ispace_dmap = context_.index_space_data_map();
  for(const auto & fi : context_.registered_fields()) {
    auto & ispace_data = ispace_dmap[fi.index_space];
    ispace_data.ghost_is_readable[fi.fid] = true;
  }
#endif

#if defined(FLECSI_ENABLE_DYNAMIC_CONTROL_MODEL)

  // Execute control
  if(context_.top_level_driver()) {
    context_.top_level_driver()(argc, argv);
  }

#else

  context_.advance_state();

  // Call the specialization color initialization function.
#if defined(FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT)
  annotation::begin<annotation::spl_spmd_init>();
  specialization_spmd_init(argc, argv);
  annotation::end<annotation::spl_spmd_init>();

  // make sure all activity has ceased
  termination_detection();
#endif // FLECSI_ENABLE_SPECIALIZATION_SPMD_INIT

  context_.advance_state();

  annotation::begin<annotation::driver>();
  // Execute the user driver.
  driver(argc, argv);
  annotation::end<annotation::driver>();

#endif // FLECSI_ENABLE_DYNAMIC_CONTROL_MODEL

} // runtime_driver

} // namespace execution
} // namespace flecsi
