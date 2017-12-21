/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <flecsi-config.h>

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <default_mapper.h>
#include <legion.h>
#include <legion_mapping.h>

#include <flecsi/execution/context.h>
#include <flecsi/execution/legion/legion_tasks.h>

clog_register_tag(legion_mapper);

/*!
 Mapper ID

 @ingroup legion-execution
 */

enum {
  MPI_MAPPER_ID = 1,
};

namespace flecsi {
namespace execution {

/*
 The mpi_mapper_t - is a custom mapper that handles mpi-legion
 interoperability in FLeCSI

 @ingroup legion-execution
*/

class mpi_mapper_t : public Legion::Mapping::DefaultMapper {
public:

  /*!
   Contructor. Derives from the Legion's Default Mapper
  
   @param machine Machine type for Legion's Realm
   @param _runtime Legion runtime
   @param local processor type: currently supports only
           LOC_PROC and TOC_PROC
   */

  mpi_mapper_t(
      Legion::Machine machine,
      Legion::Runtime * _runtime,
      Legion::Processor local)
      : Legion::Mapping::DefaultMapper(
            _runtime->get_mapper_runtime(),
            machine,
            local,
            "default"),
        machine(machine) {
    using legion_machine = Legion::Machine;
    using legion_proc = Legion::Processor;

    legion_machine::ProcessorQuery pq =
        legion_machine::ProcessorQuery(machine).same_address_space_as(local);
    for (legion_machine::ProcessorQuery::iterator pqi = pq.begin();
         pqi != pq.end(); ++pqi) {
      legion_proc p = *pqi;
      if (p.kind() == legion_proc::LOC_PROC)
        local_cpus.push_back(p);
      else if (p.kind() == legion_proc::TOC_PROC)
        local_gpus.push_back(p);
      else
        continue;

      std::map<Realm::Memory::Kind, Realm::Memory> & mem_map = proc_mem_map[p];

      legion_machine::MemoryQuery mq =
          legion_machine::MemoryQuery(machine).has_affinity_to(p);
      for (legion_machine::MemoryQuery::iterator mqi = mq.begin();
           mqi != mq.end(); ++mqi) {
        Realm::Memory m = *mqi;
        mem_map[m.kind()] = m;

        if (m.kind() == Realm::Memory::SYSTEM_MEM)
          local_sysmem = m;
      } // end for
    } // end for

    {
      clog_tag_guard(legion_mapper);
      clog(info) << "Mapper constuctor: local=" << local
                 << " cpus=" << local_cpus.size()
                 << " gpus=" << local_gpus.size() << " sysmem=" << local_sysmem
                 << std::endl;
    }
  } // end mpi_mapper_t

  /*!
    Destructor
   */
  virtual ~mpi_mapper_t(){};

  /*!
   Specialization of the map_task funtion for FLeCSI
   By default, map_task will execute Legions map_task from DefaultMapper.
   In the case the launcher has been tagged with the
   "MAPPER_COMPACTED_STORAGE" tag, mapper will create single physical
   instance for exclusive, shared and ghost partitions for each data handle
  
    @param ctx Mapper Context
    @param task Legion's task
    @param input Input information about task mapping
    @param output Output information about task mapping
   */

  virtual void map_task(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Task & task,
      const Legion::Mapping::Mapper::MapTaskInput & input,
      Legion::Mapping::Mapper::MapTaskOutput & output) {
    DefaultMapper::map_task(ctx, task, input, output);


    if ( ((task.tag & MAPPER_COMPACTED_STORAGE) != 0) &&                                                                                                                                                                                                                                                                  
            (task.regions.size()>0)){

      Legion::Memory target_mem =
          DefaultMapper::default_policy_select_target_memory(
              ctx, task.target_proc,task.regions[0]);

      // check if we get region requirements for "exclusive, shared and ghost"
      // logical regions for each data handle

      // Filling out "layout_constraints" with the defaults
      Legion::LayoutConstraintSet layout_constraints;
      // No specialization
      layout_constraints.add_constraint(Legion::SpecializedConstraint());
      layout_constraints.add_constraint(Legion::OrderingConstraint());
      // Constrained for the target memory kind
      layout_constraints.add_constraint(
          Legion::MemoryConstraint(target_mem.kind()));
      // Have all the field for the instance available
      std::vector<Legion::FieldID> all_fields;
      layout_constraints.add_constraint(Legion::FieldConstraint());

      // FIXME:: add colocation_constraints
      Legion::ColocationConstraint colocation_constraints;

      for (size_t indx = 0; indx < task.regions.size(); indx++) {

        Legion::Mapping::PhysicalInstance result;
        std::vector<Legion::LogicalRegion> regions;
        bool created;

        if (task.regions[indx].tag == EXCLUSIVE_LR) {

          clog_assert(
              (task.regions.size() >= (indx + 2)),
              "ERROR:: wrong number of regions passed to the task wirth \
               the  tag = MAPPER_COMPACTED_STORAGE");

          regions.push_back(task.regions[indx].region);
          regions.push_back(task.regions[indx + 1].region);
          regions.push_back(task.regions[indx + 2].region);

          if (!runtime->find_or_create_physical_instance(
                  ctx, target_mem, layout_constraints, regions, result, created,
                  true /*acquire*/, GC_NEVER_PRIORITY)) {
            clog(fatal) << "ERROR: FLeCSI mapper failed to allocate instance"
                        << std::endl;
          } // end if

          for (size_t j = 0; j < 3; j++)
            output.chosen_instances[indx + j].push_back(result);

          indx = indx + 2;

        } else {

          regions.push_back(task.regions[indx].region);
          if (!runtime->find_or_create_physical_instance(
                  ctx, target_mem, layout_constraints, regions, result, created,
                  true /*acquire*/, GC_NEVER_PRIORITY)) {
            clog(fatal) << "ERROR: FLeCSI mapper failed to allocate instance"
                        << std::endl;
          } // end if

          output.chosen_instances[indx].push_back(result);

        } // end if
      } // end for

    } // end if

  } // map_task

private:
  std::map<Legion::Processor, std::map<Realm::Memory::Kind, Realm::Memory>>
      proc_mem_map;
  Realm::Memory local_sysmem;
  Realm::Machine machine;
};

/*!
 mapper_registration is used to replace DefaultMapper with mpi_mapper_t in
 FLeCSI

 @ingroup legion-execution
 */

inline void
mapper_registration(
    Legion::Machine machine,
    Legion::HighLevelRuntime * rt,
    const std::set<Legion::Processor> & local_procs) {
  for (std::set<Legion::Processor>::const_iterator it = local_procs.begin();
       it != local_procs.end(); it++) {
    mpi_mapper_t * mapper = new mpi_mapper_t(machine, rt, *it);
    rt->replace_default_mapper(mapper, *it);
  }
} // mapper registration

} // namespace execution
} // namespace flecsi
