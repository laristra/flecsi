/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
// \file legion/dpd.cc
// \authors nickm
// \date Initial file creation: Dec 1, 2016
///

#include "flecsi/execution/legion/dpd.h"

#include <iostream>

using namespace std;
using namespace Legion;
using namespace LegionRuntime;

namespace flecsi {
namespace execution {

  namespace{

    const size_t MAX_INDICES = 1024;

    struct init_data_args{
      size_t reserve;
      size_t value_size;
    };

    struct commit_data_args{
      LogicalRegion entry_values_lr;
      size_t value_size;
      legion_dpd::index_pair indices[MAX_INDICES];
      legion_dpd::partition_metadata md;
    };

  } // namespace

  void legion_dpd::init_connectivity_task(
    const Task* task,
    const std::vector<PhysicalRegion>& regions,
    Context ctx, Runtime* runtime){

    LogicalRegion ent_from_lr = regions[0].get_logical_region();
    IndexSpace ent_from_is = ent_from_lr.get_index_space();

    LogicalRegion ent_from_lr_write = regions[1].get_logical_region();
    IndexSpace ent_from_is_write = ent_from_lr_write.get_index_space();
    size_t connectivity_field_id = task->regions[1].instance_fields[0];

    LogicalRegion to_lr = regions[2].get_logical_region();
    IndexSpace to_is = to_lr.get_index_space();

    LogicalRegion raw_connectivity_lr = regions[3].get_logical_region();
    IndexSpace raw_connectivity_is = raw_connectivity_lr.get_index_space();

    LogicalRegion ent_to_lr = regions[4].get_logical_region();
    IndexSpace ent_to_is = ent_to_lr.get_index_space();    

    auto from_ac = 
      regions[1].get_field_accessor(
      connectivity_field_id).typeify<ptr_count>();

    auto to_ac = 
      regions[2].get_field_accessor(PTR_FID).typeify<ptr_t>();

    auto raw_connectivity_ac = 
      regions[3].get_field_accessor(ENTITY_PAIR_FID).typeify<entity_pair>();

    multimap<size_t, ptr_t> connection_map;

    size_t act_count;
    IndexIterator raw_connectivity_itr(runtime, ctx, raw_connectivity_is);
    while(raw_connectivity_itr.has_next()){
      ptr_t ptr = raw_connectivity_itr.next();
      const entity_pair& p = raw_connectivity_ac.read(ptr);
      IndexIterator ent_to(runtime, ctx, ent_to_is);
      ent_to.next_span(act_count, p.e2);
      assert(ent_to.has_next());
      connection_map.emplace(p.e1, ent_to.next());
    }

    if(connection_map.empty()){
      return;
    }

    IndexIterator from_itr(runtime, ctx, ent_from_is);
    IndexIterator to_itr(runtime, ctx, to_is);

    size_t count = 0;
    ptr_t to_start = to_itr.next();
    ptr_t to_ptr = to_start;
    size_t last = connection_map.begin()->first;

    for(auto& itr : connection_map){
      if(itr.first != last){
        ptr_count pc;
        pc.ptr = to_start;
        pc.count = count;
        assert(from_itr.has_next());
        from_ac.write(from_itr.next(), pc);
        last = itr.first;
        count = 0;
        to_start = to_ptr;
      }
      
      to_ac.write(to_ptr, itr.second);
      ++count;

      to_ptr = to_itr.next();
    }

    ptr_count pc;
    pc.ptr = to_start;
    pc.count = count;
    assert(from_itr.has_next());
    from_ac.write(from_itr.next(), pc);
  }

  void legion_dpd::init_data_task(
    const Task* task,
    const std::vector<PhysicalRegion>& regions,
    Context ctx, Runtime* runtime){

    legion_helper h(runtime, ctx);

    const init_data_args& args = *(init_data_args*)task->args;

    size_t p = task->index_point.point_data[0];

    LogicalRegion ent_from_lr = regions[0].get_logical_region();
    IndexSpace ent_from_is = ent_from_lr.get_index_space();

    auto from_ac = 
      regions[0].get_field_accessor(OFFSET_COUNT_FID).
      typeify<offset_count>();

    auto meta_ac = regions[1].get_field_accessor(PARTITION_METADATA_FID).
      typeify<partition_metadata>();
    
    partition_metadata md;
    md.partition = p;
    md.reserve = args.reserve;
    md.size = 0;

    {
      IndexSpace is = h.create_index_space(0, args.reserve - 1);
      FieldSpace fs = h.create_field_space();
      FieldAllocator a = h.create_field_allocator(fs);
      a.allocate_field(sizeof(size_t) + args.value_size, ENTRY_VALUE_FID);
      md.lr = h.create_logical_region(is, fs);
    }

    meta_ac.write(DomainPoint::from_point<1>(p), md);

    offset_count oc;
    oc.offset = 0;
    oc.count = 0;

    IndexIterator from_itr(runtime, ctx, ent_from_is);
    while(from_itr.has_next()){
      ptr_t from_ptr = from_itr.next();
      from_ac.write(from_ptr, oc);
    }
  }

  void legion_dpd::create_data_(partitioned_unstructured& indices,
                                size_t start_reserve,
                                size_t value_size){
    from_ = indices;

    Domain cd = 
      runtime_->get_index_partition_color_space(context_, from_.ip);
    Rect<1> rect = cd.get_rect<1>();
    size_t num_partitions = rect.hi[0] + 1;

    {
      IndexSpace is = h.create_index_space(0, num_partitions - 1);
      
      FieldSpace fs = h.create_field_space();
      
      FieldAllocator a = h.create_field_allocator(fs);
      a.allocate_field(sizeof(partition_metadata), PARTITION_METADATA_FID);
      partition_metadata_lr_ = h.create_logical_region(is, fs);

      Blockify<1> coloring(1);

      partition_metadata_ip_ = 
        runtime_->create_index_partition(context_, is, coloring);
    }

    ArgumentMap arg_map;

    Domain d = h.domain_from_rect(0, num_partitions - 1);

    init_data_args args;
    args.reserve = start_reserve;
    args.value_size = value_size;

    IndexLauncher
      il(INIT_DATA_TID, d,
         TaskArgument(&args, sizeof(args)), arg_map);

    LogicalPartition ent_from_lp =
      runtime_->get_logical_partition(context_, from_.lr, from_.ip);

    il.add_region_requirement(
          RegionRequirement(ent_from_lp, 0, 
                            WRITE_DISCARD, EXCLUSIVE, from_.lr));

    il.region_requirements[0].add_field(OFFSET_COUNT_FID);

    LogicalPartition meta_lp = runtime_->get_logical_partition(context_,
      partition_metadata_lr_, partition_metadata_ip_);

    il.add_region_requirement(
      RegionRequirement(meta_lp, 0, WRITE_DISCARD, EXCLUSIVE, 
                        partition_metadata_lr_));
    il.region_requirements[1].add_field(PARTITION_METADATA_FID);

    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();
  }

  void legion_dpd::update_partition_metadata(size_t partition){
    partition_metadata md;

    Domain partition_domain = h.domain_from_point(partition);

    LogicalPartition lp =
      runtime_->get_logical_partition(context_, partition_metadata_lr_, 
                                      partition_metadata_ip_);

    {
      ArgumentMap arg_map;

      IndexLauncher il(GET_PARTITION_METADATA_TID, partition_domain,
        TaskArgument(nullptr, 0), arg_map);

      il.add_region_requirement(
            RegionRequirement(lp, 0, 
                              READ_ONLY, EXCLUSIVE, partition_metadata_lr_));
      il.region_requirements[0].add_field(PARTITION_METADATA_FID);

      FutureMap fm = h.execute_index_space(il);
      fm.wait_all_results();
      DomainPoint dp = DomainPoint::from_point<1>(make_point(partition));
      md = fm.get_result<partition_metadata>(dp);
    }


    {
      ArgumentMap arg_map;

      IndexLauncher il(PUT_PARTITION_METADATA_TID, partition_domain,
        TaskArgument(&md, sizeof(md)), arg_map);

      il.add_region_requirement(
            RegionRequirement(lp, 0, 
                              WRITE_DISCARD, EXCLUSIVE,
                              partition_metadata_lr_));
      
      il.region_requirements[0].add_field(PARTITION_METADATA_FID);

      FutureMap fm = h.execute_index_space(il);
      fm.wait_all_results();
    }

  }

  void legion_dpd::commit_(commit_data<char>& cd, size_t value_size){
    ArgumentMap arg_map;

    Domain d = h.domain_from_point(cd.partition);

    size_t size = (sizeof(size_t) + value_size) * cd.num_slots;

    size_t partition = cd.partition;

    arg_map.set_point(h.domain_point(partition),
      TaskArgument(cd.entries, size));

    commit_data_args args;
    args.value_size = value_size;
    copy(cd.indices, cd.indices + cd.num_indices, args.indices);

    partition_metadata md = get_partition_metadata(partition);

    size_t args_size = sizeof(commit_data_args) - 
      ((MAX_INDICES - cd.num_indices) * sizeof(size_t) * 2);

    IndexLauncher il(COMMIT_DATA_TID, d,
      TaskArgument(&args, args_size), arg_map);

    il.add_region_requirement(
          RegionRequirement(md.lr, 0, READ_WRITE, EXCLUSIVE, md.lr));
    il.region_requirements[0].add_field(ENTRY_VALUE_FID);


    LogicalPartition lp2 =
      runtime_->get_logical_partition(context_, from_.lr, from_.ip);
    il.add_region_requirement(
          RegionRequirement(lp2, 0, 
                            READ_WRITE, EXCLUSIVE, from_.lr));
    il.region_requirements[1].add_field(OFFSET_COUNT_FID);

    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();

    DomainPoint dp = DomainPoint::from_point<1>(make_point(partition));
    md = fm.get_result<partition_metadata>(dp);
    put_partition_metadata(md);
  }

  legion_dpd::partition_metadata
  legion_dpd::get_partition_metadata(size_t partition){
    IndexSpace is = partition_metadata_lr_.get_index_space();

    LogicalPartition lp =
      runtime_->get_logical_partition(context_,
      partition_metadata_lr_, partition_metadata_ip_);

    Domain d = h.domain_from_point(partition);

    ArgumentMap arg_map;

    IndexLauncher il(GET_PARTITION_METADATA_TID, d,
                     TaskArgument(nullptr, 0), arg_map);

    il.add_region_requirement(
          RegionRequirement(lp, 0, 
                            READ_ONLY, EXCLUSIVE, partition_metadata_lr_));
    
    il.region_requirements[0].add_field(PARTITION_METADATA_FID);

    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();
    DomainPoint dp = DomainPoint::from_point<1>(make_point(partition));
    partition_metadata md = fm.get_result<partition_metadata>(dp);
    return md;
  }

  void legion_dpd::put_partition_metadata(const partition_metadata& md){
    Domain d = h.domain_from_point(md.partition);

    ArgumentMap arg_map;

    IndexLauncher il(PUT_PARTITION_METADATA_TID, d,
                     TaskArgument(&md, sizeof(md)), arg_map);

    LogicalPartition lp =
      runtime_->get_logical_partition(context_,
      partition_metadata_lr_, partition_metadata_ip_);

    il.add_region_requirement(
          RegionRequirement(lp, 0, 
                            WRITE_DISCARD, EXCLUSIVE,
                            partition_metadata_lr_));
    
    il.region_requirements[0].add_field(PARTITION_METADATA_FID);

    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();
  }

  legion_dpd::partition_metadata
  legion_dpd::get_partition_metadata_task(
    const Task* task,
    const std::vector<PhysicalRegion>& regions,
    Context context, Runtime* runtime){

    legion_helper h(runtime, context);

    size_t p = task->index_point.point_data[0];

    auto ac = regions[0].get_field_accessor(PARTITION_METADATA_FID).
      typeify<partition_metadata>();
    partition_metadata md = ac.read(DomainPoint::from_point<1>(p));
    return md;
  }

  void legion_dpd::put_partition_metadata_task(
    const Task* task,
    const std::vector<PhysicalRegion>& regions,
    Context context, Runtime* runtime){

    legion_helper h(runtime, context);

    size_t p = task->index_point.point_data[0];
    partition_metadata md = *(partition_metadata*)task->args;

    auto ac = regions[0].get_field_accessor(PARTITION_METADATA_FID).
      typeify<partition_metadata>();
    ac.write(DomainPoint::from_point<1>(p), md);
  }

  legion_dpd::partition_metadata legion_dpd::commit_data_task(
    const Task* task,
    const std::vector<PhysicalRegion>& regions,
    Context context, Runtime* runtime){

    legion_helper h(runtime, context);

    commit_data_args& args = *(commit_data_args*)task->args;
    partition_metadata& md = args.md;

    entry_value<char>* commit_entry_values = 
      *(entry_value<char>**)task->local_args;

    size_t p = task->index_point.point_data[0];

    char* entry_values = h.get_raw_buffer(regions[0], ENTRY_VALUE_FID);

    LogicalRegion ent_lr = regions[1].get_logical_region();
    IndexSpace ent_is = ent_lr.get_index_space();

    auto ent_ac = 
      regions[1].get_field_accessor(OFFSET_COUNT_FID).
      typeify<offset_count>();

    IndexIterator ent_itr(runtime, context, ent_is);
    while(ent_itr.has_next()){
      ptr_t ptr = ent_itr.next();
      const offset_count& oc = ent_ac.read(ptr);
      np(oc.offset);
      np(oc.count);
    }

    return md;
  }  

  void
  legion_dpd::create_connectivity(
    size_t from_dim,
    partitioned_unstructured& from,
    size_t to_dim,
    partitioned_unstructured& to,
    partitioned_unstructured& raw_connectivity){
    
    from_ = from;
    to_ = to;

    Domain cd = 
      runtime_->get_index_partition_color_space(context_, from.ip);
    Rect<1> rect = cd.get_rect<1>();
    size_t num_partitions = rect.hi[0] + 1;

    {
      IndexSpace is = h.create_index_space(raw_connectivity.size);
      
      FieldSpace fs = h.create_field_space();

      FieldAllocator fa = h.create_field_allocator(fs);

      fa.allocate_field(sizeof(ptr_t), PTR_FID);

      to_lr_ = h.create_logical_region(is, fs);

      IndexAllocator ia = 
        runtime_->create_index_allocator(context_, is);

      Coloring coloring;

      for(auto& itr : raw_connectivity.count_map){
        size_t p = itr.first;
        int count = itr.second;
        ptr_t start = ia.alloc(count);

        for(int i = 0; i < count; ++i){
          coloring[p].points.insert(start + i);
        }
      }

      to_ip_ = 
        runtime_->create_index_partition(context_, is, coloring, true);

    }

    ArgumentMap arg_map;

    Domain d = h.domain_from_rect(0, num_partitions - 1);

    IndexLauncher
      il(INIT_CONNECTIVITY_TID, d, TaskArgument(nullptr, 0), arg_map);

    LogicalPartition ent_from_lp =
      runtime_->get_logical_partition(context_, from_.lr, from_.ip);

    il.add_region_requirement(
          RegionRequirement(ent_from_lp, 0, 
                            READ_ONLY, EXCLUSIVE, from_.lr));

    il.region_requirements[0].add_field(ENTITY_FID);

    il.add_region_requirement(
          RegionRequirement(ent_from_lp, 0, 
                            WRITE_DISCARD, EXCLUSIVE, from_.lr));

    il.region_requirements[1].add_field(connectivity_field_id(from_dim,
      to_dim));

    LogicalPartition to_lp =
      runtime_->get_logical_partition(context_, to_lr_, to_ip_);

    il.add_region_requirement(
          RegionRequirement(to_lp, 0, 
                            WRITE_DISCARD, EXCLUSIVE, to_lr_));

    il.region_requirements[2].add_field(PTR_FID);



    LogicalPartition raw_connectivity_lp =
      runtime_->get_logical_partition(context_,
        raw_connectivity.lr, raw_connectivity.ip);

    il.add_region_requirement(
          RegionRequirement(raw_connectivity_lp, 0, 
                            WRITE_DISCARD, EXCLUSIVE, raw_connectivity.lr));

    il.region_requirements[3].add_field(ENTITY_PAIR_FID);


    il.add_region_requirement(
          RegionRequirement(to_.lr, 0, 
                            READ_ONLY, EXCLUSIVE, to_.lr));

    il.region_requirements[4].add_field(ENTITY_FID);



    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();
  }

  void legion_dpd::dump(size_t from_dim, size_t to_dim){
    RegionRequirement rr1(from_.lr, READ_ONLY, EXCLUSIVE, from_.lr);
    
    size_t cfid = connectivity_field_id(from_dim, to_dim);

    rr1.add_field(cfid);
    InlineLauncher il1(rr1);

    PhysicalRegion from_pr = runtime_->map_region(context_, il1);

    from_pr.wait_until_valid();

    RegionRequirement rr2(to_lr_, READ_ONLY, EXCLUSIVE, to_lr_);
    rr2.add_field(PTR_FID);
    InlineLauncher il2(rr2);

    PhysicalRegion to_pr = runtime_->map_region(context_, il2);

    to_pr.wait_until_valid();

    RegionRequirement rr3(to_.lr, READ_ONLY, EXCLUSIVE, to_.lr);
    rr3.add_field(ENTITY_FID);
    InlineLauncher il3(rr3);

    PhysicalRegion to_ent_pr = runtime_->map_region(context_, il3);

    to_ent_pr.wait_until_valid();

    IndexIterator from_itr(runtime_, context_, from_.lr.get_index_space());
    auto from_ac = 
      from_pr.get_field_accessor(cfid).typeify<ptr_count>();

    for(size_t i = 0; i < from_.size; ++i){
      cout << "-------- from: " << i << endl; 

      assert(from_itr.has_next());
      ptr_t from_ptr = from_itr.next();

      const ptr_count& pc = from_ac.read(from_ptr);

      ptr_t to_ptr = pc.ptr; 

      auto to_ac = to_pr.get_field_accessor(PTR_FID).typeify<ptr_t>();
      auto to_ent_ac = 
        to_ent_pr.get_field_accessor(ENTITY_FID).typeify<size_t>();

      size_t j = 0;
      size_t n = pc.count;

      while(j < n){
        size_t to_id = to_ent_ac.read(to_ac.read(to_ptr));
        cout << "-- to: " << to_id << endl;
        to_ptr++;
        ++j;
      }
    }
  }

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
