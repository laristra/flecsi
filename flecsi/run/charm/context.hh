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
#include "flecsi/run/types.hh"
#include "flecsi/util/common.hh"
#include "flecsi/util/function_traits.hh"

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

namespace charm {
template<class R = void>
using task = R(std::vector<std::byte>&);

// This is the charm group which manages the context and makes context info
// accessible on all PEs. At the moment, it's pretty skeletal, but moving
// forward, should include field management, communication, asynchronous task
// execution, reduction logic, etc.
class ContextGroup : public CBase_ContextGroup {
public:
  ContextGroup();
  void top_level_task();

  template<class T>
  auto execute(std::vector<std::byte>& buf) {
    using traits_t = util::function_traits<decltype(T::execute)>;
    using return_t = typename traits_t::return_type;
    if constexpr(std::is_same_v<return_t, void>) {
      depth++;
      T::execute(buf);
      depth--;
    } else {
      depth++;
      return_t result = T::execute(buf);
      depth--;
      return result;
    }
  }

  int task_depth() const {
    return depth;
  }

  void regField(std::size_t i, std::size_t s) {
    if (data_map.count(i)) return;
    data_map[i] = data.size();
    data.push_back(new std::byte[s]);
  }

  std::byte* getField(std::size_t i) {
    CkAssert(data_map.count(i));
    return data[data_map[i]];
  }

private:
  int depth;
  std::unordered_map<std::size_t, std::size_t> data_map;
  std::vector<std::byte*> data;
};

}

struct context_t : context {

  /*
    Friend declarations. Some parts of this interface are intentionally private
    to avoid inadvertent corruption of initialization logic.
   */

  friend charm::task<> top_level_task, handoff_to_mpi_task, wait_on_mpi_task;
  friend charm::ContextGroup;

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

  size_t task_depth() {
    return context_proxy_.ckLocalBranch()->task_depth();
  } // task_depth

  /*
    Documentation for this interface is in the top-level context type.
   */

  // TODO: Color functionality still needs implementation. It may also need
  // to be made static (as well as some of the other functions here)
  size_t color() {
    flog_assert(
      task_depth() > 0, "this method can only be called from within a task");
    return 0;
  } // color

  /*
    Documentation for this interface is in the top-level context type.
   */

  size_t colors() {
    flog_assert(
      task_depth() > 0, "this method can only be called from within a task");
    return 0;
  } // colors

  template <class T>
  auto execute(std::vector<std::byte>& buf) {
    return context_proxy_.ckLocalBranch()->execute<T>(buf);
  }

  void regField(std::size_t i, std::size_t s) {
    context_proxy_.ckLocalBranch()->regField(i, s);
  }

  std::byte* getField(std::size_t i) {
    return context_proxy_.ckLocalBranch()->getField(i);
  }

private:

  /*--------------------------------------------------------------------------*
    Runtime data.
   *--------------------------------------------------------------------------*/

  charm::CProxy_ContextGroup context_proxy_;

  // The first element is the head of the free list.
  std::vector<void *> enumerated = {nullptr};
  const std::function<int()> * top_level_action_ = nullptr;

};

} // namespace flecsi::run
