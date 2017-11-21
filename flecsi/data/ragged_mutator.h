/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_ragged_mutator_h
#define flecsi_ragged_mutator_h

#include "flecsi/data/mutator_handle.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Nov 14, 2017
//----------------------------------------------------------------------------//

namespace flecsi {

//----------------------------------------------------------------------------//
//! The ragged_mutator_base_t type provides an empty base type for 
//! compile-time
//! identification of data handle objects.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct ragged_mutator_base_t {};

//----------------------------------------------------------------------------//
//! The ragged_mutator type captures information about permissions
//! and specifies a data policy.
//!
//! @tparam T                     The data type referenced by the handle.
//! @tparam EXCLUSIVE_PERMISSIONS The permissions required on the exclusive
//!                               indices of the index partition.
//! @tparam SHARED_PERMISSIONS    The permissions required on the shared
//!                               indices of the index partition.
//! @tparam GHOST_PERMISSIONS     The permissions required on the ghost
//!                               indices of the index partition.
//! @tparam DATA_POLICY           The data policy for this handle type.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

template<
  typename T
>
struct ragged_mutator : public ragged_mutator_base_t {
  using handle_t = mutator_handle__<T>;
  using offset_t = typename handle_t::offset_t;
  using entry_value_t = typename handle_t::entry_value_t;
  using erase_set_t = typename handle_t::erase_set_t;
  using size_map_t = typename handle_t::size_map_t;

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //--------------------------------------------------------------------------//

  ragged_mutator(const mutator_handle__<T>& h)
  : h_(h){
    assert(!h_.size_map_ && "expected null size map");
    h_.size_map_ = new size_map_t;
  }

  T &
  operator () (
    size_t index,
    size_t ragged_index
  )
  {
    assert(h_.offsets_ && "uninitialized ragged_mutator");
    assert(index < h_.num_entries_);

    offset_t& offset = h_.offsets_[index]; 

    size_t n = offset.count();

    if(n >= h_.num_slots_) {
      if(index < h_.num_exclusive_){
        (*h_.num_exclusive_insertions)++;
      }
      
      return h_.spare_map_->emplace(index,
        entry_value_t(ragged_index))->second.value;
    } // if

    entry_value_t * start = h_.entries_ + index * h_.num_slots_;
    entry_value_t * end = start + n;

    entry_value_t * itr =
      std::lower_bound(start, end, entry_value_t(ragged_index),
        [](const entry_value_t & e1, const entry_value_t & e2) -> bool{
          return e1.entry < e2.entry;
        });

    // if we are attempting to create an entry that already exists
    // just over-write the value and exit.  
    if ( itr != end && itr->entry == ragged_index) {
      return itr->value;
    }

    while(end != itr) {
      *(end) = *(end - 1);
      --end;
    } // while

    itr->entry = ragged_index;

    if(index < h_.num_exclusive_){
      (*h_.num_exclusive_insertions)++;
    }

    offset.set_count(n + 1);

    return itr->value;
  } // operator ()

  void resize(size_t index, size_t length){
    h_.size_map_->emplace(index, length);
  }

  void
  erase(
    size_t index,
    size_t ragged_index
  )
  {
    if(!h_.erase_set_){
      h_.erase_set_ = new erase_set_t;
    }

    h_.erase_set_->emplace(std::make_pair(index, ragged_index));
  }

  handle_t h_;  
};

} // namespace flecsi

#endif // flecsi_ragged_mutator_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
