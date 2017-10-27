/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_mutator_handle_h
#define flecsi_mutator_handle_h

#include "flecsi/data/common/data_types.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Sep 28, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

template<
  typename T,
  typename MUTATOR_POLICY
>
class mutator_handle_base__ : public MUTATOR_POLICY{
public:
  using entry_value_t = data::sparse_entry_value__<T>;

  using offset_t = data::sparse_data_offset_t;

  using index_t = uint64_t;

  struct partition_info_t{
    size_t count[3];
    size_t start[3];
    size_t end[3];
  };

  struct commit_info_t{
    offset_t* offsets;
    entry_value_t* entries[3];
  };

  //--------------------------------------------------------------------------//
  //! Default constructor.
  //--------------------------------------------------------------------------//

  mutator_handle_base__(
    size_t num_exclusive,
    size_t num_shared,
    size_t num_ghost,
    size_t max_entries_per_index,
    size_t num_slots
  )
  : num_entries_(num_exclusive + num_shared + num_ghost),
  num_exclusive_(num_exclusive),
  max_entries_per_index_(max_entries_per_index),
  num_slots_(num_slots),
  num_exclusive_insertions_(0) 
{
    pi_.count[0] = num_exclusive;
    pi_.count[1] = num_shared;
    pi_.count[2] = num_ghost;

    pi_.start[0] = 0;
    pi_.end[0] = num_exclusive;
    
    pi_.start[1] = num_exclusive;
    pi_.end[1] = num_exclusive + num_shared;
    
    pi_.start[2] = pi_.end[1];
    pi_.end[2] = pi_.end[1] + num_ghost;
  }

  mutator_handle_base__(const mutator_handle_base__& b) = default;

  ~mutator_handle_base__(){}

  void init(){
    offsets_ = new offset_t[num_entries_]; 
    entries_ = new entry_value_t[num_entries_ * num_slots_];
    spare_map_ = new spare_map_t;    
  }
  
  T &
  operator () (
    index_t index,
    index_t entry
  )
  {
    assert(offsets_ && "uninitialized mutator");
    assert(index < num_entries_);

    offset_t& offset = offsets_[index]; 

    index_t n = offset.count();

    if(n >= num_slots_) {
      if(index < num_exclusive_){
        num_exclusive_insertions_++;
      }
      
      return spare_map_->emplace(index,
        entry_value_t(entry))->second.value;
    } // if

    entry_value_t * start = entries_ + index * num_slots_;
    entry_value_t * end = start + n;

    entry_value_t * itr =
      std::lower_bound(start, end, entry_value_t(entry),
        [](const entry_value_t & e1, const entry_value_t & e2) -> bool{
          return e1.entry < e2.entry;
        });

    // if we are attempting to create an entry that already exists
    // just over-write the value and exit.  
    if ( itr != end && itr->entry == entry) {
      return itr->value;
    }

    while(end != itr) {
      *(end) = *(end - 1);
      --end;
    } // while

    itr->entry = entry;

    if(index < num_exclusive_){
      num_exclusive_insertions_++;
    }

    offset.set_count(n + 1);

    return itr->value;
  } // operator ()

  void dump(){
    for(size_t p = 0; p < 3; ++p){
      switch(p){
        case 0:
          std::cout << "exclusive: " << std::endl;
          break;
        case 1:
          std::cout << "shared: " << std::endl;
          break;
        case 2:
          std::cout << "ghost: " << std::endl;
          break;
        default:
          break;
      }

      size_t start = pi_.start[p];
      size_t end = pi_.end[p];

      for(size_t i = start; i < end; ++i){
        const offset_t& offset = offsets_[i];
        std::cout << "  index: " << i << std::endl;
        for(size_t j = 0; j < offset.count(); ++j){
          std::cout << "    " << entries_[i * num_slots_ + j].entry << 
            " = " << entries_[i * num_slots_ + j].value << std::endl;
        }

        auto p = spare_map_->equal_range(i);
        auto itr = p.first;
        while(itr != p.second){
          std::cout << "    +" << itr->second.entry << " = " << 
            itr->second.value << std::endl;
          ++itr;
        }
      }      
    }
  }

  size_t num_exclusive_insertions() const{
    return num_exclusive_insertions_;
  }

  void
  erase(
    size_t index,
    size_t entry
  )
  {
    if(!erase_set_){
      erase_set_ = new erase_set_t;
    }

    erase_set_->emplace(std::make_pair(index, entry));
  }

  void commit(commit_info_t* ci){
    assert(offsets_ && "uninitialized mutator");

    size_t num_exclusive_entries = ci->entries[1] - ci->entries[0];

    entry_value_t* cbuf = new entry_value_t[num_exclusive_entries];

    entry_value_t* entries = ci->entries[0];
    offset_t* offsets = ci->offsets;

    entry_value_t* cptr = cbuf;
    entry_value_t* eptr = entries;

    size_t offset = 0;

    for(size_t i = 0; i < num_exclusive_; ++i){
      const offset_t& oi = offsets_[i];
      offset_t& coi = offsets[i];

      entry_value_t* sptr = entries_ + i * num_slots_;

      size_t num_existing = coi.count();
      size_t used_slots = oi.count();

      size_t num_merged = 
        merge(i, eptr, num_existing, sptr, used_slots, *spare_map_, cptr);

      eptr += num_existing;
      coi.set_offset(offset);

      if(num_merged > 0){
        cptr += num_merged;
        coi.set_count(num_merged);
        offset += num_merged;
      }
    }

    assert(cptr - cbuf <= num_exclusive_entries);
    std::memcpy(entries, cbuf, sizeof(entry_value_t) * (cptr - cbuf));
    delete[] cbuf;

    size_t start = num_exclusive_;
    size_t end = start + pi_.count[1] + pi_.count[2];

    cbuf = new entry_value_t[max_entries_per_index_];

    for(size_t i = start; i < end; ++i){
      entry_value_t* eptr = ci->entries[1] + max_entries_per_index_ * i;

      const offset_t& oi = offsets_[i];
      offset_t& coi = offsets[i];

      entry_value_t* sptr = entries_ + i * num_slots_;

      size_t num_existing = coi.count();
      
      size_t used_slots = oi.count();

      size_t num_merged = 
        merge(i, eptr, num_existing, sptr, used_slots, *spare_map_, cbuf);

      if(num_merged > 0){
        assert(num_merged <= max_entries_per_index_);
        std::memcpy(eptr, cbuf, sizeof(entry_value_t) * num_merged);
        coi.set_count(num_merged);         
      }
    }

    delete[] cbuf;

    delete[] entries_;
    entries_ = nullptr;

    delete[] offsets_;
    offsets_ = nullptr;

    delete spare_map_;
    spare_map_ = nullptr;
  }

  size_t num_exclusive() const{
    return pi_.count[0];
  }

  size_t num_shared() const{
    return pi_.count[1];
  }

  size_t num_ghost() const{
    return pi_.count[2];
  }

  size_t max_entries_per_index() const{
    return max_entries_per_index_;
  }

private:
  using spare_map_t = std::multimap<size_t, entry_value_t>;
  using erase_set_t = std::set<std::pair<size_t, size_t>>;

  partition_info_t pi_;
  size_t num_exclusive_;
  size_t max_entries_per_index_;
  size_t num_exclusive_insertions_;
  size_t num_slots_;
  size_t num_entries_;
  offset_t* offsets_ = nullptr;
  entry_value_t* entries_ = nullptr;
  spare_map_t* spare_map_ = nullptr;
  erase_set_t * erase_set_ = nullptr;

  size_t merge(size_t index,
               entry_value_t* existing,
               size_t num_existing,
               entry_value_t* slots,
               size_t num_slots,
               const spare_map_t& spare_map,
               entry_value_t* dest){
    constexpr size_t end = std::numeric_limits<size_t>::max();
    entry_value_t* existing_end = existing + num_existing;
    entry_value_t* slots_end = slots + num_slots;

    entry_value_t* dest_start = dest;

    auto p = spare_map.equal_range(index);
    auto itr = p.first;

    size_t spare_entry = itr != p.second ? itr->second.entry : end;
    size_t slot_entry = slots < slots_end ? slots->entry : end;
    size_t existing_entry = existing < existing_end ? existing->entry : end;

    for(;;){
      if(spare_entry < end &&
         spare_entry <= slot_entry && 
         spare_entry <= existing_entry){

        dest->entry = spare_entry;
        dest->value = itr->second.value;
        ++dest;

        while(slot_entry == spare_entry){
          slot_entry = ++slots < slots_end ? slots->entry : end;
        }

        while(existing_entry == spare_entry){
          existing_entry = ++existing < existing_end ? existing->entry : end;
        } 

        spare_entry = ++itr != p.second ? itr->second.entry : end;
      }
      else if(slot_entry < end && slot_entry <= existing_entry){
        dest->entry = slot_entry;
        dest->value = slots->value;
        ++dest;

        while(existing_entry == slot_entry){
          existing_entry = ++existing < existing_end ? existing->entry : end;
        }  

        slot_entry = ++slots < slots_end ? slots->entry : end;
      }
      else if(existing_entry < end){
        dest->entry = existing_entry;
        dest->value = existing->value;
        ++dest;

        existing_entry = ++existing < existing_end ? existing->entry : end;
      }
      else{
        break;
      }
    }

    return dest - dest_start;
  }
};

} // namespace flecsi

#include "flecsi/runtime/flecsi_runtime_data_handle_policy.h"

namespace flecsi {

template<
  typename T
>  
using mutator_handle__ = mutator_handle_base__<
  T,
  FLECSI_RUNTIME_MUTATOR_HANDLE_POLICY
>;

} // namespace flecsi

#endif // flecsi_mutator_handle_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
