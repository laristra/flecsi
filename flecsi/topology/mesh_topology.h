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
#include <array>
#include <cassert>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <flecsi/execution/context.h>
#include <flecsi/topology/mesh_storage.h>
#include <flecsi/topology/mesh_types.h>
#include <flecsi/topology/partition.h>
#include <flecsi/utils/common.h>
#include <flecsi/utils/set_intersection.h>
#include <flecsi/utils/static_verify.h>

// static verification for required mesh type members such as entity types
// tuple, connectivities, bindings, etc.

namespace flecsi {
namespace topology {
namespace verify_mesh {

FLECSI_MEMBER_CHECKER(num_dimensions);
FLECSI_MEMBER_CHECKER(num_domains);
FLECSI_MEMBER_CHECKER(entity_types);
FLECSI_MEMBER_CHECKER(connectivities);
FLECSI_MEMBER_CHECKER(bindings);
FLECSI_MEMBER_CHECKER(create_entity);

} // namespace verify_mesh

//----------------------------------------------------------------------------//
// Mesh topology.
//----------------------------------------------------------------------------//

/*!
mesh_topology__ is parameterized on a class (MESH_TYPE) which gives
information about its entity types, connectivities and more. the mesh
topology is responsibly for computing connectivity info between entities
of different topological dimension, e.g: vertex -> cell,
cell -> edge, etc. and provides methods for traversing these adjancies.
It also holds vectors containing the entity instances.

Description of major features and terms specific to FleCSI:

mesh dimension MD - e.g: MD = 2 for a 2d mesh. We currently support 2d and 3d
  meshes.

topological dimension D - the dimensionality associated with entities,
  e.g: D = 0 is interpreted as a vertex, D = 1 an edge or face for MD = 2,
  D = 2 is a cell for MD = 2

domain M - a sub-mesh or mesh space that holds entities of various topological
  dimension

connectivity - a directed connection or adjancy between entities of differing
  topological dimension in the same domain. e.g: D1 -> D2 (edges -> faces)
  for MD = 3. Cell to vertex connectivity is supplied by the user and all
  other connectivies are computed by the topology.

binding - a type of connectivity that connects entities of potentially
  differing topological dimension across two different domains

entity - an object associated with a topological dimension, e.g: cell. Each
  entity has an associated integer id.

mesh topology - the top-level container for domains, entities,
  and connectivities, referred to as the low-level interface

mesh policy - the top-level class that a specialization creates to
  parameterize the mesh topology to define such things as: mesh dimension,
  number of domains, connectivity and binding pairs of interest, and entity
  classes/types per each domain/topological dimension.

entity set - contains an iterable set of entities. Support set operations such
  as intersection, union, etc. and functional operations like apply, map,
  reduce, etc. to apply a custom function to the set.

 @tparam MESH_TYPE mesh policy type by which the mesh is statically configured.

 @ingroup mesh-topology
 */
template<class MESH_TYPE>
class mesh_topology__
    : public mesh_topology_base__<
          mesh_storage__<MESH_TYPE::num_dimensions, MESH_TYPE::num_domains,
          num_index_subspaces__<MESH_TYPE>::value>> {
  // static verification of mesh policy

  static_assert(
      verify_mesh::has_member_num_dimensions<MESH_TYPE>::value,
      "mesh policy missing num_dimensions size_t");

  static_assert(
      std::is_convertible<decltype(MESH_TYPE::num_dimensions), size_t>::value,
      "mesh policy num_dimensions must be size_t");

  static_assert(
      verify_mesh::has_member_num_domains<MESH_TYPE>::value,
      "mesh policy missing num_domains size_t");

  static_assert(
      std::is_convertible<decltype(MESH_TYPE::num_domains), size_t>::value,
      "mesh policy num_domains must be size_t");

  static_assert(
      verify_mesh::has_member_entity_types<MESH_TYPE>::value,
      "mesh policy missing entity_types tuple");

  static_assert(
      utils::is_tuple<typename MESH_TYPE::entity_types>::value,
      "mesh policy entity_types is not a tuple");

  static_assert(
      verify_mesh::has_member_connectivities<MESH_TYPE>::value,
      "mesh policy missing connectivities tuple");

  static_assert(
      utils::is_tuple<typename MESH_TYPE::connectivities>::value,
      "mesh policy connectivities is not a tuple");

  static_assert(
      verify_mesh::has_member_bindings<MESH_TYPE>::value,
      "mesh policy missing bindings tuple");

  static_assert(
      utils::is_tuple<typename MESH_TYPE::bindings>::value,
      "mesh policy bindings is not a tuple");

  static_assert(
      verify_mesh::has_member_create_entity<MESH_TYPE>::value,
      "mesh policy missing create_entity()");

public:
  // mesh storage type definition
  using storage_t = mesh_storage__<MESH_TYPE::num_dimensions,
    MESH_TYPE::num_domains,
    num_index_subspaces__<MESH_TYPE>::value>;

  // mesh topology base definition
  using base_t = mesh_topology_base__<storage_t>;

  // entity ID type
  using id_t = utils::id_t;

  // offset type use by connectivities to give offsets and counts
  using offset_t = utils::offset_t;

  // used to find the entity type of topological dimension DIM and domain DOM
  template<size_t DIM, size_t DOM = 0>
  using entity_type = typename find_entity_<MESH_TYPE, DIM, DOM>::type;

  //--------------------------------------------------------------------------//
  // This type definition is needed so that data client handles can be
  // specialized for particular data client types, e.g., mesh topologies vs.
  // tree topologies. It is also useful for detecting illegal usage, such as
  // when a user adds data members.
  //--------------------------------------------------------------------------//
  using type_identifier_t = mesh_topology__;

  // Don't allow the mesh to be copied or copy constructed

  //! don't allow mesh to be assigned
  mesh_topology__ & operator=(const mesh_topology__ &) = delete;

  //! Allow move operations
  mesh_topology__(mesh_topology__ && o) = default;

  //! override default move assignement
  mesh_topology__ & operator=(mesh_topology__ && o) = default;

  //! Constructor, takes as input a mesh storage or storage can later be set
  mesh_topology__(storage_t * ms = nullptr) : base_t(ms) {
    if (ms != nullptr) {
      initialize_storage();
    } // if
  } // mesh_topology__()

  //! Copy constructor: alias another mesh
  mesh_topology__(const mesh_topology__ & m) : base_t(m.ms_) {}

  // The mesh retains ownership of the entities and deletes them
  // upon mesh destruction
  virtual ~mesh_topology__() {}

  //--------------------------------------------------------------------------//
  //! Initialize the mesh storage after it has been set
  //! This is needed to initialize raw connectivity buffers,
  //--------------------------------------------------------------------------//
  void initialize_storage() {

    for (size_t from_domain = 0; from_domain < MESH_TYPE::num_domains;
         ++from_domain) {
      for (size_t to_domain = 0; to_domain < MESH_TYPE::num_domains;
           ++to_domain) {
        base_t::ms_->topology[from_domain][to_domain].init_(
            from_domain, to_domain);
      } // for
    } // for

    for (size_t to_domain = 0; to_domain < MESH_TYPE::num_domains; ++to_domain){
      for (size_t to_dim = 0; to_dim <= MESH_TYPE::num_dimensions; ++to_dim) {
        auto & master = base_t::ms_->index_spaces[to_domain][to_dim];

        for (size_t from_domain = 0; from_domain < MESH_TYPE::num_domains;
             ++from_domain) {
          for (size_t from_dim = 0; from_dim <= MESH_TYPE::num_dimensions;
               ++from_dim) {
            get_connectivity_(from_domain, to_domain, from_dim, to_dim)
                .get_index_space()
                .set_master(master);
          } // for
        } // for
      } // for
    } // for
  } // intialize_storage

  //--------------------------------------------------------------------------//
  //! A mesh is constructed by creating cells and vertices and associating
  //! vertices with cells with this method.
  //!
  //! @tparam DOM domain
  //! @tparam C cell class
  //! @tparam VERT_TYPE vertices types
  //--------------------------------------------------------------------------//
  template<size_t DOM, class CELL_TYPE, typename VERT_TYPE>
  void init_cell(CELL_TYPE * cell, VERT_TYPE && verts) {
    init_cell_<DOM>(cell, std::forward<VERT_TYPE>(verts));
  } // init_cell

  //--------------------------------------------------------------------------//
  //! A mesh is constructed by creating cells and vertices and associating
  //! vertices with cells with this method.
  //!
  //! @tparam DOM domain
  //! @tparam CELL_TYPE cell class
  //! @tparam VERT_TYPE vertices initializer
  //--------------------------------------------------------------------------//
  template<size_t DOM, class CELL_TYPE, typename VERT_TYPE>
  void init_cell(CELL_TYPE * cell, std::initializer_list<VERT_TYPE *> verts) {
    init_cell_<DOM>(cell, verts);
  } // init_cell

  //--------------------------------------------------------------------------//
  //! Initialize an entities connectivity with a subset of another.
  //!
  //! @tparam DOM domain
  //! @tparam FROM_DIM from topological dimension
  //! @tparam TO_DIM to topological dimension
  //! @tparam ENT_TYPE1 from entity type
  //! @tparam ENT_TYPE2 to entity type
  //!
  //! @param super from entity
  //! @param subs to entities
  //--------------------------------------------------------------------------//
  template<size_t DOM, size_t FROM_DIM, size_t TO_DIM, class ENT_TYPE1,
    class ENT_TYPE2>
  void init_entity(ENT_TYPE1 * super, ENT_TYPE2 && subs) {
    init_entity_<DOM, FROM_DIM, TO_DIM>(super, std::forward<ENT_TYPE2>(subs));
  } // init_entity

  //--------------------------------------------------------------------------//
  //! Initialize an entities connectivity with a subset of another.
  //!
  //! @tparam DOM domain
  //! @tparam FROM_DIM from topological dimension
  //! @tparam TO_DIM to topological dimension
  //! @tparam ENT_TYPE1 from entity type
  //! @tparam ENT_TYPE2 to entity type
  //!
  //! @param super from entity
  //! @param subs to entities
  //--------------------------------------------------------------------------//
  template<size_t DOM, size_t FROM_DIM, size_t TO_DIM, class ENT_TYPE1,
    class ENT_TYPE2>
  void init_entity(ENT_TYPE1 * super, std::initializer_list<ENT_TYPE2 *> subs) {
    init_entity_<DOM, FROM_DIM, TO_DIM>(super, subs);
  } // init_entity

  //--------------------------------------------------------------------------//
  //! Get the number of entities that reside in a topological dimension and
  //! domain.
  //!
  //! @param domain domain
  //! @param dim topological dimension
  //--------------------------------------------------------------------------//
  size_t num_entities(size_t dim, size_t domain = 0) const override {
    return num_entities_(dim, domain);
  } // num_entities

  //------------------------------------------------------------------------//
  //! The init method builds entities as edges/faces and computes adjacencies
  //! and bindings for a given domain.
  //!
  //! @tparam DOM domain
  //------------------------------------------------------------------------//
  template<size_t DOM = 0>
  void init() {
    // Compute mesh connectivity
    using TP = typename MESH_TYPE::connectivities;
    compute_connectivity__<DOM, std::tuple_size<TP>::value, TP>::compute(*this);

    using BT = typename MESH_TYPE::bindings;
    compute_bindings__<DOM, std::tuple_size<BT>::value, BT>::compute(*this);
  } // init

  //--------------------------------------------------------------------------//
  //! Similar to init(), but only compute bindings. This method should be called
  //! when a domain is sparse, i.e: missing certain entity types such as cells
  //! and it is not possible to compute connectivities.
  //!
  //! @tparam DOM domain
  //--------------------------------------------------------------------------//
  template<size_t DOM = 0>
  void init_bindings() {
    using BT = typename MESH_TYPE::bindings;
    compute_bindings__<DOM, std::tuple_size<BT>::value, BT>::compute(*this);
  } // init

  //--------------------------------------------------------------------------//
  //! Return the number of entities contained in specified topological dimension
  //! and domain.
  //!
  //! @tparam DOM domain
  //! @tparam DIM topological dimension
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0>
  decltype(auto) num_entities() const {
    return base_t::ms_->index_spaces[DOM][DIM].size();
  } // num_entities

  //--------------------------------------------------------------------------//
  //! Return the number of entities contained in specified topological dimension
  //! and domain.
  //!
  //! @tparam DOM domain
  //! @tparam DIM topological dimension
  //!
  //! @param partition e.g. all, owned, shared, etc.
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0>
  decltype(auto) num_entities(partition_t partition) const {
    return base_t::ms_->partition_index_spaces[partition][DOM][DIM].size();
  } // num_entities

  //--------------------------------------------------------------------------//
  //! Get the connectivity of the specified from/to domain and from/to
  //! topological dimensions.
  //--------------------------------------------------------------------------//
  const connectivity_t & get_connectivity(
      size_t from_domain,
      size_t to_domain,
      size_t from_dim,
      size_t to_dim) const override {
    return get_connectivity_(from_domain, to_domain, from_dim, to_dim);
  } // get_connectivity

  //--------------------------------------------------------------------------//
  //! Get the connectivity of the specified from/to domain and from/to
  //! topological dimensions.
  //--------------------------------------------------------------------------//
  connectivity_t & get_connectivity(
      size_t from_domain,
      size_t to_domain,
      size_t from_dim,
      size_t to_dim) override {
    return get_connectivity_(from_domain, to_domain, from_dim, to_dim);
  } // get_connectivity

  //--------------------------------------------------------------------------//
  //! Get the connectivity of the specified domain and from/to topological
  //! dimensions.
  //--------------------------------------------------------------------------//
  const connectivity_t & get_connectivity(
      size_t domain,
      size_t from_dim,
      size_t to_dim) const override {
    return get_connectivity_(domain, domain, from_dim, to_dim);
  } // get_connectivity

  //--------------------------------------------------------------------------//
  //! Get the connectivity of the specified domain and from/to topological
  //! dimensions.
  //--------------------------------------------------------------------------//
  connectivity_t &
  get_connectivity(size_t domain, size_t from_dim, size_t to_dim) override {
    return get_connectivity_(domain, domain, from_dim, to_dim);
  } // get_connectivity

  //--------------------------------------------------------------------------//
  //! Return mesh topological dimension.
  //--------------------------------------------------------------------------//
  size_t topological_dimension() const override {
    return MESH_TYPE::num_dimensions;
  }

  //--------------------------------------------------------------------------//
  //! Return the index space of specified domain and topological dimension.
  //!
  //! @tparam DOM domain
  //!
  //! @param dim topological dimension
  //--------------------------------------------------------------------------//
  template<size_t DOM = 0>
  const auto & get_index_space_(size_t dim) const {
    return base_t::ms_->index_spaces[DOM][dim];
  } // get_entities_

  //--------------------------------------------------------------------------//
  //! Return the index space of specified domain and topological dimension.
  //!
  //! @tparam DOM domain
  //!
  //! @param dim topological dimension
  //--------------------------------------------------------------------------//
  template<size_t DOM = 0>
  auto & get_index_space_(size_t dim) {
    return base_t::ms_->index_spaces[DOM][dim];
  } // get_entities_

  //--------------------------------------------------------------------------//
  //! Return the index space of specified domain and topological dimension and
  //! associated partition, e.g. owned, shared, ghost, etc.
  //!
  //! @tparam DOM domain
  //!
  //! @param dim topological dimension
  //!
  //! @param partition e.g. all, owned, shared, etc.
  //--------------------------------------------------------------------------//
  template<size_t DOM = 0>
  const auto & get_index_space_(size_t dim, partition_t partition) const {
    return base_t::ms_->partition_index_spaces[partition][DOM][dim];
  } // get_entities_

  //--------------------------------------------------------------------------//
  //! Return the index space of specified domain and topological dimension and
  //! associated partition, e.g. owned, shared, ghost, etc.
  //!
  //! @tparam DOM domain
  //!
  //! @param dim topological dimension
  //!
  //! @param partition e.g. all, owned, shared, etc.
  //--------------------------------------------------------------------------//
  template<size_t DOM = 0>
  auto & get_index_space_(size_t dim, partition_t partition) {
    return base_t::ms_->partition_index_spaces[partition][DOM][dim];
  } // get_entities_

  //--------------------------------------------------------------------------//
  //! Get an entity in domain DOM of topological dimension DIM with specified id.
  //!
  //! @tparam DOM domain
  //! @tparam DIM topological dimension
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0>
  auto get_entity(id_t global_id) const {
    using etype = entity_type<DIM, DOM>;
    return static_cast<etype *>(
        base_t::ms_->index_spaces[DOM][DIM][global_id.entity()]);
  } // get_entity

  //--------------------------------------------------------------------------//
  //! Get an entity in domain DOM of topological dimension DIM with specified id.
  //!
  //! @tparam DOM domain
  //! @tparam DIM topological dimension
  //!
  //! @param dim topological dimension
  //!
  //! @param global_id entity global id
  //--------------------------------------------------------------------------//
  template<size_t DOM = 0>
  auto get_entity(size_t dim, id_t global_id) {
    return base_t::ms_->index_spaces[DOM][dim][global_id.entity()];
  } // get_entity

  //--------------------------------------------------------------------------//
  //! Get an entity in domain DOM of topological dimension DIM with specified id.
  //!
  //! @tparam DOM domain
  //!
  //! @tparam DIM topological dimension
  //!
  //! @param dim topological dimension
  //!
  //! @param partition e.g. all, owned, shared, etc.
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0>
  auto get_entity(id_t global_id, partition_t partition) const {
    using etype = entity_type<DIM, DOM>;
    return static_cast<etype *>(
        base_t::ms_
            ->partition_index_spaces[partition][DOM][DIM][global_id.entity()]);
  } // get_entity

  //--------------------------------------------------------------------------//
  //! Get an entity in domain DOM of topological dimension DIM with specified id.
  //!
  //! @tparam DOM domain
  //! @tparam DIM topological dimension
  //!
  //! @param dim topological dimension
  //!
  //! @param global_id entity global id
  //--------------------------------------------------------------------------//
  template<size_t DOM = 0>
  auto get_entity(size_t dim, id_t global_id, partition_t partition) {
    return base_t::ms_
        ->partition_index_spaces[partition][DOM][dim][global_id.entity()];
  } // get_entity

  //--------------------------------------------------------------------------//
  //! Get the entities of topological dimension DIM connected to another entity
  //! by specified connectivity from domain FROM_DOM and to domain TO_DOM.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam DIM to topological dimension
  //! @tparam ENT_TYPE entity type
  //!
  //! @param e from entity
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t FROM_DOM, size_t TO_DOM = FROM_DOM,
    class ENT_TYPE>
  const auto entities(const ENT_TYPE * e) const {

    const connectivity_t & c = 
      get_connectivity(FROM_DOM, TO_DOM, ENT_TYPE::dimension, DIM);
    assert(!c.empty() && "empty connectivity");

    using etype = entity_type<DIM, TO_DOM>;
    using dtype = domain_entity__<TO_DOM, etype>;

    return c.get_index_space().slice<dtype>(
      c.range(e->template id<FROM_DOM>()));
  } // entities

  //--------------------------------------------------------------------------//
  //! Get the entities of topological dimension DIM connected to another entity
  //! by specified connectivity from domain FROM_DOM and to domain TO_DOM.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam DIM to topological dimension
  //! @tparam ENT_TYPE entity type
  //!
  //! @param e from entity
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t FROM_DOM, size_t TO_DOM = FROM_DOM,
    class ENT_TYPE>
  auto entities(ENT_TYPE * e) {
    connectivity_t & c = 
      get_connectivity(FROM_DOM, TO_DOM, ENT_TYPE::dimension, DIM);
    assert(!c.empty() && "empty connectivity");

    using etype = entity_type<DIM, TO_DOM>;
    using dtype = domain_entity__<TO_DOM, etype>;

    return c.get_index_space().slice<dtype>(
      c.range(e->template id<FROM_DOM>()));
  } // entities

  //--------------------------------------------------------------------------//
  //! Get the entities of topological dimension DIM connected to another entity
  //! by specified connectivity from domain FROM_DOM and to domain TO_DOM.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam DIM to topological dimension
  //! @tparam ENT_TYPE entity type
  //!
  //! @param e from entity with compile-time domain
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t FROM_DOM = 0, size_t TO_DOM = FROM_DOM,
    class ENT_TYPE>
  decltype(auto) entities(domain_entity__<FROM_DOM, ENT_TYPE> & e) const {
    return entities<DIM, FROM_DOM, TO_DOM>(e.entity());
  } // entities

  //--------------------------------------------------------------------------//
  //! Get the entities of topological dimension DIM connected to another entity
  //! by specified connectivity from domain FROM_DOM and to domain TO_DOM.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam DIM to topological dimension
  //! @tparam ENT_TYPE entity type
  //!
  //! @param e from entity with compile-time domain
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t FROM_DOM = 0, size_t TO_DOM = FROM_DOM,
    class ENT_TYPE>
  decltype(auto) entities(domain_entity__<FROM_DOM, ENT_TYPE> & e) {
    return entities<DIM, FROM_DOM, TO_DOM>(e.entity());
  } // entities

  //--------------------------------------------------------------------------//
  //! Get the top-level entities of topological dimension DIM of the specified
  //! domain DOM. e.g: cells of the mesh.
  //!
  //! @tparam DOM domain
  //! @tparam DIM to topological dimension
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0>
  auto entities() const {
    using etype = entity_type<DIM, DOM>;
    using dtype = domain_entity__<DOM, etype>;
    return base_t::ms_->index_spaces[DOM][DIM].template slice<dtype>();
  } // entities

  //--------------------------------------------------------------------------//
  //! Get the top-level entities of topological dimension DIM of the specified
  //! domain DOM. e.g: cells of the mesh.
  //!
  //! @tparam n! n!
  //! @tparam DOM domain
  //! @tparam DIM to topological dimension
  //!
  //! @param partition e.g. all, owned, shared, etc.
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0>
  auto entities(partition_t partition) const {
    using etype = entity_type<DIM, DOM>;
    using dtype = domain_entity__<DOM, etype>;
    return base_t::ms_->partition_index_spaces[partition][DOM][DIM]
        .template slice<dtype>();
  } // entities

  //--------------------------------------------------------------------------//
  //! Get the top-level entity id's of topological dimension DIM of the specified
  //! domain DOM. e.g: cells of the mesh.
  //!
  //! @tparam DOM domain
  //! @tparam DIM to topological dimension
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0>
  auto entity_ids() const {
    return base_t::ms_->index_spaces[DOM][DIM].ids();
  } // entity_ids

  //--------------------------------------------------------------------------//
  //! Get the top-level entity id's of topological dimension DIM of the specified
  //! domain DOM. e.g: cells of the mesh.
  //!
  //! @tparam DOM domain
  //! @tparam DIM to topological dimension
  //!
  //! @param partition e.g. all, owned, shared, etc.
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t DOM = 0>
  auto entity_ids(partition_t partition) const {
    return base_t::ms_->partition_index_spaces[partition][DOM][DIM].ids();
  } // entity_ids

  //--------------------------------------------------------------------------//
  //! Get the entity id's of topological dimension DIM connected to another entity
  //! by specified connectivity from domain FROM_DOM and to domain TO_DOM.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam DIM to topological dimension
  //! @tparam ENT_TYPE entity type
  //!
  //! @param e from entity with compile-time domain
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t FROM_DOM = 0, size_t TO_DOM = FROM_DOM,
    class ENT_TYPE>
  decltype(auto) entity_ids(domain_entity__<FROM_DOM, ENT_TYPE> & e) {
    return entity_ids<DIM, FROM_DOM, TO_DOM>(e.entity());
  } // entities

  //--------------------------------------------------------------------------//
  //! Get the entity id's of topological dimension DIM connected to another entity
  //! by specified connectivity from domain FROM_DOM and to domain TO_DOM.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam DIM to topological dimension
  //! @tparam ENT_TYPE entity type
  //!
  //! @param e from entity
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t FROM_DOM = 0, size_t TO_DOM = FROM_DOM,
    class ENT_TYPE>
  auto entity_ids(const ENT_TYPE * e) const {
    const connectivity_t & c = 
      get_connectivity(FROM_DOM, TO_DOM, ENT_TYPE::dimension, DIM);
    assert(!c.empty() && "empty connectivity");
    return c.get_index_space().ids(c.range(e->template id<FROM_DOM>()));
  } // entities

  //--------------------------------------------------------------------------//
  //! Get the entities of topological dimension DIM connected to another entity
  //! by specified connectivity from domain FROM_DOM and to domain TO_DOM.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam DIM to topological dimension
  //! @tparam ENT_TYPE entity type
  //!
  //! @param e from entity
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t FROM_DOM, size_t TO_DOM = FROM_DOM,
    class ENT_TYPE>
  void reverse_entities(ENT_TYPE * e) {
    auto & c = get_connectivity(FROM_DOM, TO_DOM, ENT_TYPE::dimension, DIM);
    assert(!c.empty() && "empty connectivity");
    c.reverse_entities(e->template id<FROM_DOM>());
  } // entities

  //--------------------------------------------------------------------------//
  //! Get the entities of topological dimension DIM connected to another entity
  //! by specified connectivity from domain FROM_DOM and to domain TO_DOM.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam DIM to topological dimension
  //! @tparam ENT_TYPE entity type
  //!
  //! @param e from entity with compile-time domain
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t FROM_DOM = 0, size_t TO_DOM = FROM_DOM,
    class ENT_TYPE>
  void reverse_entities(domain_entity__<FROM_DOM, ENT_TYPE> & e) {
    return reverse_entities<DIM, FROM_DOM, TO_DOM>(e.entity());
  } // entities

  //--------------------------------------------------------------------------//
  //! Get the entities of topological dimension DIM connected to another entity
  //! by specified connectivity from domain FROM_DOM and to domain TO_DOM.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam DIM to topological dimension
  //! @tparam ENT_TYPE entity type
  //!
  //! @param e from entity
  //! @param order order specification
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t FROM_DOM, size_t TO_DOM = FROM_DOM,
    class ENT_TYPE, class U>
  void reorder_entities(ENT_TYPE * e, U && order) {
    auto & c = get_connectivity(FROM_DOM, TO_DOM, ENT_TYPE::dimension, DIM);
    assert(!c.empty() && "empty connectivity");
    c.reorder_entities(e->template id<FROM_DOM>(), std::forward<U>(order));
  } // entities

  //--------------------------------------------------------------------------//
  //! Get the entities of topological dimension DIM connected to another entity
  //! by specified connectivity from domain FROM_DOM and to domain TO_DOM.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam DIM to topological dimension
  //! @tparam ENT_TYPE entity type
  //!
  //! @param e from entity
  //! @param order order specification
  //--------------------------------------------------------------------------//
  template<size_t DIM, size_t FROM_DOM = 0, size_t TO_DOM = FROM_DOM,
    class ENT_TYPE, class U>
  void reverse_entities(domain_entity__<FROM_DOM, ENT_TYPE> & e, U && order) {
    return reorder_entities<DIM, FROM_DOM, TO_DOM>(
      e.entity(), std::forward<U>(order));
  } // entities

  //--------------------------------------------------------------------------//
  //! Debug method to dump the connectivity of the mesh over all domains and
  //! topological dimensions.
  //!
  //! @param stream output stream
  //--------------------------------------------------------------------------//
  std::ostream & dump(std::ostream & stream) {
    for (size_t from_domain = 0; from_domain < MESH_TYPE::num_domains;
         ++from_domain) {
      stream << "=================== from domain: " << from_domain << std::endl;
      for (size_t to_domain = 0; to_domain < MESH_TYPE::num_domains;
          ++to_domain) {
        stream << "========== to domain: " << to_domain << std::endl;
        base_t::ms_->topology[from_domain][to_domain].dump(stream);
      }
    }
    return stream;
  } // dump

  //--------------------------------------------------------------------------//
  //! Debug method to dump the connectivity of the mesh over all domains and
  //! topological dimensions.
  //--------------------------------------------------------------------------//
  void dump() {
    dump(std::cout);
  } // dump

  //--------------------------------------------------------------------------//
  //! Serialize and save to archive.
  //--------------------------------------------------------------------------//
  template<typename A>
  void save(A & archive) const {
    size_t size;
    char * data = serialize_(size);
    archive.saveBinary(&size, sizeof(size));

    archive.saveBinary(data, size);
    delete[] data;
  } // save

  //--------------------------------------------------------------------------//
  //! Deserialize and load from archive.
  //--------------------------------------------------------------------------//
  template<typename A>
  void load(A & archive) {
    size_t size;
    archive.loadBinary(&size, sizeof(size));

    char * data = new char[size];
    archive.loadBinary(data, size);
    unserialize_(data);
    delete[] data;
  } // load

  //--------------------------------------------------------------------------//
  //! Serialize and set size in bytes.
  //--------------------------------------------------------------------------//
  char * serialize_(uint64_t & size) const {
    const size_t alloc_size = 1048576;
    size = alloc_size;

    char * buf = new char[alloc_size];
    uint64_t pos = 0;

    uint32_t num_domains = MESH_TYPE::num_domains;
    std::memcpy(buf + pos, &num_domains, sizeof(num_domains));
    pos += sizeof(num_domains);

    uint32_t num_dimensions = MESH_TYPE::num_dimensions;
    std::memcpy(buf + pos, &num_dimensions, sizeof(num_dimensions));
    pos += sizeof(num_dimensions);

    for (size_t domain = 0; domain < MESH_TYPE::num_domains; ++domain) {
      for (size_t dimension = 0; dimension <= MESH_TYPE::num_dimensions;
        ++dimension) {
        uint64_t num_entities = base_t::ms_->entities[domain][dimension].size();
        std::memcpy(buf + pos, &num_entities, sizeof(num_entities));
        pos += sizeof(num_entities);
      }
    }

    for (size_t from_domain = 0; from_domain < MESH_TYPE::num_domains;
         ++from_domain) {
      for (size_t to_domain = 0; to_domain < MESH_TYPE::num_domains;
           ++to_domain) {

        auto & dc = base_t::ms_->topology[from_domain][to_domain];

        for (size_t from_dim = 0; from_dim <= MESH_TYPE::num_dimensions;
             ++from_dim) {
          for (size_t to_dim = 0; to_dim <= MESH_TYPE::num_dimensions;
               ++to_dim) {
            const connectivity_t & c = dc.get(from_dim, to_dim);

            auto & tv = c.to_id_storage();
            uint64_t num_to = tv.size();
            std::memcpy(buf + pos, &num_to, sizeof(num_to));
            pos += sizeof(num_to);

            size_t bytes = num_to * sizeof(id_vector_t::value_type);

            if (size - pos < bytes) {
              size += bytes + alloc_size;
              buf = (char *)std::realloc(buf, size);
            }

            std::memcpy(buf + pos, tv.data(), bytes);
            pos += bytes;

            uint64_t num_offsets = c.offsets().size();
            std::memcpy(buf + pos, &num_offsets, sizeof(num_offsets));
            pos += sizeof(num_offsets);

            bytes = num_offsets * sizeof(offset_t);

            if (size - pos < bytes) {
              size += bytes + alloc_size;
              buf = (char *)std::realloc(buf, size);
            }

            std::memcpy(buf + pos, c.offsets().storage().buffer(), bytes);
            pos += bytes;
          }
        }
      }
    }

    size = pos;

    return buf;
  }

  //--------------------------------------------------------------------------//
  //! Deserialize from byte buffer.
  //--------------------------------------------------------------------------//
  void unserialize_(char * buf) {
    uint64_t pos = 0;

    uint32_t num_domains;
    std::memcpy(&num_domains, buf + pos, sizeof(num_domains));
    pos += sizeof(num_domains);
    assert(num_domains == MESH_TYPE::num_domains && "domain size mismatch");

    uint32_t num_dimensions;
    std::memcpy(&num_dimensions, buf + pos, sizeof(num_dimensions));
    pos += sizeof(num_dimensions);
    assert(num_dimensions == MESH_TYPE::num_dimensions && 
      "dimension size mismatch");

    unserialize_domains_<
        storage_t, MESH_TYPE, MESH_TYPE::num_domains, MESH_TYPE::num_dimensions,
        0>::unserialize(*this, buf, pos);

    for (size_t from_domain = 0; from_domain < MESH_TYPE::num_domains; 
         ++from_domain) {
      for (size_t to_domain = 0; to_domain < MESH_TYPE::num_domains; 
           ++to_domain) {

        auto & dc = base_t::ms_->topology[from_domain][to_domain];

        for (size_t from_dim = 0; from_dim <= MESH_TYPE::num_dimensions; 
             ++from_dim) {
          for (size_t to_dim = 0; to_dim <= MESH_TYPE::num_dimensions; 
               ++to_dim) {
            connectivity_t & c = dc.get(from_dim, to_dim);

            auto & tv = c.to_id_storage();
            uint64_t num_to;
            std::memcpy(&num_to, buf + pos, sizeof(num_to));
            pos += sizeof(num_to);
            auto ta = (id_vector_t::value_type *)(buf + pos);
            tv.resize(num_to);
            tv.assign(ta, ta + num_to);
            pos += num_to * sizeof(id_vector_t::value_type);

            auto offsets_buf = c.offsets().storage().buffer();
            uint64_t num_offsets;
            std::memcpy(&num_offsets, buf + pos, sizeof(num_offsets));
            pos += sizeof(num_offsets);
            std::memcpy(offsets_buf, buf + pos, num_offsets * sizeof(offset_t));
            pos += num_offsets * sizeof(offset_t);
          }
        }
      }
    }
  }

  //--------------------------------------------------------------------------//
  //! Internal method to append entities to an index space.
  //--------------------------------------------------------------------------//
  void append_to_index_space_(
      size_t domain,
      size_t dim,
      std::vector<mesh_entity_base_ *> & ents,
      std::vector<id_t> & ids) override {
    auto & is = base_t::ms_->index_spaces[domain][dim];
    is.append_(ents, ids);
  }

private:
  template<size_t, size_t, class>
  friend class compute_connectivity__;

  template<size_t, size_t, class>
  friend class compute_bindings__;

  template<size_t DOM, typename VERT_TYPE>
  void init_cell_(entity_type<MESH_TYPE::num_dimensions, DOM> * cell, 
                  VERT_TYPE && verts) {
    auto & c = get_connectivity_(DOM, MESH_TYPE::num_dimensions, 0);

    assert(cell->template id<DOM>() == c.from_size() && "id mismatch");

    for (entity_type<0, DOM> * v : std::forward<VERT_TYPE>(verts)) {
      c.push(v->template global_id<DOM>());
    } // for

    c.add_count(verts.size());
  } // init_cell

  template<size_t DOM, size_t FROM_DIM, size_t TO_DIM, class ENT_TYPE2>
  void init_entity_(entity_type<FROM_DIM, DOM> * super, ENT_TYPE2 && subs) {
    auto & c = get_connectivity_(DOM, FROM_DIM, TO_DIM);

    assert(super->template id<DOM>() == c.from_size() && "id mismatch");

    for (auto e : std::forward<ENT_TYPE2>(subs)) {
      c.push(e->template global_id<DOM>());
    } // for

    c.add_count(subs.size());
  } // init_entity

  // Get the number of entities in a given domain and topological dimension
  size_t num_entities_(size_t dim, size_t domain = 0) const {
    return base_t::ms_->index_spaces[domain][dim].size();
  } // num_entities_

  // Get the number of entities in a given domain and topological dimension
  size_t num_entities_(size_t dim, size_t domain, partition_t partition) const {
    return base_t::ms_->partition_index_spaces[partition][domain][dim].size();
  } // num_entities_

  //--------------------------------------------------------------------------//
  //! Build connectivity informaiton and add entities to the mesh for the
  //! given dimension.
  //!
  //! \remark this is the general one that gets instantiated even though
  //! it may never get called
  //--------------------------------------------------------------------------//
  template<size_t Domain, size_t DimensionToBuild, size_t UsingDimension>
  typename std::enable_if<
      (UsingDimension <= 1 || UsingDimension > MESH_TYPE::num_dimensions)>::type
  build_connectivity() {
    assert(false && "shouldn't be in here");
  }

  //--------------------------------------------------------------------------//
  //! Build connectivity informaiton and add entities to the mesh for the
  //! given dimension.
  //!
  //! \remark This one is enable_if'd so it never gets instantiated in certain
  //! cases, otherwise we would need create_entities in wedges
  //! and vertices
  //!
  //! @tparam Domain domain
  //! @tparam DimensionToBuild topological dimension to build
  //! @tparam UsingDimension using topological dimension to build
  //--------------------------------------------------------------------------//
  template<size_t Domain, size_t DimensionToBuild, size_t UsingDimension>
  typename std::enable_if<
      (UsingDimension > 1 && UsingDimension <= MESH_TYPE::num_dimensions)>::type
  build_connectivity() {
    // std::cerr << "build: " << DimensionToBuild
    // << " using " << UsingDimension << std::endl;

    // Sanity check
    static_assert(
        DimensionToBuild <= MESH_TYPE::num_dimensions,
        "DimensionToBuild must be <= total number of dimensions");
    static_assert(
        UsingDimension <= MESH_TYPE::num_dimensions,
        "UsingDimension must be <= total number of dimensions");
    static_assert(
        Domain < MESH_TYPE::num_domains, 
        "Domain must be < total number of domains");

    // Reference to storage from cells to the entity (to be created here).
    connectivity_t & cell_to_entity =
        get_connectivity_(Domain, UsingDimension, DimensionToBuild);

    // Storage for entity-to-vertex connectivity information.
    connection_vector_t entity_vertex_conn;

    // Helper variables
    size_t max_cell_entity_conns = 1;

    // keep track of the local ids, since they may be added out of order
    std::vector<size_t> entity_ids;

    domain_connectivity__<MESH_TYPE::num_dimensions> & dc =
        base_t::ms_->topology[Domain][Domain];

    // Get connectivity for cells to vertices.
    connectivity_t & cell_to_vertex = dc.template get<UsingDimension>(0);
    assert(!cell_to_vertex.empty());

    const size_t _num_cells = num_entities<UsingDimension, Domain>();

    // Storage for cell-to-entity connectivity information.
    connection_vector_t cell_entity_conn(_num_cells);

    // This map is primarily used to make sure that entities are not
    // created multiple times, i.e., that they are unique.  The
    // emplace method of the map is used to only define a new entity
    // if it does not already exist in the map.
    id_vector_map_t entity_vertices_map;

    // This buffer should be large enough to hold all entities
    // vertices that potentially need to be created
    std::array<id_t, 4096> entity_vertices;

    using cell_type = entity_type<UsingDimension, Domain>;
    using entity_type = entity_type<DimensionToBuild, Domain>;

    auto & is = base_t::ms_->index_spaces[Domain][DimensionToBuild]
                    .template cast<domain_entity__<Domain, entity_type>>();

    auto & cis = base_t::ms_->index_spaces[Domain][UsingDimension]
                     .template cast<domain_entity__<Domain, cell_type>>();

    // Lookup the index space for the entity type being created.
    constexpr size_t cell_index_space = find_index_space_from_dimension__<
        std::tuple_size<typename MESH_TYPE::entity_types>::value,
        typename MESH_TYPE::entity_types, UsingDimension, Domain>::find();

    // Lookup the index space for the vertices from the mesh
    // specialization.
    constexpr size_t vertex_index_space = find_index_space_from_dimension__<
        std::tuple_size<typename MESH_TYPE::entity_types>::value,
        typename MESH_TYPE::entity_types, 0, Domain>::find();

    // Lookup the index space for the entity type being created.
    constexpr size_t entity_index_space = find_index_space_from_dimension__<
        std::tuple_size<typename MESH_TYPE::entity_types>::value,
        typename MESH_TYPE::entity_types, DimensionToBuild, Domain>::find();

    // get the global to local index space map
    auto & context_ = flecsi::execution::context_t::instance();
    size_t color = context_.color();
    auto & gis_to_cis = context_.reverse_index_map(cell_index_space);

    // Get the reverse map of the intermediate ids. This map takes
    // vertices defining an entity to the entity id in MIS.
    auto & reverse_intermediate_map =
        context_.reverse_intermediate_map(DimensionToBuild, Domain);
    auto has_intermediate_map = !reverse_intermediate_map.empty();

    // Get the index map for the entity.
    auto & entity_index_map = context_.reverse_index_map(entity_index_space);

    // Get the map of the vertex ids. This map takes
    // local compacted vertex ids to mesh index space ids.
    // CIS -> MIS.
    auto & vertex_map = context_.index_map(vertex_index_space);

    // a counter for added entityes
    size_t entity_counter{0};

    for (auto & citr : gis_to_cis) {
      size_t c = citr.second;

      // Get the cell object
      auto cell = static_cast<cell_type *>(cis[c]);
      id_t cell_id = cell->template global_id<Domain>();

      // Get storage reference.
      id_vector_t & conns = cell_entity_conn[c];

      // Try to optimize storage.
      conns.reserve(max_cell_entity_conns);

      // This call allows the users specialization to create
      // whatever entities are needed to complete the mesh.
      //
      // p.first:   The number of entities per cell.
      // p.second:  A std::vector of id_t containing the ids of the
      //            vertices that define the entity.
      auto sv = cell->template create_entities(
          cell_id, DimensionToBuild, dc, entity_vertices.data());

      size_t n = sv.size();

      // iterate over the newly-defined entities
      for (size_t i = 0; i < n; ++i) {
        size_t m = sv[i];

        // Get the vertices that define this entity by getting
        // a pointer to the vector-of-vector data and then constructing
        // a vector of ids for only this entity.
        id_t * a = &entity_vertices[i * m];
        id_vector_t ev(a, a + m);

        // Sort the ids for the current entity so that they are
        // monotonically increasing. This ensures that entities are
        // created uniquely (using emplace_back below) because the ids
        // will always occur in the same order for the same entity.
        std::sort(ev.begin(), ev.end());

        //
        // The following set of steps use the vertices that define
        // the entity to be created to lookup the id so
        // that the topology creates it at the correct offset.
        // This requires:
        //
        // 1) lookup the MIS vertex ids
        // 2) create a vector of the MIS vertex ids
        // 3) lookup the MIS id of the entity
        // 4) lookup the CIS id of the entity
        //
        // The CIS id of the entity is passed to the create_entity
        // method. The specialization developer must pass this
        // information to 'make' so that the coloring id of the
        // entity is consitent with the id/offset of the entity
        // created by the topology.
        //

        size_t entity_id;
        if (has_intermediate_map) {

          std::vector<size_t> vertices_mis;
          vertices_mis.reserve(m);

          // Push the MIS vertex ids onto a vector to search for the
          // associated entity.
          for (id_t * aptr{a}; aptr < (a + m); ++aptr) {
            vertices_mis.push_back(vertex_map[aptr->entity()]);
          } // for

          // Lookup the MIS id of the entity.
          const auto entity_id_mis = reverse_intermediate_map.at(vertices_mis);

          // Lookup the CIS id of the entity.
          entity_id = entity_index_map.at(entity_id_mis);

        } else {

          entity_id = entity_counter;

        } // intermediate_map

        id_t id = id_t::make<DimensionToBuild, Domain>(entity_id, color);

        // Emplace the sorted vertices into the entity map
        auto itr = entity_vertices_map.emplace(
            std::move(ev), id_t::make<DimensionToBuild, Domain>(
                               entity_id, cell_id.partition()));

        // Add this id to the cell to entity connections
        conns.push_back(itr.first->second);

        // If the insertion took place
        if (itr.second) {

          // what does this do?
          id_vector_t ev2 = id_vector_t(a, a + m);
          entity_vertex_conn.emplace_back(std::move(ev2));
          entity_ids.emplace_back(entity_id);

          max_cell_entity_conns = std::max(max_cell_entity_conns, conns.size());

          auto ent =
              MESH_TYPE::template create_entity<Domain, DimensionToBuild>(
              this, m, id);

          ++entity_counter;

        } // if
      } // for
    } // for

    // sort the entity connectivity. Entities may have been created out of
    // order.  Sort them using the list of entity ids we kept track of
    if (has_intermediate_map)
      utils::reorder_destructive(
          entity_ids.begin(), entity_ids.end(), entity_vertex_conn.begin());

    // Set the connectivity information from the created entities to
    // the vertices.
    connectivity_t & entity_to_vertex = dc.template get<DimensionToBuild>(0);
    entity_to_vertex.init(entity_vertex_conn);
    cell_to_entity.init(cell_entity_conn);
  } // build_connectivity

  //--------------------------------------------------------------------------//
  //! used internally to compute connectivity information for
  //! topological dimension
  //! FROM_DIM -> TO_DIM where FROM_DIM < TO_DIM
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam FROM_DIM from topological dimension
  //! @tparam TO_DIM to topological dimension
  //--------------------------------------------------------------------------//
  template<size_t FROM_DOM, size_t TO_DOM, size_t FROM_DIM, size_t TO_DIM>
  void transpose() {
    // std::cerr << "transpose: " << FROM_DIM << " -> " << TO_DIM << std::endl;

    // The connectivity we will be populating
    auto & out_conn = get_connectivity_(FROM_DOM, TO_DOM, FROM_DIM, TO_DIM);
    if (!out_conn.empty()) {
      return;
    } // if

    // get the list of "to" entities
    const auto & to_entities = entities<TO_DIM, TO_DOM>();

    index_vector_t pos(num_entities_(FROM_DIM, FROM_DOM), 0);

    // Count how many connectivities go into each slot
    for (auto to_entity : to_entities) {
      for (id_t from_id : entity_ids<FROM_DIM, TO_DOM, FROM_DOM>(to_entity)) {
        ++pos[from_id.entity()];
      }
    }

    out_conn.resize(pos);

    std::fill(pos.begin(), pos.end(), 0);

    // now do the actual transpose
    for (auto to_entity : to_entities) {
      for (auto from_id : entity_ids<FROM_DIM, TO_DOM, FROM_DOM>(to_entity)) {
        auto from_lid = from_id.entity();
        out_conn.set(
            from_lid, to_entity->template global_id<TO_DOM>(), pos[from_lid]++);
      }
    }

    // now we need to sort the connecvtivity arrays:
    // .. we have to make sure the order of connectivity information apears in
    //    in order of global id

    // we need the context to get the global-to-local mapping
    const auto & context_ = flecsi::execution::context_t::instance();

    // find the from index space and get the mapping from global to local
    constexpr size_t to_index_space = find_index_space_from_dimension__<
        std::tuple_size<typename MESH_TYPE::entity_types>::value,
        typename MESH_TYPE::entity_types, TO_DIM, TO_DOM>::find();

    const auto & to__cis_to_gis = context_.index_map(to_index_space);

    // do the final sort of the connectivity arrays
    for (auto from_id : entity_ids<FROM_DIM, TO_DOM>()) {
      // get the connectivity array
      size_t count;
      auto conn = out_conn.get_entities(from_id.entity(), count);
      // pack it into a list of id and global id pairs
      std::vector<std::pair<size_t, id_t>> gids(count);
      std::transform(conn, conn + count, gids.begin(), [&](auto id) {
        return std::make_pair(to__cis_to_gis.at(id.entity()), id);
      });
      // sort via global id
      std::sort(gids.begin(), gids.end(), [](auto a, auto b) {
        return a.first < b.first;
      });
      // upack the results
      std::transform(gids.begin(), gids.end(), conn, [](auto id_pair) {
        return id_pair.second;
      });
    }
  } // transpose

  //--------------------------------------------------------------------------//
  //! Used internally to compute connectivity information for
  //! topological dimension
  //! FROM_DIM -> TO_DIM using FROM_DIM -> DIM' and DIM' -> TO_DIM
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam FROM_DIM from topological dimension
  //! @tparam TO_DIM to topological dimension
  //! @tparam DIM intermediate topological dimension DIM'
  //--------------------------------------------------------------------------//
  //--------------------------------------------------------------------------//
  template<size_t FROM_DOM, size_t TO_DOM, size_t FROM_DIM, size_t TO_DIM,
    size_t DIM>
  void intersect() {
    // std::cerr << "intersect: " << FROM_DIM << " -> " << TO_DIM << std::endl;

    // The connectivity we will be populating
    connectivity_t & out_conn = 
      get_connectivity_(FROM_DOM, TO_DOM, FROM_DIM, TO_DIM);
    if (!out_conn.empty()) {
      return;
    } // if

    // the number of each entity type
    auto num_from_ent = num_entities_(FROM_DIM, FROM_DOM);
    auto num_to_ent = num_entities_(TO_DIM, FROM_DOM);

    // Temporary storage for connection id's
    connection_vector_t conns(num_from_ent);

    // Keep track of which to id's we have visited
    using visited_vec = std::vector<bool>;
    visited_vec visited(num_to_ent);

    size_t max_size = 1;

    // Read connectivities
    connectivity_t & c = get_connectivity_(FROM_DOM, FROM_DIM, DIM);
    assert(!c.empty());

    connectivity_t & c2 = get_connectivity_(TO_DOM, TO_DIM, DIM);
    assert(!c2.empty());

    // Iterate through entities in "from" topological dimension
    for (auto from_entity : entities<FROM_DIM, FROM_DOM>()) {

      id_t from_id = from_entity->template global_id<FROM_DOM>();
      id_vector_t & ents = conns[from_id.entity()];
      ents.reserve(max_size);

      size_t count;
      id_t * ep = c.get_entities(from_id.entity(), count);

      // Create a copy of to vertices so they can be sorted
      id_vector_t from_verts(ep, ep + count);
      // sort so we have a unique key for from vertices
      std::sort(from_verts.begin(), from_verts.end());

      // initially set all to id's to unvisited
      for (auto from_ent2 : entities<DIM, FROM_DOM>(from_entity)) {
        for (id_t to_id : entity_ids<TO_DIM, TO_DOM>(from_ent2)) {
          visited[to_id.entity()] = false;
        }
      }

      // Loop through each from entity again
      for (auto from_ent2 : entities<DIM, FROM_DOM>(from_entity)) {
        for (id_t to_id : entity_ids<TO_DIM, TO_DOM>(from_ent2)) {

          // If we have already visited, skip
          if (visited[to_id.entity()]) {
            continue;
          } // if

          visited[to_id.entity()] = true;

          // If the topological dimensions are the same, always add to id
          if (FROM_DIM == TO_DIM) {
            if (from_id != to_id) {
              ents.push_back(to_id);
            } // if
          } else {
            size_t count;
            id_t * ep = c2.get_entities(to_id.entity(), count);

            // Create a copy of to vertices so they can be sorted
            id_vector_t to_verts(ep, ep + count);
            // Sort to verts so we can do an inclusion check
            std::sort(to_verts.begin(), to_verts.end());

            // If from vertices contains the to vertices add to id
            // to this connection set
            if (DIM < TO_DIM) {
              if (std::includes(
                      from_verts.begin(), from_verts.end(), to_verts.begin(),
                      to_verts.end()))
                ents.emplace_back(to_id);
            }
            // If we are going through a higher level, then set
            // intersection is sufficient. i.e. one set does not need to
            // be a subset of the other
            else {
              if (utils::intersects(
                      from_verts.begin(), from_verts.end(), to_verts.begin(),
                      to_verts.end()))
                ents.emplace_back(to_id);
            } // if

          } // if
        } // for
      } // for

      max_size = std::max(ents.size(), max_size);
    } // for

    // Finally create the connection from the temporary conns
    out_conn.init(conns);
  } // intersect

  //--------------------------------------------------------------------------//
  //! Used to compute connectivity information for topological dimension
  //! D1 -> D2
  //!
  //! @tparam DOM domain
  //! @tparam FROM_DIM from topological dimension
  //! @tparam TO_DIM to topological dimension
  //--------------------------------------------------------------------------//
  template<size_t DOM, size_t FROM_DIM, size_t TO_DIM>
  void compute_connectivity() {
    // std::cerr << "compute: " << FROM_DIM << " -> " << TO_DIM << std::endl;

    // Get the output connectivity
    connectivity_t & out_conn = get_connectivity_(DOM, FROM_DIM, TO_DIM);

    // Check if we have already computed it
    if (!out_conn.empty()) {
      return;
    } // if

    // if we don't have cell -> vertex connectivities, then
    // try building cell -> vertex connectivity through the
    // faces (3d) or edges(2d)
    static_assert(
        MESH_TYPE::num_dimensions <= 3,
        "this needs to be re-thought for higher dimensions");

    if (get_connectivity_(DOM, MESH_TYPE::num_dimensions, 0).empty()) {
      assert(
          !get_connectivity_(DOM, MESH_TYPE::num_dimensions - 1, 0).empty() &&
          " need at least edges(2d)/faces(3) -> vertex connectivity");
      // assume we have cell -> faces, so invert it to get faces -> cells
      transpose<DOM, DOM, MESH_TYPE::num_dimensions - 1,
        MESH_TYPE::num_dimensions>();
      // invert faces -> vertices to get vertices -> faces
      transpose<DOM, DOM, 0, MESH_TYPE::num_dimensions - 1>();
      // build cells -> vertices via intersections with faces
      intersect<DOM, DOM, MESH_TYPE::num_dimensions, 0,
        MESH_TYPE::num_dimensions - 1>();
    }

    // Check if we need to build entities, e.g: edges or faces
    if (num_entities_(FROM_DIM, DOM) == 0) {
      if (get_connectivity_(DOM, FROM_DIM + 1, 0).empty())
        build_connectivity<DOM, FROM_DIM, MESH_TYPE::num_dimensions>();
      else
        build_connectivity<DOM, FROM_DIM, FROM_DIM + 1>();
    } // if

    if (num_entities_(TO_DIM, DOM) == 0) {
      if (get_connectivity_(DOM, TO_DIM + 1, 0).empty())
        build_connectivity<DOM, TO_DIM, MESH_TYPE::num_dimensions>();
      else
        build_connectivity<DOM, TO_DIM, TO_DIM + 1>();
    } // if

    if (num_entities_(FROM_DIM, DOM) == 0 && num_entities_(TO_DIM, DOM) == 0) {
      return;
    } // if

    // Depending on the corresponding topological dimensions, call transpose
    // or intersect as need
    if (FROM_DIM < TO_DIM) {
      compute_connectivity<DOM, TO_DIM, FROM_DIM>();
      transpose<DOM, DOM, FROM_DIM, TO_DIM>();
    } else {
      if (FROM_DIM == 0 && TO_DIM == 0) {
        // compute vertex to vertex connectivities through shared cells.
        compute_connectivity<DOM, FROM_DIM, MESH_TYPE::num_dimensions>();
        compute_connectivity<DOM, MESH_TYPE::num_dimensions, TO_DIM>();
        intersect<DOM, DOM, FROM_DIM, TO_DIM, MESH_TYPE::num_dimensions>();
      } else {
        // computer connectivities through shared vertices.
        compute_connectivity<DOM, FROM_DIM, 0>();
        compute_connectivity<DOM, 0, TO_DIM>();
        intersect<DOM, DOM, FROM_DIM, TO_DIM, 0>();
      }
    } // if
  } // compute_connectivity

  //--------------------------------------------------------------------------//
  //! if the to-dimension is larger than the from-dimension, build the bindings
  //! using the create_bound_entities functionality
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam FROM_DIM from topological dimension
  //! @tparam TO_DIM to topological dimension
  //--------------------------------------------------------------------------//
  template<
      size_t FROM_DOM,
      size_t TO_DOM,
      size_t FROM_DIM,
      size_t TO_DIM,
      typename std::enable_if<(FROM_DOM < TO_DOM)>::type * = nullptr>
  void compute_bindings_() {

    // if the connectivity for a transpose exists, do it
    if (!get_connectivity_(TO_DOM, FROM_DOM, TO_DIM, FROM_DIM).empty())
      transpose<FROM_DOM, TO_DOM, FROM_DIM, TO_DIM>();

    // otherwise try building the connectivity directly
    else if (num_entities_(TO_DIM, TO_DOM) == 0)
      build_bindings<FROM_DOM, TO_DOM, TO_DIM>();

  } // compute_bindings

  //--------------------------------------------------------------------------//
  //! if the from-dimension is larger than the to-dimension, we want
  //! to transpose.  So make sure the opposite connectivity exists first
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam FROM_DIM from topological dimension
  //! @tparam TO_DIM to topological dimension
  //--------------------------------------------------------------------------//
  template<
      size_t FROM_DOM,
      size_t TO_DOM,
      size_t FROM_DIM,
      size_t TO_DIM,
      typename = typename std::enable_if<(FROM_DOM > TO_DOM)>::type>
  void compute_bindings_() {

    // build the opposite connectivity first
    compute_bindings_<TO_DOM, FROM_DOM, TO_DIM, FROM_DIM>();

    // now apply a transpose to get the requested connectivity
    transpose<FROM_DOM, TO_DOM, FROM_DIM, TO_DIM>();

  } // compute_bindings

  //--------------------------------------------------------------------------//
  //! if the from-dimension is larger than the to-dimension, we want
  //! to transpose.  So make sure the opposite connectivity exists first
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam FROM_DIM from topological dimension
  //! @tparam TO_DIM to topological dimension
  //--------------------------------------------------------------------------//
  template<size_t FROM_DOM, size_t TO_DOM, size_t FROM_DIM, size_t TO_DIM>
  typename std::enable_if<(FROM_DOM == TO_DOM)>::type compute_bindings_() {

    // compute connectivities through shared vertices at the at the lowest
    // dimension (doesn't matter which one really)
    compute_bindings_<0, TO_DOM, 0, FROM_DIM>();
    compute_bindings_<0, TO_DOM, 0, TO_DIM>();

    // now try and transpose it
    auto & trans_conn = get_connectivity_(TO_DOM, FROM_DOM, TO_DIM, FROM_DIM);
    if (!trans_conn.empty())
      transpose<FROM_DOM, TO_DOM, FROM_DIM, TO_DIM>();

  } // compute_bindings

  //--------------------------------------------------------------------------//
  //! Main driver for computing bindings
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam FROM_DIM from topological dimension
  //! @tparam TO_DIM to topological dimension
  //--------------------------------------------------------------------------//
  template<size_t FROM_DOM, size_t TO_DOM, size_t FROM_DIM, size_t TO_DIM>
  void compute_bindings() {
    // std::cerr << "compute: , dom " << FROM_DOM << " -> " << TO_DOM
    //           <<  ", dim " << FROM_DIM << " -> " << TO_DIM << std::endl;

    // check if requested connectivity is already there, nothing to do
    connectivity_t & out_conn = 
      get_connectivity_(FROM_DOM, TO_DOM, FROM_DIM, TO_DIM);

    if (!out_conn.empty())
      return;

    compute_bindings_<FROM_DOM, TO_DOM, FROM_DIM, TO_DIM>();
  }

  //--------------------------------------------------------------------------//
  //! Build bindings associated with a from/to domain and topological dimension.
  //! compute_bindings will call this on each binding found in the tuple of
  //! bindings specified in the mesh type/traits mesh specialization.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam TO_DOM to domain
  //! @tparam TO_DIM to topological dimension
  //--------------------------------------------------------------------------//
  template<size_t FROM_DOM, size_t TO_DOM, size_t TO_DIM>
  void build_bindings() {

    // std::cerr << "build bindings: dom " << FROM_DOM << " -> " << TO_DOM
    //           << " dim " << TO_DIM << std::endl;

    // Sanity check
    static_assert(TO_DIM <= MESH_TYPE::num_dimensions, "invalid dimension");

    // Helper variables
    size_t entity_id = 0;
    const size_t _num_cells = num_entities<MESH_TYPE::num_dimensions, FROM_DOM>();

    // Storage for cell connectivity information
    connection_vector_t cell_conn(_num_cells);

    // Get cell definitions from domain 0
    auto & cells = base_t::ms_->index_spaces[FROM_DOM][MESH_TYPE::num_dimensions];

    static constexpr size_t M0 = 0;

    // This buffer should be large enough to hold all entities
    // that potentially need to be created
    std::array<id_t, 4096> entity_ids;

    using to_entity_type = entity_type<TO_DIM, TO_DOM>;

    // Iterate over cells
    for (auto c : cells) {
      // Map used to ensure unique entity creation
      id_vector_map_t entity_ids_map;

      // Get a cell object.
      auto cell = static_cast<entity_type<MESH_TYPE::num_dimensions, M0> *>(c);
      id_t cell_id = cell->template global_id<FROM_DOM>();

      domain_connectivity__<MESH_TYPE::num_dimensions> & primal_conn =
          base_t::ms_->topology[FROM_DOM][FROM_DOM];
      domain_connectivity__<MESH_TYPE::num_dimensions> & domain_conn =
          base_t::ms_->topology[FROM_DOM][TO_DOM];

      // p.first:   The number of entities per cell.
      // p.second:  A std::vector of id_t containing the ids of the
      //            entities that define the bound entity.

      auto sv = cell->create_bound_entities(
          FROM_DOM, TO_DOM, TO_DIM, cell_id, primal_conn, domain_conn,
          entity_ids.data());

      size_t n = sv.size();

      // Iterate over the newly-defined entities
      id_vector_t & conns = cell_conn[cell_id.entity()];

      conns.reserve(n);

      size_t pos = 0;

      auto & is = base_t::ms_->index_spaces[TO_DOM][TO_DIM]
                      .template cast<domain_entity__<TO_DOM, to_entity_type>>();

      for (size_t i = 0; i < n; ++i) {
        size_t m = sv[i];

        id_t create_id = 
          id_t::make<TO_DIM, TO_DOM>(entity_id, cell_id.partition());

        // Add this id to the cell entity connections
        conns.push_back(create_id);

        uint32_t dim_flags = 0;
        uint32_t dom_flags = 0;
        size_t num_vertices = 0;

        for (size_t k = 0; k < m; ++k) {
          id_t global_id = entity_ids[pos + k];
          auto dim = global_id.dimension();
          auto dom = global_id.domain();
          get_connectivity_(TO_DOM, dom, TO_DIM, dim).push(global_id);
          if (dom == FROM_DOM) {
            dim_flags |= 1U << dim;
            num_vertices += dim == 0 ? 1 : 0;
          } else
            dom_flags |= 1U << dim;
        }

        for (size_t i = 0; i < MESH_TYPE::num_dimensions; ++i) {
          if (dim_flags & (1U << i)) {
            get_connectivity_<TO_DOM, FROM_DOM, TO_DIM>(i).end_from();
          }
        }

        for (size_t i = 0; i < TO_DIM; ++i) {
          if (dom_flags & (1U << i)) {
            get_connectivity_(TO_DOM, TO_DOM, TO_DIM, i).end_from();
          }
        }

        auto ent = 
          MESH_TYPE::template create_entity<TO_DOM, TO_DIM>(this, num_vertices);

        ++entity_id;

        pos += m;
      } // for
    } // for

    // Reference to storage from cells to the entity (to be created here).
    connectivity_t & cell_out =
        get_connectivity_(FROM_DOM, TO_DOM, MESH_TYPE::num_dimensions, TO_DIM);
    cell_out.init(cell_conn);

  } // build_bindings

  //--------------------------------------------------------------------------//
  //! Implementation of get_connectivity for various get_connectivity
  //! convenience methods.
  //--------------------------------------------------------------------------//
  const connectivity_t & get_connectivity_(
      size_t from_domain,
      size_t to_domain,
      size_t from_dim,
      size_t to_dim) const {
    assert(from_domain < MESH_TYPE::num_domains && "invalid from domain");
    assert(to_domain < MESH_TYPE::num_domains && "invalid to domain");
    return base_t::ms_->topology[from_domain][to_domain].get(from_dim, to_dim);
  } // get_connectivity

  //--------------------------------------------------------------------------//
  //! Implementation of get_connectivity for various get_connectivity
  //! convenience methods.
  //--------------------------------------------------------------------------//
  connectivity_t & get_connectivity_(
      size_t from_domain,
      size_t to_domain,
      size_t from_dim,
      size_t to_dim) {
    assert(from_domain < MESH_TYPE::num_domains && "invalid from domain");
    assert(to_domain < MESH_TYPE::num_domains && "invalid to domain");
    return base_t::ms_->topology[from_domain][to_domain].get(from_dim, to_dim);
  } // get_connectivity

  //--------------------------------------------------------------------------//
  //! Implementation of get_connectivity for various get_connectivity
  //! convenience methods.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam FROM_DOM to domain
  //! @tparam FROM_DIM from topological dimension
  //--------------------------------------------------------------------------//
  template<size_t FROM_DOM, size_t TO_DOM, size_t FROM_DIM>
  connectivity_t & get_connectivity_(size_t to_dim) {
    return base_t::ms_->topology[FROM_DOM][TO_DOM].template get<FROM_DIM>(to_dim);
  } // get_connectivity

  //--------------------------------------------------------------------------//
  //! Implementation of get_connectivity for various get_connectivity
  //! convenience methods.
  //!
  //! @tparam FROM_DOM from domain
  //! @tparam FROM_DOM to domain
  //! @tparam FROM_DIM from topological dimension
  //! @tparam TO_DIM to topological dimension
  //--------------------------------------------------------------------------//
  template<size_t FROM_DOM, size_t TO_DOM, size_t FROM_DIM, size_t TO_DIM>
  connectivity_t & get_connectivity_() {
    return base_t::ms_->topology[FROM_DOM][TO_DOM].
      template get<FROM_DIM, TO_DIM>();
  } // get_connectivity

  //--------------------------------------------------------------------------//
  //! Implementation of get_connectivity for various get_connectivity
  //! convenience methods.
  //--------------------------------------------------------------------------//
  const connectivity_t &
  get_connectivity_(size_t domain, size_t from_dim, size_t to_dim) const {
    return get_connectivity_(domain, domain, from_dim, to_dim);
  } // get_connectivity

  //--------------------------------------------------------------------------//
  //! Implementation of get_connectivity for various get_connectivity
  //! convenience methods.
  //--------------------------------------------------------------------------//
  connectivity_t &
  get_connectivity_(size_t domain, size_t from_dim, size_t to_dim) {
    return get_connectivity_(domain, domain, from_dim, to_dim);
  } // get_connectivity

}; // class mesh_topology__

} // namespace topology
} // namespace flecsi
