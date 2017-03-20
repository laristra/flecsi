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

#ifndef flecsi_execution_task_ids_h
#define flecsi_execution_task_ids_h

#include "flecsi/utils/common.h"

///
//\file task_ids.h
///

///
//  HelperTaskIDs enum struct includes TASK_IDs and Field_IDs for the legion 
//  tasks used in mpi_legion_interop_t class, in dpd class and some 
//  temporary legion tasks used in sprint.h
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
  using unique_task_id_t = flecsi::utils::unique_id_t<task_id_t>;
  size_t spmd_task_id = unique_task_id_t::instance().next();
  size_t connect_mpi_task_id  = unique_task_id_t::instance().next();
  size_t handoff_to_mpi_task_id = unique_task_id_t::instance().next();
  size_t wait_on_mpi_task_id  = unique_task_id_t::instance().next();
  size_t update_mappers_task_id=unique_task_id_t::instance().next();
  size_t unset_call_mpi_id = unique_task_id_t::instance().next();
  size_t get_numbers_of_cells_task_id = unique_task_id_t::instance().next();
  size_t init_task_id = unique_task_id_t::instance().next();
  size_t shared_part_task_id = unique_task_id_t::instance().next();
  size_t exclusive_part_task_id = unique_task_id_t::instance().next();
  size_t ghost_part_task_id = unique_task_id_t::instance().next();
  size_t first_compaction_task_id = unique_task_id_t::instance().next();
  size_t check_partitioning_task_id = unique_task_id_t::instance().next();
  size_t ghost_access_task_id = unique_task_id_t::instance().next();
  size_t ghost_init_task_id = unique_task_id_t::instance().next();
  size_t ghost_check_task_id = unique_task_id_t::instance().next();
  size_t halo_copy_task_id = unique_task_id_t::instance().next();
  size_t init_raw_conn_task_id = unique_task_id_t::instance().next();
  size_t dpd_init_connectivity_task_id = unique_task_id_t::instance().next();
  size_t dpd_init_data_task_id = unique_task_id_t::instance().next();
  size_t dpd_get_partition_metadata_task_id = 
    unique_task_id_t::instance().next();
  size_t dpd_put_partition_metadata_task_id = 
    unique_task_id_t::instance().next();
  size_t dpd_commit_data_task_id = unique_task_id_t::instance().next();
  size_t lax_wendroff_task_id = unique_task_id_t::instance().next();
  size_t lax_halo_task_id = unique_task_id_t::instance().next();
  size_t lax_init_task_id = unique_task_id_t::instance().next();
  size_t lax_write_task_id = unique_task_id_t::instance().next();
  size_t lax_adv_x_task_id = unique_task_id_t::instance().next();
  size_t lax_adv_y_task_id = unique_task_id_t::instance().next();
  size_t lax_calc_excl_x_task_id = unique_task_id_t::instance().next();
  size_t lax_calc_excl_y_task_id = unique_task_id_t::instance().next();
};//task_ids_t

class field_ids_t
 {
  private:

   field_ids_t(){}
   ~field_ids_t(){}
   field_ids_t (const field_ids_t&);
   field_ids_t& operator=(const field_ids_t&);

  public:

    static field_ids_t& instance()
   {
     static  field_ids_t ids;
     return ids;
   }

  using task_id_t = LegionRuntime::HighLevel::TaskID;
  using unique_fid_t = flecsi::utils::unique_id_t<task_id_t>;
  size_t fid_cell  = unique_fid_t::instance().next();
  size_t fid_vert  = unique_fid_t::instance().next();
  size_t fid_entity_pair  = unique_fid_t::instance().next();
  size_t fid_data  = unique_fid_t::instance().next();
  size_t fid_ptr_t = unique_fid_t::instance().next();
  size_t fid_entity = unique_fid_t::instance().next();
  size_t fid_ptr_count = unique_fid_t::instance().next();
  size_t fid_offset_count = unique_fid_t::instance().next();
  size_t fid_index = unique_fid_t::instance().next();
  size_t fid_entity_offset = unique_fid_t::instance().next();
  size_t fid_entry_offset = unique_fid_t::instance().next();
  size_t fid_value = unique_fid_t::instance().next();
  size_t fid_partition_metadata = unique_fid_t::instance().next();
//  size_t fid_connectivity = unique_fid_t::instance().next(); 
};

}//namespace execution
}//namespace flecsi

#endif //flecsi_execution_task_ids_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

