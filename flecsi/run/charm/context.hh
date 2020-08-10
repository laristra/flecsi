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
using task = R(std::vector<std::byte>&);

class ContextGroup : public CBase_ContextGroup {
public:
  ContextGroup();
  void top_level_task();

  template<class T>
  auto execute(std::vector<std::byte>& buf) {
    depth++;
    return T::execute(buf);
    depth--;
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
    // TODO: Must be some way to get this from Charm runtime
    return context_proxy_.ckLocalBranch()->task_depth();
  } // task_depth

  /*
    Documentation for this interface is in the top-level context type.
   */

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
