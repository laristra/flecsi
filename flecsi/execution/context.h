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

#include <flecsi/execution/global_object_wrapper.h>
#include <flecsi/runtime/types.h>
#include <flecsi/utils/common.h>
#include <flecsi/utils/demangle.h>
#include <flecsi/utils/flog.h>

#include <cassert>
#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <unordered_map>

flog_register_tag(context);

namespace flecsi {
namespace execution {

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

  using client_registration_function_t = std::function<void(size_t)>;
  using client_value_t = std::pair<field_id_t, client_registration_function_t>;
  using client_entry_t = std::unordered_map<size_t, client_value_t>;

  using field_registration_function_t = std::function<void(size_t, size_t)>;
  using field_value_t = std::pair<field_id_t, field_registration_function_t>;
  using field_entry_t = std::unordered_map<size_t, field_value_t>;

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

  int start(int argc, char ** argv) {
    return CONTEXT_POLICY::start(argc, argv);
  } // start

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
  } // color

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
    added within the NAMESPACE_HASH).

    @tparam NAMESPACE_HASH A unique hash that identifies the object.
    @tparam OBJECT_TYPE    The C++ type of the object.

    @param index The index of the global object within the given namespace.
    @param args  A variadic argument list to be passed to the constructor
                 of the global object.

    @return A pointer to the newly allocated object.

    @note The global object instance will automatically be deleted by
          the runtime at shutdown.
   */

  template<size_t NAMESPACE_HASH, typename OBJECT_TYPE, typename... ARGS>
  OBJECT_TYPE * add_global_object(size_t index, ARGS &&... args) {
    size_t KEY = NAMESPACE_HASH ^ index;

    flog_assert(
      global_object_registry_.find(KEY) == global_object_registry_.end(),
      "global key already exists");

    auto ptr = new OBJECT_TYPE(std::forward<ARGS>(args)...);

    flog(internal) << "Adding global object" << std::endl <<
      "\tindex: " << index << std::endl <<
      "\thash: " << NAMESPACE_HASH << std::endl <<
      "\ttype: " << utils::demangle(typeid(OBJECT_TYPE).name()) << std::endl <<
      "\taddress: " << ptr << std::endl;

    std::get<0>(global_object_registry_[KEY]) =
      reinterpret_cast<uintptr_t>(ptr);

    using wrapper_t = global_object_wrapper_u<OBJECT_TYPE>;
    std::get<1>(global_object_registry_[KEY]) = &wrapper_t::cleanup;

    return ptr;
  } // add_global_object

  /*!
    Get a global object instance.

    @tparam NAMESPACE_HASH A unique hash that identifies the object.
    @tparam OBJECT_TYPE    The C++ type of the object.

    @param index The index of the global object within the given namespace.

    @return A pointer to the object.
   */

  template<size_t NAMESPACE_HASH, typename OBJECT_TYPE>
  OBJECT_TYPE * get_global_object(size_t index) {
    size_t KEY = NAMESPACE_HASH ^ index;
    flog_assert(
      global_object_registry_.find(KEY) != global_object_registry_.end(),
      "key does not exist");

    auto ptr = reinterpret_cast<OBJECT_TYPE *>(
      std::get<0>(global_object_registry_[KEY]));

    flog(internal) << "Getting global object" << std::endl <<
      "\tindex: " << index << std::endl <<
      "\thash: " << NAMESPACE_HASH << std::endl <<
      "\ttype: " << utils::demangle(typeid(OBJECT_TYPE).name()) << std::endl <<
      "\taddress: " << ptr << std::endl;

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
    Client interface.
   *--------------------------------------------------------------------------*/

  /*!
    Register a data client with the runtime.

    @param type_hash  The data client indentifier hash.
    @param key        The identifier hash.
    @param callback   The registration call back function.
   */

  bool register_client(size_t type_hash,
    size_t key,
    const client_registration_function_t & callback) {
    if(client_registry_.find(type_hash) != client_registry_.end()) {
      flog_assert(client_registry_[type_hash].find(key) ==
                    client_registry_[type_hash].end(),
        "client key already exists");
    } // if

    client_registry_[type_hash][key] =
      std::make_pair(unique_fid_t::instance().next(), callback);

    return true;
  } // register_client

  /*!
    Return a boolean indicating whether or not the given instance of
    a data client has had its internal fields registered with the
    data model.

    @param type_key     The hash key for the data client type.
    @param instance_key The hash key for the data client instance.
   */

  bool client_fields_registered(size_t type_key, size_t instance_key) {
    return !registered_client_fields_
              .insert(std::make_pair(type_key, instance_key))
              .second;
  } // client_fields_registered

  /*--------------------------------------------------------------------------*
    Field interface.
   *--------------------------------------------------------------------------*/

  /*!
    Register a field with the runtime.

    @param client_type_key The data client indentifier hash.
    @param key             The identifier hash.
    @param callback        The registration call back function.
   */

  bool register_field(size_t client_type_key,
    size_t key,
    const field_registration_function_t & callback) {
    if(field_registry_.find(client_type_key) != field_registry_.end()) {
      if(field_registry_[client_type_key].find(key) !=
         field_registry_[client_type_key].end()) {
        flog(warn) << "field key already exists" << std::endl;
      } // if
    } // if

    field_registry_[client_type_key][key] =
      std::make_pair(unique_fid_t::instance().next(), callback);

    return true;
  } // register_field

private:
  /*--------------------------------------------------------------------------*
    Private types.
   *--------------------------------------------------------------------------*/

  using unique_fid_t = utils::unique_id_t<field_id_t, FLECSI_GENERATED_ID_MAX>;

  /*--------------------------------------------------------------------------*
    Singleton.
   *--------------------------------------------------------------------------*/

  context_u() : CONTEXT_POLICY() { std::cout << "constructor" << std::endl; }

  ~context_u() {
    std::cout << "destructor" << std::endl << std::flush;

    // Cleanup the global objects
    for(auto & go : global_object_registry_) {
      std::get<1>(go.second)(std::get<0>(go.second));
    } // for
  } // ~context_u

  /*--------------------------------------------------------------------------*
    Basic runtime data members.
   *--------------------------------------------------------------------------*/

  size_t color_ = 0;
  size_t colors_ = 0;
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
    Client members.
   *--------------------------------------------------------------------------*/

  std::unordered_map<size_t, client_entry_t> client_registry_;
  std::set<std::pair<size_t, size_t>> registered_client_fields_;

  /*--------------------------------------------------------------------------*
    Field members.
   *--------------------------------------------------------------------------*/

  std::unordered_map<size_t, field_entry_t> field_registry_;

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
