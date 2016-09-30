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
#ifndef _MPI_MAPPERS_H_
#define _MPI_MAPPERS_H_

#include <legion.h>
#include "task_ids.h"
#include <legion_mapping.h>
#include <default_mapper.h>

#include "flecsi/execution/mpilegion/task_ids.h"


enum {
  MPI_MAPPER_ID       = 1,
};

enum {
  // Use the first 8 bits for storing the rhsf index
  MAPPER_FORCE_RANK_MATCH = 0x00001000,
  MAPPER_SUBRANK_LAUNCH   = 0x00080000,
  MAPPER_ALL_PROC         = 0x00020000
};

namespace flecsi {
namespace execution {

template <unsigned DIM>
struct STLComparator {
   //   STLComparator(){};
   //  ~STLComparator(){};
   
        bool operator()(const Point<DIM>& a, const Point<DIM>& b) const
        {
          for(unsigned i = 0; i < DIM; i++)  {
            int d = a.x[i] - b.x[i];
            if (d < 0) return true;
            if (d > 0) return false;
          }
          return false;
        }
  };


class MPIMapper : public Legion::Mapping::DefaultMapper {
public:
  MPIMapper(LegionRuntime::HighLevel::Machine machine,
            Legion::Runtime *_runtime,
            LegionRuntime::HighLevel::Processor local);
  virtual ~MPIMapper(void);

  // the mapper uses semantic information attached to field spaces to determine
  //  which instances should be used for each task's region requirements
  static const int FS_TYPE_PROC_SPACE = 1;
  static const int FS_TYPE_GHOST = 2;
  static const int FS_TYPE_STATE = 3;
  static const int FS_TYPE_Q = 4;
  static const int FS_TYPE_INT = 5;
#ifdef USE_CEMA
  static const int FS_TYPE_CEMA = 6;
  static const int FS_TYPE_CEMA_LOCAL = 7;
#endif


  static const Legion::SemanticTag SEMANTIC_TAG_FS_TYPE = 1;
  typedef int FS_TYPE_VALUE_TYPE;

  static void tag_field_space(Legion::Runtime *runtime,
              LegionRuntime::HighLevel::FieldSpace fs,
              FS_TYPE_VALUE_TYPE fs_type);

  virtual const char* get_mapper_name(void) const;

  virtual MapperSyncModel get_mapper_sync_model(void) const;

  virtual 
  void
  select_task_options(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Task& task,
      TaskOptions& output
  );

  void
  premap_task(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Task& task,
      const Legion::Mapping::Mapper::PremapTaskInput& input,
      Legion::Mapping::Mapper::PremapTaskOutput& output
  );

  virtual
  void
  slice_task(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Task& legiontask,
      const Legion::Mapping::Mapper::SliceTaskInput& input,
      Legion::Mapping::Mapper::SliceTaskOutput& output
   );
                         
  virtual
  void
  map_task(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Task&  task,
      const Legion::Mapping::Mapper::MapTaskInput&   input,
      Legion::Mapping::Mapper::MapTaskOutput& output
   );

/*  virtual void select_task_variant(const Legion::Mapping::MapperContext ctx, 
                 const Legion::Task& task,
                 const Legion::Mapping::Mapper::SelectVariantInput& input,
                 Legion::Mapping::Mapper::SelectVariantOutput& output);

  virtual void postmap_task(const Legion::Mapping::MapperContext ctx,
                            const Legion::Task& task,
                            const Legion::Mapping::Mapper::PostMapInput& input,
                            Legion::Mapping::Mapper::PostMapOutput& output);
 */
  virtual
  void
  select_task_sources(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Task& task,
      const Legion::Mapping::Mapper::SelectTaskSrcInput& input,
      Legion::Mapping::Mapper::SelectTaskSrcOutput& output
  );
/*
  virtual void create_task_temporary_instance(
                const Legion::Mapping::MapperContext ctx,
                const Legion::Task& task,
                const Legion::Mapping::Mapper::CreateTaskTemporaryInput& input,
                Legion::Mapping::Mapper::CreateTaskTemporaryOutput& output);


  virtual void speculate(const Legion::Mapping::MapperContext ctx,
                         const Legion::Task&task,
                         Legion::Mapping::Mapper::SpeculativeOutput& output);

  virtual void report_profiling(const Legion::Mapping::MapperContext ctx,
                       const Legion::Task& task,
                       const Legion::Mapping::Mapper::TaskProfilingInfo& input);
*/
  virtual 
  void
  map_inline(
      const Legion::Mapping::MapperContext ctx,
      const Legion::InlineMapping& inline_op,
      const Legion::Mapping::Mapper::MapInlineInput& input,
      Legion::Mapping::Mapper::MapInlineOutput& output
  );

/*  virtual void select_inline_sources(const Legion::Mapping::MapperContext ctx,
                     const Legion::InlineMapping& inline_op,
                     const Legion::Mapping::Mapper::SelectInlineSrcInput& input,
                     Legion::Mapping::Mapper::SelectInlineSrcOutput& output);

  virtual void create_inline_temporary_instance(
              const Legion::Mapping::MapperContext ctx,
              const Legion::InlineMapping& inline_op,
              const Legion::Mapping::Mapper::CreateInlineTemporaryInput& input,
              Legion::Mapping::Mapper::CreateInlineTemporaryOutput& output);

  virtual void report_profiling(const Legion::Mapping::MapperContext ctx,
              const Legion::InlineMapping& inline_op,
              const Legion::Mapping::Mapper::InlineProfilingInfo& input);
*/
  virtual
  void
  map_copy(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Copy& copy,
      const Legion::Mapping::Mapper::MapCopyInput& input,
      Legion::Mapping::Mapper::MapCopyOutput& output
  );

/*  virtual void select_copy_sources(const Legion::Mapping::MapperContext ctx,
                   const Legion::Copy& copy,
                   const Legion::Mapping::Mapper::SelectCopySrcInput& input,
                   Legion::Mapping::Mapper::SelectCopySrcOutput& output);

  virtual void create_copy_temporary_instance(
                 const Legion::Mapping::MapperContext ctx,
                 const Legion::Copy& copy,
                 const Legion::Mapping::Mapper::CreateCopyTemporaryInput& input,
                 Legion::Mapping::Mapper::CreateCopyTemporaryOutput& output);

  virtual void speculate(const Legion::Mapping::MapperContext ctx,
                         const Legion::Copy& copy,
                         Legion::Mapping::Mapper::SpeculativeOutput& output);

  virtual void report_profiling(const Legion::Mapping::MapperContext ctx,
                     const Legion::Copy& copy,
                     const Legion::Mapping::Mapper::CopyProfilingInfo& input);
*/
  virtual
  void
  map_close(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Close& close,
      const Legion::Mapping::Mapper::MapCloseInput& input,
      Legion::Mapping::Mapper::MapCloseOutput& output
  );

/*  virtual void select_close_sources(const Legion::Mapping::MapperContext ctx,
                   const Legion::Close& close,
                   const Legion::Mapping::Mapper::SelectCloseSrcInput& input,
                   Legion::Mapping::Mapper::SelectCloseSrcOutput& output);

  virtual void create_close_temporary_instance(
               const Legion::Mapping::MapperContext ctx,
               const Legion::Close& close,
               const Legion::Mapping::Mapper::CreateCloseTemporaryInput& input,
               Legion::Mapping::Mapper::CreateCloseTemporaryOutput& output);

  virtual void report_profiling(const Legion::Mapping::MapperContext ctx,
                      const Legion::Close& close,
                      const Legion::Mapping::Mapper::CloseProfilingInfo& input);

  virtual void map_acquire(const Legion::Mapping::MapperContext ctx,
                         const Legion::Acquire& acquire,
                         const Legion::Mapping::Mapper::MapAcquireInput& input,
                         Legion::Mapping::Mapper::MapAcquireOutput& output);

  virtual void speculate(const Legion::Mapping::MapperContext ctx,
                         const Legion::Acquire& acquire,
                         Legion::Mapping::Mapper::SpeculativeOutput& output);

  virtual void report_profiling(const Legion::Mapping::MapperContext ctx,
                 const Legion::Acquire& acquire,
                 const Legion::Mapping::Mapper::AcquireProfilingInfo& input);
*/
/*  virtual void map_release(const Legion::Mapping::MapperContext ctx,
                  const Legion::Release& release,
                  const Legion::Mapping::Mapper::MapReleaseInput& input,
                  Legion::Mapping::Mapper::MapReleaseOutput& output);

  virtual void select_release_sources(const Legion::Mapping::MapperContext ctx,
                 const Legion::Release& release,
                 const Legion::Mapping::Mapper::SelectReleaseSrcInput& input,
                 Legion::Mapping::Mapper::SelectReleaseSrcOutput& output);

  virtual void create_release_temporary_instance(
              const Legion::Mapping::MapperContext ctx,
              const Legion::Release& release,
              const Legion::Mapping::Mapper::CreateReleaseTemporaryInput& input,
              Legion::Mapping::Mapper::CreateReleaseTemporaryOutput& output);

  virtual void speculate(const Legion::Mapping::MapperContext ctx,
                         const Legion::Release& release,
                         Legion::Mapping::Mapper::SpeculativeOutput& output);

  virtual void report_profiling(const Legion::Mapping::MapperContext ctx,
                   const Legion::Release& release,
                   const Legion::Mapping::Mapper::ReleaseProfilingInfo& input) ;
*/
  virtual
  void
  configure_context(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Task& task,
      Legion::Mapping::Mapper::ContextConfigOutput& output
 );


  virtual
  void
  select_tunable_value(
     const Legion::Mapping::MapperContext ctx,
     const Legion::Task& task,
     const Legion::Mapping::Mapper::SelectTunableInput& input,
     Legion::Mapping::Mapper::SelectTunableOutput& output
  );


  virtual
  void
  map_must_epoch(
     const Legion::Mapping::MapperContext ctx,
     const Legion::Mapping::Mapper::MapMustEpochInput& input,
     Legion::Mapping::Mapper::MapMustEpochOutput& output
  );

/*  virtual void map_dataflow_graph(const Legion::Mapping::MapperContext ctx,
                   const Legion::Mapping::Mapper::MapDataflowGraphInput& input,
                   Legion::Mapping::Mapper::MapDataflowGraphOutput& output);
*/
  virtual 
  void 
  select_tasks_to_map( 
     const Legion::Mapping::MapperContext ctx,
     const Legion::Mapping::Mapper::SelectMappingInput& input,
     Legion::Mapping::Mapper::SelectMappingOutput& output
  );

  virtual
  void
  select_steal_targets(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Mapping::Mapper::SelectStealingInput& input,
      Legion::Mapping::Mapper::SelectStealingOutput& output
  );

  virtual
  void
  permit_steal_request(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Mapping::Mapper::StealRequestInput& input,
      Legion::Mapping::Mapper::StealRequestOutput& output
  );


/*  virtual void handle_message(const Legion::Mapping::MapperContext ctx,
                 const Legion::Mapping::Mapper::MapperMessage& message);

  virtual void handle_task_result(const Legion::Mapping::MapperContext ctx,
                  const Legion::Mapping::Mapper::MapperTaskResult& result); 
 */

protected:

  //Legion::Runtime *runtime;

  Legion::Mapping::PhysicalInstance
  choose_instance(
      const Legion::Mapping::MapperContext ctx,
      const Legion::Task& task,
      const LegionRuntime::HighLevel::RegionRequirement& req,
      Realm::Memory mem
  );

  typedef std::map<LegionRuntime::HighLevel::LogicalRegion,
         Legion::Mapping::PhysicalInstance> InstanceMap;
  std::map<Realm::Memory, InstanceMap> mem_inst_map;

  std::vector<LegionRuntime::HighLevel::Processor> local_cpus, local_gpus;
  std::vector<unsigned> local_next_cpu;
  std::map<Point<1>, std::vector<LegionRuntime::HighLevel::Processor>, 
     STLComparator<1> > rank_cpus, rank_gpus;
  std::map<LegionRuntime::HighLevel::Processor,
       std::map<Realm::Memory::Kind, Realm::Memory> > proc_mem_map;
  Realm::Memory local_sysmem;
  Realm::Machine machine;
};

void 
mapper_registration(
   LegionRuntime::HighLevel::Machine machine,
   LegionRuntime::HighLevel::HighLevelRuntime *rt,
   const std::set<LegionRuntime::HighLevel::Processor> &local_procs
);

inline 
MPIMapper::MPIMapper(
  LegionRuntime::HighLevel::Machine _machine,
  Legion::Runtime *_rt,
  LegionRuntime::HighLevel::Processor local
)
  : Legion::Mapping::DefaultMapper(_rt->get_mapper_runtime(),
                                   _machine, local, "default")
  , machine(_machine)
{  
   using legion_machine=LegionRuntime::HighLevel::Machine;
   using legion_proc=LegionRuntime::HighLevel::Processor;
   
   legion_machine::ProcessorQuery pq = 
           legion_machine::ProcessorQuery(machine).same_address_space_as(local);   for(legion_machine::ProcessorQuery::iterator pqi = pq.begin();
        pqi != pq.end();
        ++pqi)
   {   
       legion_proc p = *pqi;
       if(p.kind() == legion_proc::LOC_PROC)
       local_cpus.push_back(p);
       else if(p.kind() == legion_proc::TOC_PROC)
          local_gpus.push_back(p);
        else
          continue;
      
      std::map<Realm::Memory::Kind, Realm::Memory>& mem_map = proc_mem_map[p];
      
      legion_machine::MemoryQuery mq = 
            legion_machine::MemoryQuery(machine).has_affinity_to(p);
      for(legion_machine::MemoryQuery::iterator mqi = mq.begin();
         mqi != mq.end();
         ++mqi)   
      {           
                  Realm::Memory m = *mqi;
                  mem_map[m.kind()] = m;
                  
                  if(m.kind() == Realm::Memory::SYSTEM_MEM)
                    local_sysmem = m;
      }//end for
   }//end for
   
   std::cout << "Mapper constuctor: local=" << local << " cpus=" <<
         local_cpus.size() << " gpus=" << local_gpus.size() <<
          " sysmem=" << local_sysmem<<std::endl;
}

inline 
MPIMapper::~MPIMapper(void)
{
}

inline 
/*static */void
MPIMapper::tag_field_space(
 Legion::Runtime *runtime,
 LegionRuntime::HighLevel::FieldSpace fs,
 MPIMapper::FS_TYPE_VALUE_TYPE fs_type
)
{
  runtime->attach_semantic_information(fs,
               MPIMapper::SEMANTIC_TAG_FS_TYPE,
               &fs_type,
               sizeof(FS_TYPE_VALUE_TYPE));
}

inline 
const
char*
MPIMapper::get_mapper_name(void) const
{
  return "MPIFMapper";
}

inline
Legion::Mapping::Mapper::MapperSyncModel
MPIMapper::get_mapper_sync_model(void) const
{
  // TODO: concurrent should be ok here?
    return SERIALIZED_NON_REENTRANT_MAPPER_MODEL;
}

inline 
void
MPIMapper::select_task_options(
  const Legion::Mapping::MapperContext ctx,
  const Legion::Task& task,
  TaskOptions& output
)
{
 //TOFIX: add logic for GPU
  bool prefer_cpu;
  output.map_locally = true;
}

inline 
void
MPIMapper::premap_task(
  const Legion::Mapping::MapperContext ctx,
  const Legion::Task& task,
  const Legion::Mapping::Mapper::PremapTaskInput& input,
  Legion::Mapping::Mapper::PremapTaskOutput& output
)
{

  using iterator_t=std::map<unsigned,
     std::vector<Legion::Mapping::PhysicalInstance> >::const_iterator;

  for(
      iterator_t it = input.valid_instances.begin();
      it != input.valid_instances.end();
      ++it)
   {
      std::vector<Legion::Mapping::PhysicalInstance>& pmi =
                   output.premapped_instances[it->first];
      pmi.insert(pmi.end(), it->second.begin(), it->second.end());
      bool ok = runtime->acquire_and_filter_instances(ctx, pmi);
      assert(ok);
   }//end for
}

inline 
void
MPIMapper::slice_task(
  const Legion::Mapping::MapperContext ctx,
  const Legion::Task& task,
  const Legion::Mapping::Mapper::SliceTaskInput& input,
  Legion::Mapping::Mapper::SliceTaskOutput& output
)
{
//TOFIX: check logic here
  using legion_proc=LegionRuntime::HighLevel::Processor;

 // tag-based decisions here
  if((task.tag & MAPPER_FORCE_RANK_MATCH) != 0) {
    // expect a 1-D index domain
    assert(input.domain.get_dim() == 1);
    Rect<1> r = input.domain.get_rect<1>();

    // each point in the domain goes to CPUs assigned to that rank
    output.slices.resize(r.volume());
    size_t idx = 0;
    for(GenericPointInRectIterator<1> pir(r); pir; ++pir, ++idx) {
      output.slices[idx].domain = Legion::Domain::from_rect<1>(Rect<1>(pir.p, pir.p));
      if(task.target_proc.kind() == legion_proc::TOC_PROC) {
         assert(rank_gpus.count(pir.p) > 0);
          output.slices[idx].proc = rank_gpus[pir.p][0];
      } else {
        assert(rank_cpus.count(pir.p) > 0);
        output.slices[idx].proc = rank_cpus[pir.p][0];
      }//end if task.target_proc.kind
    }//end for
    return;
  }//endi if MAPPER_FORCE_RANK_MATCH

  if((task.tag & MAPPER_SUBRANK_LAUNCH) != 0) {
    // expect a 1-D index domain
    assert(input.domain.get_dim() == 1);

    // send the whole domain to our local processor
    output.slices.resize(1);
    output.slices[0].domain = input.domain;
    output.slices[0].proc = task.target_proc;
    return;
  } //end if MAPPER_SUBRANK_LAUNCH

  size_t connect_mpi_task_id  = task_ids_t::instance().connect_mpi_task_id;
  size_t handoff_to_mpi_task_id = task_ids_t::instance().handoff_to_mpi_task_id;
  size_t wait_on_mpi_task_id  = task_ids_t::instance().wait_on_mpi_task_id;
  size_t init_cell_partitions_task_id =
          task_ids_t::instance().init_cell_partitions_task_id;
  size_t update_mappers_task_id = task_ids_t::instance().update_mappers_task_id;

  // task-specific cases below
  if (task.task_id == connect_mpi_task_id||
      task.task_id == handoff_to_mpi_task_id||
      task.task_id == wait_on_mpi_task_id ||
      task.task_id == init_cell_partitions_task_id ||
      ((task.tag & MAPPER_ALL_PROC) != 0)) {
    // expect a 2-D index domain - first component is the processor index,
    //  and second is the MPI rank
    assert(input.domain.get_dim() == 2);
    Rect<2> r = input.domain.get_rect<2>();

    using legion_machine=LegionRuntime::HighLevel::Machine;
    using legion_proc=LegionRuntime::HighLevel::Processor;

     // get list of all processors and make sure the count matches
    legion_machine::ProcessorQuery pq =
       legion_machine::ProcessorQuery(machine).only_kind(legion_proc::LOC_PROC);
    std::vector<legion_proc> all_procs(pq.begin(), pq.end());
    assert((r.lo[0] == 0) && (r.hi[0] == (int)(all_procs.size() - 1)));

    // now create a slice for each processor, allowing it to try to connect to
    //  each MPI rank
    output.slices.resize(all_procs.size());
    for(size_t i = 0; i < all_procs.size(); i++) {
      Rect<2> subrect = r;
      subrect.lo.x[0] = subrect.hi.x[0] = i;
      output.slices[i].domain = Legion::Domain::from_rect<2>(subrect);
      output.slices[i].proc = all_procs[i];
    }//end for
    return;
  } //end if task.task_id=

  else if( task.task_id ==update_mappers_task_id) {
    // expect a 1-D index domain - each point goes to the corresponding node
    assert(input.domain.get_dim() == 1);
    Rect<1> r = input.domain.get_rect<1>();

    // go through all the CPU processors and find a representative for each
    //  node (i.e. address space)
    std::map<int, legion_proc> targets;

    using legion_machine=LegionRuntime::HighLevel::Machine;
    legion_machine::ProcessorQuery pq =
      legion_machine::ProcessorQuery(machine).only_kind(legion_proc::LOC_PROC);

    for(legion_machine::ProcessorQuery::iterator it = pq.begin();
       it != pq.end(); ++it)
    {
      legion_proc p = *it;
      int a = p.address_space();
      if(targets.count(a) == 0)
       targets[a] = p;
    }//end for

    output.slices.resize(r.volume());
    for(int a = r.lo[0]; a <= r.hi[0]; a++) {
       assert(targets.count(a) > 0);
       output.slices[a].domain = Realm::Domain::from_rect<1>(Rect<1>(a, a));
       output.slices[a].proc = targets[a];
    }//end for
    return;
  } //end else if task.task_id=

   else{
    DefaultMapper::default_slice_task(task, local_cpus, remote_cpus,
                               input, output, cpu_slices_cache);
   }//end else
}//end slice_task

inline 
void
MPIMapper::map_task(
   const Legion::Mapping::MapperContext ctx,
   const Legion::Task&  task,
   const Legion::Mapping::Mapper::MapTaskInput&   input,
   Legion::Mapping::Mapper::MapTaskOutput& output
)
{

 // CPU vs. GPU was chosen in select_task_options
  LegionRuntime::HighLevel::Processor::Kind pkind;
  {
    LegionRuntime::HighLevel::Processor p = task.target_proc;
    pkind = p.kind();
    // allow runtime to load-balance non-must-epoch-cpu-tasks across all cpus
    if((pkind == LegionRuntime::HighLevel::Processor::LOC_PROC)
         && !task.must_epoch_task
         &&(task.target_proc.address_space() == local_cpus[0].address_space()))
      output.target_procs = local_cpus;
    else
      output.target_procs.push_back(task.target_proc);

    std::vector<Legion::VariantID> variants;
    runtime->find_valid_variants(ctx, task.task_id, variants, pkind);
    // get rid of any inner variants for now
    std::vector<Legion::VariantID>::iterator it = variants.begin();

    while(it != variants.end()) {
      if(runtime->is_inner_variant(ctx, task.task_id, *it))
        it = variants.erase(it);
      else
        ++it;
    }//end while

    // hopefully have exactly one left
    if(variants.size() != 1) {
      assert(0);
    }//end if

    output.chosen_variant = variants[0];
  }

  
  // if we're mapping something in another address space,
  // we have to do things differently
  size_t update_mappers_task_id = task_ids_t::instance().update_mappers_task_id;
  if(task.target_proc.address_space() != local_cpus[0].address_space()) {
    assert(task.regions.empty() ||
           (task.task_id == update_mappers_task_id)); // ||
           //(task.task_id == DISTRIBUTE_TASK_ID));

    std::vector<unsigned>::const_iterator pmi = input.premapped_regions.begin();
    for(size_t i = 0; i < task.regions.size(); i++) {
      // make sure to skip any premapped instances - assuming vector is sorted
      if((pmi != input.premapped_regions.end()) && (*pmi == i)) {
        ++pmi;
        continue;
      }//end if
     Realm::Memory::Kind mkind =  Realm::Memory::SYSTEM_MEM;

     Realm::Memory mem = LegionRuntime::HighLevel::Machine::MemoryQuery(machine)
        .same_address_space_as(task.target_proc)
        .only_kind(mkind)
        .first();

      std::vector<LegionRuntime::HighLevel::LogicalRegion> regions(1,
                                                 task.regions[i].region);
      Legion::LayoutConstraintSet lcs;
      // add in all the needed fields
      lcs.add_constraint(Legion::FieldConstraint(
                         task.regions[i].privilege_fields,
                         false /*contiguous*/,
                         false /*inorder*/));
     {
        std::vector<Legion::DimensionKind> dimension_ordering(4);
        dimension_ordering[0] = DIM_X;
        dimension_ordering[1] = DIM_Y;
        dimension_ordering[2] = DIM_Z;
        dimension_ordering[3] = DIM_F;
        lcs.add_constraint(Legion::OrderingConstraint(dimension_ordering,
                                              false /*!contiguous*/));
      }

     Legion::Mapping::PhysicalInstance inst;
      bool created;
      bool ok = runtime->find_or_create_physical_instance(ctx,
                                                          mem,
                                                          lcs,
                                                          regions,
                                                          inst,
                                                          created);
      assert(ok);
      output.chosen_instances[i].push_back(inst);
    }//end for
  } else {
   // each instance's requirement will tell us which kind of memory it wants
    //  (or maybe two choices, one if cpu and one if gpu)
    const std::map< Realm::Memory::Kind,  Realm::Memory>& mem_map =
                                        proc_mem_map[task.target_proc];

    // get instances
    std::vector<unsigned>::const_iterator pmi = input.premapped_regions.begin();
    for(size_t i = 0; i < task.regions.size(); i++) {
      // make sure to skip any premapped instances - assuming vector is sorted
      if((pmi != input.premapped_regions.end()) && (*pmi == i)) {
        ++pmi;
        continue;
      }//endif
      Realm::Memory::Kind mkind =  Realm::Memory::SYSTEM_MEM;
      std::map< Realm::Memory::Kind,  Realm::Memory>::const_iterator
      it = mem_map.find(mkind);
      if(it == mem_map.end()) {
        assert(0);
      }

      Realm::Memory mem = it->second;

      Legion::Mapping::PhysicalInstance inst =
                choose_instance(ctx, task, task.regions[i], mem);
      output.chosen_instances[i].push_back(inst);
      }//end for
  }//end if

  output.postmap_task = false;
}

inline 
void
MPIMapper::select_task_sources(
   const Legion::Mapping::MapperContext ctx,
   const Legion::Task& task,
   const Legion::Mapping::Mapper::SelectTaskSrcInput& input,
   Legion::Mapping::Mapper::SelectTaskSrcOutput& output
)
{
  output.chosen_ranking.insert(output.chosen_ranking.end(),
                               input.source_instances.begin(),
                               input.source_instances.end());

}//select_task_sources

inline 
void
MPIMapper::map_inline(
   const Legion::Mapping::MapperContext ctx,
   const Legion::InlineMapping& inline_op,
   const Legion::Mapping::Mapper::MapInlineInput& input,
   Legion::Mapping::Mapper::MapInlineOutput& output
)
{
  // if we have a valid instance, choose it
  if(!input.valid_instances.empty()) {
    output.chosen_instances.push_back(input.valid_instances[0]);
    return;
  }//end if

  // make one in our system memory
  std::vector< LegionRuntime::HighLevel::LogicalRegion> regions(1,
                                      inline_op.requirement.region);
  Legion::LayoutConstraintSet lcs;
  // add in all the needed fields
  lcs.add_constraint(Legion::FieldConstraint(
              inline_op.requirement.privilege_fields,
              true /*contiguous*/,
              true /*inorder*/));
  {
    std::vector<Legion::DimensionKind> dimension_ordering(4);
    dimension_ordering[0] = DIM_X;
    dimension_ordering[1] = DIM_Y;
    dimension_ordering[2] = DIM_Z;
    dimension_ordering[3] = DIM_F;
    lcs.add_constraint(Legion::OrderingConstraint(dimension_ordering,
                                          false /*!contiguous*/));
  }
  Legion::Mapping::PhysicalInstance inst;
  bool created;
  bool ok = runtime->find_or_create_physical_instance(ctx,
                                                      local_sysmem,
                                                      lcs,
                                                      regions,
                                                      inst,
                                                      created);
  assert(ok);
  output.chosen_instances.push_back(inst);
}//end map_inline

inline 
void
MPIMapper::map_copy(
   const Legion::Mapping::MapperContext ctx,
   const Legion::Copy& copy,
   const Legion::Mapping::Mapper::MapCopyInput& input,
   Legion::Mapping::Mapper::MapCopyOutput& output
)
{
  for(size_t i = 0; i < copy.src_requirements.size(); i++) {
    Legion::Mapping::PhysicalInstance inst =
       choose_instance(ctx, *(copy.parent_task),
                       copy.src_requirements[i], local_sysmem);
    output.src_instances[i].push_back(inst);
#if 0
    //const RegionRequirement& rr = copy.src_requirements[i];
    if(!input.src_instances[i].empty()) {
      output.src_instances[i] = input.src_instances[i];
      bool ok = runtime->acquire_and_filter_instances(ctx,
                                                      output.src_instances[i]);
      assert(ok);
    }
#endif
  }
  for(size_t i = 0; i < copy.dst_requirements.size(); i++) {
    // our copies should always be going into a restricted instance, so pick it
    assert(copy.dst_requirements[i].is_restricted());
    assert(!input.dst_instances[i].empty());
    Legion::Mapping::PhysicalInstance inst = input.dst_instances[i][0];
    bool ok = runtime->acquire_instance(ctx, inst);
    assert(ok);
    output.dst_instances[i].push_back(inst);
  }

}//map_copy

inline 
void
MPIMapper::map_close(
   const Legion::Mapping::MapperContext ctx,
   const Legion::Close& close,
   const Legion::Mapping::Mapper::MapCloseInput& input,
   Legion::Mapping::Mapper::MapCloseOutput& output
)
{
 // assume sysmem for now - these result from subrank'd tasks
  Realm::Memory::Kind mkind = Realm::Memory::SYSTEM_MEM;

  const std::map<Realm::Memory::Kind, Realm::Memory>& mem_map =
         proc_mem_map[close.parent_task->target_proc];
  std::map<Realm::Memory::Kind, Realm::Memory>::const_iterator it =
                                                 mem_map.find(mkind);
  if(it == mem_map.end()) {
    assert(0);
  }//end if

  Realm::Memory mem = it->second;

  Legion::Mapping::PhysicalInstance inst = choose_instance(ctx,
                                       *(close.parent_task),
                                       close.requirement,
                                       mem);
  output.chosen_instances.push_back(inst);
}//map_close

inline 
void
MPIMapper::configure_context(
   const Legion::Mapping::MapperContext ctx,
   const Legion::Task& task,
   Legion::Mapping::Mapper::ContextConfigOutput& output
)
{
  //make sure we can keep way to have a lot of tasks in front
  output.min_tasks_to_schedule = 1024;
  output.min_frames_to_schedule = 2;
}//configure_context

inline 
void
MPIMapper::select_tunable_value(
   const Legion::Mapping::MapperContext ctx,
   const Legion::Task& task,
   const Legion::Mapping::Mapper::SelectTunableInput& input,
   Legion::Mapping::Mapper::SelectTunableOutput& output
)
{
  /*switch(input.tunable_id) {
  case TUNABLE_PROCS_PER_NODE: { 
    output.value = new int(local_cpus.size());
    output.size = sizeof(int);
    return;  
  }

  default:*/
    assert(0);
 // } 

}//end select_tunable_value



inline 
void
MPIMapper::map_must_epoch(
   const Legion::Mapping::MapperContext ctx,
   const Legion::Mapping::Mapper::MapMustEpochInput& input,
   Legion::Mapping::Mapper::MapMustEpochOutput& output
)
{

  for(size_t i = 0; i < input.tasks.size(); i++) {
    const Legion::Task& t = *(input.tasks[i]);
    if((t.tag & MAPPER_FORCE_RANK_MATCH) != 0) {
      Point<1> p = t.index_point.get_point<1>();
      assert(rank_cpus.count(p) > 0);
      output.task_processors[i] = rank_cpus[p][0];
    } else {
      // unexpected
      assert(0);
    }//end if
  }//end for
  for(size_t i = 0; i < input.constraints.size(); i++) {
    const MappingConstraint& c = input.constraints[i];
    // these are ghost cell exchange regions, so expect exactly
    // 2 tasks in each constraint, and exactly one to have the
    // NO_ACCESS flag - the other gets the instance
    assert(c.constrained_tasks.size() == 2);
    bool t0_match =
     !c.constrained_tasks[0]->regions[c.requirement_indexes[0]].is_no_access();
    bool t1_match =
      !c.constrained_tasks[1]->regions[c.requirement_indexes[1]].is_no_access();
    assert(t0_match || t1_match);
    assert(!t0_match || !t1_match);
    Point<1> p =
       c.constrained_tasks[t0_match ? 0 : 1]->index_point.get_point<1>();
    assert(rank_cpus.count(p) > 0);
    LegionRuntime::HighLevel::Processor match_proc = rank_cpus[p][0];
    Realm::Memory m =
         LegionRuntime::HighLevel::Machine::MemoryQuery(machine).same_address_space_as(match_proc).only_kind(Realm::Memory::REGDMA_MEM).first();
    // if we don't have registered memory, we're toast because the 
    // application code counts on  being able to do remote reads during
    // initialization
    if(!m.exists()) {
      assert(0);
    }
    for(size_t j = 0; j < c.constrained_tasks.size(); j++) {
    }

// the instance should already exist, so we don't need to put a lot
    // of constraints - just enough to find it
   std::vector<LegionRuntime::HighLevel::LogicalRegion> regions(1,
            c.constrained_tasks[0]->regions[c.requirement_indexes[0]].region);
    Legion::LayoutConstraintSet lcs;
    // add in all the needed fields
    lcs.add_constraint(Legion::FieldConstraint(
     c.constrained_tasks[0]->regions[c.requirement_indexes[0]].privilege_fields,
     false /*contiguous*/,
     false /*inorder*/));
    {
      std::vector<Legion::DimensionKind> dimension_ordering(4);
      dimension_ordering[0] = DIM_X;
      dimension_ordering[1] = DIM_Y;
      dimension_ordering[2] = DIM_Z;
      dimension_ordering[3] = DIM_F;
      lcs.add_constraint(Legion::OrderingConstraint(dimension_ordering,
                                            false /*!contiguous*/));
    }
    Legion::Mapping::PhysicalInstance inst;
    bool created;
    bool ok = runtime->find_or_create_physical_instance(ctx,
                                                        m,
                                                        lcs,
                                                        regions,
                                                        inst,
                                                        created,
                                                        true /*acquire*/,
                                                        GC_NEVER_PRIORITY);
    assert(ok);
    output.constraint_mappings[i].push_back(inst);
  }

}

inline 
void
MPIMapper::select_tasks_to_map(
   const Legion::Mapping::MapperContext ctx,
   const Legion::Mapping::Mapper::SelectMappingInput& input,
   Legion::Mapping::Mapper::SelectMappingOutput& output
)
{
  // map all ready tasks
   output.map_tasks.insert(input.ready_tasks.begin(), input.ready_tasks.end());
}//end select_tasks_to_map

inline 
void
MPIMapper::select_steal_targets(
   const Legion::Mapping::MapperContext ctx,
   const Legion::Mapping::Mapper::SelectStealingInput& input,
   Legion::Mapping::Mapper::SelectStealingOutput& output
)
{
      // TODO: implement a work-stealing algorithm here 
}

inline 
void
MPIMapper::permit_steal_request(
   const Legion::Mapping::MapperContext ctx,
   const Legion::Mapping::Mapper::StealRequestInput& input,
   Legion::Mapping::Mapper::StealRequestOutput& output
)
{
      // TODO: implement a work stealing algorithm here
}


inline 
Legion::Mapping::PhysicalInstance
MPIMapper::choose_instance(
   const Legion::Mapping::MapperContext ctx,
   const Legion::Task& task,
   const LegionRuntime::HighLevel::RegionRequirement& req,
   Realm::Memory mem
)
{
  // decide which map to use based on the memory
  InstanceMap& imap = mem_inst_map[mem];

  // common case - we've already seen the instance before
  {
    InstanceMap::const_iterator it = imap.find(req.region);
    if(it != imap.end()) {
#ifdef MAPPER_SANITY_CHECKS
      {
        // let's double-check that it has the fields we need
        std::set<FieldID> inst_fields;
        it->second.get_fields(inst_fields);
        bool ok = true;
        for(std::set<FieldID>::const_iterator it2 =
                           req.privilege_fields.begin();
            it2 != req.privilege_fields.end();
            ++it2) {
        }
        if(!ok) {
          assert(0);
        }
      }//end if it != imap.end()
#endif
      bool ok = runtime->acquire_instance(ctx, it->second);
      assert(ok);
      return it->second;
    }// end if it != imap.end()
  }

  // less common case - we need to construct the instance and remember it for
  //  future mappings

  std::vector<LegionRuntime::HighLevel::LogicalRegion> regions(1, req.region);
  Legion::LayoutConstraintSet lcs;
  // determine which fields to add based on the field space, which we
  //  identify via a semantic tag
  {
    const void *fs_tag_ptr;
    size_t fs_tag_size;
    bool ok = runtime->retrieve_semantic_information(ctx,
                                      req.region.get_field_space(),
                                      SEMANTIC_TAG_FS_TYPE,
                                      fs_tag_ptr, fs_tag_size,
                                      false /*!can_fail*/,
                                      true /*wait*/);
    assert(ok && (fs_tag_size == sizeof(FS_TYPE_VALUE_TYPE)));
    switch(*(const FS_TYPE_VALUE_TYPE *)fs_tag_ptr) {
    case FS_TYPE_PROC_SPACE:
      {
        // the region requirement has all the fields we need
        lcs.add_constraint(Legion::FieldConstraint(req.privilege_fields,
                                           false /*contiguous*/,
                                           false /*inorder*/));
        break;
      }
#if 0
    case FS_TYPE_STATE:
      {
        lcs.add_constraint(Legion::FieldConstraint(all_state_fields,
                                           true /*contiguous*/,
                                           true /*inorder*/));
        break;
      }

    case FS_TYPE_Q:
      {
        lcs.add_constraint(Legion::FieldConstraint(all_q_fields,
                                           true /*contiguous*/,
                                           true /*inorder*/));
        break;
      }

    case FS_TYPE_INT:
      {
        if(mem.kind() == Realm::Memory::GPU_FB_MEM)
          // all_gpu_temp_fields is missing some of the fields we need,
          //  so use all_temp_fields for now, which will waste a little space
          lcs.add_constraint(Legion::FieldConstraint(all_temp_fields,
                                             //all_gpu_temp_fields,
                                             true /*contiguous*/,
                                             true /*inorder*/));
        else
          lcs.add_constraint(Legion::FieldConstraint(all_temp_fields,
                                             true /*contiguous*/,
                                             true /*inorder*/));
        break;
      }

       case FS_TYPE_GHOST:
      {
        lcs.add_constraint(Legion::FieldConstraint(all_ghost_fields,
                                           true /*contiguous*/,
                                           true /*inorder*/));
        break;
      }
#endif
#ifdef USE_CEMA
    // TODO - CEMA
    case FS_TYPE_CEMA:
    case FS_TYPE_CEMA_LOCAL:
#endif
    default: assert(0);
    }
  }

  // always use Fortran-style dimension ordering
  {
    std::vector<Legion::DimensionKind> dimension_ordering(4);
    dimension_ordering[0] = DIM_X;
    dimension_ordering[1] = DIM_Y;
    dimension_ordering[2] = DIM_Z;
    dimension_ordering[3] = DIM_F;
    lcs.add_constraint(Legion::OrderingConstraint(dimension_ordering,
                                         false /*!contiguous*/));
  }

  Legion::Mapping::PhysicalInstance inst;
  bool created;
  // prevent these instances from ever being garbage collected
  bool ok = runtime->find_or_create_physical_instance(ctx,
                                                      mem,
                                                      lcs,
                                                      regions,
                                                      inst,
                                                      created,
                                                      true /*acquire*/,
                                                      GC_NEVER_PRIORITY);
  assert(ok);

  imap[req.region] = inst;
  return inst;
}//choose_instance

inline 
void
mapper_registration(
   LegionRuntime::HighLevel::Machine machine,
   LegionRuntime::HighLevel::HighLevelRuntime *rt,
   const std::set<LegionRuntime::HighLevel::Processor> &local_procs
)
{
  for (std::set<LegionRuntime::HighLevel::Processor>::const_iterator
        it = local_procs.begin();
        it != local_procs.end(); it++)
  {
    MPIMapper *mapper = new MPIMapper(machine, rt, *it);
    rt->replace_default_mapper(mapper, *it);
  }
}//mapper registration



}//namespace execution
}//namespace flecsi


#endif

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

