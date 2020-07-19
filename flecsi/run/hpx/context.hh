/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi-config.h>

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include "flecsi/run/context.hh"
//#include "flecsi/exec/launch.hh"
//#include "flecsi/exec/processor.hh"
#include "flecsi/run/types.hh"
#include "flecsi/util/common.hh"

#if !defined(FLECSI_ENABLE_HPX)
#error FLECSI_ENABLE_HPX not defined! This file depends on HPX!
#endif

#include <hpx/modules/threading_base.hpp>

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#include <functional>
#include <memory>

namespace flecsi::run {

const size_t FLECSI_TOP_LEVEL_TASK_ID = 0;
const size_t FLECSI_MAPPER_FORCE_RANK_MATCH = 0x00001000;
const size_t FLECSI_MAPPER_COMPACTED_STORAGE = 0x00002000;
const size_t FLECSI_MAPPER_SUBRANK_LAUNCH = 0x00003000;
const size_t FLECSI_MAPPER_EXCLUSIVE_LR = 0x00004000;

namespace hpx {
template<typename R = void>
using task = R();

/*
  Runtime data associated with each HPX thread
 */
struct runtime_data {

  runtime_data(task_id_t task_id, size_t depth = 0)
    : task_id(task_id), depth_(depth) {}

  /*
    FleCSI task-id associated with this HPX thread
   */
  task_id_t get_task_id() const {
    return task_id_;
  }

  /*
    FleCSI task-depth associated with this HPX thread
   */
  size_t get_depth() const {
    return depth_;
  }

private:
  task_id_t task_id_ = 0;
  size_t depth_;
};

} // namespace hpx

struct context_t : context {

  /*
    Friend declarations. Some parts of this interface are intentionally private
    to avoid inadvertent corruption of initialization logic.
   */

  friend hpx::task<> top_level_task;

  //--------------------------------------------------------------------------//
  //  Runtime.
  //--------------------------------------------------------------------------//

  /*
    Documentation for this interface is in the top-level context type.
   */

  int initialize(int argc, char ** argv, bool dependent);

  /*
    Documentation for this interface is in the top-level context type.
   */

  void finalize();

  /*
    Documentation for this interface is in the top-level context type.
   */

  int start(const std::function<int()> &);

  /*
    Documentation for this interface is in the top-level context type.
   */

  void clear() {} // clear

  /*
    Documentation for this interface is in the top-level context type.
   */

  size_t process() const {
    return context::process_;
  } // process

  /*
    Documentation for this interface is in the top-level context type.
   */

  size_t processes() const {
    return context::processes_;
  } // processes

  /*
    Documentation for this interface is in the top-level context type.
   */

  size_t threads_per_process() const {
    return context::threads_per_process_;
  } // threads_per_process

  /*
    Documentation for this interface is in the top-level context type.
   */

  size_t threads() const {
    return context::threads_;
  } // threads

  /*
    Documentation for this interface is in the top-level context type.
   */

  static size_t task_depth() {
    return get_task_data()->get_depth();
  } // task_depth

  /*
    Documentation for this interface is in the top-level context type.
   */

  static size_t color() {
    flog_assert(get_task_data() != nullptr,
      "this method can only be called from within a task");
    return get_task_data()->index_point.point_data[0];
  } // color

  /*
    Documentation for this interface is in the top-level context type.
   */

  static size_t colors() {
    flog_assert(get_task_data() != nullptr,
      "this method can only be called from within a task");
    return get_task_data()->index_domain.get_volume();
  } // colors

  /*
    Return task-specific data
   */
  static flecsi::run::hpx::runtime_data * get_task_data() {
    size_t data = ::hpx::threads::get_self_id_data();
    return reinterpret_cast<flecsi::run::hpx::runtime_data *>(data);
  }

  /*
    Initialize task-specific data
   */
  static set_task_data(flecsi::run::hpx::runtime_data * data) {
    ::hpx::threads::set_self_id_data(reinterpret_cast<size_t>(data));
  }

  /*
    Reset task-specific data
   */
  static reset_task_data() {
    ::hpx::threads::set_self_id_data(size_t(0));
  }

private:
  /*--------------------------------------------------------------------------*
    Runtime data.
   *--------------------------------------------------------------------------*/

  int argc_ = 0;
  char * argv_ = nullptr;
  const std::function<int()> & top_level_action_ = nullptr;
};

namespace hpx {

/*
  Helper to manage runtime data associated with each HPX thread
 */
struct runtime_data_wrapper {

  runtime_data_wrapper(task_id_t task_id, size_t depth = 0)
    : data_(std::make_unique<runtime_data>(task_id, depth)) {
    context_t::set_task_data(data_.get());
  }

  ~runtime_data_wrapper() {
    context_t::reset_task_data();
  }

  std::unique_ptr<runtime_data> data_;
};
} // namespace hpx

} // namespace flecsi::run
