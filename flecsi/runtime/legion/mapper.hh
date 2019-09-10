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

#include <flecsi-config.h>

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#else
#include "../backend.hh"
#include "tasks.hh"
#endif

#if !defined(FLECSI_ENABLE_LEGION)
#error FLECSI_ENABLE_LEGION not defined! This file depends on Legion!
#endif

#include <legion.h>
#include <legion/legion_mapping.h>
#include <mappers/default_mapper.h>

flog_register_tag(legion_mapper);

namespace flecsi::runtime {

/*
 The mpi_mapper_t - is a custom mapper that handles mpi-legion
 interoperability in FLeCSI

 @ingroup legion-runtime
*/

class mpi_mapper_t : public Legion::Mapping::DefaultMapper
{
public:
  /*!
   Contructor. Derives from the Legion's Default Mapper

   @param machine Machine type for Legion's Realm
   @param _runtime Legion runtime
   @param local processor type: currently supports only
           LOC_PROC and TOC_PROC
   */

  mpi_mapper_t(Legion::Machine machine,
    Legion::Runtime * _runtime,
    Legion::Processor local)
    : Legion::Mapping::DefaultMapper(_runtime->get_mapper_runtime(),
        machine,
        local,
        "default"),
      machine(machine) {
    using legion_machine = Legion::Machine;
    using legion_proc = Legion::Processor;

    legion_machine::ProcessorQuery pq =
      legion_machine::ProcessorQuery(machine).same_address_space_as(local);
    for(legion_machine::ProcessorQuery::iterator pqi = pq.begin();
        pqi != pq.end();
        ++pqi) {
      legion_proc p = *pqi;
      if(p.kind() == legion_proc::LOC_PROC)
        local_cpus.push_back(p);
      else if(p.kind() == legion_proc::TOC_PROC)
        local_gpus.push_back(p);
      else
        continue;

      std::map<Realm::Memory::Kind, Realm::Memory> & mem_map = proc_mem_map[p];

      legion_machine::MemoryQuery mq =
        legion_machine::MemoryQuery(machine).has_affinity_to(p);
      for(legion_machine::MemoryQuery::iterator mqi = mq.begin();
          mqi != mq.end();
          ++mqi) {
        Realm::Memory m = *mqi;
        mem_map[m.kind()] = m;

        if(m.kind() == Realm::Memory::SYSTEM_MEM)
          local_sysmem = m;
      } // end for
    } // end for

    {
      flog_tag_guard(legion_mapper);
      flog_devel(info) << "Mapper constructor" << std::endl
                       << "\tlocal: " << local << std::endl
                       << "\tcpus: " << local_cpus.size() << std::endl
                       << "\tgpus: " << local_gpus.size() << std::endl
                       << "\tsysmem: " << local_sysmem << std::endl;
    }
  } // end mpi_mapper_t

  /*!
    Destructor
   */
  virtual ~mpi_mapper_t(){};

  Legion::LayoutConstraintID default_policy_select_layout_constraints(
    Legion::Mapping::MapperContext ctx,
    Realm::Memory target_memory,
    const Legion::RegionRequirement & req,
    Legion::Mapping::DefaultMapper::MappingKind mapping_kind,
    bool needs_field_constraint_check,
    bool & force_new_instances) {
    // We always set force_new_instances to false since we are
    // deciding to optimize for minimizing memory usage instead
    // of avoiding Write-After-Read (WAR) dependences
    force_new_instances = false;
    std::vector<Legion::DimensionKind> ordering;
    ordering.push_back(Legion::DimensionKind::DIM_Y);
    ordering.push_back(Legion::DimensionKind::DIM_X);
    ordering.push_back(Legion::DimensionKind::DIM_F); // SOA
    Legion::OrderingConstraint ordering_constraint(
      ordering, true /*contiguous*/);
    Legion::LayoutConstraintSet layout_constraint;
    layout_constraint.add_constraint(ordering_constraint);

    // Do the registration
    Legion::LayoutConstraintID result =
      runtime->register_layout(ctx, layout_constraint);
    return result;
  }

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

  virtual void map_task(const Legion::Mapping::MapperContext ctx,
    const Legion::Task & task,
    const Legion::Mapping::Mapper::MapTaskInput & input,
    Legion::Mapping::Mapper::MapTaskOutput & output) {
    DefaultMapper::map_task(ctx, task, input, output);

    if((task.tag == FLECSI_MAPPER_COMPACTED_STORAGE) &&
       (task.regions.size() > 0)) {

      Legion::Memory target_mem =
        DefaultMapper::default_policy_select_target_memory(
          ctx, task.target_proc, task.regions[0]);

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

      for(size_t indx = 0; indx < task.regions.size(); indx++) {

        Legion::Mapping::PhysicalInstance result;
        std::vector<Legion::LogicalRegion> regions;
        bool created;

        if(task.regions[indx].tag == FLECSI_MAPPER_EXCLUSIVE_LR) {

          flog_assert((task.regions.size() >= (indx + 2)),
            "ERROR:: wrong number of regions passed to the task wirth \
               the  tag = FLECSI_MAPPER_COMPACTED_STORAGE");

          flog_assert((!task.regions[indx].region.exists()),
            "ERROR:: pasing not existing REGION to the mapper");
          regions.push_back(task.regions[indx].region);
          regions.push_back(task.regions[indx + 1].region);
          regions.push_back(task.regions[indx + 2].region);

          flog_assert(runtime->find_or_create_physical_instance(ctx,
                        target_mem,
                        layout_constraints,
                        regions,
                        result,
                        created,
                        true /*acquire*/,
                        GC_NEVER_PRIORITY),
            "FLeCSI mapper failed to allocate instance");

          for(size_t j = 0; j < 3; j++) {
            output.chosen_instances[indx + j].push_back(result);
          } // for

          indx = indx + 2;
        }
        else {

          regions.push_back(task.regions[indx].region);

          flog_assert(runtime->find_or_create_physical_instance(ctx,
                        target_mem,
                        layout_constraints,
                        regions,
                        result,
                        created,
                        true /*acquire*/,
                        GC_NEVER_PRIORITY),
            "FLeCSI mapper failed to allocate instance");

          output.chosen_instances[indx].push_back(result);

        } // end if
      } // end for

    } // end if

  } // map_task

  virtual void slice_task(const Legion::Mapping::MapperContext ctx,
    const Legion::Task & task,
    const Legion::Mapping::Mapper::SliceTaskInput & input,
    Legion::Mapping::Mapper::SliceTaskOutput & output) {
    using legion_proc = Legion::Processor;
    context_t & context_ = context_t::instance();

    if(task.tag == FLECSI_MAPPER_SUBRANK_LAUNCH) {
      // expect a 1-D index domain
      assert(input.domain.get_dim() == 1);
      // send the whole domain to our local processor
      output.slices.resize(1);
      output.slices[0].domain = input.domain;
      output.slices[0].proc = task.target_proc;
      return;
    } // end if FLECSI_MAPPER_SUBRANK_LAUNCH

    if(task.tag == FLECSI_MAPPER_FORCE_RANK_MATCH) {
      // expect a 1-D index domain - each point goes to the corresponding node
      assert(input.domain.get_dim() == 1);
      LegionRuntime::Arrays::Rect<1> r = input.domain.get_rect<1>();

      // go through all the CPU processors and find a representative for each
      //  node (i.e. address space)
      std::map<int, Legion::Processor> targets;

      Legion::Machine::ProcessorQuery pq =
        Legion::Machine::ProcessorQuery(machine).only_kind(
          Legion::Processor::LOC_PROC);
      for(Legion::Machine::ProcessorQuery::iterator it = pq.begin();
          it != pq.end();
          ++it) {
        Legion::Processor p = *it;
        int a = p.address_space();
        if(targets.count(a) == 0)
          targets[a] = p;
      }

      output.slices.resize(1);
      for(int a = r.lo[0]; a <= r.hi[0]; a++) {
        assert(targets.count(a) > 0);
        output.slices[0].domain = // Legion::Domain::from_rect<1>(
          Legion::Rect<1>(a, a);
        output.slices[0].proc = targets[a];
      }
      return;
    } // FLECSI_MAPPER_FORCE_RANK_MATCH

    DefaultMapper::slice_task(ctx, task, input, output);
    // end else
  }

private:
  std::map<Legion::Processor, std::map<Realm::Memory::Kind, Realm::Memory>>
    proc_mem_map;
  Realm::Memory local_sysmem;
  Realm::Machine machine;
};

/*!
 mapper_registration is used to replace DefaultMapper with mpi_mapper_t in
 FLeCSI

 @ingroup legion-runtime
 */

inline void
mapper_registration(Legion::Machine machine,
  Legion::HighLevelRuntime * rt,
  const std::set<Legion::Processor> & local_procs) {
  for(std::set<Legion::Processor>::const_iterator it = local_procs.begin();
      it != local_procs.end();
      it++) {
    mpi_mapper_t * mapper = new mpi_mapper_t(machine, rt, *it);
    rt->replace_default_mapper(mapper, *it);
  }
} // mapper registration

} // namespace flecsi::runtime
