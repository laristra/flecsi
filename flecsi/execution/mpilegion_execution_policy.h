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

#ifndef flecsi_mpilegion_execution_policy_h
#define flecsi_mpilegion_execution_policy_h

#include <iostream>
#include <utility>
#include <legion.h>
#include <mpi.h>

#include "flecsi/execution/legion_execution_policy.h"
#include "flecsi/execution/context.h"
#include "flecsi/utils/tuple_for_each.h"


/*!
 * \file mpilegion_execution_policy.h
 * \authors Demeshko
 * \date Initial file creation: June 4 , 2016
 */
namespace flecsi
{
class mpilegion_execution_policy_t;

extern void mpilegion_top_level_task(context_t<flecsi::mpilegion_execution_policy_t> &&ctx,int argc, char** argv);

/*!
  \class mpilegion_execution_policy mpilegion_execution_policy.h
  \brief mpilegion_execution_policy provides...
 */
class mpilegion_execution_policy_t: public legion_execution_policy_t
{
//TOFIX: add Rank information to the context
  public: // Member Classes
  class context_ep
  {
  public:
    context_ep():legiontask(NULL),regions( std::vector<LegionRuntime::HighLevel::PhysicalRegion>()){}
    context_ep(const LegionRuntime::HighLevel::Task *_task,
          const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &_regions,
          LegionRuntime::HighLevel::Context ctx, 
          LegionRuntime::HighLevel::HighLevelRuntime *runtime) 
          : ctx_l(ctx),rt(runtime),legiontask(_task),regions(_regions){}

//these 2 functions might be moved to context.h 

   LegionRuntime::HighLevel::HighLevelRuntime * runtime(){
     return rt;
   }
 
   LegionRuntime::HighLevel::Context legion_ctx() {
    return ctx_l;
  }
   
  protected:
     LegionRuntime::HighLevel::Context ctx_l;
     LegionRuntime::HighLevel::HighLevelRuntime* rt;
     const LegionRuntime::HighLevel::Task *legiontask;
     const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions;
  };
 

 public: // Member Functions
	  template <typename T>
	  static void driver_top_task(const LegionRuntime::HighLevel::Task *legiontask,
	                             const std::vector<LegionRuntime::HighLevel::PhysicalRegion> &regions,
	                             LegionRuntime::HighLevel::Context ctx, 
                               LegionRuntime::HighLevel::HighLevelRuntime *runtime)
	  {
		  const LegionRuntime::HighLevel::InputArgs &args = 
             LegionRuntime::HighLevel::HighLevelRuntime::get_input_args();


		  mpilegion_top_level_task(context_t<flecsi::mpilegion_execution_policy_t>(0,
                    legiontask,regions,ctx,runtime),args.argc,args.argv);

	  }

 public:
  typedef int32_t return_type_t;

  enum Task_IDs
  {
  	TOP_LEVEL_TASK_ID
  };


  template <typename T>
  static return_type_t execute_driver(T && legiontask, int argc, char** argv);


  template <typename T, typename... Args>
  static return_type_t execute_task(T && legiontask, Args &&... args)
  {
    utils::tuple_for_each(std::make_tuple(args...),
        [&](auto arg) { std::cout << "test" << std::endl; });

//    context_t<mpilegion_execution_policy_t>::instance().entry();
    auto value = legiontask(std::forward<Args>(args)...);
//    context_t<mpilegion_execution_policy_t>::instance().exit();
    return value;
  } // execute_task


  // Builds up the function signature for a task from
  template<typename... sArgs,typename... aArgs,typename T>
  static void build_task_sig(context_t<mpilegion_execution_policy_t> && ctx,
							 std::tuple<sArgs...> && sArgT, std::tuple<aArgs...> && aArgT,
							 T && legiontask)
  {

  }


}; // class mpilegion_execution_policy_t

} //// namespace flecsi

#ifndef MPI_LEGION_INTEROP_HPP_INCLUDED_IN_EXECUTION_POLICY_H
#define MPI_LEGION_INTEROP_HPP_INCLUDED_IN_EXECUTION_POLICY_H
#endif

#include "flecsi/utils/mpi_legion_interoperability/mpi_legion_interop.h"

namespace flecsi
{

 void MPILegion_Init(void){
   flecsi::mpilegion::MPILegionInteropHelper = new flecsi::mpilegion::MPILegionInterop();
 }

 template <typename T>
 mpilegion_execution_policy_t::return_type_t 
        mpilegion_execution_policy_t::execute_driver(T && task, int argc, char** argv)
  {
    LegionRuntime::HighLevel::HighLevelRuntime::set_top_level_task_id(TOP_LEVEL_TASK_ID);
    LegionRuntime::HighLevel::HighLevelRuntime::register_legion_task
         <mpilegion_execution_policy_t::driver_top_task<T>>(TOP_LEVEL_TASK_ID,
          LegionRuntime::HighLevel::Processor::LOC_PROC, true/*single*/, false/*index*/, 
          AUTO_GENERATE_ID, LegionRuntime::HighLevel::TaskConfigOptions(), "top_level_task");

    flecsi::mpilegion::MPILegionInteropHelper->register_tasks();
    LegionRuntime::HighLevel::HighLevelRuntime::set_registration_callback(flecsi::mpilegion::mapper_registration);
    return LegionRuntime::HighLevel::HighLevelRuntime::start(argc, argv, true);

  }

} // namespace flecsi

#endif // flecsi_mpilegion_execution_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
