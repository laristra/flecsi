#include <vector>
#include <iostream>
#include <cassert>
#include <map>
#include <algorithm>
#include <cstring>
//#include <np.h>

double uniform(){
  return double(rand())/RAND_MAX;
}

double uniform(double a, double b){
  return a + (b - a) * uniform();
}

size_t equilikely(size_t a, size_t b){
  return uniform(a, b + 1);
}

template<typename T>
struct entry_value__
{
  using index_t = uint64_t;

  entry_value__(index_t entry)
  : entry(entry){}

  entry_value__(index_t entry, T value)
  : entry(entry),
  value(value){}

  entry_value__(){}

  bool operator<(const entry_value__& ev) const{
    return entry < ev.entry;
  }

  index_t entry;
  T value;
};

namespace flecsi {
namespace utils {

template<size_t COUNT_BITS>
class offset__{
public:
  static_assert(COUNT_BITS <= 32, "COUNT_BITS max exceeded");

  static constexpr uint64_t count_mask = (1ul << COUNT_BITS) - 1;
  static constexpr uint32_t count_max = 1ul << COUNT_BITS;

  offset__()
  : o_(0ul){}

  offset__(uint64_t start, uint32_t count)
  : o_(start << COUNT_BITS | count){}

  offset__(const offset__& prev, uint32_t count)
  : o_(prev.end() << COUNT_BITS | count){}

  uint64_t start() const{
    return o_ >> COUNT_BITS;
  }

  uint32_t count() const{
    return o_ & count_mask;
  }

  uint64_t end() const{
    return start() + count();
  }

  void
  set_count(uint32_t count)
  {
    assert(count < count_max);
    o_ = o_ & ~count_mask | count;
  }

  void
  set_offset(uint64_t offset)
  {
    o_ = (o_ & count_mask) | (offset << COUNT_BITS);
  }

  std::pair<size_t, size_t>
  range()
  const
  {
    uint64_t s = start(); 
    return {s, s + count()};    
  }

private:
  uint64_t o_;
};

} // namespace utils
} // namespace flecsi


using namespace std;
using namespace flecsi;

using offset_t = utils::offset__<16>;


template<class T>
size_t merge(size_t index,
             entry_value__<T>* existing,
             size_t num_existing,
             entry_value__<T>* slots,
             size_t num_slots,
             const std::multimap<size_t, entry_value__<T>>& spare_map,
             entry_value__<T>* dest){

  constexpr size_t end = std::numeric_limits<size_t>::max();
  entry_value__<T>* existing_end = existing + num_existing;
  entry_value__<T>* slots_end = slots + num_slots;

  entry_value__<T>* dest_start = dest;

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

template<typename T>
class mutator{
public:
  using entry_value_t = entry_value__<T>;

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

  mutator(
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
  num_exclusive_insertions_(0), 
  offsets_(new offset_t[num_entries_]), 
  entries_(new entry_value_t[num_entries_ * num_slots]){
    
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

  T &
  operator () (
    index_t index,
    index_t entry
  )
  {
    assert(entries_ && "mutator has alread been committed");
    assert(index < num_entries_);

    offset_t& offset = offsets_[index]; 

    index_t n = offset.count();

    if(n >= num_slots_) {
      if(index < num_exclusive_){
        num_exclusive_insertions_++;
      }
      
      return spare_map_.emplace(index,
        entry_value_t(entry))->second.value;
    } // if

    entry_value_t * start = entries_ + index * num_slots_;
    entry_value_t * end = start + n;

    entry_value_t * itr =
      std::lower_bound(start, end, entry_value_t(entry),
        [](const auto & k1, const auto & k2) -> bool{
          return k1.entry < k2.entry;
        });

    // if we are creating an that has already been created, just
    // over-write the value and exit.  No need to increment the
    // counters or move data.
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

        auto p = spare_map_.equal_range(i);
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

  void init_resized(offset_t* old_offsets,
                    entry_value_t* old_entries,
                    offset_t* new_offsets,
                    entry_value_t* new_entries){

  }

  void commit(commit_info_t* ci){
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
        merge(i, eptr, num_existing, sptr, used_slots, spare_map_, cptr);
      
      cptr += num_merged;
      eptr += num_existing;

      coi.set_offset(offset);
      coi.set_count(num_merged);

      offset += num_merged;
    }

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
        merge(i, eptr, num_existing, sptr, used_slots, spare_map_, cbuf);

      std::memcpy(eptr, cbuf, sizeof(entry_value_t) * num_merged);

      coi.set_count(num_merged);  
    }

    delete[] cbuf;
  }

private:
  using spare_map_t = std::multimap<size_t, entry_value__<T>>;

  partition_info_t pi_;
  size_t num_exclusive_;
  size_t max_entries_per_index_;
  size_t num_exclusive_insertions_;
  size_t num_slots_;
  size_t num_entries_;
  offset_t* offsets_;
  entry_value_t* entries_;
  entry_value_t* shared_entries_;
  spare_map_t spare_map_;
};

int main(int argc, char** argv){
  using T = double;

  using entry_value_t = entry_value__<T>;

  size_t num_exclusive = 10;
  size_t num_shared = 2;
  size_t num_ghost = 2;
  size_t num_total = num_exclusive + num_shared + num_ghost;
  size_t reserve_chunk = 8192;
  size_t reserve = reserve_chunk;
  size_t exclusive_entries = 0;

  size_t max_entries_per_index = 5;

  size_t num_phases = 20;
  size_t insertions_per_phase = 20;

  mutator<T>::commit_info_t ci;
  ci.offsets = new offset_t[num_total];
  for(size_t i = num_exclusive; i < num_total; ++i){
    offset_t& oi = ci.offsets[i];
    oi.set_offset(exclusive_entries + reserve + i * max_entries_per_index);
  }

  entry_value_t* all_entries = 
    new entry_value_t[exclusive_entries + reserve + (num_shared + num_ghost) *
      max_entries_per_index];

  ci.entries[0] = all_entries;
  ci.entries[1] = all_entries + exclusive_entries + reserve;
  ci.entries[2] = ci.entries[1] + num_shared * max_entries_per_index;

  for(size_t i = 0; i < num_phases; ++i){
    size_t num_slots = 5;

    mutator<T> m(num_exclusive, num_shared, num_ghost, max_entries_per_index, num_slots);

    for(size_t j = 0; j < insertions_per_phase; ++j){
      size_t index = equilikely(0, num_total - 1);
      size_t entry = equilikely(0, max_entries_per_index - 2);
      m(index, entry) = uniform(0, 100);
    }

    size_t num_exclusive_insertions = m.num_exclusive_insertions();

    if(num_exclusive_insertions > reserve){
      size_t old_exclusive_entries = exclusive_entries;
      size_t old_reserve = reserve;

      size_t needed = num_exclusive_insertions - reserve;

      exclusive_entries += num_exclusive_insertions;
      reserve = std::max(reserve_chunk, needed);

      entry_value_t* new_entries = 
        new entry_value_t[exclusive_entries + reserve + 
        (num_shared + num_ghost) * max_entries_per_index];
      
      std::memcpy(new_entries, all_entries, old_exclusive_entries * sizeof(T));

      std::memcpy(new_entries + exclusive_entries + reserve, 
        all_entries + old_exclusive_entries + old_reserve,
        (num_shared + num_ghost) * max_entries_per_index * sizeof(T));

      delete[] all_entries;
      all_entries = new_entries;

      for(size_t k = num_exclusive; k < num_total; ++k){
        offset_t& ok = ci.offsets[k];
        ok.set_offset(exclusive_entries + reserve + k * max_entries_per_index);
      }

      ci.entries[0] = all_entries;
      ci.entries[1] = all_entries + exclusive_entries + reserve;
      ci.entries[2] = ci.entries[1] + num_shared * max_entries_per_index;
    }
    else{
      reserve -= num_exclusive_insertions;
    }

    m.commit(&ci);
  }

  cout << "---------- committed data" << endl;

  for(size_t i = 0; i < num_total; ++i){
    const offset_t& offset = ci.offsets[i];
    std::cout << "  index: " << i << std::endl;
    for(size_t j = 0; j < offset.count(); ++j){
      std::cout << "    " << ci.entries[0][offset.start() + j].entry << 
        " = " << ci.entries[0][offset.start() + j].value << std::endl;
    }
  }
}
