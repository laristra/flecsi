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

#include <cassert>
#include <cstdint>
#include <iostream>

#include <flecsi/topology/entity_storage.h>
#include <flecsi/topology/index_space.h>
#include <flecsi/topology/types.h>
#include <flecsi/utils/array_ref.h>
#include <flecsi/utils/reorder.h>

namespace flecsi {
namespace topology {

/*----------------------------------------------------------------------------*
 * class connectivity_t
 *----------------------------------------------------------------------------*/

//-----------------------------------------------------------------//
//! \class connectivity_t connectivity.h
//! \brief connectivity_t provides basic connectivity information in a
//! compressed storage format.
//-----------------------------------------------------------------//

class connectivity_t {
public:
  using id_t = utils::id_t;
  using offset_t = utils::offset_t;

  connectivity_t(const connectivity_t &) = delete;

  connectivity_t & operator=(const connectivity_t &) = delete;

  // allow move operations
  connectivity_t(connectivity_t &&) = default;
  connectivity_t & operator=(connectivity_t &&) = default;

  //! Constructor.
  connectivity_t() : index_space_(false) {}

  auto entity_storage() {
    return index_space_.storage();
  }

  template<class STORAGE_TYPE>
  void set_entity_storage(STORAGE_TYPE s) {
    index_space_.set_storage(s);
  }

  //-----------------------------------------------------------------//
  //! Clear the storage arrays for this instance.
  //-----------------------------------------------------------------//
  void clear() {
    index_space_.clear();
    offsets_.clear();
  } // clear

  //-----------------------------------------------------------------//
  //! Initialize the connectivity information from a given connectivity
  //! vector.
  //!
  //! \param cv The connectivity information.
  //-----------------------------------------------------------------//
  void init(const connection_vector_t & cv) {

    clear();

    // populate the to id's and add from offsets for each connectivity group

    size_t start = index_space_.begin_push_();

    size_t n = cv.size();

    for (size_t i = 0; i < n; ++i) {
      const id_vector_t & iv = cv[i];

      for (id_t id : iv) {
        index_space_.batch_push_(id);
      } // for

      offsets_.add_count(static_cast<std::uint32_t>(iv.size()));
    } // for

    index_space_.end_push_(start);
  } // init

  //-----------------------------------------------------------------//
  //! Resize a connection.
  //!
  //! \param num_conns Number of connections for each group
  //-----------------------------------------------------------------//
  void resize(index_vector_t & num_conns) {
    clear();

    size_t n = num_conns.size();

    uint64_t size = 0;

    for (size_t i = 0; i < n; ++i) {
      uint32_t count = static_cast<std::uint32_t>(num_conns[i]);
      offsets_.add_count(count);
      size += count;
    } // for

    index_space_.resize_(size);
    index_space_.fill_(id_t(0));
  } // resize

  //-----------------------------------------------------------------//
  //! Push a single id into the current from group.
  //-----------------------------------------------------------------//
  void push(id_t id) {
    index_space_.push_(id);
  } // push

  //-----------------------------------------------------------------//
  //! Debugging method. Dump the raw vectors of the connection.
  //-----------------------------------------------------------------//
  std::ostream & dump(std::ostream & stream) {
    for (size_t i = 0; i < offsets_.size(); ++i) {
      offset_t oi = offsets_[i];
      for (size_t j = 0; j < oi.count(); ++j) {
        stream << index_space_(oi.start() + j).entity() << std::endl;
      }
      stream << std::endl;
    }

    stream << "=== indices" << std::endl;
    for (id_t id : index_space_.ids()) {
      stream << id.entity() << std::endl;
    } // for

    stream << "=== offsets" << std::endl;
    for (size_t i = 0; i < offsets_.size(); ++i) {
      offset_t oi = offsets_[i];
      stream << oi.start() << " : " << oi.count() << std::endl;
    } // for
    return stream;
  } // dump

  void dump() {
    dump(std::cout);
  } // dump

  //-----------------------------------------------------------------//
  //! Get the to id's vector.
  //-----------------------------------------------------------------//
  const auto & get_entities() const {
    return index_space_.id_storage();
  }
  //-----------------------------------------------------------------//
  //! Get the entities of the specified from index.
  //-----------------------------------------------------------------//
  id_t * get_entities(size_t index) {
    assert(index < offsets_.size());
    return index_space_.id_array() + offsets_[index].start();
  }

  //-----------------------------------------------------------------//
  //! Get the entities of the specified from index and return the count.
  //-----------------------------------------------------------------//
  id_t * get_entities(size_t index, size_t & count) {
    assert(index < offsets_.size());
    offset_t o = offsets_[index];
    count = o.count();
    return index_space_.id_array() + o.start();
  }

  //-----------------------------------------------------------------//
  //! Get the entities of the specified from index and return the count.
  //-----------------------------------------------------------------//
  auto get_entity_vec(size_t index) const {
    assert(index < offsets_.size());
    offset_t o = offsets_[index];
    return utils::make_array_ref(
        index_space_.id_array() + o.start(), o.count());
  }

  //-----------------------------------------------------------------//
  //! Get the entities of the specified from index and return the count.
  //-----------------------------------------------------------------//
  void reverse_entities(size_t index) {
    assert(index < offsets_.size());
    offset_t o = offsets_[index];
    std::reverse(
        index_space_.index_begin_() + o.start(),
        index_space_.index_begin_() + o.end());
  }

  //-----------------------------------------------------------------//
  //! Get the entities of the specified from index and return the count.
  //-----------------------------------------------------------------//
  template<class U>
  void reorder_entities(size_t index, U && order) {
    assert(index < offsets_.size());
    offset_t o = offsets_[index];
    assert(order.size() == o.count());
    utils::reorder(
        order.begin(), order.end(), index_space_.id_array() + o.start());
  }

  //-----------------------------------------------------------------//
  //! True if the connectivity is empty (hasn't been populated).
  //-----------------------------------------------------------------//
  bool empty() const {
    return index_space_.empty();
  }

  //-----------------------------------------------------------------//
  //! Set a single connection.
  //-----------------------------------------------------------------//
  void set(size_t from_local_id, id_t to_id, size_t pos) {
    index_space_(offsets_[from_local_id].start() + pos) = to_id;
  }

  //-----------------------------------------------------------------//
  //! Return the number of from entities.
  //-----------------------------------------------------------------//
  size_t from_size() const {
    return offsets_.size();
  }

  //-----------------------------------------------------------------//
  //! Return the number of to entities.
  //-----------------------------------------------------------------//
  size_t to_size() const {
    return index_space_.size();
  }

  //-----------------------------------------------------------------//
  //! Set/init the connectivity use by compute topology methods like transpose.
  //-----------------------------------------------------------------//
  template<size_t DOM, size_t NUM_DOMAINS>
  void set(entity_vector_t<NUM_DOMAINS> & ev, connection_vector_t & conns) {
    clear();

    size_t n = conns.size();

    size_t size = 0;

    for (size_t i = 0; i < n; i++) {
      uint32_t count = conns[i].size();
      offsets_.add_count(count);
      size += count;
    }

    index_space_.begin_push_(size);

    for (size_t i = 0; i < n; ++i) {
      const id_vector_t & conn = conns[i];
      uint64_t m = conn.size();

      for (size_t j = 0; j < m; ++j) {
        index_space_.batch_push_(ev[conn[j]]->template global_id<DOM>());
      }
    }
  }

  const auto & to_id_storage() const {
    return index_space_.id_storage();
  }

  auto & to_id_storage() {
    return index_space_.id_storage_();
  }

  auto & get_index_space() {
    return index_space_;
  }

  auto & get_index_space() const {
    return index_space_;
  }

  auto range(size_t i) const {
    return offsets_.range(i);
  }

  auto & offsets() {
    return offsets_;
  }

  const auto & offsets() const {
    return offsets_;
  }

  void add_count(uint32_t count) {
    offsets_.add_count(count);
  }

  //-----------------------------------------------------------------//
  //! End a from entity group by setting the end offset in the
  //! from connection vector.
  //-----------------------------------------------------------------//
  void end_from() {
    offsets_.add_end(index_space_.size());
  } // end_from

  index_space_u<entity_base_ *, false, true, false, void, entity_storage_t>
      index_space_;

  offset_storage_t offsets_;
}; // class connectivity_t

} // namespace topology
} // namespace flecsi
