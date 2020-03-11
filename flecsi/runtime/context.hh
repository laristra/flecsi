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

#include "flecsi/data/layout.hh"
#include "flecsi/topology/core.hh"
#include <flecsi/data/field_info.hh>
#include <flecsi/execution/task_attributes.hh>
#include <flecsi/flog.hh>
#include <flecsi/runtime/types.hh>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <any>
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

namespace flecsi {

inline flog::devel_tag context_tag("context");

namespace runtime {

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

  using field_info_store_t = data::fields;

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

  /*
    Meyer's singleton instance.
   */

  static inline context_t & instance();

  bool initialized() {
    return initialized_;
  }

  /*--------------------------------------------------------------------------*
    Program options interface.
   *--------------------------------------------------------------------------*/

  boost::program_options::positional_options_description &
  positional_description() {
    return positional_desc_;
  }

  std::map<std::string, std::string> & positional_help() {
    return positional_help_;
  }

  boost::program_options::options_description & hidden_options() {
    return hidden_options_;
  }

  std::map<std::string,
    std::pair<bool, std::function<bool(boost::any const &)>>> &
  option_checks() {
    return option_checks_;
  }

  std::vector<char *> & argv() {
    return argv_;
  }

  auto & descriptions_map() {
    return descriptions_map_;
  }

  std::vector<std::string> const & unrecognized_options() {
    flog_assert(initialized_,
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
    program = program.substr(program.rfind('/') + 1);

    boost::program_options::options_description master("Basic Options");
    master.add_options()("help,h", "Print this message and exit.");

    // Add all of the user-defined descriptions to the main description
    for(auto & od : descriptions_map_) {
      master.add(od.second);
    } // for

    boost::program_options::options_description flecsi_desc("FleCSI Options");
#if defined(FLECSI_ENABLE_FLOG)
    // Add FleCSI options
    flecsi_desc.add_options() // clang-format off
      (
        "flog-tags",
        boost::program_options::value(&flog_tags_)
          ->default_value("all"),
        "Enable the specified output tags, e.g., --flog-tags=tag1,tag2."
      )
      (
        "flog-verbose",
        boost::program_options::value(&flog_verbose_)
          ->implicit_value(1)
          ->default_value(0),
        "Enable verbose output. Passing '-1' will strip any additional"
        " decorations added by flog and will only output the user's message."
      )
      (
        "flog-process",
        boost::program_options::value(&flog_output_process_)->default_value(-1),
        "Restrict output to the specified process id."
      ); // clang-format on
#endif

    // Make an options description to hold all options. This is useful
    // to exlude hidden options from help.
    boost::program_options::options_description all("All Options");
    all.add(master);
    all.add(flecsi_desc);
    all.add(hidden_options_);

    boost::program_options::parsed_options parsed =
      boost::program_options::command_line_parser(argc, argv)
        .options(all)
        .positional(positional_desc_)
        .allow_unregistered()
        .run();

    auto print_usage = [this](std::string program,
                         auto const & master,
                         auto const & flecsi) {
      if(process_ == 0) {
        std::cout << "Usage: " << program << " ";

        size_t positional_count = positional_desc_.max_total_count();
        size_t max_label_chars = std::numeric_limits<size_t>::min();

        for(size_t i{0}; i < positional_count; ++i) {
          std::cout << "<" << positional_desc_.name_for_position(i) << "> ";

          const size_t size = positional_desc_.name_for_position(i).size();
          max_label_chars = size > max_label_chars ? size : max_label_chars;
        } // for

        max_label_chars += 2;

        std::cout << std::endl << std::endl;

        if(positional_count) {
          std::cout << "Positional Options:" << std::endl;

          for(size_t i{0}; i < positional_desc_.max_total_count(); ++i) {
            auto const & name = positional_desc_.name_for_position(i);
            auto help = positional_help_.at(name);
            std::cout << "  " << name << " ";
            std::cout << std::string(max_label_chars - name.size() - 2, ' ');

            if(help.size() > 78 - max_label_chars) {
              std::string first = help.substr(
                0, help.substr(0, 78 - max_label_chars).find_last_of(' '));
              std::cout << first << std::endl;
              help =
                help.substr(first.size() + 1, help.size() - first.size() + 1);

              while(help.size() > 78 - max_label_chars) {
                std::string part = help.substr(
                  0, help.substr(0, 78 - max_label_chars).find_last_of(' '));
                std::cout << std::string(max_label_chars + 1, ' ') << part
                          << std::endl;
                help = help.substr(part.size() + 1, help.size() - part.size());
              } // while

              std::cout << std::string(max_label_chars + 1, ' ') << help
                        << std::endl;
            }
            else {
              std::cout << help << std::endl;
            } // if

          } // for

          std::cout << std::endl;
        } // if

        std::cout << master << std::endl;
        std::cout << flecsi << std::endl;

#if defined(FLECSI_ENABLE_FLOG)
        auto const & tm = flog::flog_t::instance().tag_map();

        if(tm.size()) {
          std::cout << "Available FLOG Tags (FleCSI Logging Utility):"
                    << std::endl;
        } // if

        for(auto t : tm) {
          std::cout << "  " << t.first << std::endl;
        } // for
#endif
      } // if
    }; // print_usage

    try {
      boost::program_options::variables_map vm;
      boost::program_options::store(parsed, vm);

      if(vm.count("help")) {
        print_usage(program, master, flecsi_desc);
        return status::help;
      } // if

      boost::program_options::notify(vm);

      // Call option check methods
      for(auto const & [name, boost_any] : vm) {
        auto const & ita = option_checks_.find(name);
        if(ita != option_checks_.end()) {
          auto const & [positional, check] = ita->second;

          if(!check(boost_any.value())) {
            std::string dash = positional ? "" : "--";
            std::cerr << FLOG_COLOR_LTRED << "ERROR: " << FLOG_COLOR_RED
                      << "invalid argument for '" << dash << name
                      << "' option!!!" << FLOG_COLOR_PLAIN << std::endl;
            print_usage(program, master, flecsi_desc);
            return status::help;
          } // if
        } // if
      } // for
    }
    catch(boost::program_options::error & e) {
      std::string error(e.what());

      auto pos = error.find("--");
      if(pos != std::string::npos) {
        error.replace(pos, 2, "");
      } // if

      std::cerr << FLOG_COLOR_LTRED << "ERROR: " << FLOG_COLOR_RED << error
                << "!!!" << FLOG_COLOR_PLAIN << std::endl
                << std::endl;
      print_usage(program, master, flecsi_desc);
      return status::command_line_error;
    } // try

    unrecognized_options_ = boost::program_options::collect_unrecognized(
      parsed.options, boost::program_options::include_positional);

#if defined(FLECSI_ENABLE_FLOG)
    if(flog::flog_t::instance().initialize(
         flog_tags_, flog_verbose_, flog_output_process_)) {
      return status::error;
    } // if
#endif

#if defined(FLECSI_ENABLE_KOKKOS)
    if(initialize_dependent_) {
      // Need to capture status from this
      Kokkos::initialize(argc, argv);
    } // if
#endif

    initialized_ = true;

    return status::success;
  } // initialize_generic

  inline void finalize_generic() {
#if defined(FLECSI_ENABLE_FLOG)
    flog::flog_t::instance().finalize();
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
    @param field_info               Field information.
   */
  template<class Topo, std::size_t Index = 0>
  void add_field_info(const data::field_info_t & field_info) {
    constexpr std::size_t NIndex = topology::index_spaces<Topo>;
    static_assert(Index < NIndex, "No such index space");
    topology_field_info_map_.try_emplace(topology::id<Topo>(), NIndex)
      .first->second[Index]
      .push_back(&field_info);
  } // add_field_information

  /*!
    Return the stored field info for the given topology type and layout.
    Const version.

    \tparam Topo topology type
    \tparam Index topology-relative index space
   */
  template<class Topo, std::size_t Index = 0>
  field_info_store_t const & get_field_info_store() const {
    static_assert(Index < topology::index_spaces<Topo>, "No such index space");

    static const field_info_store_t empty;

    auto const & tita = topology_field_info_map_.find(topology::id<Topo>());
    if(tita == topology_field_info_map_.end())
      return empty;

    return tita->second[Index];
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

  // Option Descriptions
  std::map<std::string, boost::program_options::options_description>
    descriptions_map_;

  // Positional options
  boost::program_options::positional_options_description positional_desc_;
  boost::program_options::options_description hidden_options_;
  std::map<std::string, std::string> positional_help_;

  // Validation functions
  std::map<std::string,
    std::pair<bool, std::function<bool(boost::any const &)>>>
    option_checks_;

  std::vector<std::string> unrecognized_options_;

  /*--------------------------------------------------------------------------*
    Basic runtime data members.
   *--------------------------------------------------------------------------*/

  bool initialized_ = false;
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
    The size_t key is the topology ID; the vector index is the index space.
   */

  std::unordered_map<size_t, std::vector<field_info_store_t>>
    topology_field_info_map_;

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

} // namespace runtime
} // namespace flecsi
