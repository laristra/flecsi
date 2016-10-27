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

#ifndef TASK_IDS
#define TASK_IDS

#include "flecsi/utils/common.h"

///
//\file mpilegion/task_ids.h
//\authors demeshko
// \date Initial file creation: Jul 2016
//
//  HelperTaskIDs enum struct includes TASK_IDs for the legion tasks used
//  in mpi_legion_interop_t class
//  These needs to be a singleton due to the fact that the same IDs are used in
//  mapper.h to se up special rools on executing the tasks used to switch
//  between MPI and Legion runtimes
///

namespace flecsi
{
namespace execution
{

class task_ids_t 
 {
  private:
  
   task_ids_t(){}
   ~task_ids_t(){}
   task_ids_t (const task_ids_t&);
   task_ids_t& operator=(const task_ids_t&);   
   
  public:

    static task_ids_t& instance()
   {
     static  task_ids_t ids;
     return ids;
   }

  using task_id_t = LegionRuntime::HighLevel::TaskID;
  using unique_fid_t = flecsi::unique_id_t<task_id_t>;
  size_t connect_mpi_task_id  = unique_fid_t::instance().next();
  size_t handoff_to_mpi_task_id = unique_fid_t::instance().next();
  size_t wait_on_mpi_task_id  = unique_fid_t::instance().next();
  size_t init_cell_partitions_task_id= unique_fid_t::instance().next();
  size_t update_mappers_task_id=unique_fid_t::instance().next();
  size_t unset_call_mpi_id = unique_fid_t::instance().next();
  size_t init_cells_global_task_id = unique_fid_t::instance().next();
};//task_ids_t

}//namespace execution
}//namespace flecsi

#endif

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

