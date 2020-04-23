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

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

#include <flecsi/data/data_client.h>
#include <flecsi/execution/context.h>
#include <flecsi/topology/connectivity.h>
#include <flecsi/topology/mesh_utils.h>
#include <flecsi/topology/partition.h>
#include <flecsi/topology/types.h>

namespace flecsi {
namespace topology {

/*----------------------------------------------------------------------------*
 * class mesh_entity_base_u
 *----------------------------------------------------------------------------*/

template<class>
class mesh_topology_base_u;

// Aliases for backward compatibility
using mesh_entity_base_ = entity_base_;

template<size_t NUM_DOMAINS>
using mesh_entity_base_u = entity_base_u<NUM_DOMAINS>;

/*----------------------------------------------------------------------------*
 * class mesh_entity_u
 *----------------------------------------------------------------------------*/

//-----------------------------------------------------------------//
//! \class mesh_entity_u mesh_types.h
//! \brief mesh_entity_u parameterizes a mesh entity base with its dimension and
//! number of domains
//!
//! \tparam DIM The dimension of the entity.
//! \tparam NUM_DOMAINS The number of domains.
//-----------------------------------------------------------------//

template<size_t DIM, size_t NUM_DOMAINS>
class mesh_entity_u : public entity_base_u<NUM_DOMAINS>
{
public:
  static constexpr size_t dimension = DIM;

  mesh_entity_u() {}
  ~mesh_entity_u() {}
}; // class mesh_entity_u

// Redecalre the dimension.  This is redundant, and no longer needed in C++17.
template<size_t DIM, size_t NUM_DOMAINS>
constexpr size_t mesh_entity_u<DIM, NUM_DOMAINS>::dimension;

/*----------------------------------------------------------------------------*
 * class domain_entity_t
 *----------------------------------------------------------------------------*/

//-----------------------------------------------------------------//
//! \class domain_entity_u mesh_types.h
//!
//! \brief domain_entity_u is a simple wrapper to mesh entity that
//! associates with its a domain id
//!
//! \tparam DOM Domain
//! \tparam E Entity type
//-----------------------------------------------------------------//

template<size_t DOM, class ENTITY_TYPE>
class domain_entity_u
{
public:
  using id_t = typename ENTITY_TYPE::id_t;
  using item_t = ENTITY_TYPE *;

  // implicit type conversions are evil.  This one tries to convert
  // all pointers to domain_entities
  FLECSI_INLINE_TARGET
  explicit domain_entity_u(ENTITY_TYPE * entity) : entity_(entity) {}
  domain_entity_u & operator=(const domain_entity_u & e) {
    entity_ = e.entity_;
    return *this;
  }

  FLECSI_INLINE_TARGET
  ENTITY_TYPE * entity() {
    return entity_;
  }

  FLECSI_INLINE_TARGET
  const ENTITY_TYPE * entity() const {
    return entity_;
  }

  FLECSI_INLINE_TARGET
  operator ENTITY_TYPE *() {
    return entity_;
  }

  FLECSI_INLINE_TARGET
  ENTITY_TYPE * operator->() {
    return entity_;
  }

  FLECSI_INLINE_TARGET
  const ENTITY_TYPE * operator->() const {
    return entity_;
  }

  FLECSI_INLINE_TARGET
  ENTITY_TYPE * operator*() {
    return entity_;
  }

  FLECSI_INLINE_TARGET
  const ENTITY_TYPE * operator*() const {
    return entity_;
  }

  FLECSI_INLINE_TARGET
  operator size_t() const {
    return entity_->id();
  }

  id_t & global_id() {
    return entity_->template global_id<DOM>();
  }

  FLECSI_INLINE_TARGET
  id_t global_id() const {
    return entity_->template global_id<DOM>();
  }

  FLECSI_INLINE_TARGET
  size_t id() const {
    return entity_->id();
  }

  FLECSI_INLINE_TARGET
  bool operator==(domain_entity_u e) const {
    return entity_ == e.entity_;
  }

  FLECSI_INLINE_TARGET
  bool operator!=(domain_entity_u e) const {
    return entity_ != e.entity_;
  }

  FLECSI_INLINE_TARGET
  bool operator<(domain_entity_u e) const {
    return entity_ < e.entity_;
  }

  FLECSI_INLINE_TARGET
  id_t index_space_id() const {
    return entity_->template global_id<DOM>();
  }

private:
  ENTITY_TYPE * entity_;
};

//-----------------------------------------------------------------//
//! Holds the connectivities from domain M1 -> M2 for all topological
//! dimensions.
//-----------------------------------------------------------------//

template<size_t DIM>
class domain_connectivity_u
{
public:
  using id_t = flecsi::utils::id_t;

  void init_(size_t from_domain, size_t to_domain) {
    from_domain_ = from_domain;
    to_domain_ = to_domain;
  }

  template<size_t FROM_DIM, size_t TO_DIM>
  connectivity_t & get() {
    static_assert(FROM_DIM <= DIM, "invalid from dimension");
    static_assert(TO_DIM <= DIM, "invalid to dimension");
    return conns_[FROM_DIM][TO_DIM];
  }

  template<size_t FROM_DIM, size_t TO_DIM>
  const connectivity_t & get() const {
    static_assert(FROM_DIM <= DIM, "invalid from dimension");
    static_assert(TO_DIM <= DIM, "invalid to dimension");
    return conns_[FROM_DIM][TO_DIM];
  }

  template<size_t FROM_DIM>
  connectivity_t & get(size_t to_dim) {
    static_assert(FROM_DIM <= DIM, "invalid from dimension");
    assert(to_dim <= DIM && "invalid to dimension");
    return conns_[FROM_DIM][to_dim];
  }

  template<size_t FROM_DIM>
  const connectivity_t & get(size_t to_dim) const {
    static_assert(FROM_DIM <= DIM, "invalid from dimension");
    assert(to_dim <= DIM && "invalid to dimension");
    return conns_[FROM_DIM][to_dim];
  }

  connectivity_t & get(size_t from_dim, size_t to_dim) {
    assert(from_dim <= DIM && "invalid from dimension");
    assert(to_dim <= DIM && "invalid to dimension");
    return conns_[from_dim][to_dim];
  }

  const connectivity_t & get(size_t from_dim, size_t to_dim) const {
    assert(from_dim <= DIM && "invalid from dimension");
    assert(to_dim <= DIM && "invalid to dimension");
    return conns_[from_dim][to_dim];
  }

  template<size_t FROM_DIM, size_t NUM_DOMAINS>
  auto get_entities(mesh_entity_u<FROM_DIM, NUM_DOMAINS> * from_ent,
    size_t to_dim) {
    return get<FROM_DIM>(to_dim).get_entities(from_ent->id(from_domain_));
  }
  template<size_t FROM_DIM, size_t NUM_DOMAINS>
  auto get_entities(mesh_entity_u<FROM_DIM, NUM_DOMAINS> * from_ent,
    std::size_t to_dim) const {
    return get<FROM_DIM>(to_dim).get_entities(from_ent->id(from_domain_));
  }

  auto get_entities(id_t from_id, std::size_t to_dim) {
    return get(from_id.dimension(), to_dim).get_entities(from_id.entity());
  }
  auto get_entities(id_t from_id, std::size_t to_dim) const {
    return get(from_id.dimension(), to_dim).get_entities(from_id.entity());
  }

  template<size_t FROM_DIM, size_t NUM_DOMAINS>
  auto get_entities_vec(mesh_entity_u<FROM_DIM, NUM_DOMAINS> * from_ent,
    size_t to_dim) const {
    auto & conn = get<FROM_DIM>(to_dim);
    return conn.get_entities_vec(from_ent->id(from_domain_));
  }

  auto get_entities_vec(id_t from_id, std::size_t to_dim) const {
    auto & conn = get(from_id.dimension(), to_dim);
    return conn.get_entities_vec(from_id.entity());
  }

  std::ostream & dump(std::ostream & stream) {
    for(size_t i = 0; i < conns_.size(); ++i) {
      auto & ci = conns_[i];
      for(size_t j = 0; j < ci.size(); ++j) {
        auto & cj = ci[j];
        stream << "------------- " << i << " -> " << j << std::endl;
        cj.dump(stream);
      }
    }
    return stream;
  }

  void dump() {
    dump(std::cout);
  }

private:
  using conn_array_t = std::array<std::array<connectivity_t, DIM + 1>, DIM + 1>;

  conn_array_t conns_;
  size_t from_domain_;
  size_t to_domain_;
};

/*----------------------------------------------------------------------------*
 * class mesh_topology_base_u
 *----------------------------------------------------------------------------*/

//-----------------------------------------------------------------//
//! \class mesh_topology_base_u mesh_topology.h
//! \brief contains methods and data about the mesh topology that do not depend
//! on type parameterization, e.g: entity types, domains, etc.
//-----------------------------------------------------------------//

class mesh_topology_base_t
{
};

template<class STORAGE_TYPE>
class mesh_topology_base_u : public data::data_client_t,
                             public mesh_topology_base_t
{
public:
  using id_t = utils::id_t;

  STORAGE_TYPE storage;

  //-----------------------------------------------------------------//
  //! Return the number of entities in for a specific domain and topology dim.
  //-----------------------------------------------------------------//
  virtual size_t num_entities(size_t dim, size_t domain) const = 0;

  //-----------------------------------------------------------------//
  //! Return the topological dimension of the mesh.
  //-----------------------------------------------------------------//
  virtual size_t topological_dimension() const = 0;

  //-----------------------------------------------------------------//
  //! Get the normal (non-binding) connectivity of a domain.
  //-----------------------------------------------------------------//
  virtual const connectivity_t &
  get_connectivity(size_t domain, size_t from_dim, size_t to_dim) const = 0;

  //-----------------------------------------------------------------//
  //! Get the normal (non-binding) connectivity of a domain.
  //-----------------------------------------------------------------//
  virtual connectivity_t &
  get_connectivity(size_t domain, size_t from_dim, size_t to_dim) = 0;

  //-----------------------------------------------------------------//
  //! Get the binding connectivity of specified domains.
  //-----------------------------------------------------------------//
  virtual const connectivity_t & get_connectivity(size_t from_domain,
    size_t to_domain,
    size_t from_dim,
    size_t to_dim) const = 0;

  //-----------------------------------------------------------------//
  //! Get the binding connectivity of specified domains.
  //-----------------------------------------------------------------//
  virtual connectivity_t & get_connectivity(size_t from_domain,
    size_t to_domain,
    size_t from_dim,
    size_t to_dim) = 0;

  //-----------------------------------------------------------------//
  //! This method should be called to construct and entity rather than
  //! calling the constructor directly. This way, the ability to have
  //! extra initialization behavior is reserved.
  //-----------------------------------------------------------------//
  template<class T, size_t DOM = 0, class... S>
  T * make(S &&... args) {
    return storage.template make<T, DOM>(std::forward<S>(args)...);
  } // make
}; // mesh_topology_base_u

template<class MESH_TYPE, size_t DIM, size_t DOM>
using entity_type_ = typename find_entity_<MESH_TYPE, DIM, DOM>::type;

template<class STORAGE_TYPE, class MESH_TYPE, size_t NM, size_t DOM, size_t DIM>
void
unserialize_dimension_(mesh_topology_base_u<STORAGE_TYPE> & mesh,
  char * buf,
  uint64_t & pos) {
  uint64_t num_entities;
  std::memcpy(&num_entities, buf + pos, sizeof(num_entities));
  pos += sizeof(num_entities);

  using id_t = utils::id_t;

  std::vector<entity_base_ *> ents;
  std::vector<id_t> ids;
  ents.reserve(num_entities);
  ids.reserve(num_entities);

  auto & context_ = flecsi::execution::context_t::instance();

  size_t partition_id = context_.color();

  for(size_t local_id = 0; local_id < num_entities; ++local_id) {
    id_t global_id = id_t::make<DIM, DOM>(local_id, partition_id);

    auto ent = new entity_type_<MESH_TYPE, DIM, DOM>();
    ent->template set_global_id<DOM>(global_id);
    ents.push_back(ent);
    ids.push_back(global_id);
  }

  mesh.append_to_index_space_(DOM, DIM, ents, ids);
}

template<class STORAGE_TYPE,
  class MESH_TYPE,
  size_t NUM_DOMAINS,
  size_t NUM_DIMS,
  size_t DOM,
  size_t DIM>
struct unserialize_dimensions_ {

  static void unserialize(mesh_topology_base_u<STORAGE_TYPE> & mesh,
    char * buf,
    uint64_t & pos) {
    unserialize_dimension_<STORAGE_TYPE, MESH_TYPE, NUM_DOMAINS, DOM, DIM>(
      mesh, buf, pos);
    unserialize_dimensions_<STORAGE_TYPE, MESH_TYPE, NUM_DOMAINS, NUM_DIMS, DOM,
      DIM + 1>::unserialize(mesh, buf, pos);
  }
};

template<class STORAGE_TYPE,
  class MESH_TYPE,
  size_t NUM_DOMAINS,
  size_t NUM_DIMS,
  size_t DOM>
struct unserialize_dimensions_<STORAGE_TYPE,
  MESH_TYPE,
  NUM_DOMAINS,
  NUM_DIMS,
  DOM,
  NUM_DIMS> {

  static void unserialize(mesh_topology_base_u<STORAGE_TYPE> & mesh,
    char * buf,
    uint64_t & pos) {
    unserialize_dimension_<STORAGE_TYPE, MESH_TYPE, NUM_DOMAINS, DOM, NUM_DIMS>(
      mesh, buf, pos);
  }
};

template<class STORAGE_TYPE,
  class MESH_TYPE,
  size_t NUM_DOMAINS,
  size_t NUM_DIMS,
  size_t DOM>
struct unserialize_domains_ {

  static void unserialize(mesh_topology_base_u<STORAGE_TYPE> & mesh,
    char * buf,
    uint64_t & pos) {
    unserialize_dimensions_<STORAGE_TYPE, MESH_TYPE, NUM_DOMAINS, NUM_DIMS, DOM,
      0>::unserialize(mesh, buf, pos);
    unserialize_domains_<STORAGE_TYPE, MESH_TYPE, NUM_DOMAINS, NUM_DIMS,
      DOM + 1>::unserialize(mesh, buf, pos);
  }
};

template<class STORAGE_TYPE,
  class MESH_TYPE,
  size_t NUM_DOMAINS,
  size_t NUM_DIMS>
struct unserialize_domains_<STORAGE_TYPE,
  MESH_TYPE,
  NUM_DOMAINS,
  NUM_DIMS,
  NUM_DOMAINS> {

  static void unserialize(mesh_topology_base_u<STORAGE_TYPE> & mesh,
    char * buf,
    uint64_t & pos) {}
};

} // namespace topology
} // namespace flecsi
