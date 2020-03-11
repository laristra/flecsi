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

#include "flecsi/data/storage_classes.hh"
#include "flecsi/topology/core.hh"
#include <flecsi/data/field_info.hh>
#include <flecsi/execution/task_attributes.hh>
#include <flecsi/runtime/types.hh>
#include <flecsi/utils/flog.hh>

#include <boost/program_options.hpp>

#include <cassert>
#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

flog_register_tag(context);

namespace flecsi::runtime {

struct context_t; // supplied by backend

enum status : int {
  success,
  help,
  command_line_error,
  error, // add specific error modes
}; // initialization_codes

/*!
  The context type provides a high-level execution context interface that
  is implemented by a specific backend.

  @ingroup runtime
 */

struct context {

  /*--------------------------------------------------------------------------*
    Public types.
   *--------------------------------------------------------------------------*/

  /*!
    This type allows the storage of field information per storage class. The
    size_t key is the storage class.
   */

  using field_info_store_t = data::fields;
  using field_info_map_t =
    std::unordered_map<size_t, std::vector<field_info_store_t>>;

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
    Program options interface.
   *--------------------------------------------------------------------------*/

  std::vector<char *> & argv() {
    return argv_;
  }

  std::string & flog_tags() {
    return flog_tags_;
  }

  int & flog_verbose() {
    return flog_verbose_;
  }

  int64_t & flog_output_process() {
    return flog_output_process_;
  }

  /*
    The boolean return is necessary for assignment so that this code block is
    executed at namespace scope.
   */

  template<typename... ARGS>
  bool add_program_option(std::string const & label, ARGS &&... args) {
    auto ita = descriptions_map_.find(label);

    if(ita == descriptions_map_.end()) {
      boost::program_options::options_description desc(label.c_str());
      descriptions_map_.emplace(label, desc);
    } // if

    descriptions_map_[label].add_options()(std::forward<ARGS>(args)...);
    return true;
  }

  boost::program_options::variables_map const &
  program_options_variables_map() {
    flog_assert(program_options_initialized_,
      "unitialized program options -> "
      "invoke flecsi::initialize_program_options");
    return variables_map_;
  }

  std::vector<std::string> const & unrecognized_options() {
    flog_assert(program_options_initialized_,
      "unitialized program options -> "
      "invoke flecsi::initialize_program_options");
    return unrecognized_options_;
  }

  /*--------------------------------------------------------------------------*
    Runtime interface.
   *--------------------------------------------------------------------------*/

  inline int initialize_generic(int argc, char ** argv, bool dependent) {

    initialize_dependent_ = dependent;

    // Save command-line arguments
    for(auto i(0); i < argc; ++i) {
      argv_.push_back(argv[i]);
    } // for

    std::string program(argv[0]);
    program = "Basic Options (" + program.substr(program.rfind('/') + 1) + ")";

    boost::program_options::options_description master(program);
    master.add_options()("help,h", "Print this message and exit.");

    // Add all of the descriptions to the main description
    for(auto & od : descriptions_map_) {
      master.add(od.second);
    } // for

    boost::program_options::parsed_options parsed =
      boost::program_options::command_line_parser(argc, argv)
        .options(master)
        .allow_unregistered()
        .run();

    try {
      boost::program_options::store(parsed, variables_map_);

      if(variables_map_.count("help")) {
        if(process_ == 0) {
          std::cout << master << std::endl;
        } // if

        return status::help;
      } // if

      boost::program_options::notify(variables_map_);
    }
    catch(boost::program_options::error & e) {
      std::cerr << FLOG_COLOR_LTRED << "ERROR: " << FLOG_COLOR_RED << e.what()
                << "!!!" << FLOG_COLOR_PLAIN << std::endl
                << std::endl;
      std::cerr << master << std::endl;
      return status::command_line_error;
    } // try

    unrecognized_options_ = boost::program_options::collect_unrecognized(
      parsed.options, boost::program_options::include_positional);

    program_options_initialized_ = true;

#if defined(FLECSI_ENABLE_FLOG)
    if(flog_tags_ == "0") {
      if(process_ == 0) {
        std::cout << "Available tags (FLOG):" << std::endl;

        for(auto t : flog_tag_map()) {
          std::cout << " " << t.first << std::endl;
        } // for
      } // if

      return status::help;
    } // if

    if(flog_initialize(flog_tags_, flog_verbose_, flog_output_process_)) {
      return status::error;
    } // if
#endif

#if defined(FLECSI_ENABLE_KOKKOS)
    if(initialize_dependent_) {
      // Need to capture status from this
      Kokkos::initialize(argc, argv);
    } // if
#endif

    return status::success;
  } // initialize_generic

  inline void finalize_generic() {
#if defined(FLECSI_ENABLE_FLOG)
    flog_finalize();
#endif

    if(initialize_dependent_) {
#if defined(FLECSI_ENABLE_KOKKOS)
      Kokkos::finalize();
#endif
    } // if
  } // finalize_generic

#ifdef DOXYGEN // these functions are implemented per-backend
  /*
    Documented in execution.hh
   */

  int initialize(int argc, char ** argv, bool dependent);

  /*!
    Perform FleCSI runtime finalization. If FleCSI was initialized with
    the \em dependent flag set to true, FleCSI will also finalize any runtimes
    on which it depends.
   */

  void finalize();

  /*!
    Start the FleCSI runtime.

    @param action The top-level action FleCSI should execute.

    @return An integer with \em 0 being success, and any other value
            being failure.
   */

  int start(const std::function<int(int, char **)> action &);

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

  /*!
    Return the exit status of the FleCSI runtime.
   */

  int & exit_status() {
    return exit_status_;
  }

  /*--------------------------------------------------------------------------*
    Reduction interface.
   *--------------------------------------------------------------------------*/

  void register_reduction_operation(void callback()) {
    reduction_registry_.push_back(callback);
  } // register_reduction_operation

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

    \tparam Topo topology type
    \tparam Index topology-relative index space
    @param storage_class            Storage class identifier.
    @param field_info               Field information.
   */
  template<class Topo, std::size_t Index = 0>
  void add_field_info(data::storage_label_t storage_class,
    const data::field_info_t & field_info) {
    constexpr std::size_t NIndex = topology::index_spaces<Topo>;
    static_assert(Index < NIndex, "No such index space");
    topology_field_info_map_[topology::id<Topo>()]
      .try_emplace(storage_class, NIndex)
      .first->second[Index]
      .push_back(&field_info);
  } // add_field_information

  /*!
    Return the stored field info for the given topology type and storage class.
    Const version.

    \tparam Topo topology type
    \tparam Index topology-relative index space
    @param storage_class            Storage class identifier.
   */
  template<class Topo, std::size_t Index = 0>
  field_info_store_t const & get_field_info_store(
    data::storage_label_t storage_class) const {
    static_assert(Index < topology::index_spaces<Topo>, "No such index space");

    static const field_info_store_t empty;

    auto const & tita = topology_field_info_map_.find(topology::id<Topo>());
    if(tita == topology_field_info_map_.end())
      return empty;

    auto const & sita = tita->second.find(storage_class);
    if(sita == tita->second.end())
      return empty;

    return sita->second[Index];
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
    Program options data members.
   *--------------------------------------------------------------------------*/

  std::vector<char *> argv_;

  std::string flog_tags_;
  int flog_verbose_;
  int64_t flog_output_process_;

  bool initialize_dependent_ = true;
  bool program_options_initialized_ = false;
  std::map<std::string, boost::program_options::options_description>
    descriptions_map_;
  boost::program_options::variables_map variables_map_;
  std::vector<std::string> unrecognized_options_;

  /*--------------------------------------------------------------------------*
    Basic runtime data members.
   *--------------------------------------------------------------------------*/

  size_t process_ = std::numeric_limits<size_t>::max();
  size_t processes_ = std::numeric_limits<size_t>::max();
  size_t threads_per_process_ = std::numeric_limits<size_t>::max();
  size_t threads_ = std::numeric_limits<size_t>::max();

  int exit_status_ = 0;

  /*--------------------------------------------------------------------------*
    Reduction data members.
   *--------------------------------------------------------------------------*/

  std::vector<void (*)()> reduction_registry_;

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
    Task count.
   *--------------------------------------------------------------------------*/

  size_t tasks_executed_ = 0;

}; // struct context

} // namespace flecsi::runtime
