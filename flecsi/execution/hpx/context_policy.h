/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_execution_hpx_context_policy_h
#define flecsi_execution_hpx_context_policy_h

#if !defined(ENABLE_HPX)
#error ENABLE_HPX not defined! This file depends on HPX!
#endif

#include <hpx/include/lcos.hpp>
#include <hpx/include/parallel_execution.hpp>
#include <hpx/runtime_fwd.hpp>

#include <functional>
#include <map>
#include <mutex>
#include <tuple>
#include <unordered_map>

#include <cinchlog.h>
#include <flecsi-config.h>

#include <flecsi/coloring/mpi_utils.h>
#include <flecsi/execution/common/launch.h>
#include <flecsi/execution/common/processor.h>
#include <flecsi/execution/hpx/runtime_driver.h>
#include <flecsi/utils/common.h>
#include <flecsi/utils/const_string.h>
#include <flecsi/utils/export_definitions.h>

///
/// \file hpx/execution_policy.h
/// \authors bergen
/// \date Initial file creation: Nov 15, 2015
///

namespace flecsi {
namespace execution {

///////////////////////////////////////////////////////////////////////////////
struct hpx_context_policy_t {

  using field_id_t = size_t;

  //------------------------------------------------------------------------//
  // Initialization.
  //------------------------------------------------------------------------//

  FLECSI_EXPORT hpx_context_policy_t();

  ///
  /// Initialize the context runtime. The arguments to this method should
  /// be passed from the main function.
  ///
  /// \param argc The number of command-line arguments.
  /// \param argv The array of command-line arguments.
  ///
  /// \return Zero upon clean initialization, non-zero otherwise.
  ///
  int initialize(int argc, char * argv[]) {
    // start HPX runtime system, execute driver code in the context of HPX
    return start_hpx(&hpx_runtime_driver, argc, argv);
  } // hpx_context_policy_t::initialize

  ///
  /// Return the color for which the context was initialized.
  ///
  FLECSI_EXPORT size_t
  color()
  const;

  //------------------------------------------------------------------------//
  // Function registration.
  //------------------------------------------------------------------------//

  using task_info_t =
      std::tuple<processor_type_t, launch_t, std::string, void *>;

  ///
  /// \tparam T The type of the function being registered.
  ///
  /// \param key A unique function identifier.
  ///
  /// \return A boolean value that is true if the registration succeeded,
  ///         false otherwise.
  ///
  template<
      typename RETURN,
      typename ARG_TUPLE,
      RETURN (*FUNCTION)(ARG_TUPLE),
      size_t KEY>
  bool register_function() {
    clog_assert(
        function_registry_.find(KEY) == function_registry_.end(),
        "function has already been registered");

    clog(info) << "Registering function: " << FUNCTION << std::endl;

    function_registry_[KEY] = reinterpret_cast<void *>(FUNCTION);
    return true;
  } // register_function

  ///
  /// Return the function associated with \e key.
  ///
  /// \param key The unique function identifier.
  ///
  /// \return A pointer to a std::function<void(void)> that may be cast
  ///         back to the original function type using reinterpret_cast.
  ///
  void * function(size_t key) {
    return function_registry_[key];
  } // function

  ///
  /// Register a task with the runtime.
  ///
  /// \param key       The task hash key.
  /// \param processor The task processor.
  /// \param launch    The task launch type.
  /// \param name      The task name string.
  ///
  template<
      std::size_t KEY,
      typename RETURN,
      typename ARG_TUPLE,
      RETURN (*FUNCTION)(ARG_TUPLE)>
  bool register_task(
      processor_type_t processor,
      launch_t launch,
      std::string const & name)
  {
    clog(info) << "Registering task " << name << " with key " << KEY
               << std::endl;

    clog_assert(
        task_registry_.find(KEY) == task_registry_.end(),
        "task key already exists");

    task_registry_[KEY] = std::make_tuple(
        processor, launch, name, reinterpret_cast<void *>(FUNCTION));

    return true;
  } // register_task

  ///
  /// Return the task associated with \e key.
  ///
  /// \param key The unique task identifier.
  ///
  /// \return A pointer to a std::function<void(void)> that may be cast
  ///         back to the original function type using reinterpret_cast.
  ///
  template <std::size_t KEY>
  void * task() const {

    auto it = task_registry_.find(KEY);

    clog_assert(
        it != task_registry_.end(),
        "task key does not exists");

    return std::get<3>(it->second);
  } // task

  template <std::size_t KEY>
  processor_type_t processor_type() const
  {
    auto it = task_registry_.find(KEY);

    clog_assert(
        it != task_registry_.end(),
        "task key does not exists");

    return std::get<0>(it->second);
  }

  //--------------------------------------------------------------------
  // Data maps
  //--------------------------------------------------------------------
  struct index_space_data_t {
    // TODO: to be defined.
  };

   struct index_subspace_data_t {
    size_t capacity;
  };

  struct local_index_space_data_t{
    size_t size;
    size_t capacity;
  };

  auto&
  index_space_data_map()
  {
    return index_space_data_map_;
  }

  /*!
    Get the index subspace data map.
   */

  auto & index_subspace_data_map() {
    return index_subspace_data_map_;
  }

  auto&
  local_index_space_data_map()
  {
    return local_index_space_data_map_;
  }

  struct field_metadata_t {
    //TODO: to be defined
  };

  struct sparse_field_data_t {
    //TODO: to be defined
  };

  struct sparse_field_metadata_t {
    //TODO: to be defined
  };

  /*!
    return <double> max reduction
   */

  auto&
  max_reduction()
  {
    return max_reduction_;
  }

  /*!
   Set max_reduction

   @param double max_reduction
   */

  void
  set_max_reduction(double max_reduction)
  {
    max_reduction_ = max_reduction;
  }

  /*!
   Perform reduction for the maximum value type <double>

   @param
   */

  template <typename T>
  auto
  reduce_max(hpx::shared_future<T>& local_future) -> hpx::shared_future<T>
  {
    auto gloabl_max_f = local_future.then(
      mpi_exec_,
      [](hpx::shared_future<T> && local_future) -> T {
        T global_max{};
        T local_max = local_future.get();
        MPI_Allreduce(&local_max, &global_max, 1,
           flecsi::coloring::mpi_typetraits__<T>::type(), MPI_MAX,
           MPI_COMM_WORLD);
        return global_max;
      });
    return gloabl_max_f;
  }


  /*!
    return <double> min reduction
   */

  auto&
  min_reduction()
  {
    return min_reduction_;
  }

  /*!
   Set min_reduction

   @param double min_reduction
   */

  void
  set_min_reduction(double min_reduction)
  {
    min_reduction_ = min_reduction;
  }

  /*!
   Perform reduction for the minimum value type <double>

   @param
   */

  template <typename T>
  auto
  reduce_min(hpx::shared_future<T> & local_future) -> hpx::shared_future<T>
  {
    auto global_min_f = local_future.then(
      mpi_exec_,
      [](hpx::shared_future<T> && local_future) -> T {
        T global_min{};
        T local_min = local_future.get();
        MPI_Allreduce(&local_min, &global_min, 1,
           flecsi::coloring::mpi_typetraits__<T>::type(), MPI_MAX,
           MPI_COMM_WORLD);
        return global_min;
      });
    return global_min_f;
  }

  hpx::threads::executors::pool_executor& get_default_executor() {
    return exec_;
  }

  hpx::threads::executors::pool_executor& get_mpi_executor() {
    return mpi_exec_;
  }

protected:
  // Helper function for HPX start-up and shutdown
  FLECSI_EXPORT int
  hpx_main(int (*driver)(int, char * []), int argc, char * argv[]);

  // Start the HPX runtime system,
  FLECSI_EXPORT int
  start_hpx(int (*driver)(int, char * []), int argc, char * argv[]);

private:
  //--------------------------------------------------------------------------//
  // Task data members.
  //--------------------------------------------------------------------------//

  // Map to store task registration callback methods.
  std::map<std::size_t, task_info_t> task_registry_;

  // Function registry
  std::unordered_map<size_t, void *> function_registry_;

  std::map<field_id_t, std::vector<uint8_t>> field_data;
  std::map<field_id_t, field_metadata_t> field_metadata;

  std::map<size_t, index_space_data_t> index_space_data_map_;
  std::map<size_t, local_index_space_data_t> local_index_space_data_map_;
  std::map<size_t, index_subspace_data_t> index_subspace_data_map_;

  std::map<field_id_t, sparse_field_data_t> sparse_field_data;
  std::map<field_id_t, sparse_field_metadata_t> sparse_field_metadata;

  double min_reduction_;
  double max_reduction_;

private:
  hpx::threads::executors::pool_executor exec_;
  hpx::threads::executors::pool_executor mpi_exec_;

}; // struct hpx_context_policy_t

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_hpx_context_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
