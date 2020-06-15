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

#include "../context.hh"
//#include "flecsi/execution/launch.hh"
//#include "flecsi/execution/processor.hh"
#include <flecsi/run/types.hh>
#include <flecsi/util/common.hh>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>

#if !defined(FLECSI_ENABLE_MPI)
#error FLECSI_ENABLE_MPI not defined! This file depends on MPI!
#endif

#include <mpi.h>

#if !defined(FLECSI_ENABLE_CHARM)
#error FLECSI_ENABLE_CHARM not defined! This file depends on Charm!
#endif

#include <charm++.h>

#include <map>
#include <string_view>
#include <unordered_map>

#include "context.decl.h"

namespace flecsi::run {

const size_t FLECSI_TOP_LEVEL_TASK_ID = 0;
const size_t FLECSI_MAPPER_FORCE_RANK_MATCH = 0x00001000;
const size_t FLECSI_MAPPER_COMPACTED_STORAGE = 0x00002000;
const size_t FLECSI_MAPPER_SUBRANK_LAUNCH = 0x00003000;
const size_t FLECSI_MAPPER_EXCLUSIVE_LR = 0x00004000;

namespace charm {
template<class R = void>
using task = R(const Legion::Task *,
  const std::vector<Legion::PhysicalRegion> &,
  Legion::Context,
  Legion::Runtime *);

class ContextGroup : public CBase_ContextGroup {
public:
  ContextGroup() {
    CkPrintf("Group created on %i\n", CkMyPe());
  }

  void testEntry() {
    CkPrintf("Hello from element %i\n", thisIndex);
    contribute(CkCallback(CkCallback::ckExit));
  }
};

}

struct context_t : context {

  /*
    Friend declarations. Some parts of this interface are intentionally private
    to avoid inadvertent corruption of initialization logic.
   */

  friend charm::task<> top_level_task, handoff_to_mpi_task, wait_on_mpi_task;

  /*!
     The registration_function_t type defines a function type for
     registration callbacks.
   */

  using registration_function_t = void (*)();

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
    return Legion::Runtime::get_runtime()
      ->get_current_task(Legion::Runtime::get_context())
      ->get_depth();
  } // task_depth

  /*
    Documentation for this interface is in the top-level context type.
   */

  static size_t color() {
    flog_assert(
      task_depth() > 0, "this method can only be called from within a task");
    return Legion::Runtime::get_runtime()
      ->get_current_task(Legion::Runtime::get_context())
      ->index_point.point_data[0];
  } // color

  /*
    Documentation for this interface is in the top-level context type.
   */

  static size_t colors() {
    flog_assert(
      task_depth() > 0, "this method can only be called from within a task");
    return Legion::Runtime::get_runtime()
      ->get_current_task(Legion::Runtime::get_context())
      ->index_domain.get_volume();
  } // colors

  /// Store a reference to the argument under a small unused positive integer.
  /// Its type is forgotten.
  template<class T>
  std::size_t record(T & t) {
    const auto tp = const_cast<void *>(static_cast<const void *>(&t));
    if(auto & f = enumerated.front()) { // we have a free slot
      auto & slot = *static_cast<void **>(f);
      f = slot;
      slot = tp;
      return &slot - &f;
    }
    // NB: reallocation invalidates all zero of the free list pointers
    enumerated.push_back(tp);
    return enumerated.size() - 1;
  }
  /// Discard a recorded reference.  Its index may be reused.
  void forget(std::size_t i) {
    void *&f = enumerated.front(), *&p = enumerated[i];
    p = f;
    f = &p;
  }
  /// Obtain a reference from its index.
  /// \tparam T the object's forgotten type
  template<class T>
  T & recall(std::size_t i) {
    return *static_cast<T *>(enumerated[i]);
  }

  //--------------------------------------------------------------------------//
  //  MPI interoperability.
  //--------------------------------------------------------------------------//

  /*!
    Set the MPI user task. When control is given to the MPI runtime
    it will execute whichever function is currently set.
   */

  void set_mpi_task(std::function<void()> mpi_task) {
    {
      log::devel_guard guard(context_tag);
      flog_devel(info) << "In set_mpi_task" << std::endl;
    }

    mpi_task_ = std::move(mpi_task);
  }

  /*!
    Set the distributed-memory domain.
   */

  void set_all_processes(const LegionRuntime::Arrays::Rect<1> & all_processes) {
    all_processes_ = all_processes;
  } // all_processes

  /*!
     Return the distributed-memory domain.
   */

  const LegionRuntime::Arrays::Rect<1> & all_processes() const {
    return all_processes_;
  } // all_processes

  /*!
    Switch execution to the MPI runtime.

    @param ctx The Legion runtime context.
    @param runtime The Legion task runtime pointer.
   */

  void handoff_to_mpi(Legion::Context & ctx, Legion::Runtime * runtime);

  /*!
    Wait on the MPI runtime to finish the current task execution.

    @param ctx The Legion runtime context.
    @param runtime The Legion task runtime pointer.

    @return A future map with the result of the task execution.
   */

  Legion::FutureMap wait_on_mpi(Legion::Context & ctx,
    Legion::Runtime * runtime);

  /*!
    Connect with the MPI runtime.

    @param ctx The Legion runtime context.
    @param runtime The Legion task runtime pointer.
   */

  void connect_with_mpi(Legion::Context & ctx, Legion::Runtime * runtime);

  //--------------------------------------------------------------------------//
  // Task interface.
  //--------------------------------------------------------------------------//

  /*!
    Register a task with the runtime.

    @param name       The task name string.
    @param callback   The registration call back function.
    \return task ID
   */
  std::size_t register_task(std::string_view name,
    const registration_function_t & callback) {
    flog_devel(info) << "Registering task callback: " << name << std::endl;

    flog_assert(
      task_registry_.size() < FLECSI_GENERATED_ID_MAX, "too many tasks");
    task_registry_.push_back(callback);
    return task_registry_.size(); // 0 is the top-level task
  } // register_task

private:
  /*!
     Handoff to legion runtime from MPI.
   */

  void handoff_to_legion() {
    {
      log::devel_guard guard(context_tag);
      flog_devel(info) << "In handoff_to_legion" << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    handshake_.mpi_handoff_to_legion();
  } // handoff_to_legion

  /*!
    Wait for Legion runtime to complete.
   */

  void wait_on_legion() {
    {
      log::devel_guard guard(context_tag);
      flog_devel(info) << "In wait_on_legion" << std::endl;
    }

    handshake_.mpi_wait_on_legion();
    MPI_Barrier(MPI_COMM_WORLD);
  } // wait_on_legion

  // When GCC fixes bug #83258, these can be lambdas in the public functions:
  /*!
    Handoff to MPI from Legion.
   */

  static void mpi_handoff() {
    instance().handshake_.legion_handoff_to_mpi();
  }

  /*!
    Wait for MPI runtime to complete task execution.
   */

  static void mpi_wait() {
    instance().handshake_.legion_wait_on_mpi();
  }

  /*!
    Invoke the current MPI task, if any, and clear it.

    \return whether there was a task to invoke
   */

  bool invoke_mpi_task() {
    const bool ret(mpi_task_);
    if(ret) {
      mpi_task_();
      mpi_task_ = nullptr;
    }
    return ret;
  } // invoke_mpi_task

  /*--------------------------------------------------------------------------*
    Runtime data.
   *--------------------------------------------------------------------------*/

  // The first element is the head of the free list.
  std::vector<void *> enumerated = {nullptr};
  const std::function<int()> * top_level_action_ = nullptr;

  /*--------------------------------------------------------------------------*
    Interoperability data members.
   *--------------------------------------------------------------------------*/

  std::function<void()> mpi_task_;
  Legion::MPILegionHandshake handshake_;
  LegionRuntime::Arrays::Rect<1> all_processes_;

  /*--------------------------------------------------------------------------*
    Task data members.
   *--------------------------------------------------------------------------*/

  std::vector<registration_function_t> task_registry_;
};

} // namespace flecsi::run
