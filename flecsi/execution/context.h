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

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <unordered_map>

#include <cinchlog.h>

#include <flecsi/coloring/adjacency_types.h>
#include <flecsi/coloring/coloring_types.h>
#include <flecsi/coloring/index_coloring.h>
#include <flecsi/execution/common/execution_state.h>
#include <flecsi/execution/global_object_wrapper.h>
#include <flecsi/runtime/types.h>
#include <flecsi/utils/dag.h>
#include <flecsi/utils/const_string.h>
#include <flecsi/utils/simple_id.h>

clog_register_tag(context);

namespace flecsi {
namespace execution {

/*!
  The context__ type provides a high-level runtime context interface that
  is implemented by the given context policy.

  @tparam CONTEXT_POLICY The backend context policy.

  @ingroup execution
 */

template<class CONTEXT_POLICY>
struct context__ : public CONTEXT_POLICY {
  using index_coloring_t = flecsi::coloring::index_coloring_t;
  using coloring_info_t = flecsi::coloring::coloring_info_t;
  using set_coloring_info_t = flecsi::coloring::set_coloring_info_t;
  using adjacency_info_t = flecsi::coloring::adjacency_info_t;

  /*!
    Adjacency triple: index space, from index space, to index space
   */

  using adjacency_triple_t = std::tuple<size_t, size_t, size_t>;

  /*!
    Gathers info about registered data fields.
   */

  struct field_info_t {
    size_t data_client_hash;
    size_t storage_class=0;
    size_t size;
    size_t namespace_hash;
    size_t name_hash;
    size_t versions;
    field_id_t fid;
    size_t index_space;
    size_t key;
  }; // struct field_info_t

  /*!
    Gathers info about sparse index spaces.
   */
  struct sparse_index_space_info_t {
    size_t index_space;
    size_t exclusive_reserve;
    size_t max_entries_per_index;
    // flecsi internal variable, do not set it up
    size_t sparse_fields_registered_=0; 
  };

  struct index_subspace_info_t {
    size_t index_subspace;
    size_t capacity;
    size_t size = 0;
  };

  /*!
    Structure needed to initialize a set topology.
   */
  struct set_index_space_info_t {
    /*!
      Gathers info about set topology index spaces per color.
     */
    struct color_info_t {
      size_t main_capacity;
      size_t active_migrate_capacity;
    };

    // key = color
    using color_info_map_t = std::unordered_map<size_t, color_info_t>;

    color_info_map_t color_info_map;
  };

  //--------------------------------------------------------------------------//
  // Field info map for fields in SPMD task, key1 =
  //   (data client hash, index space), key2 = fid
  //--------------------------------------------------------------------------//

  using field_info_map_t =
      std::map<std::pair<size_t, size_t>, std::map<field_id_t, field_info_t>>;

  //--------------------------------------------------------------------------//
  // Top-level driver interface.
  //--------------------------------------------------------------------------//

  using tlt_driver_t = std::function<int(int, char **)>;

  bool register_top_level_driver(tlt_driver_t const & driver) {
    tlt_driver_ = driver;
    return true;
  } // register_top_level_driver

  tlt_driver_t const & top_level_driver() const { return tlt_driver_; }

  //--------------------------------------------------------------------------//
  // Object interface.
  //--------------------------------------------------------------------------//

  template<size_t NAMESPACE_HASH, size_t INDEX, typename OBJECT_TYPE>
  bool register_global_object() {
    size_t KEY = NAMESPACE_HASH ^ INDEX;

    using wrapper_t = global_object_wrapper__<OBJECT_TYPE>;

    std::get<0>(global_object_registry_[KEY]) = {};
    std::get<1>(global_object_registry_[KEY]) = &wrapper_t::cleanup;

    return true;
  } // register_global_object

  template<size_t NAMESPACE_HASH, typename OBJECT_TYPE>
  bool set_global_object(size_t index, OBJECT_TYPE * obj) {
    size_t KEY = NAMESPACE_HASH ^ index;
    assert(global_object_registry_.find(KEY) != global_object_registry_.end());
    std::get<0>(global_object_registry_[KEY]) =
        reinterpret_cast<uintptr_t>(obj);
    return true;
  } // set_global_object

  template<size_t NAMESPACE_HASH, typename OBJECT_TYPE, typename... ARGS>
  bool initialize_global_object(size_t index, ARGS &&... args) {
    size_t KEY = NAMESPACE_HASH ^ index;
    assert(global_object_registry_.find(KEY) != global_object_registry_.end());
    std::get<0>(global_object_registry_[KEY]) = reinterpret_cast<uintptr_t>(
        new OBJECT_TYPE(std::forward<ARGS>(args)...));
    return true;
  } // new_global_object

  template<size_t NAMESPACE_HASH, typename OBJECT_TYPE>
  OBJECT_TYPE * get_global_object(size_t index) {
    size_t KEY = NAMESPACE_HASH ^ index;
    assert(global_object_registry_.find(KEY) != global_object_registry_.end());
    return reinterpret_cast<OBJECT_TYPE *>(
        std::get<0>(global_object_registry_[KEY]));
  } // get_global_object

  //--------------------------------------------------------------------------//
  // Function interface.
  //--------------------------------------------------------------------------//

  /*!
    FIXME: This interface needs to be updated.
   */

  template<
      size_t KEY,
      typename RETURN,
      typename ARG_TUPLE,
      RETURN (*FUNCTION)(ARG_TUPLE)>
  bool register_function() {
    clog_assert(
        function_registry_.find(KEY) == function_registry_.end(),
        "function has already been registered");

    const std::size_t addr = reinterpret_cast<std::size_t>(FUNCTION);
    clog(info) << "Registering function: " << addr << std::endl;

    function_registry_[KEY] = reinterpret_cast<void *>(FUNCTION);
    return true;
  } // register_function

  /*!
    FIXME: Add description.
   */

  void * function(size_t key) {
    return function_registry_[key];
  } // function

  /*!
    Meyer's singleton instance.

    @return The single instance of this type.
   */

  static context__ & instance() {
    static context__ context;
    return context;
  } // instance

  /*!
    Return the color for which the context was initialized.
   */

  size_t color() const {
    return CONTEXT_POLICY::color();
  } // color

  /*!
    Return the number of colors.
   */

  size_t colors() const {
    return CONTEXT_POLICY::colors();
  } // color

  /*!
    Add an index map. This map can be used to go between mesh and locally
    compacted index spaces.

    @param index_space The map key.
    @param index_map   The map to add.
   */

  void add_index_map(size_t index_space, std::map<size_t, size_t> & index_map) {
    index_map_[index_space] = index_map;

    for (auto i : index_map) {
      reverse_index_map_[index_space][i.second] = i.first;
    } // for
  } // add_index_map

  /*!
    Return the index map associated with the given index space.

    @param index_space The map key.
   */

  auto & index_map(size_t index_space) {
    auto it = index_map_.find(index_space);
    clog_assert(it != index_map_.end(), "invalid index space");

    return it->second;
  } // index_map

  /*!
    \todo DOCUMENT!
   */

  const auto & index_map(size_t index_space) const {
    auto it = index_map_.find(index_space);
    clog_assert(it != index_map_.end(), "invalid index space");

    return it->second;
  } // index_map

  /*!
    Register set topology index space sizes and other needed metadata.
   */

  void
  add_set_index_space(size_t index_space, const set_index_space_info_t & info) {
    auto itr = set_index_space_map_.insert({index_space, info});
    clog_assert(itr->second, "set index space exists: " << index_space);
  }

  const auto & set_index_space_map() const {
    return set_index_space_map_;
  }

  void set_sparse_index_space_info( const sparse_index_space_info_t & info) {
    sparse_index_space_info_map_[info.index_space]=info;
  }

  /*!
    Return the map of sparse index space info.
   */

  const auto & sparse_index_space_info_map() const {
    return sparse_index_space_info_map_;
  }

  void increment_sparse_fields(size_t sparse_idx_space)
  {
    auto iterator = sparse_index_space_info_map_.find(sparse_idx_space);
    clog_assert(iterator!=sparse_index_space_info_map_.end(),
                "sparse data map doesn't have this index space");
       iterator->second.sparse_fields_registered_++;
  }

  bool sparse_fields(size_t sparse_idx_space)
  {
    auto iterator = sparse_index_space_info_map_.find(sparse_idx_space);
    clog_assert(iterator!=sparse_index_space_info_map_.end(),
                "sparse data map doesn't have this index space");
    if (iterator->second.sparse_fields_registered_>0)
      return true;
    else
      return false;
  }

  const auto & cis_to_gis_map(size_t index_space) const {
    return cis_to_gis_map_.at(index_space);
  }

  /*!
    \todo DOCUMENT!
   */

  auto & cis_to_gis_map(size_t index_space) {
    return cis_to_gis_map_[index_space];
  }

  /*!
    \todo DOCUMENT!
   */

  const auto & gis_to_cis_map(size_t index_space) const {
    return gis_to_cis_map_.at(index_space);
  }

  /*!
    \todo DOCUMENT!
   */

  auto & gis_to_cis_map(size_t index_space) {
    return gis_to_cis_map_[index_space];
  }

  /*!
    Return the index map associated with the given index space.

    @param index_space The map key.
   */

  auto & reverse_index_map(size_t index_space) {
    auto it = reverse_index_map_.find(index_space);
    clog_assert(it != reverse_index_map_.end(), "invalid index space");

    return it->second;
  } // reverse_index_map

  /*!
    \todo DOCUMENT!
   */

  const auto & reverse_index_map(size_t index_space) const {
    auto it = reverse_index_map_.find(index_space);
    clog_assert(it != reverse_index_map_.end(), "invalid index space");

    return it->second;
  } // reverse_index_map

  //--------------------------------------------------------------------------//
  // Intermediate mapping interface.
  //--------------------------------------------------------------------------//

  /*!
    Return a reference to the intermediate mapping.
    This map can be used to go between mesh and
    locally compacted index spaces for intermediate entities.

    @param dimension        The entity dimension.
    @param domain           The entity domain.
    @return The map to add.
   */

  auto & intermediate_map(size_t dimension, size_t domain) {

    const auto key = utils::hash::intermediate_hash(dimension, domain);
    return intermediate_map_[key];
  } // intermediate_map

  /*!
    Return a const reference to the reverse intermediate mapping.

    @param dimension The entity dimension.
    @param domain    The entity domain.
   */

  auto const & reverse_intermediate_map(size_t dimension, size_t domain) const {
    const auto key = utils::hash::intermediate_hash(dimension, domain);

    auto it = reverse_intermediate_map_.find(key);

    clog_assert(
        it != reverse_intermediate_map_.end(), "invalid intermediate mapping");

    return it->second;
  } // reverse_intermediate_map

  /*!
    Return a modifyable reference to the reverse intermediate mapping.

    This lets the user create the reverse intermediate mapping themselves.
    Or they can build it by calling build_reverse_intermediate_map.

    @param dimension The entity dimension.
    @param domain    The entity domain.
   */

  auto & reverse_intermediate_map(size_t dimension, size_t domain) {
    const auto key = utils::hash::intermediate_hash(dimension, domain);
    return reverse_intermediate_map_[key];
  } // reverse_intermediate_map

  /*!
    A utility to automatically flip the intermediate maps.

    @param reset  If true, clear the map before proceding.
    @param sort  If true, assume the entries are unsorted.
   */
  void build_reverse_intermediate_maps(bool reset = false, bool sort = false) {

    // clear the map for safety
    if (reset)
      reverse_intermediate_map_.clear();

    // now flip all the mappings
    for (const auto & forward_map : intermediate_map_) {
      auto key = forward_map.first;
      auto & reverse_map = reverse_intermediate_map_[key];
      // it will be empty if never set, or reset==true
      if (reverse_map.empty()) {

        // assume unsorted
        if (sort) {
          for (auto & entry : forward_map.second) {
            std::sort(entry.second.begin(), entry.second.end());
            reverse_map[entry.second] = entry.first;
          }
        }
        // assume sorted
        else {
          for (const auto & entry : forward_map.second)
            reverse_map[entry.second] = entry.first;
        }
      }
    }
  }

  /*!
    Add an intermediate binding map. This map can be used to go between mesh and
    locally compacted index spaces for intermediate entities.

    @param dimension        The entity dimension.
    @param domain           The entity domain.
    @return A reference to the map.
   */

  auto & intermediate_binding_map(size_t from_dimension, size_t from_domain) {
    const auto key =
        utils::hash::intermediate_hash(from_dimension, from_domain);
    return intermediate_binding_map_[key];
  } // add_intermediate_binding_map

  /*!
    Return a const reference to the reverse intermediate mapping.

    @param dimension The entity dimension.
    @param domain    The entity domain.
   */

  auto const &
  reverse_intermediate_binding_map(size_t dimension, size_t domain) const {
    const auto key = utils::hash::intermediate_hash(dimension, domain);

    auto it = reverse_intermediate_binding_map_.find(key);
    clog_assert(
        it != reverse_intermediate_binding_map_.end(), "invalid index space");

    return it->second;
  } // reverse_intermediate_map

  /*!
    Return a modifyable reference to the reverse intermediate mapping.

    This lets the user create the reverse intermediate mapping themselves.
    Or they can build it by calling build_reverse_intermediate_map.

    @param dimension The entity dimension.
    @param domain    The entity domain.
   */

  auto & reverse_intermediate_binding_map(size_t dimension, size_t domain) {
    const auto key = utils::hash::intermediate_hash(dimension, domain);
    return reverse_intermediate_binding_map_[key];
  } // reverse_intermediate_map

  /*!
    A utility to automatically flip the intermediate maps.

    @param reset  If true, clear the map before proceding.
    @param sort  If true, assume the entries are unsorted.
   */
  void build_reverse_intermediate_binding_maps(
      bool reset = false,
      bool sort = false) {

    // clear the map for safety
    if (reset)
      reverse_intermediate_binding_map_.clear();

    // now flip all the mappings
    for (const auto & forward_map : intermediate_binding_map_) {
      auto key = forward_map.first;
      auto & reverse_map = reverse_intermediate_binding_map_[key];
      // it will be empty if never set, or reset==true
      if (reverse_map.empty()) {

        // assume unsorted
        if (sort) {
          for (auto & entry : forward_map.second) {
            std::sort(entry.second.begin(), entry.second.end());
            reverse_map[entry.second] = entry.first;
          }
        }
        // assume sorted
        else {
          for (const auto & entry : forward_map.second)
            reverse_map[entry.second] = entry.first;
        }
      }
    }
  }

  //--------------------------------------------------------------------------//
  // Coloring interface.
  //--------------------------------------------------------------------------//

  /*!
    Add an index coloring.

    @param index_space The map key.
    @param coloring The index coloring to add.
    @param coloring The index coloring information to add.
   */

  void add_coloring(
      size_t index_space,
      index_coloring_t & coloring,
      std::unordered_map<size_t, coloring_info_t> & coloring_info) {
    clog_assert(
        colorings_.find(index_space) == colorings_.end(),
        "color index already exists");

    colorings_[index_space] = coloring;
    coloring_info_[index_space] = coloring_info;
  } // add_coloring

  /*!
    Return the index coloring referenced by key.

    @param index_space The key associated with the coloring to be returned.
   */

  index_coloring_t & coloring(size_t index_space) {
    auto it = colorings_.find(index_space);
    clog_assert(it != colorings_.end(), "invalid index space: " << index_space);
    return it->second;
  } // coloring

  /*!
    Return the index coloring information referenced by key.

    @param index_space The key associated with the coloring information
                       to be returned.
   */

  const std::unordered_map<size_t, coloring_info_t> &
  coloring_info(size_t index_space) {
    auto it = coloring_info_.find(index_space);
    clog_assert(
      it != coloring_info_.end(), "invalid index space: " << index_space);
    return it->second;
  } // coloring_info

  /*!
    Return the coloring map (convenient for iterating through all
    of the colorings.

    @return The map of index colorings.
   */

  const std::map<size_t, index_coloring_t> & coloring_map() const {
    return colorings_;
  } // colorings

  /*!
    Return the coloring info map (convenient for iterating through all
    of the colorings.

    @return The map of index coloring information.
   */

  const std::map<size_t, std::unordered_map<size_t, coloring_info_t>> &
  coloring_info_map() const {
    return coloring_info_;
  } // colorings

  /*!
    Add an adjacency/connectivity from one index space to another.

    @param from_index_space The index space id of the from side
    @param to_index_space The index space id of the to side
   */

  void add_adjacency(adjacency_info_t & adjacency_info) {
    clog_assert(
        adjacency_info_.find(adjacency_info.index_space) ==
            adjacency_info_.end(),
        "adjacency exists");

    adjacency_info_.emplace(
        adjacency_info.index_space, std::move(adjacency_info));
  } // add_adjacency

  /*!
    Return the set of registered adjacencies.

    @return The set of registered adjacencies
   */

  const std::map<size_t, adjacency_info_t> & adjacency_info() const {
    return adjacency_info_;
  } // adjacencies

  void add_index_subspace(size_t index_subspace, size_t capacity) {
    index_subspace_info_t info;
    info.index_subspace = index_subspace;
    info.capacity = capacity;

    index_subspace_map_.emplace(index_subspace, std::move(info));
  }

  void add_index_subspace(const index_subspace_info_t & info) {
    index_subspace_map_.emplace(info.index_subspace, info);
  }

  std::map<size_t, index_subspace_info_t> & index_subspace_info() {
    return index_subspace_map_;
  }

  /*!
    Register field info for index space and field id.

    @param index_space virtual index space
    @param field allocated field id
    @param field_info field info as registered
   */

  void register_field_info(field_info_t & field_info) {
    field_info_vec_.emplace_back(std::move(field_info));
  }

  /*!
    Return registered fields
   */

  const std::vector<field_info_t> & registered_fields() const {
    return field_info_vec_;
  }

  /*!
    Add an adjacency index space.

    @param index_space index space to add.
   */

  void add_adjacency_triple(const adjacency_triple_t & triple) {
    adjacencies_.emplace(std::get<0>(triple), triple);
  }

  /*!
    Return set of all adjacency index spaces.
   */

  auto & adjacencies() const {
    return adjacencies_;
  }

  /*!
    Put field info for index space and field id.

    @param field_info field info as registered
   */

  void put_field_info(const field_info_t & field_info) {
    size_t index_space = field_info.index_space;
    size_t data_client_hash = field_info.data_client_hash;
    field_id_t fid = field_info.fid;

    field_info_map_[{data_client_hash, index_space}].emplace(fid, field_info);

    field_name_map_.insert(
        {{field_info.data_client_hash, field_info.key}, {index_space, fid}});
  } // put_field_info

  /*!
    Get registered field info map for read access.
   */

  const field_info_map_t & field_info_map() const {
    return field_info_map_;
  } // field_info_map

  /*!
    Lookup registered field info from data client and namespace hash.

    @param data_client_hash data client type hash
    @param namespace_hash namespace/field name hash
   */

  const field_info_t & get_field_info_from_name(
      size_t data_client_hash,
      size_t namespace_hash) const {
    auto itr = field_name_map_.find({data_client_hash, namespace_hash});
    clog_assert(itr != field_name_map_.end(), "invalid field");

    auto iitr = field_info_map_.find({data_client_hash, itr->second.first});
    clog_assert(iitr != field_info_map_.end(), "invalid index_space");

    auto fitr = iitr->second.find(itr->second.second);
    clog_assert(fitr != iitr->second.end(), "invalid fid");

    return fitr->second;
  }

  /*!
    Lookup registered field info from data client and namespace hash.

    @param data_client_hash data client type hash
    @param key key hash
   */

  const field_info_t *
  get_field_info_from_key(size_t data_client_hash, size_t key_hash) const {
    auto itr = field_name_map_.find({data_client_hash, key_hash});
    if (itr == field_name_map_.end()) {
      return nullptr;
    }

    auto iitr = field_info_map_.find({data_client_hash, itr->second.first});
    if (iitr == field_info_map_.end()) {
      return nullptr;
    }

    auto fitr = iitr->second.find(itr->second.second);
    if (fitr == iitr->second.end()) {
      return nullptr;
    }

    return &fitr->second;
  } // get_field_info_from_key

  /*!
    Advance the state of the execution flow.
   */

  void advance_state() {
    execution_state_++;
  } // advance_state

  /*!
     Reduce the state of the execution flow.
   */

  void lower_state() {
    execution_state_--;
  } // lower_state

  /*!
    Return the current execution state
   */

  size_t execution_state() {
    return execution_state_;
  } // execution_state

private:

  // Default constructor
  context__() : CONTEXT_POLICY() {}

  // Destructor
  ~context__() {
    // Invoke the cleanup function for each global object
    for (auto & go : global_object_registry_) {
      std::get<1>(go.second)(std::get<0>(go.second));
    } // for
  } // ~context_t

  //--------------------------------------------------------------------------//
  // We don't need any of these
  //--------------------------------------------------------------------------//

  context__(const context__ &) = delete;
  context__ & operator=(const context__ &) = delete;
  context__(context__ &&) = delete;
  context__ & operator=(context__ &&) = delete;

  //--------------------------------------------------------------------------//
  // Top-level driver.
  //--------------------------------------------------------------------------//

  tlt_driver_t tlt_driver_ = {};

  //--------------------------------------------------------------------------//
  // Object data members.
  //--------------------------------------------------------------------------//

  using global_object_data_t =
      std::pair<uintptr_t, std::function<void(uintptr_t)>>;

  std::unordered_map<size_t, global_object_data_t> global_object_registry_;

  //--------------------------------------------------------------------------//
  // Function data members.
  //--------------------------------------------------------------------------//

  std::unordered_map<size_t, void *> function_registry_;

  //--------------------------------------------------------------------------//
  // Field info vector for registered fields in TLT
  //--------------------------------------------------------------------------//

  std::vector<field_info_t> field_info_vec_;

  //--------------------------------------------------------------------------//
  // Field info map for fields in SPMD task, key1 =
  //   (data client hash, index space), key2 = fid
  //--------------------------------------------------------------------------//

  field_info_map_t field_info_map_;

  //--------------------------------------------------------------------------//
  // Map of adjacency triples. key: adjacency index space
  //--------------------------------------------------------------------------//

  std::map<size_t, adjacency_triple_t> adjacencies_;

  //--------------------------------------------------------------------------//
  // Field name map, key1 = (data client hash, name/namespace hash)
  // value = (index space, fid)
  //--------------------------------------------------------------------------//

  std::map<std::pair<size_t, size_t>, std::pair<size_t, field_id_t>>
      field_name_map_;

  //--------------------------------------------------------------------------//
  // key: virtual index space id
  // value: coloring indices (exclusive, shared, ghost)
  //--------------------------------------------------------------------------//

  std::map<size_t, index_coloring_t> colorings_;

  //--------------------------------------------------------------------------//
  // key: mesh index space entity id
  //--------------------------------------------------------------------------//

  std::map<size_t, std::map<size_t, size_t>> index_map_;
  std::map<size_t, std::map<size_t, size_t>> reverse_index_map_;

  //--------------------------------------------------------------------------//
  // key: index space
  //--------------------------------------------------------------------------//

  std::map<size_t, sparse_index_space_info_t> sparse_index_space_info_map_;

#if 0
  std::map<size_t, std::map<size_t, size_t>> cis_to_mis_map_;
  std::map<size_t, std::map<size_t, size_t>> mis_to_cis_map_;
#endif

  // key: mesh index space entity id
  std::map<size_t, std::map<size_t, size_t>> cis_to_gis_map_;
  std::map<size_t, std::map<size_t, size_t>> gis_to_cis_map_;

  //--------------------------------------------------------------------------//
  // Data members for ntermediate mapping
  //--------------------------------------------------------------------------//

  // key: intermediate entity to vertex ids
  std::map<size_t, std::unordered_map<size_t, std::vector<size_t>>>
      intermediate_map_;

  struct vector_hash_t {
    size_t operator()(std::vector<size_t> const & v) const {
      size_t h{0};
      for (auto i : v) {
        h |= i;
      } // for

      return h;
    } // operator ()
  }; // struct vector_hash_t

  struct vector_equal_t {
    bool operator()(
        const std::vector<size_t> & a,
        const std::vector<size_t> & b) const {
      // assume sorted for performance
      // std::sort(a.begin(), a.end());
      // std::sort(b.begin(), b.end());
      return (a == b);
    } // operator ()
  }; // struct vector_hash_t

  std::map<
      size_t,
      std::unordered_map<
          std::vector<size_t>,
          size_t,
          vector_hash_t,
          vector_equal_t>>
      reverse_intermediate_map_;

  //! the packed types used for simple_id_t
  using simple_id_types_t = std::tuple<int, int, size_t>;

  //! the simple id type used for comparing ids of different dimensions
  using simple_id_t = utils::simple_id_t<
      simple_id_types_t,
      utils::lexical_comparison<simple_id_types_t>>;

  //! the storage type for arrays of simple_id_t's
  using simple_id_vector_t = std::vector<simple_id_t>;

  //! A lexical comparison function for simple_id_t's
  struct simple_id_vector_compare_t {
    bool operator()(const simple_id_vector_t & a, const simple_id_vector_t & b)
        const {
      return std::lexicographical_compare(
          a.begin(), a.end(), b.begin(), b.end());
    }
  };

  //! The forward intermediate binding mapping
  std::map<size_t, std::unordered_map<size_t, simple_id_vector_t>>
      intermediate_binding_map_;

  //! the reverse intermediate binding mapping
  std::map<
      size_t,
      std::map<simple_id_vector_t, size_t, simple_id_vector_compare_t>>
      reverse_intermediate_binding_map_;

  //--------------------------------------------------------------------------//
  // key: virtual index space.
  // value: map of color to coloring info
  //--------------------------------------------------------------------------//

  std::map<size_t, std::unordered_map<size_t, coloring_info_t>> coloring_info_;

  //--------------------------------------------------------------------------//
  // key: index space
  //--------------------------------------------------------------------------//

  std::map<size_t, adjacency_info_t> adjacency_info_;

  //--------------------------------------------------------------------------//
  // key: index subspace
  //--------------------------------------------------------------------------//

  std::map<size_t, index_subspace_info_t> index_subspace_map_;

  //--------------------------------------------------------------------------//
  // key: set index space
  //--------------------------------------------------------------------------//

  std::map<size_t, set_index_space_info_t> set_index_space_map_;

  //--------------------------------------------------------------------------//
  // Execution state
  //--------------------------------------------------------------------------//

  size_t execution_state_ = SPECIALIZATION_TLT_INIT;

}; // class context__

} // namespace execution
} // namespace flecsi

// This include file defines the FLECSI_RUNTIME_CONTEXT_POLICY used below.

#include <flecsi/runtime/flecsi_runtime_context_policy.h>

namespace flecsi {
namespace execution {

/*!
  The context_t type is the high-level interface to the FleCSI runtime
  context.

  @ingroup execution
 */

using context_t = context__<FLECSI_RUNTIME_CONTEXT_POLICY>;

} // namespace execution
} // namespace flecsi
