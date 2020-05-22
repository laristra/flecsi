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

#include "flecsi/utils/array_ref.h"
#include <flecsi/utils/target.h>

#include <algorithm>
#include <functional>
#include <map>
#include <type_traits>
#include <vector>

namespace flecsi {
namespace topology {
struct view_tag {}; // indicates that a type can be copied cheaply

//----------------------------------------------------------------------------//
//! index_space_u is a container of entities along with one of their IDs.
//! The containers must be contiguous to support non-copying views.
//!
//! @ingroup topology
//----------------------------------------------------------------------------//
template<class T,
  template<typename, typename...> class ID_STORAGE_TYPE = flecsi::vector,
  template<typename, typename...> class STORAGE_TYPE = ID_STORAGE_TYPE>
class index_space_u
{
  template<class C, class X>
  using copy_const = std::conditional_t<std::is_const_v<C>, const X, X>;

public:
  //! ID type
  using id_t = copy_const<T, typename std::remove_pointer<T>::type::id_t>;

  //! ID storage type
  using id_storage_t = ID_STORAGE_TYPE<id_t>;

  //! Storage type
  using storage_t = STORAGE_TYPE<T>;

  //! Reference type
  using ref_t = T &;

  storage_t data;
  id_storage_t ids;

  //-----------------------------------------------------------------//
  //! Get offset
  //-----------------------------------------------------------------//
  ref_t get_offset(size_t offset) const {
    return data[ids[offset].index_space_index()];
  }

  /// Supply missing entity type information.
  /// The result uses a \c span for \c ids if copying them might be expensive;
  /// it uses a \c vector_ref for \c data if possible, or else a \c span for
  /// that as well.
  /// \tparam U actual (dynamic) entity type
  template<class U>
  auto cast() const {
    return rebind<U>{data, ids};
  }
  template<class U>
  auto cast() {
    return rebind<U>{data, ids};
  }

  //-----------------------------------------------------------------//
  //! Slice an index space. A slice aliases the current index
  //! space, definining a new iteration range of offsets to indices.
  //!
  //! @param begin begin offset
  //! @param end end offset
  //-----------------------------------------------------------------//
  auto slice(size_t begin, size_t end) const {
    const auto b = ids.data();
    return rebind<>{data, {b + begin, b + end}};
  }

  //-----------------------------------------------------------------//
  //! Slice an index space. A slice aliases the current index
  //! space, definining a new iteration range of offsets to indices.
  //!
  //! @param r offset range
  //-----------------------------------------------------------------//
  auto slice(const std::pair<size_t, size_t> & range) const {
    return slice(range.first, range.second);
  }

  /// Return a view of (a subset of) the data as selected by the IDs.
  auto ordered() {
    return utils::transform_view(ids, [&](const id_t & i) -> auto & {
      return data[i.index_space_index()];
    });
  }

  //-----------------------------------------------------------------//
  //! Clear all indices and entities
  //-----------------------------------------------------------------//
  void clear() {
    data.clear();
    ids.clear();
  }

  //-----------------------------------------------------------------//
  //! Add an entity to the index space.
  //-----------------------------------------------------------------//
  FLECSI_INLINE_TARGET
  T & push_back(const T & item) {
    return append(item);
  }

  T & push_back(T && item) {
    return append(std::move(item));
  }

private:
  // We duplicate the template-id to avoid generating additional
  // specializations that involve a member alias template.
  template<class U = T, bool View = std::is_base_of_v<view_tag, id_storage_t>>
  using rebind = std::conditional_t<
    std::is_convertible_v<decltype(data), utils::vector_ref<U>>,
    std::conditional_t<View,
      index_space_u<U, ID_STORAGE_TYPE, utils::vector_ref>,
      index_space_u<U, utils::span, utils::vector_ref>>,
    std::conditional_t<View,
      index_space_u<U, ID_STORAGE_TYPE, utils::span>,
      index_space_u<U, utils::span, utils::span>>>;

  template<class U>
  T & append(U && item) {
    const std::remove_pointer_t<T> * p;
    if constexpr(std::is_pointer_v<T>)
      p = item;
    else
      p = &item;
    ids.push_back(p->index_space_id());
    data.push_back(std::forward<U>(item));
    return data.back();
  }
};

//----------------------------------------------------------------------------//
//! this class provides a simple ID which is simply a size_t and the index is
//! the ID as opposed to more complex IDs which might need to store different
//! pieces of data such as partition, topological dimension, etc.
//----------------------------------------------------------------------------//
class simple_id
{
public:
  //-----------------------------------------------------------------//
  //! Constructor
  //-----------------------------------------------------------------//
  FLECSI_INLINE_TARGET
  simple_id(size_t id) : id_(id) {}

  FLECSI_INLINE_TARGET
  simple_id() : id_(0) {}

  //-----------------------------------------------------------------//
  //! Conversion the size_t
  //-----------------------------------------------------------------//
  operator size_t() const {
    return id_;
  }

  //-----------------------------------------------------------------//
  //! Comparison operator
  //-----------------------------------------------------------------//
  bool operator<(const simple_id & eid) const {
    return id_ < eid.id_;
  }

  //-----------------------------------------------------------------//
  //! Define the index space ID
  //-----------------------------------------------------------------//
  size_t index_space_index() const {
    return id_;
  }

private:
  size_t id_;
};

//----------------------------------------------------------------------------//
//! a convenience class which associates a simple ID with type T
//----------------------------------------------------------------------------//
template<typename T>
class simple_entry_u
{
public:
  using id_t = simple_id;

  //-----------------------------------------------------------------//
  //! Simple constructor
  //----------------------------------------------------------------//
  FLECSI_INLINE_TARGET
  simple_entry_u() : id_(0), entry_(T(0)) {}

  //-----------------------------------------------------------------//
  //! Constructor to associate an id with an entry
  //-----------------------------------------------------------------//
  FLECSI_INLINE_TARGET
  simple_entry_u(id_t id, const T & entry) : id_(id), entry_(entry) {}

  //-----------------------------------------------------------------//
  //! Conversion operator
  //-----------------------------------------------------------------//
  operator T() const {
    return entry_;
  }

  //-----------------------------------------------------------------//
  //! Get the entry ID
  //-----------------------------------------------------------------//
  auto entry_id() const {
    return entry_;
  }

  //-----------------------------------------------------------------//
  //! Get the index space ID
  //-----------------------------------------------------------------//
  id_t index_space_id() const {
    return id_;
  }

private:
  id_t id_;
  T entry_;
};

} // namespace topology
} // namespace flecsi
