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

#include <flecsi/data/mutator.h>
#include <flecsi/data/mutator_handle.h>

namespace flecsi {

//----------------------------------------------------------------------------//
//! The mutator_base_t type provides an empty base type for
//! compile-time
//! identification of data handle objects.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

struct sparse_mutator_base_t {};

//----------------------------------------------------------------------------//
//! The mutator type captures information about permissions
//! and specifies a data policy. The sparse mutator uses a temporary slots
//! buffer and overflow "spare" map for insertions which are then commited
//! to the persistent sparse data buffer by the sparse handle's commit method.
//! A mutator is instantiated with a fixed number of slots which for optimal
//! performance should roughly approximate the expected number of entry
//! insertions per index. operator() is used to insert entries at a given index
//! into these buffers, and keeps entries in sorted order per index. Entries
//! may also be deleted with erase().
//!
//! @tparam T                     The data type referenced by the handle.
//!
//! @ingroup data
//----------------------------------------------------------------------------//

template<typename T>
struct mutator_u<data::sparse, T> : public mutator_u<data::ragged, data::sparse_entry_value_u<T>>,
                                    public sparse_mutator_base_t {
  using entry_value_t = data::sparse_entry_value_u<T>;
  using handle_t = mutator_handle_u<entry_value_t>;
  using offset_t = typename handle_t::offset_t;
  using erase_set_t = typename handle_t::erase_set_t;

  using base_t = mutator_u<data::ragged, entry_value_t>;

  //--------------------------------------------------------------------------//
  //! Copy constructor.
  //--------------------------------------------------------------------------//

  mutator_u(const handle_t & h) : base_t(h) {}

  T & operator()(size_t index, size_t entry) {
    size_t ragged_idx;
    auto itr = lower_bound(index, entry, &ragged_idx);

    // if we are attempting to create an entry that already exists
    // just over-write the value and exit.
    if(itr && itr->entry == entry) {
      return itr->value;
    }

    // otherwise, create a new entry
    auto & ragged = static_cast<base_t &>(*this);
    auto ritr = ragged.insert(index, ragged_idx, {entry, T()});
    return ritr->value;

  } // operator ()

#if 0
  void dump() {
    auto & h_ = base_t::h_;
    for(size_t p = 0; p < 3; ++p) {
      switch(p) {
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

      size_t start = h_.pi_.start[p];
      size_t end = h_.pi_.end[p];

      for(size_t i = start; i < end; ++i) {
        const offset_t & offset = h_.offsets_[i];
        std::cout << "  index: " << i << std::endl;
        for(size_t j = 0; j < offset.count(); ++j) {
          std::cout << "    " << h_.entries_[i * h_.num_slots_ + j].entry
                    << " = " << h_.entries_[i * h_.num_slots_ + j].value
                    << std::endl;
        }

        auto p = h_.spare_map_->equal_range(i);
        auto itr = p.first;
        while(itr != p.second) {
          std::cout << "    +" << itr->second.entry << " = "
                    << itr->second.value << std::endl;
          ++itr;
        }
      }
    }
  }
#endif

  void erase(size_t index, size_t entry) {
    size_t ragged_idx;
    auto itr = lower_bound(index, entry, &ragged_idx);

    // if we are attempting to erase an entry that doesn't exist,
    // then just return
    if(!itr || itr->entry != entry) {
      return;
    }

    // otherwise, erase
    auto & ragged = static_cast<base_t &>(*this);
    ragged.erase(index, ragged_idx);

  } // erase

  // for row 'index', return pointer to first entry not less
  // than 'entry'
  entry_value_t * lower_bound(size_t index, size_t entry, size_t * pos = nullptr) {
    auto & h_ = base_t::h_;
    assert(h_.offsets_ && "uninitialized mutator");
    assert(index < h_.num_entries_);

    offset_t & offset = h_.offsets_[index];

    size_t n = offset.count();
    size_t nnew = h_.new_count(index);

    entry_value_t * start = h_.entries_ + offset.start();
    entry_value_t * end = start + std::min(n, nnew);

    // try to find entry in overflow, if appropriate
    bool use_overflow = (nnew > n &&
            (n == 0 || entry > end[-1].entry));
    if(use_overflow) {
      auto & overflow = h_.overflow_map_->at(index);
      start = overflow.data();
      end = start + (nnew - n);
    }

    // find where entry should be
    entry_value_t * itr = std::lower_bound(start, end, entry_value_t(entry),
      [](const entry_value_t & e1, const entry_value_t & e2) -> bool {
        return e1.entry < e2.entry;
      });

    if(pos) {
      size_t ragged_idx = itr - start;
      if(use_overflow) ragged_idx += n;
      *pos = ragged_idx;
    }

    return (itr == end ? nullptr : itr);

  } // lower_bound

}; // mutator_u

template<typename T>
using sparse_mutator_u = mutator_u<data::sparse, T>;

template<typename T>
using sparse_mutator = sparse_mutator_u<T>;

} // namespace flecsi
