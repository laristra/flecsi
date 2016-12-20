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

#ifndef flecsi_execution_mpilegion_task_ids_h
#define flecsi_execution_mpilegion_task_ids_h

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
  using unique_fid_t = flecsi::utils::unique_id_t<task_id_t>;
  size_t connect_mpi_task_id  = unique_fid_t::instance().next();
  size_t handoff_to_mpi_task_id = unique_fid_t::instance().next();
  size_t wait_on_mpi_task_id  = unique_fid_t::instance().next();
  size_t update_mappers_task_id=unique_fid_t::instance().next();
  size_t unset_call_mpi_id = unique_fid_t::instance().next();
  size_t get_numbers_of_cells_task_id = unique_fid_t::instance().next();
  size_t init_task_id = unique_fid_t::instance().next();
  size_t shared_part_task_id = unique_fid_t::instance().next();
  size_t exclusive_part_task_id = unique_fid_t::instance().next();
  size_t ghost_part_task_id = unique_fid_t::instance().next();
  size_t check_partitioning_task_id = unique_fid_t::instance().next();
};//task_ids_t

}//namespace execution
}//namespace flecsi

#endif //flecsi_execution_mpilegion_task_ids_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

