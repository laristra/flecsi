/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_mutator_handle_h
#define flecsi_mutator_handle_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Sep 28, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

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

  index_t entry;
  T value;
}; // struct entry_value__

template<
  typename T,
  typename MUTATOR_POLICY
>
class mutator_handle_base__ : public MUTATOR_POLICY{
public:
  using entry_value_t = entry_value__<T>;

  using index_t = uint64_t;

  //--------------------------------------------------------------------------//
  //! Default constructor.
  //--------------------------------------------------------------------------//

  mutator_handle_base__(
    size_t num_indices,
    size_t num_entries,
    size_t num_slots
  )
  : num_indices_(num_indices),
  num_entries_(num_entries),
  num_slots_(num_slots),
  num_insertions_(0), 
  indices_(new index_t(num_indices_)), 
  entries_(new entry_value_t[num_indices * num_slots]){}

  mutator_handle_base__(const mutator_handle_base__& b) = default;

  ~mutator_handle_base__(){}

  T &
  operator () (
    index_t index,
    index_t entry
  )
  {
    assert(indices_ && "mutator has alread been committed");
    assert(index < num_indices_ && entry < num_entries_);

    index_t n = indices_[index];

    if(n >= num_slots_) {
      ++num_insertions_;
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

    ++num_insertions_;
    ++indices_[index];

    return itr->value;
  } // operator ()

  size_t num_insertions() const{
    return num_insertions_;
  }

  void
  erase(
    size_t index,
    size_t entry
  )
  {
    assert(indices_ && "mutator has alread been committed");
    assert(index < num_indices_ && entry < num_entries_);

    if(!erase_set_){
      erase_set_ = new erase_set_t;
    }

    erase_set_->emplace(std::make_pair(index, entry));
  }

  // at the time of commit, the entries buffer should have been
  // sized (as needed) to hold num_insertions() additional entries

  void
  commit(index_t* indices,
         entry_value_t* entries,
         size_t& entries_size)
  {
    if(!indices_) {
      return;
    } // if

    constexpr size_t ev_bytes = sizeof(entry_value_t);

    size_t s = entries_size;

    entry_value_t * entries_end = entries + s;

    for(size_t i = 0; i < num_indices_; ++i) {
      size_t n = indices_[i];

      size_t pos = indices[i];

      if(n == 0) {
        indices[i + 1] = pos;
        continue;
      } // if

      auto p = spare_map_.equal_range(i);

      size_t m = distance(p.first, p.second);

      entry_value_t * start = entries_ + i * num_slots_;
      entry_value_t * end = start + n;

      auto iitr = entries + pos;

      std::copy(iitr, entries_end, iitr + n + m);
      std::copy(start, end, iitr);
      entries_end += n + m;

      auto cmp = [](const auto & k1, const auto & k2) -> bool {
        return k1.entry < k2.entry;
      };

      auto nitr = iitr + indices[i + 1];

      std::inplace_merge(iitr, nitr, nitr + n, cmp);

      if(m == 0) {
        indices[i + 1] = pos + n;
        continue;
      } // if

      indices[i + 1] = pos + n + m;

      auto vitr = nitr + n;

      for(auto itr = p.first; itr != p.second; ++itr) {
        vitr->entry = itr->second.entry;
        vitr->value = itr->second.value;
        ++vitr;
      } // for

      std::inplace_merge(iitr, nitr + n, nitr + n + m, cmp);
    } // for

    delete[] entries_;
    entries_ = nullptr;

    entries_size += num_insertions_;

    if(!erase_set_){
      indices_ = nullptr;
      return;
    }

    constexpr size_t erased_marker = std::numeric_limits<size_t>::max();

    entry_value_t * start;
    entry_value_t * end;

    size_t last = std::numeric_limits<size_t>::max();

    size_t num_erased = 0;

    for(auto p : *erase_set_){
      if(p.first != last){
        start = entries + indices[p.first];
        end = entries + indices[p.first + 1];
        last = p.first;
        indices[p.first] -= num_erased;
      }

      entry_value_t * m =
        std::lower_bound(start, end, entry_value_t(p.second),
          [](const auto & k1, const auto & k2) -> bool {
            return k1.entry < k2.entry;
        });

      m->entry = erased_marker;
      start = m + 1;
      ++num_erased;
    }

    while(last < num_indices_){
      indices[++last] -= num_erased;
    }

    std::remove_if(entries, entries_end,
      [](const auto & k) -> bool {
        return k.entry == erased_marker;
    });

    entries_size -= num_erased;

    delete erase_set_;
    erase_set_ = nullptr;
    indices_ = nullptr;
  } // commit

private:
  using spare_map_t = std::multimap<size_t, entry_value__<T>>;
  using erase_set_t = std::set<std::pair<size_t, size_t>>;

  size_t num_indices_;
  size_t num_entries_;
  size_t num_slots_;
  size_t num_insertions_ = 0;
  index_t* indices_;
  entry_value_t* entries_;
  spare_map_t spare_map_;
  erase_set_t * erase_set_ = nullptr;
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
