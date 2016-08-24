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
#ifndef MPI_LEGION_INTEROP_HPP
#define MPI_LEGION_INTEROP_HPP


#include <iostream>
#include <string>
#include <cstdio>
#include <mutex>
#include <condition_variable>

#include <mpi.h>
#include <legion.h>
#include <realm.h>

#include "flecsi/execution/mpilegion/legion_handshake.h"
#include "flecsi/execution/mpilegion/mapper.h"
#include "flecsi/execution/mpilegion/task_ids.h"

/*!
* \file mpilegion/mpi_legion_interop.h
* \authors demeshko
* \date Initial file creation: Jul 2016
*/

namespace flecsi{
namespace execution{

class mpi_array_storage_t
{
  public:
  mpi_array_storage_t(){};
  ~mpi_array_storage_t(){};
 // virtual void instance(void) = 0;
};

template <typename Type,  uint64_t N>
class array__ : public mpi_array_storage_t{

 public:
  array__(){};
  ~array__(){};

  Type * accessor(void)
  {
   return array_.data();
  }   

  private: 
  std::array<Type,N> array_;
};


class mpi_legion_interop_t
{
  public:
  mpi_legion_interop_t() {};
  ~mpi_legion_interop_t(){};

   void initialize(void)
   {
    register_tasks(); 
    LegionRuntime::HighLevel::HighLevelRuntime::set_registration_callback(
      mapper_registration);
   }
 
  //TOFIX:: nex two methods don't have definition for current implementation
  //variadic arguments for data to be shared
  //Ts - common data types
  template <
   typename... Ts
  >
  void 
  copy_data_from_mpi_to_legion (
  Ts&&... comm_data, 
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime *runtime
  );

  template <
   typename... Ts
  >
  void
  copy_data_from_legion_to_mpi (
   Ts&&... comm_data,
   LegionRuntime::HighLevel::Context ctx,
   LegionRuntime::HighLevel::HighLevelRuntime *runtime);
 
  void 
  legion_configure()
  { 
     ext_legion_handshake_t::instance().ext_init();
  }//legion_configure

/*! a legion task that created User Events/Queques for synchronization 
 *  between MPI and Legion
 */
  static 
  void 
  connect_to_mpi_task (
    const Legion::Task *legiontask,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
    LegionRuntime::HighLevel::Context ctx, 
    LegionRuntime::HighLevel::HighLevelRuntime *runtime)
  {
     std::cout <<"inside connect_to_mpi"<<std::endl;
     ext_legion_handshake_t::instance().legion_init();
  }//connect_to_mpi_task

/*! swithches mutex to Legion runtime
 */
  void 
  handoff_to_legion(void)
  {
    ext_legion_handshake_t::instance().ext_handoff_to_legion();
  }//handoff_to_legion

/*! wait untill mutex switched to MPI runtime and run MPI
 */
  void 
  wait_on_legion(void)
  {
    ext_legion_handshake_t::instance().ext_wait_on_legion();
  }//wait_on_legion

/*! a legion task that switches Legion runtime to MPI
 */
  static 
  void  
  handoff_to_mpi_task (
    const Legion::Task *legiontask,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
    LegionRuntime::HighLevel::Context ctx, 
    LegionRuntime::HighLevel::HighLevelRuntime *runtime);

/*! wait on mutex to be switch to Legion runtime
 */
  static 
  void 
  wait_on_mpi_task(
    const Legion::Task *legiontask,
    const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
    LegionRuntime::HighLevel::Context ctx, 
    LegionRuntime::HighLevel::HighLevelRuntime *runtime);

  static 
  void 
  register_tasks(void);

  void 
  calculate_number_of_procs (void);

  void 
  connect_with_mpi(
    LegionRuntime::HighLevel::Context ctx,
    LegionRuntime::HighLevel::HighLevelRuntime *runtime);

  void 
  handoff_to_mpi(
    LegionRuntime::HighLevel::Context ctx,
    LegionRuntime::HighLevel::HighLevelRuntime *runtime);
 
  void 
  wait_on_mpi(
    LegionRuntime::HighLevel::Context ctx,
    LegionRuntime::HighLevel::HighLevelRuntime *runtime);
 
  std::function<void()> shared_func_;

  bool call_mpi_=false;

  Rect<2> all_processes_;
  Rect<1> local_procs_;

  std::vector <std::shared_ptr<mpi_array_storage_t>> data_storage_;

  private:
  mpi_legion_interop_t(const mpi_legion_interop_t&);
  mpi_legion_interop_t& operator=(const mpi_legion_interop_t&);

};//mpi_legion_interop_t

/*--------------------------------------------------------------------------*/

inline 
void 
mpi_legion_interop_t::handoff_to_mpi_task (
  const Legion::Task *legiontask,
  const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
  LegionRuntime::HighLevel::Context ctx, 
  LegionRuntime::HighLevel::HighLevelRuntime *runtime
)
{
   ext_legion_handshake_t::instance().legion_handoff_to_ext();
}//handoff_to_mpi_task

/*--------------------------------------------------------------------------*/

inline 
void 
mpi_legion_interop_t::wait_on_mpi_task(
 const Legion::Task *legiontask,
 const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
 LegionRuntime::HighLevel::Context ctx, 
 LegionRuntime::HighLevel::HighLevelRuntime *runtime
)
{
  ext_legion_handshake_t::instance().legion_wait_on_ext();
}//wait_on_mpi_task

/*--------------------------------------------------------------------------*/

/*! register all Legion tasks used in the mpi_legion_interop_t class
 */
 //static:
inline 
void 
mpi_legion_interop_t::register_tasks(void)
{
  LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
        <connect_to_mpi_task>( 
        CONNECT_MPI_TASK_ID,
        LegionRuntime::HighLevel::Processor::LOC_PROC, 
        false/*single*/, true/*index*/,
        AUTO_GENERATE_ID, 
        LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/), 
        "connect_to_mpi_task");

  LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
        <handoff_to_mpi_task>( 
        HANDOFF_TO_MPI_TASK_ID,
        LegionRuntime::HighLevel::Processor::LOC_PROC,
        false/*single*/, true/*index*/,
        AUTO_GENERATE_ID, 
        LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/), 
        "handoff_to_mpi_task");
 
  LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
        <wait_on_mpi_task>( 
        WAIT_ON_MPI_TASK_ID,
        LegionRuntime::HighLevel::Processor::LOC_PROC, 
        false/*single*/, true/*index*/,
        AUTO_GENERATE_ID, 
        LegionRuntime::HighLevel::TaskConfigOptions(true/*leaf*/), 
        "wait_on_mpi_task");

}//register_tasks

/*--------------------------------------------------------------------------*/

/*! helper function to calculate number of local/all processors available
 */
inline 
void 
mpi_legion_interop_t::calculate_number_of_procs(void)
{
  int num_local_procs=0;
#ifndef SHARED_LOWLEVEL
  // Only the shared lowlevel runtime needs to iterate over all points
  // on each processor.
  int num_points = 1;
  int num_procs = 0;
  {
   std::set<LegionRuntime::HighLevel::Processor> all_procs;
   Realm::Machine::get_machine().get_all_processors(all_procs);
   for(std::set<LegionRuntime::HighLevel::Processor>::const_iterator 
      it = all_procs.begin();
      it != all_procs.end();
      it++){
            if((*it).kind() == LegionRuntime::HighLevel::Processor::LOC_PROC)
               num_procs++;
            }//end for
  }
  num_local_procs=num_procs;
#else
  int num_procs = LegionRuntime::HighLevel::Machine::get_machine()->
                                            get_all_processors().size();
  int num_points = rank->proc_grid_size.x[0] * rank->proc_grid_size.x[1] 
                                            * rank->proc_grid_size.x[2];
#endif
  printf("Attempting to connect %d processors with %d points per processor\n",
         num_procs, num_points);
  LegionRuntime::Arrays::Point<2> all_procs_lo, all_procs_hi;
  all_procs_lo.x[0] = all_procs_lo.x[1] = 0;
  all_procs_hi.x[0] = num_procs - 1;
  all_procs_hi.x[1] = num_points - 1;
  this->all_processes_ =  LegionRuntime::Arrays::Rect<2>(all_procs_lo,
                                                        all_procs_hi);
  this->local_procs_ = LegionRuntime::Arrays::Rect<1>(0,num_local_procs);
}//calculate_number_of_procs

/*--------------------------------------------------------------------------*/

inline 
void 
mpi_legion_interop_t::connect_with_mpi(
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime *runtime
)
{
   calculate_number_of_procs();
   LegionRuntime::HighLevel::ArgumentMap arg_map;
   LegionRuntime::HighLevel::IndexLauncher connect_mpi_launcher(
         CONNECT_MPI_TASK_ID,
         LegionRuntime::HighLevel::Domain::from_rect<2>(all_processes_),
         LegionRuntime::HighLevel::TaskArgument(0, 0),
         arg_map);

  //run legion_init() from each thead
  LegionRuntime::HighLevel::FutureMap fm1 =
     runtime->execute_index_space(ctx, connect_mpi_launcher);

  //run some legion task here
  fm1.wait_all_results();
}//connect_with_mpi

/*--------------------------------------------------------------------------*/

inline 
void 
mpi_legion_interop_t::handoff_to_mpi(
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime *runtime
)
{
   LegionRuntime::HighLevel::ArgumentMap arg_map;
   LegionRuntime::HighLevel::IndexLauncher handoff_to_mpi_launcher(
         HANDOFF_TO_MPI_TASK_ID,
         LegionRuntime::HighLevel::Domain::from_rect<2>(all_processes_),
         LegionRuntime::HighLevel::TaskArgument(0, 0),
         arg_map);
   LegionRuntime::HighLevel::FutureMap fm2 =runtime->execute_index_space( ctx, handoff_to_mpi_launcher);
   fm2.wait_all_results();
}//handoff_to_mpi

/*--------------------------------------------------------------------------*/

inline 
void 
mpi_legion_interop_t::wait_on_mpi(
  LegionRuntime::HighLevel::Context ctx,
  LegionRuntime::HighLevel::HighLevelRuntime *runtime)
{
   LegionRuntime::HighLevel::ArgumentMap arg_map;
   LegionRuntime::HighLevel::IndexLauncher wait_on_mpi_launcher(
         WAIT_ON_MPI_TASK_ID,
         LegionRuntime::HighLevel::Domain::from_rect<2>(all_processes_),
         LegionRuntime::HighLevel::TaskArgument(0, 0),
         arg_map);
   LegionRuntime::HighLevel::FutureMap fm3 =
        runtime->execute_index_space(ctx,wait_on_mpi_launcher);
  fm3.wait_all_results();
}//wait_on_mpi

} //end namespace execution
} //end namespace flecsi


#endif
/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

