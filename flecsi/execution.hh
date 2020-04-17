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
#define __FLECSI_PRIVATE__
#endif

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/program_options.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include <flecsi/execution/backend.hh>
#include <flecsi/execution/launch.hh>
#include <flecsi/execution/reduction.hh>
#include <flecsi/execution/task_attributes.hh>
#include <flecsi/runtime/backend.hh>

/*----------------------------------------------------------------------------*
  Basic runtime interface
 *----------------------------------------------------------------------------*/

namespace flecsi {

/*!
  Perform FleCSI runtime initialization. If \em dependent is true, this call
  will also initialize any runtime on which FleCSI depends.

  @param argc      The number of command-line arguments.
  @param argv      The command-line arguments.
  @param dependent A boolean telling FleCSI whether or not to initialize
                   runtimes on which it depends.

  @return An integer indicating the initialization status. This may be
          interpreted as a \em flecsi::runtime::status enumeration, e.g.,
          a value of 1 is equivalent to flecsi::runtime::status::help.
 */

inline int
initialize(int argc, char ** argv, bool dependent = true) {
  return runtime::context_t::instance().initialize(argc, argv, dependent);
}

/*!
  Perform FleCSI runtime start. This causes the runtime to begin execution
  of the top-level action.

  @return An integer indicating the finalization status. This will either
          be 0 for successful completion, or an error code from
          flecsi::runtime::status.
 */

inline int
start(const std::function<int(int, char **)> & action) {
  return runtime::context_t::instance().start(action);
}

/*!
  Perform FleCSI runtime finalization. If FleCSI was initialized with the \em
  dependent flag set to true, FleCSI will also finalize any runtimes on which
  it depends.
 */

inline void
finalize() {
  runtime::context_t::instance().finalize();
}

enum option_attribute : size_t {
  option_default,
  option_implicit,
  option_zero,
  option_multi
}; // enum

/*
  Auxilliary type for program options.
 */

using any = boost::any;

/*!
  Convert an option value into its underlying type.

  @tparam ValueType The option underlying value type.
 */

template<typename ValueType>
ValueType
option_value(any const & v) {
  return boost::any_cast<boost::optional<ValueType>>(v).value();
}

/*!
  The program_option type is a wrapper that implements a useful subset of
  Boost's Program Options utility. Creating an instance of this type at
  namespace scope will add a program option that can be queried after the
  \ref initialize function is called.
 */

template<typename ValueType>
struct program_option {

  struct initializer_value
    : public std::pair<option_attribute, boost::optional<ValueType>> {

    initializer_value(option_attribute key) : initializer_value::pair(key, {}) {
      flog_assert(key == option_zero || key == option_multi,
        "invalid constructor for initializer_value: "
          << (key == option_default ? "option_default" : "option_implicit")
          << " must take a value");
    }
    initializer_value(option_attribute key, ValueType value)
      : initializer_value::pair(key, value) {
      flog_assert(key == option_default || key == option_implicit,
        "invalid constructor for initializer_value: "
          << (key == option_zero ? "option_zero" : "option_multi")
          << " does not take a value");
    }
  };

  /*!
    Construct a program option.

    @param section The options description label.
    @param flag    The command-line option in long, short, or dual form,
                   e.g., "option,o" -> \em --option or \em -o.
    @param help    The help message for the option.
    @param values  Mechanism to set optional value attributes.  Supported keys
                   are \em option_default, \em option_implicit, \em
                   option_zero, and \em option_multi. If an \em option_default
                   value is specified, it will be used if the \em flag is not
                   passed to the command line. If an \em option_implicit value
                   is specified, it will be used if the flag is passed to the
                   command line without a value. If \em option_zero is passed,
                   the flag will not take any values, and must have an \em
                   option_implicit value specified. If \em option_multi is
                   passed, the flag will take multiple values.
    @param check   An optional, user-defined predicate to validate the option
                   passed by the user.

    @code
      program_option<int> my_flag("My Section",
        "flag,f",
        "Specify the flag [0-9].",
        {
          {option_default, 1},
          {option_implicit, 0}
        },
        [](flecsi::any const & v) {
          const int value = flecsi::option_value<int>(v);
          return value >= 0 && value < 10;
        });
    @endcode
   */

  program_option(const char * section,
    const char * flag,
    const char * help,
    std::initializer_list<initializer_value> values = {},
    std::function<bool(const any &)> const & check = default_check) {

    auto semantic_ = boost::program_options::value(&value_);

    bool zero{false}, implicit{false};
    for(auto const & v : values) {
      if(v.first == option_default) {
        semantic_->default_value(v.second);
      }
      else if(v.first == option_implicit) {
        semantic_->implicit_value(v.second);
        implicit = true;
      }
      else if(v.first == option_zero) {
        semantic_->zero_tokens();
        zero = true;
      }
      else if(v.first == option_multi) {
        semantic_->multitoken();
      }
      else {
        flog_fatal("invalid value type");
      } // if
    } // for

    if(zero) {
      flog_assert(implicit, "option_zero specified without option_implicit");
    } // if

    auto option =
      boost::make_shared<boost::program_options::option_description>(
        flag, semantic_, help);

    runtime::context_t::instance()
      .descriptions_map()
      .try_emplace(section, section)
      .first->second.add(option);

    std::string sflag(flag);
    sflag = sflag.substr(0, sflag.find(','));

    runtime::context_t::instance().option_checks().try_emplace(
      sflag, false, check);
  } // program_option

  /*!
    Construct a positional program option.

    @param name  The name for the positional option.
    @param help  The help message for the option.
    @param count The number of values to consume for this positional option. If
                 \em -1 is passed, this option will consume all remainging
                 values.
    @param check An optional, user-defined predicate to validate the option
                 passed by the user.
   */

  program_option(const char * name,
    const char * help,
    size_t count,
    std::function<bool(const any &)> const & check = default_check) {
    auto semantic_ = boost::program_options::value(&value_);
    semantic_->required();

    auto option =
      boost::make_shared<boost::program_options::option_description>(
        name, semantic_, help);

    runtime::context_t::instance().positional_description().add(name, count);
    runtime::context_t::instance().positional_help().try_emplace(name, help);
    runtime::context_t::instance().hidden_options().add(option);
    runtime::context_t::instance().option_checks().try_emplace(
      name, true, check);
  } // program_options

  ValueType value() const {
    return value_.value();
  }

  operator ValueType() const {
    return value();
  }

  bool has_value() const {
    return value_.has_value();
  }

private:
  static bool default_check(const boost::any &) {
    return true;
  }

  boost::optional<ValueType> value_{};

}; // struct program_option

/*!
  Return the current process id.
 */

inline size_t
process() {
  return runtime::context_t::instance().process();
}

/*!
  Return the number of processes.
 */

inline size_t
processes() {
  return runtime::context_t::instance().processes();
}

/*!
  Return the number of threads per process.
 */

inline size_t
threads_per_process() {
  return runtime::context_t::instance().threads_per_process();
}

/*!
  Return the number of execution instances with which the runtime was
  invoked. In this context a \em thread is defined as an instance of
  execution, and does not imply any other properties. This interface can be
  used to determine the full subscription of the execution instances of the
  running process that invokded the FleCSI runtime.
 */

inline size_t
threads() {
  return runtime::context_t::instance().threads();
}

/*!
  Return the color of the current execution instance. This function is only
  valid if invoked from within a task.
 */

inline size_t
color() {
  return runtime::context_t::instance().color();
}

/*!
  Return the number of colors of the current task invocation. This function is
  only valid if invoked from within a task.
 */

inline size_t
colors() {
  return runtime::context_t::instance().colors();
}

/*!
  Execute a reduction task.

  @tparam TASK                The user task.
  @tparam LAUNCH_DOMAIN       The launch domain.
  @tparam REDUCTION_OPERATION The reduction operation type.
  @tparam ATTRIBUTES          The task attributes mask.
  @tparam ARGS                The user-specified task arguments.
 */

template<auto & TASK,
  const execution::launch_domain & LAUNCH_DOMAIN,
  class REDUCTION_OPERATION,
  size_t ATTRIBUTES,
  typename... ARGS>
decltype(auto) reduce(ARGS &&... args);

/*!
  Execute a task.

  @tparam TASK          The user task.
    Its parameters may be of any default-constructible,
    trivially-move-assignable, non-pointer type, any type that supports the
    Legion return-value serialization interface, or any of several standard
    containers of such types.
    If \a ATTRIBUTES specifies an MPI task, parameters need merely be movable.
  @tparam LAUNCH_DOMAIN The launch domain object.
  @tparam ATTRIBUTES    The task attributes mask.
  @tparam ARGS The user-specified task arguments, implicitly converted to the
    parameter types for \a TASK.

  \note Additional types may be supported by defining appropriate
    specializations of \c utils::serial or \c utils::serial_convert.  Avoid
    passing large objects to tasks repeatedly; use global variables (and,
    perhaps, pass keys to select from them) or fields.
 */

template<auto & TASK,
  const execution::launch_domain & LAUNCH_DOMAIN = flecsi::index,
  size_t ATTRIBUTES = flecsi::loc | flecsi::leaf,
  typename... ARGS>
decltype(auto)
execute(ARGS &&... args) {
  return reduce<TASK, LAUNCH_DOMAIN, void, ATTRIBUTES>(
    std::forward<ARGS>(args)...);
} // execute

/*!
  Execute a test task. This interface is provided for FleCSI's unit testing
  framework. Test tasks must return an integer that is non-zero on failure, and
  zero otherwise.

  @tparam TASK          The user task.
    Its parameters may be of any default-constructible,
    trivially-move-assignable, non-pointer type, any type that supports the
    Legion return-value serialization interface, or any of several standard
    containers of such types.
    If \a ATTRIBUTES specifies an MPI task, parameters need merely be movable.
  @tparam LAUNCH_DOMAIN The launch domain object.
  @tparam ATTRIBUTES    The task attributes mask.
  @tparam ARGS The user-specified task arguments, implicitly converted to the
    parameter types for \a TASK.

  @return zero on success, non-zero on failure.
 */

template<auto & TASK,
  const execution::launch_domain & LAUNCH_DOMAIN = flecsi::index,
  size_t ATTRIBUTES = flecsi::loc | flecsi::leaf,
  typename... ARGS>
int
test(ARGS &&... args) {
  return reduce<TASK,
    LAUNCH_DOMAIN,
    execution::reduction::sum<int>,
    ATTRIBUTES>(std::forward<ARGS>(args)...)
    .get();
} // test

} // namespace flecsi
