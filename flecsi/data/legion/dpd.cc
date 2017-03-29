/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

///
// \file legion/dpd.cc
// \authors nickm
// \date Initial file creation: Dec 1, 2016
///

#include "flecsi/data/legion/dpd.h"

#include <iostream>

#include <legion_utilities.h>
#include "flecsi/execution/task_ids.h"

using namespace std;
using namespace Legion;
using namespace LegionRuntime;

namespace flecsi {
namespace execution {

  namespace{

    const size_t MAX_INDICES = 1024;

    struct init_data_args{
      size_t reserve;
      size_t size;
      size_t value_size;
    };

    struct commit_data_args{
      LogicalRegion entry_values_lr;
      size_t value_size;
      size_t slot_size;
      size_t num_slots;
      size_t num_indices;
      size_t buf_size;
      size_t indices_buf_size;
      size_t entries_buf_size;
      legion_dpd::partition_metadata md;
    };

  } // namespace

  void
  legion_dpd::init_connectivity_task(
    const Task* task,
    const std::vector<PhysicalRegion>& regions,
    Context ctx, Runtime* runtime)
  {
 
    field_ids_t & fid_t = field_ids_t::instance(); 

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
      regions[2].get_field_accessor(fid_t.fid_ptr_t).typeify<ptr_t>();

    auto raw_connectivity_ac = 
      regions[3].get_field_accessor(fid_t.fid_entity_pair).
        typeify<entity_pair>();

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

      if(to_itr.has_next()){
        to_ptr = to_itr.next();
      }
    }

    ptr_count pc;
    pc.ptr = to_start;
    pc.count = count;
    assert(from_itr.has_next());
    from_ac.write(from_itr.next(), pc);
  }

  void
  legion_dpd::init_data_task(
    const Task* task,
    const std::vector<PhysicalRegion>& regions,
    Context ctx, Runtime* runtime)
  {

    field_ids_t & fid_t = field_ids_t::instance();

    legion_helper h(runtime, ctx);

    const init_data_args& args = *(init_data_args*)task->args;

    size_t p = task->index_point.point_data[0];

    LogicalRegion ent_from_lr = regions[0].get_logical_region();
    IndexSpace ent_from_is = ent_from_lr.get_index_space();

    auto from_ac = 
      regions[0].get_field_accessor(fid_t.fid_offset_count).
      typeify<offset_count>();

    auto meta_ac = regions[1].get_field_accessor(fid_t.fid_partition_metadata).
      typeify<partition_metadata>();
    
    partition_metadata md;
    md.partition = p;
    md.reserve = args.reserve;
    md.size = args.size;

    {
      IndexSpace is = h.create_index_space(0, args.reserve - 1);
      FieldSpace fs = h.create_field_space();
      FieldAllocator a = h.create_field_allocator(fs);
      a.allocate_field(sizeof(entry_offset), fid_t.fid_entry_offset);
      a.allocate_field(args.value_size, fid_t.fid_value);
      md.lr = h.create_logical_region(is, fs);
      DomainPointColoring coloring;

      DomainPoint dp = DomainPoint::from_point<1>(
        LegionRuntime::Arrays::make_point(p));
      coloring.emplace(dp, h.domain_from_rect(0, args.reserve - 1));
      Domain cd = h.domain_from_point(p);
      md.ip = runtime->create_index_partition(ctx, is, cd, coloring);
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

  void
  legion_dpd::create_data_(
    partitioned_unstructured& indices,
    size_t start_reserve,
    size_t start_size,
    size_t value_size)
  {
    field_ids_t & fid_t = field_ids_t::instance();

    from_ = indices;

    Domain cd = 
      runtime_->get_index_partition_color_space(context_, from_.ip);
    LegionRuntime::Arrays::Rect<1> rect = cd.get_rect<1>();
    size_t num_partitions = rect.hi[0] + 1;

    {
      IndexSpace is = h.create_index_space(0, num_partitions - 1);
      
      FieldSpace fs = h.create_field_space();
      
      FieldAllocator a = h.create_field_allocator(fs);
      a.allocate_field(sizeof(partition_metadata),fid_t.fid_partition_metadata);
      partition_metadata_lr_ = h.create_logical_region(is, fs);

      LegionRuntime::Arrays::Blockify<1> coloring(1);

      partition_metadata_ip_ = 
        runtime_->create_index_partition(context_, is, coloring);
    }

    ArgumentMap arg_map;

    Domain d = h.domain_from_rect(0, num_partitions - 1);

    init_data_args args;
    args.reserve = start_reserve;
    args.size = start_size;
    args.value_size = value_size;

    IndexLauncher
      il(task_ids_t::instance().dpd_init_data_task_id, d,
         TaskArgument(&args, sizeof(args)), arg_map);

    LogicalPartition ent_from_lp =
      runtime_->get_logical_partition(context_, from_.lr, from_.ip);

    il.add_region_requirement(
          RegionRequirement(ent_from_lp, 0, 
                            WRITE_DISCARD, EXCLUSIVE, from_.lr));

    il.region_requirements[0].add_field(fid_t.fid_offset_count);

    LogicalPartition meta_lp = runtime_->get_logical_partition(context_,
      partition_metadata_lr_, partition_metadata_ip_);

    il.add_region_requirement(
      RegionRequirement(meta_lp, 0, WRITE_DISCARD, EXCLUSIVE, 
                        partition_metadata_lr_));
    il.region_requirements[1].add_field(fid_t.fid_partition_metadata);

    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();
  }

  void
  legion_dpd::commit_(
    commit_data<char>& cd,
    size_t value_size)
  {
    field_ids_t & fid_t = field_ids_t::instance();

    ArgumentMap arg_map;

    Domain d = h.domain_from_point(cd.partition);

    size_t partition = cd.partition;

    commit_data_args args;
    args.value_size = value_size;
    args.slot_size = cd.slot_size;
    args.num_slots = cd.num_slots;
    args.num_indices = cd.num_indices;
    args.indices_buf_size = sizeof(size_t) * cd.num_indices;
    args.entries_buf_size = cd.num_slots * (sizeof(size_t) + value_size);

    Serializer serializer;
    serializer.serialize(cd.indices, args.indices_buf_size); 
    serializer.serialize(cd.entries, args.entries_buf_size);
    
    const void* args_buf = serializer.get_buffer();
    args.buf_size = serializer.get_used_bytes();

    args.md = get_partition_metadata(partition);

    arg_map.set_point(h.domain_point(partition),
                      TaskArgument(args_buf, args.buf_size));

    IndexLauncher il(task_ids_t::instance().dpd_commit_data_task_id, d,
      TaskArgument(&args, sizeof(args)), arg_map);

    LogicalPartition lp =
      runtime_->get_logical_partition(context_, args.md.lr, args.md.ip);

    il.add_region_requirement(
          RegionRequirement(lp, 0, READ_WRITE,
                            EXCLUSIVE, args.md.lr));
    
    il.region_requirements[0].add_field(fid_t.fid_entry_offset);
    il.region_requirements[0].add_field(fid_t.fid_value);

    LogicalPartition lp2 =
      runtime_->get_logical_partition(context_, from_.lr, from_.ip);
    
    il.add_region_requirement(
          RegionRequirement(lp2, 0, 
                            READ_WRITE, EXCLUSIVE, from_.lr));
    
    il.region_requirements[1].add_field(fid_t.fid_offset_count);

    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();

    DomainPoint dp = DomainPoint::from_point<1>(
      LegionRuntime::Arrays::make_point(partition));
    partition_metadata md = fm.get_result<partition_metadata>(dp);
    put_partition_metadata(md);
  }

  legion_dpd::partition_metadata
  legion_dpd::get_partition_metadata(
    size_t partition)
  {

    field_ids_t & fid_t = field_ids_t::instance();  
 
    IndexSpace is = partition_metadata_lr_.get_index_space();

    LogicalPartition lp =
      runtime_->get_logical_partition(context_,
      partition_metadata_lr_, partition_metadata_ip_);

    Domain d = h.domain_from_point(partition);

    ArgumentMap arg_map;

    IndexLauncher il(task_ids_t::instance().
                       dpd_get_partition_metadata_task_id, d,
                     TaskArgument(nullptr, 0), arg_map);

    il.add_region_requirement(
          RegionRequirement(lp, 0, 
                            READ_ONLY, EXCLUSIVE, partition_metadata_lr_));
    
    il.region_requirements[0].add_field(fid_t.fid_partition_metadata);

    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();
    DomainPoint dp = DomainPoint::from_point<1>(
      LegionRuntime::Arrays::make_point(partition));
    partition_metadata md = fm.get_result<partition_metadata>(dp);
    return md;
  }

  void legion_dpd::put_partition_metadata(const partition_metadata& md){
    field_ids_t & fid_t = field_ids_t::instance();

    Domain d = h.domain_from_point(md.partition);

    ArgumentMap arg_map;

    IndexLauncher il(task_ids_t::instance().
                       dpd_put_partition_metadata_task_id, d,
                     TaskArgument(&md, sizeof(md)), arg_map);

    LogicalPartition lp =
      runtime_->get_logical_partition(context_,
      partition_metadata_lr_, partition_metadata_ip_);

    il.add_region_requirement(
          RegionRequirement(lp, 0, 
                            WRITE_DISCARD, EXCLUSIVE,
                            partition_metadata_lr_));
    
    il.region_requirements[0].add_field(fid_t.fid_partition_metadata);

    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();
  }

  legion_dpd::partition_metadata
  legion_dpd::get_partition_metadata_task(
    const Task* task,
    const std::vector<PhysicalRegion>& regions,
    Context context, Runtime* runtime)
  {

    field_ids_t & fid_t = field_ids_t::instance();
  
    legion_helper h(runtime, context);

    size_t p = task->index_point.point_data[0];

    auto ac = regions[0].get_field_accessor(fid_t.fid_partition_metadata).
      typeify<partition_metadata>();
    partition_metadata md = ac.read(DomainPoint::from_point<1>(p));
    return md;
  }

  void
  legion_dpd::put_partition_metadata_task(
    const Task* task,
    const std::vector<PhysicalRegion>& regions,
    Context context, Runtime* runtime)
  {

    field_ids_t & fid_t = field_ids_t::instance();

    legion_helper h(runtime, context);

    size_t p = task->index_point.point_data[0];
    partition_metadata md = *(partition_metadata*)task->args;

    auto ac = regions[0].get_field_accessor(fid_t.fid_partition_metadata).
      typeify<partition_metadata>();
    ac.write(DomainPoint::from_point<1>(p), md);
  }

  legion_dpd::partition_metadata
  legion_dpd::commit_data_task(
    const Task* task,
    const std::vector<PhysicalRegion>& regions,
    Context context, Runtime* runtime)
  {

    field_ids_t & fid_t = field_ids_t::instance();

    legion_helper h(runtime, context);

    size_t p = task->index_point.point_data[0];

    commit_data_args& args = *(commit_data_args*)task->args;
    size_t value_size = args.value_size;
    size_t slot_size = args.slot_size;
    size_t num_slots = args.num_slots;
    size_t num_indices = args.num_indices;
    size_t entry_value_size = value_size + sizeof(size_t);

    partition_metadata& md = args.md;

    void* args_buf = task->local_args;

    Deserializer deserializer(args_buf, args.buf_size);
    void* indices_buf = malloc(args.indices_buf_size);
    deserializer.deserialize(indices_buf, args.indices_buf_size);

    void* entries_buf = malloc(args.entries_buf_size);
    deserializer.deserialize(entries_buf, args.entries_buf_size);

    char* commit_entry_values = (char*)entries_buf;

    size_t* commit_indices = (size_t*)indices_buf;

    entry_offset* entry_offsets;
    h.get_buffer(regions[0], entry_offsets, fid_t.fid_entry_offset);

    char* values = h.get_raw_buffer(regions[0], fid_t.fid_value);

    LogicalRegion ent_lr = regions[1].get_logical_region();
    IndexSpace ent_is = ent_lr.get_index_space();

    auto ent_ac = 
      regions[1].get_field_accessor(fid_t.fid_offset_count).
      typeify<offset_count>();

    size_t s = md.size;
    size_t c = md.reserve;
    size_t d = num_indices * num_slots;

    PhysicalRegion pr;
    bool resized = false;

    if(c - s < d){
      resized = true;

      md.reserve *= 2;

      IndexSpace is = h.create_index_space(0, md.reserve - 1);
      FieldSpace fs = h.create_field_space();
      FieldAllocator a = h.create_field_allocator(fs);
      a.allocate_field(sizeof(entry_offset), fid_t.fid_entry_offset);
      a.allocate_field(value_size, fid_t.fid_value);
      LogicalRegion lr2 = h.create_logical_region(is, fs);

      RegionRequirement rr(lr2, WRITE_DISCARD, EXCLUSIVE, lr2);
      rr.add_field(fid_t.fid_entry_offset);
      rr.add_field(fid_t.fid_value);
      InlineLauncher il(rr);

      pr = runtime->map_region(context, il);
      pr.wait_until_valid();

      entry_offset* entry_offsets2;
      h.get_buffer(pr, entry_offsets2, fid_t.fid_entry_offset);
      char* values2 = h.get_raw_buffer(pr, fid_t.fid_value);

      copy(entry_offsets, entry_offsets + md.size, entry_offsets2);
      copy(values, values + md.size * value_size, values2);

      runtime->unmap_region(context, regions[0]);

      runtime->destroy_logical_region(context, md.lr);
      runtime->destroy_index_partition(context, md.ip);
      
      md.lr = lr2;

      DomainPointColoring coloring;
      DomainPoint dp = DomainPoint::from_point<1>(
        LegionRuntime::Arrays::make_point(p));
      coloring.emplace(dp, h.domain_from_rect(0, md.reserve - 1));
      Domain cd = h.domain_from_point(p);
      
      md.ip = runtime->create_index_partition(context, is, cd, coloring);
    }

    entry_offset* entry_offsets_end = entry_offsets + md.size;

    IndexIterator ent_itr(runtime, context, ent_is);

    size_t offset = 0;

    char* value_ptr = values + md.size * value_size;
    size_t value_offset = md.size;

    vector<size_t> offsets(md.size);

    size_t added = 0;

    for(size_t i = 0; i < num_indices; ++i){
      assert(ent_itr.has_next());

      ptr_t ptr = ent_itr.next();

      size_t n = commit_indices[i];

      offset_count oc = ent_ac.read(ptr);
      oc.offset = offset;

      if(n == 0){
        offset += oc.count;
        ent_ac.write(ptr, oc);
        continue;
      }

      for(int j = md.size + added; j > offset; --j){
        entry_offsets[j + n] = entry_offsets[j];
      }

      added += n;

      for(size_t j = 0; j < n; ++j){
        entry_offset& ej = entry_offsets[offset + j];
        
        size_t pos = (i * slot_size + j) * entry_value_size;

        ej.entry = 
          ((entry_offset*)(commit_entry_values + pos))->entry;

        offsets.push_back(value_offset);
        ej.offset = value_offset++;

        char* value = commit_entry_values + pos + sizeof(size_t);
        memcpy(value_ptr, value, value_size);
        value_ptr += value_size;
      }

      auto cmp = [](const auto & k1, const auto & k2) -> bool{
        return k1.entry < k2.entry;
      };

      inplace_merge(entry_offsets + offset,
                    entry_offsets + offset + n,
                    entry_offsets + offset + oc.count + n, cmp);

      oc.count += n;
      offset += oc.count;

      ent_ac.write(ptr, oc);
    }

    for(size_t i = 0; i < md.size; ++i){
      offsets[entry_offsets[i].offset] = i;
    }

    md.size = offset;

    char* temp = new char[value_size];

    for(size_t i = 0; i < md.size; ++i){
      size_t j = offsets[i];

      if(j != i){
        memcpy(temp, values + i * value_size, value_size);
        
        memcpy(values + i * value_size,
               values + j * value_size,
               value_size);
        
        memcpy(values + j * value_size, temp, value_size);

        offsets[i] = i;
        offsets[j] = j;

        entry_offsets[i].offset = i;
        entry_offsets[j].offset = j;
      }
    }

    delete[] temp;        

    /*
    np("************");

    char* value_ptr2 = values;
    for(size_t i = 0; i < md.size; ++i){
      cout << "entry: " << entry_offsets[i].entry << 
        " value: " << *((double*)value_ptr2) << endl;
      value_ptr2 += value_size;
    }
    */

    return md;
  }  

  void
  legion_dpd::create_connectivity(
    size_t from_dim,
    partitioned_unstructured& from,
    size_t to_dim,
    partitioned_unstructured& to,
    partitioned_unstructured& raw_connectivity)
  {
    
    field_ids_t & fid_t = field_ids_t::instance();

    from_ = from;
    to_ = to;

    Domain cd = 
      runtime_->get_index_partition_color_space(context_, from.ip);
    LegionRuntime::Arrays::Rect<1> rect = cd.get_rect<1>();
    size_t num_partitions = rect.hi[0] + 1;

    {
      IndexSpace is = h.create_index_space(raw_connectivity.size);
      
      FieldSpace fs = h.create_field_space();

      FieldAllocator fa = h.create_field_allocator(fs);

      fa.allocate_field(sizeof(ptr_t), fid_t.fid_ptr_t);

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
      il(task_ids_t::instance().dpd_init_connectivity_task_id,
        d, TaskArgument(nullptr, 0), arg_map);

    LogicalPartition ent_from_lp =
      runtime_->get_logical_partition(context_, from_.lr, from_.ip);

    il.add_region_requirement(
          RegionRequirement(ent_from_lp, 0, 
                            READ_ONLY, EXCLUSIVE, from_.lr));

    il.region_requirements[0].add_field(fid_t.fid_cell);

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

    il.region_requirements[2].add_field(fid_t.fid_ptr_t);



    LogicalPartition raw_connectivity_lp =
      runtime_->get_logical_partition(context_,
        raw_connectivity.lr, raw_connectivity.ip);

    il.add_region_requirement(
          RegionRequirement(raw_connectivity_lp, 0, 
                            WRITE_DISCARD, EXCLUSIVE, raw_connectivity.lr));

    il.region_requirements[3].add_field(fid_t.fid_entity_pair);


    il.add_region_requirement(
          RegionRequirement(to_.lr, 0, 
                            READ_ONLY, EXCLUSIVE, to_.lr));

    il.region_requirements[4].add_field(fid_t.fid_vert);



    FutureMap fm = h.execute_index_space(il);
    fm.wait_all_results();
  }

  void
  legion_dpd::dump(
    size_t from_dim,
    size_t to_dim)
  {
    RegionRequirement rr1(from_.lr, READ_ONLY, EXCLUSIVE, from_.lr);

    field_ids_t & fid_t = field_ids_t::instance();    

    size_t cfid = connectivity_field_id(from_dim, to_dim);

    rr1.add_field(cfid);
    InlineLauncher il1(rr1);

    PhysicalRegion from_pr = runtime_->map_region(context_, il1);

    from_pr.wait_until_valid();

    RegionRequirement rr2(to_lr_, READ_ONLY, EXCLUSIVE, to_lr_);
    rr2.add_field(fid_t.fid_ptr_t);
    InlineLauncher il2(rr2);

    PhysicalRegion to_pr = runtime_->map_region(context_, il2);

    to_pr.wait_until_valid();

    RegionRequirement rr3(to_.lr, READ_ONLY, EXCLUSIVE, to_.lr);
    rr3.add_field(fid_t.fid_vert);
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

      auto to_ac = 
        to_pr.get_field_accessor(fid_t.fid_ptr_t).typeify<ptr_t>();
      
      auto to_ent_ac = 
        to_ent_pr.get_field_accessor(fid_t.fid_vert).typeify<size_t>();

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

  void
  legion_dpd::map_data(
    size_t partition,
    offset_count*& indices,
    entry_offset*& entries,
    void*& values)
  {
    field_ids_t & fid_t = field_ids_t::instance();    

    partition_metadata md = get_partition_metadata(partition);

    LogicalPartition ent_from_lp =
      runtime_->get_logical_partition(context_, from_.lr, from_.ip);

    RegionRequirement rr(ent_from_lp, 0, READ_ONLY, EXCLUSIVE, from_.lr);
    rr.add_field(fid_t.fid_offset_count);
    
    InlineLauncher il(rr);

    data_from_pr_ = runtime_->map_region(context_, il);

    h.get_buffer(data_from_pr_, indices, fid_t.fid_offset_count);

    LogicalPartition data_lp =
      runtime_->get_logical_partition(context_, md.lr, md.ip);

    RegionRequirement rr2(data_lp, 0, READ_ONLY, EXCLUSIVE, md.lr);
    rr2.add_field(fid_t.fid_entry_offset);

    RegionRequirement rr3(data_lp, 0, READ_WRITE, EXCLUSIVE, md.lr);
    
    rr3.add_field(fid_t.fid_value);

    InlineLauncher il2(rr2);

    InlineLauncher il3(rr3);
    data_pr_ = runtime_->map_region(context_, il2);
    data_values_pr_ = runtime_->map_region(context_, il3);

    h.get_buffer(data_pr_, indices, fid_t.fid_entry_offset);
    values = h.get_raw_buffer(data_values_pr_, fid_t.fid_value);
  }

  void
  legion_dpd::map_data_values(
    size_t partition,
    void*& values)
  {
    field_ids_t & fid_t = field_ids_t::instance();    

    partition_metadata md = get_partition_metadata(partition);

    RegionRequirement rr(md.lr, READ_WRITE, EXCLUSIVE, md.lr);
    rr.add_field(fid_t.fid_value);

    InlineLauncher il(rr);
    data_values_pr_ = runtime_->map_region(context_, il);

    data_values_pr_.wait_until_valid();

    values = h.get_raw_buffer(data_values_pr_, fid_t.fid_value);
  }

  void
  legion_dpd::unmap_data()
  {
    runtime_->unmap_region(context_, data_from_pr_);
    runtime_->unmap_region(context_, data_pr_);
    runtime_->unmap_region(context_, data_values_pr_);
  }

  void
  legion_dpd::unmap_data_values()
  {
    runtime_->unmap_region(context_, data_values_pr_);
  }

} // namespace execution 
} // namespace flecsi

/*~------------------------------------------------------------------------~--*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
