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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include <flecsi/data/field_info.hh>
#include <flecsi/execution/launch.hh>
#include <flecsi/execution/task_attributes.hh>
#include <flecsi/runtime/types.hh>
#include <flecsi/topology/base_topology_types.hh>
#include <flecsi/utils/common.hh>
#include <flecsi/utils/const_string.hh>
#include <flecsi/utils/demangle.hh>
#include <flecsi/utils/flog.hh>
#include <flecsi/utils/hash.hh>

#include <boost/program_options.hpp>

#include <cassert>
#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <utility>

flog_register_tag(context);

namespace flecsi::runtime {

struct context_t; // supplied by backend

/*!
  The context type provides a high-level execution context interface that
  is implemented by a specific backend.

  @ingroup runtime
 */

struct context {

  /*--------------------------------------------------------------------------*
    Public types.
   *--------------------------------------------------------------------------*/

  using topology_registration_function_t = std::function<void(size_t)>;
  using topology_registration_entry_t =
    std::pair<field_id_t, topology_registration_function_t>;
  using topology_registration_map_t =
    std::unordered_map<size_t, topology_registration_entry_t>;

  using field_registration_function_t = std::function<void(size_t, size_t)>;
  using field_registration_entry_t =
    std::pair<field_id_t, field_registration_function_t>;
  using field_registration_map_t =
    std::unordered_map<size_t, field_registration_entry_t>;

  /*!
    This type allows the storage of field information per storage class. The
    size_t key is the storage class.
   */

  using field_info_store_t = data::field_info_store_t;
  using field_info_map_t = std::unordered_map<size_t, field_info_store_t>;

  /*!
   This type allows storage of launch_domains, the key is a hash from the
   domain name, the value is # of index points.
   */

  using launch_domain_map_t = std::unordered_map<size_t, size_t>;

  /*!
    This type allows storage of subspace information.
    @todo Currently only for unstructured mesh types. However, this needs to be
    extended.)
   */

  struct index_subspace_info_t {
    size_t index_subspace;
    size_t capacity;
    size_t size = 0;
  }; // struct index_subspace_info_t

  /*--------------------------------------------------------------------------*
    Deleted contructor and assignment interfaces.
   *--------------------------------------------------------------------------*/

  context(const context &) = delete;
  context & operator=(const context &) = delete;
  context(context &&) = delete;
  context & operator=(context &&) = delete;

  /*!
    Meyer's singleton instance.
   */

  static inline context_t & instance();

  /*--------------------------------------------------------------------------*
    Runtime interface.
   *--------------------------------------------------------------------------*/

#ifdef DOXYGEN // these functions are implemented per-backend
  /*!
    Start the FleCSI runtime.

    @param argc The number of command-line arguments.
    @param argv The command-line arguments in a char **.

    @return An integer with \em 0 being success, and any other value
            being failure.
   */

  int start(int argc, char ** argv, variables_map & vm);

  /*!
    Return the current process id.
   */

  std::size_t process() const;

  /*!
    Return the number of processes.
   */

  std::size_t processes() const;

  /*!
    Return the number of threads per process.
   */

  std::size_t threads_per_process() const;

  /*!
    Return the number of execution instances with which the runtime was
    invoked. In this context a \em thread is defined as an instance of
    execution, and does not imply any other properties. This interface can be
    used to determine the full subscription of the execution instances of the
    running process that invokded the FleCSI runtime.
   */

  std::size_t threads() const;

  /*!
    Return the current task depth within the execution hierarchy. The
    top-level task has depth \em 0. This interface is primarily intended
    for FleCSI developers to use in enforcing runtime constraints.
   */

  static std::size_t task_depth();

  /*!
    Get the color of this process.
   */

  std::size_t color() const;

  /*!
    Get the number of colors.
   */

  std::size_t colors() const;
#endif

  using top_level_action_t = std::function<int(int, char **)>;

  /*!
    Set the top-level action.
   */

  bool register_top_level_action(top_level_action_t tla) {
    top_level_action_ = tla;
    return true;
  } // register_top_level_action

  /*!
    Return the top-level action.
   */

  top_level_action_t & top_level_action() {
    return top_level_action_;
  } // top_level_action

  /*!
    Return the exit status of the FleCSI runtime.
   */

  int & exit_status() {
    return exit_status_;
  }

  /*--------------------------------------------------------------------------*
    Reduction interface.
   *--------------------------------------------------------------------------*/

  bool register_reduction_operation(size_t key,
    const std::function<void()> & callback) {
    reduction_registry_[key] = callback;
    return true;
  } // register_reduction_operation

  std::map<size_t, std::function<void()>> & reduction_registry() {
    return reduction_registry_;
  } // reduction_registry

  /*--------------------------------------------------------------------------*
    Function interface.
   *--------------------------------------------------------------------------*/

  /*!
    Register a funtion with the runtime. Internally, this interface is
    used both for user-defined functions, and for task registration for MPI.

    @tparam KEY       A hash key identifying the function.
    @tparam RETURN    The return type of the function.
    @tparam ARG_TUPLE A std::tuple of the function argument types.
    @tparam FUNCTION  A pointer to the function.

    @return a boolean indicating the succes or failure of the registration.
   */

  template<size_t KEY,
    typename RETURN,
    typename ARG_TUPLE,
    RETURN (*FUNCTION)(ARG_TUPLE)>
  bool register_function() {
    flog_assert(function_registry_.find(KEY) == function_registry_.end(),
      "function has already been registered");

    // clang-format off
    flog_devel(info) << "Registering function" << std::endl <<
      "\thash: " << KEY << std::endl <<
      "\taddress: " << reinterpret_cast<std::size_t>(FUNCTION) << std::endl <<
      "\treturn type: " <<
        utils::demangle(typeid(RETURN).name()) << std::endl <<
      "\ttuple type: " <<
        utils::demangle(typeid(ARG_TUPLE).name()) << std::endl;
    // clang-format on

    function_registry_[KEY] = reinterpret_cast<void *>(FUNCTION);
    return true;
  } // register_function

  /*--------------------------------------------------------------------------*
    Topology interface.
   *--------------------------------------------------------------------------*/

  /*!
    Return a boolean indicating whether or not the given instance of
    a data topology has had its internal fields registered with the
    data model.

    @param topology_type_identifier Topology type identifier.
    @param instance_identifier      Instance identifier.
   */

  bool topology_fields_registered(size_t type_key, size_t instance_key) {
    return !registered_topology_fields_
              .insert(std::make_pair(type_key, instance_key))
              .second;
  } // topology_fields_registered

  /*--------------------------------------------------------------------------*
    Field interface.
   *--------------------------------------------------------------------------*/

  /*!
    Register field information.

    @param topology_type_identifier Topology type identifier.
    @param storage_class            Storage class identifier.
    @param field_info               Field information.
   */

  void add_field_info(size_t topology_type_identifier,
    size_t storage_class,
    const data::field_info_t & field_info,
    size_t key) {
    flog_devel(info) << "Registering field info (context)" << std::endl
                     << "\ttopology type identifier: "
                     << topology_type_identifier << std::endl
                     << "\tstorage class: " << storage_class << std::endl;
    topology_field_info_map_[topology_type_identifier][storage_class]
      .add_field_info(field_info, key);
  } // add_field_information

  /*!
    Return the stored field info for the given topology type and storage class.

    @param topology_type_identifier Topology type identifier.
    @param storage_class            Storage class identifier.
   */

  field_info_store_t const &
  get_field_info_store(size_t topology_type_identifier, size_t storage_class) {
    return topology_field_info_map_[topology_type_identifier][storage_class];
  } // get_field_info_store

  /*!
    Return the stored field info for the given topology type and storage class.
    Const version.

    @param topology_type_identifier Topology type identifier.
    @param storage_class            Storage class identifier.
   */

  field_info_store_t const & get_field_info_store(
    size_t topology_type_identifier,
    size_t storage_class) const {

    flog_devel(info) << "Type identifier: " << topology_type_identifier
                     << std::endl;

    auto const & tita = topology_field_info_map_.find(topology_type_identifier);
    flog_assert(tita != topology_field_info_map_.end(),
      "topology lookup failed for " << topology_type_identifier);

    auto const & sita = tita->second.find(storage_class);
    flog_assert(sita != tita->second.end(),
      "storage class lookup failed for " << storage_class);

    return sita->second;
  } // get_field_info_store

  /*!
FIXME: Do we need to make this general for other topology types? This is
currently only for unstructured mesh topologies.

    Add an index subspace to the specified index space with the specified
    capcity.

    @param index_space The parent index space.
    @param capacity    The maximum size of the subspace in indices.
   */

  void add_index_subspace(size_t index_subspace, size_t capacity) {
    index_subspace_info_t info;
    info.index_subspace = index_subspace;
    info.capacity = capacity;

    index_subspace_map_.emplace(index_subspace, std::move(info));
  }

  /*!
FIXME: Do we need to make this general for other topology types? This is
currently only for unstructured mesh topologies.

    Add an index subspace struct instance.

    @param info An initialized instance of index_subspace_info_t.
   */

  void add_index_subspace(const index_subspace_info_t & info) {
    index_subspace_map_.emplace(info.index_subspace, info);
  }

  /*!
FIXME: Do we need to make this general for other topology types? This is
currently only for unstructured mesh topologies.

    Return the map of index subspaces.
   */

  std::map<size_t, index_subspace_info_t> & index_subspace_info() {
    return index_subspace_map_;
  }

  /*--------------------------------------------------------------------------*
    Task Launch iterface.
   *--------------------------------------------------------------------------*/

  /*!
    Set the number of indices for the associated launch identifier.

    @param hash    A hash key identifies the launch domain.
    @param indices The size to set the launch domain.
   */

  void set_launch_domain_size(const size_t hash, size_t indices) {
    launch_domain_map_[hash] = indices;
  }

  /*!
    Returns domain information from the domain key
   */

  size_t get_launch_domain_size(const size_t hash) {
    return launch_domain_map_[hash];
  }

  /*!
    Return the count of executed tasks. Const version.
   */

  size_t const & tasks_executed() const {
    return tasks_executed_;
  } // tasks_executed

  /*!
    Return the count of executed tasks.
   */

  size_t & tasks_executed() {
    return tasks_executed_;
  } // tasks_executed

protected:
  context() = default;

private:
#ifdef DOXYGEN
  /*
    Clear the runtime state of the context.

    Notes:
      - This does not clear objects that cannot be serialized, e.g.,
        std::function objects.
   */

  void clear();
#endif

  /*--------------------------------------------------------------------------*
    Basic runtime data members.
   *--------------------------------------------------------------------------*/

  int exit_status_ = 0;
  top_level_action_t top_level_action_ = {};

  /*--------------------------------------------------------------------------*
    Reduction data members.
   *--------------------------------------------------------------------------*/

  std::map<size_t, std::function<void()>> reduction_registry_;

  /*--------------------------------------------------------------------------*
    Function data members.
   *--------------------------------------------------------------------------*/

  std::unordered_map<size_t, void *> function_registry_;

  /*--------------------------------------------------------------------------*
    Topology data members.
   *--------------------------------------------------------------------------*/

  std::set<std::pair<size_t, size_t>> registered_topology_fields_;

  /*--------------------------------------------------------------------------*
    Field data members.
   *--------------------------------------------------------------------------*/

  /*
    This type allows storage of runtime field information per topology type.
    The size_t key is the topology type hash.
   */

  std::unordered_map<size_t, field_info_map_t> topology_field_info_map_;

  /*
    This type allows storage of runtime index subspace information.
    The size_t key is the index subspace identifier.
   */

  std::map<size_t, index_subspace_info_t> index_subspace_map_;

  /*--------------------------------------------------------------------------*
    Launch data members.
   *--------------------------------------------------------------------------*/

  launch_domain_map_t launch_domain_map_;

  /*--------------------------------------------------------------------------*
    Task count.
   *--------------------------------------------------------------------------*/

  size_t tasks_executed_ = 0;

}; // struct context

} // namespace flecsi::runtime
