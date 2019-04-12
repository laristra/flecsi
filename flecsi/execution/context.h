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
#pragma once

/*! @file */

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include <flecsi/data/common/field_info.h>
#include <flecsi/execution/global_object_wrapper.h>
#include <flecsi/runtime/types.h>
#include <flecsi/utils/demangle.h>
#include <flecsi/utils/flog.h>
#endif

#include <boost/program_options.hpp>

#include <cassert>
#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <unordered_map>
#include <utility>

flog_register_tag(context);

namespace flecsi {
namespace execution {

using namespace boost::program_options;

/*!
  The context_u type provides a high-level execution context interface that
  is implemented by the given context policy.

  @tparam CONTEXT_POLICY The backend context policy.

  @ingroup execution
 */

template<class CONTEXT_POLICY>
struct context_u : public CONTEXT_POLICY {

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

  /*--------------------------------------------------------------------------*
    Deleted contructor and assignment interfaces.
   *--------------------------------------------------------------------------*/

  context_u(const context_u &) = delete;
  context_u & operator=(const context_u &) = delete;
  context_u(context_u &&) = delete;
  context_u & operator=(context_u &&) = delete;

  /*!
    Meyer's singleton instance.
   */

  static context_u & instance() {
    static context_u context;
    return context;
  } // instance

  /*--------------------------------------------------------------------------*
    Runtime interface.
   *--------------------------------------------------------------------------*/

  /*!
    Start the FleCSI runtime.

    @param argc The number of command-line arguments.
    @param argv The command-line arguments in a char **.

    @return An integer with \em 0 being success, and any other value
            being failure.
   */

  int start(int argc, char ** argv, variables_map & vm) {
    return CONTEXT_POLICY::start(argc, argv, vm);
  } // start

  // FIXME

  size_t rank() const {
    return CONTEXT_POLICY::rank();
  }

  // FIXME

  size_t size() const {
    return CONTEXT_POLICY::size();
  }

  // FIXME

  size_t colors_per_rank() const {
    return CONTEXT_POLICY::colors_per_rank();
  } // colors_per_shared

  /*!
    Return the current task depth within the execution hierarchy. The
    top-level task has depth \em 0. This interface is primarily intended
    for FleCSI developers to use in enforcing runtime constraints.
   */

  static size_t task_depth() {
    return CONTEXT_POLICY::task_depth();
  } // task_level

  /*!
    Get the color of this process.
   */

  size_t color() const {
    return CONTEXT_POLICY::color();
  } // color

  /*!
    Get the number of colors.
   */

  size_t colors() const {
    return CONTEXT_POLICY::colors();
  } // colors

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
    Global object interface.
   *--------------------------------------------------------------------------*/

  /*!
    Add a global object to the context. Global objects cannot be added
    from within a task. Attempts to do so will generate a runtime error.

    The intent of the global object interface is to allow users
    to use normal C++ inheritance patterns (with virtual functions)
    for \b globally-constant objects within the FleCSI task system.
    That said, this interface should be used with care, e.g., it is
    expensive to add global objects because it requires synchronization
    across runtime threads. Additionally, this interface also uses simple
    type erasure so that all added global objects can be stored in a
    single unordered map. Therefore, the user is responsible for
    correctly specifying the OBJECT_TYPE parameter to this method,
    and to the \ref get_global_object method.

    In normal usage, the user should call this method with a derived
    type, and the \ref get_global_object method should be called with
    a base type (assuming a common base type for all global objects
    added within the NAMESPACE).

    @tparam NAMESPACE   A unique hash that identifies the object.
    @tparam OBJECT_TYPE The C++ type of the object.

    @param index The index of the global object within the given namespace.
    @param args  A variadic argument list to be passed to the constructor
                 of the global object.

    @return A pointer to the newly allocated object.

    @note The global object instance will automatically be deleted by
          the runtime at shutdown.
   */

  template<size_t NAMESPACE, typename OBJECT_TYPE, typename... ARGS>
  OBJECT_TYPE * add_global_object(size_t index, ARGS &&... args) {
    size_t KEY = NAMESPACE ^ index;

    flog_assert(
      task_depth() == 0, "you cannot add global objects from within a task");

    flog_assert(
      global_object_registry_.find(KEY) == global_object_registry_.end(),
      "global key already exists");

    auto ptr = new OBJECT_TYPE(std::forward<ARGS>(args)...);

    flog(internal) << "Adding global object" << std::endl
                   << "\tindex: " << index << std::endl
                   << "\thash: " << NAMESPACE << std::endl
                   << "\ttype: " << utils::demangle(typeid(OBJECT_TYPE).name())
                   << std::endl
                   << "\taddress: " << ptr << std::endl;

    std::get<0>(global_object_registry_[KEY]) =
      reinterpret_cast<uintptr_t>(ptr);

    using wrapper_t = global_object_wrapper_u<OBJECT_TYPE>;
    std::get<1>(global_object_registry_[KEY]) = &wrapper_t::cleanup;

    return ptr;
  } // add_global_object

  /*!
    Get a global object instance.

    @tparam NAMESPACE   A unique hash that identifies the object.
    @tparam OBJECT_TYPE The C++ type of the object.

    @param index The index of the global object within the given namespace.

    @return A pointer to the object.
   */

  template<size_t NAMESPACE, typename OBJECT_TYPE>
  OBJECT_TYPE * get_global_object(size_t index) {
    size_t KEY = NAMESPACE ^ index;
    flog_assert(
      global_object_registry_.find(KEY) != global_object_registry_.end(),
      "key does not exist");

    auto ptr = reinterpret_cast<OBJECT_TYPE *>(
      std::get<0>(global_object_registry_[KEY]));

    flog(internal) << "Getting global object" << std::endl
                   << "\tindex: " << index << std::endl
                   << "\thash: " << NAMESPACE << std::endl
                   << "\ttype: " << utils::demangle(typeid(OBJECT_TYPE).name())
                   << std::endl
                   << "\taddress: " << ptr << std::endl;

    return ptr;
  } // get_global_object

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
    flog(internal) << "Registering function" << std::endl <<
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
    Register a topology with the runtime.

    @param topology_identifier The topology type indentifier.
    @param instance_identifier The topology instance identifier.
    @param callback            The registration call back function.
   */

  bool register_topology(size_t topology_identifier,
    size_t instance_identifier,
    const topology_registration_function_t & callback) {
#if 0
    if(topology_callback_registry_.find(topology_identifier) !=
       topology_callback_registry_.end()) {
      flog_assert(topology_callback_registry_[topology_identifier].find(
                    instance_identifier) ==
                    topology_callback_registry_[topology_identifier].end(),
        "topology key already exists");
    } // if

    topology_callback_registry_[topology_identifier][instance_identifier] =
      std::make_pair(unique_fid_t::instance().next(), callback);

#endif
    return true;
  } // register_topology

  /*!
    Return the topology registry.
   */

  std::unordered_map<size_t, topology_registration_map_t> &
  topology_registry() {
    return topology_callback_registry_;
  } // topology_registry

  /*!
    Return a boolean indicating whether or not the given instance of
    a data topology has had its internal fields registered with the
    data model.

    @param type_key     The hash key for the topology type.
    @param instance_key The hash key for the topology instance.
   */

  bool topology_fields_registered(size_t type_key, size_t instance_key) {
    fixme() << "Do we really need this method?" << std::endl;
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
    @param fi                       Field information.
   */

  void register_field_info(size_t topology_type_identifier,
    size_t storage_class,
    const data::field_info_t & fi) {
    field_info_map_[topology_type_identifier][storage_class].emplace_back(fi);
  } // register_field_information

private:
  /*--------------------------------------------------------------------------*
    Singleton.
   *--------------------------------------------------------------------------*/

  context_u() : CONTEXT_POLICY() {}

  ~context_u() {
    // Cleanup the global objects
    for(auto & go : global_object_registry_) {
      std::get<1>(go.second)(std::get<0>(go.second));
    } // for
  } // ~context_u

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
    Global object data members.
   *--------------------------------------------------------------------------*/

  using global_object_data_t =
    std::pair<uintptr_t, std::function<void(uintptr_t)>>;

  std::unordered_map<size_t, global_object_data_t> global_object_registry_;

  /*--------------------------------------------------------------------------*
    Function data members.
   *--------------------------------------------------------------------------*/

  std::unordered_map<size_t, void *> function_registry_;

  /*--------------------------------------------------------------------------*
    Topology data members.
   *--------------------------------------------------------------------------*/

  std::unordered_map<size_t, topology_registration_map_t>
    topology_callback_registry_;

  // FIXME?
  std::set<std::pair<size_t, size_t>> registered_topology_fields_;

  /*--------------------------------------------------------------------------*
    Field data members.
   *--------------------------------------------------------------------------*/

  /*!
    This is the map of registration callback functions for fields. The map is
    keyed off of topology type.
   */

  std::unordered_map<size_t, field_registration_map_t> field_callback_registry_;

  /*!
    This type allows the storage of field information per storage class. The
    size_t key is the storage class.
   */

  using field_info_vector_t = std::vector<data::field_info_t>;
  using field_info_t = std::unordered_map<size_t, field_info_vector_t>;

  /*!
    This type allows storage of runtime field information per topology type.
    The size_t key is the topology type hash.
   */

  std::unordered_map<size_t, field_info_t> field_info_map_;

#if 0
  // handle state
  using data_reference_map_t = std::unordered_map<size_t, data_reference_state_t>;
  std::unordered_map<std::pair<size_t, size_t>, std::unordered_map<size_t, data_reference_map_t>>

  std::unordered_map<std::pair<size_t, size_t>, std::unordered_map<std::pair<size_t, size_t>, data_reference_state_t>>;
#endif
}; // struct context_u

} // namespace execution
} // namespace flecsi

// This include file defines the FLECSI_RUNTIME_CONTEXT_POLICY used below.

#include <flecsi/runtime/context_policy.h>

namespace flecsi {
namespace execution {

/*!
  The context_t type is the high-level interface to the FleCSI execution
  context.

  @ingroup execution
 */

using context_t = context_u<FLECSI_RUNTIME_CONTEXT_POLICY>;

} // namespace execution
} // namespace flecsi
