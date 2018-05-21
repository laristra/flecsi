#pragma once

/*! @file */

#include <flecsi/utils/tuple_walker.h>
#include <context.h>

using namespace flecsi::execution;

namespace flecsi {
namespace control {

struct phase_walker_t
  : public flecsi::utils::tuple_walker__<phase_walker_t>
{

  phase_walker_t(int argc, char ** argv)
    : argc_(argc), argv_(argv) {}

  template<typename PHASE_TYPE>
  void handle_type() {

    auto & sorted = context_t::instance().sorted_phase_map(PHASE_TYPE::value);

    for(auto & e: sorted) {
      std::cout << "bits: " << e.bitset() << std::endl;
      e.action()(argc_, argv_);
      std::cout << std::endl;
    } // for

  } // handle_type

private:

  int argc_;
  char ** argv_;

}; // struct phase_walker_t

template<bool CYCLE, typename PHASE_TYPE>
struct phase_wrapper__ {};

template<typename PHASE_TYPE>
struct phase_wrapper__<true, PHASE_TYPE>
{

  static void handle_type(int argc, char ** argv) {

    while(PHASE_TYPE::run()) {
      phase_walker_t phase_walker(argc, argv);
      phase_walker.template walk_types<typename PHASE_TYPE::TYPE>();
      context_t::instance().advance();
    }

  } // handle_type

}; // phase_wrapper__

template<typename PHASE_TYPE>
struct phase_wrapper__<false, PHASE_TYPE>
{

  static void handle_type(int argc, char ** argv) {

    auto & sorted = context_t::instance().sorted_phase_map(PHASE_TYPE::value);

    for(auto & e: sorted) {
      e.action()(argc, argv);
    } // for

  } // handle_type

}; // phase_wrapper__

struct cycle_walker_t
  : public flecsi::utils::tuple_walker__<cycle_walker_t>
{

  cycle_walker_t(int argc, char ** argv) : argc_(argc), argv_(argv) {}

  template<typename PHASE_TYPE>
  void handle_type() {

    constexpr bool cycle =
      !std::is_same<typename PHASE_TYPE::TYPE, size_t>::value;

    phase_wrapper__<cycle, PHASE_TYPE>::handle_type(argc_, argv_);

  } // handle_type

private:

  int argc_;
  char ** argv_;

}; // struct cycle_walker_t

/*!
  \todo Add Documentation!
 */

template<typename CONTROL_POLICY>
struct control__
{

  using phases_t = typename CONTROL_POLICY::phases;

  static int execute(int argc, char ** argv) {
    cycle_walker_t cycle_walker(argc, argv);
    cycle_walker.template walk_types<phases_t>();
  } // execute

}; // class control__

} // namespace flecsi
} // namespace control
