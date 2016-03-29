/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_mesh_types_h
#define flecsi_mesh_types_h

/*!
   \file mesh_types.h
   \authors nickm@lanl.gov, bergen@lanl.gov
   \date Initial file creation: Dec 23, 2015
 */

#include <array>
#include <unordered_map>
#include <cassert>
#include <iostream>
#include <vector>

#include "flecsi/data/data_client.h"
#include "flecsi/mesh/mesh_utils.h"

namespace flecsi
{
/*----------------------------------------------------------------------------*
 * struct typeify
 *----------------------------------------------------------------------------*/

template <typename T, T M>
struct typeify {
  static constexpr T value = M;
};

template <size_t M>
using dimension_ = typeify<size_t, M>;

template <size_t M>
using domain_ = typeify<size_t, M>;

/*----------------------------------------------------------------------------*
 * Simple types
 *----------------------------------------------------------------------------*/

using id_vector_t = std::vector<id_t>;
using connection_vector_t = std::vector<id_vector_t>;

// hash use for mapping in building topology connectivity
struct id_vector_hash_t {
  size_t operator()(const id_vector_t & v) const
  {
    size_t h = 0;
    for (id_t id : v) {
      h |= id;
    } // for

    return h;
  } // operator()

}; // struct id_vector_hash_t

// used when building the topology connectivities
using id_vector_map_t = std::unordered_map<id_vector_t, id_t, id_vector_hash_t>;

// the second topology vector holds the offsets into to from dimension
using index_vector_t = std::vector<size_t>;

/*----------------------------------------------------------------------------*
 * class mesh_entity_base_t
 *----------------------------------------------------------------------------*/

class mesh_topology_base_t;

/*!
  \class mesh_entity_base_t mesh_types.h
  \brief mesh_entity_base_t defines a base class that stores the raw info that
    the mesh topology needs, i.e: id and rank data

  \tparam N The number of mesh domains.
 */

template <size_t N>
class mesh_entity_base_t
{
 public:

  virtual ~mesh_entity_base_t() {}

  /*!
    Return the id of this entity.

    \return The id of the entity.
   */
  template <size_t M>
  id_t global_id() const
  {
    return ids_[M];
  } // id

  template <size_t M>
  size_t id() const
  {
    return ids_[M].entity();
  } // id

  template <size_t M>
  uint16_t info() const
  {
    return ids_[M] >> 48;
  } // info

  /*!
   */

  static constexpr size_t get_dim_(size_t meshDim, size_t dim)
  {
    return dim > meshDim ? meshDim : dim;
  } // get_dim_

  template <class MT, size_t M>
  static mesh_entity_base_t * create_(
      size_t dim, size_t id, mesh_topology_base_t & mesh)
  {
    switch (dim) {
      // FIXME: switch is probably unnecessary...
      case 1: {
        using entity_type =
            typename find_entity_<MT, get_dim_(MT::dimension, 1), M>::type;
        auto entity = new entity_type(mesh);
        entity->ids_[M] = id;
        return entity;
      }
      case MT::dimension: {
        using entity_type =
            typename find_entity_<MT, get_dim_(MT::dimension, 2), M>::type;
        auto entity = new entity_type(mesh);
        entity->ids_[M] = id;
        return entity;
      }
#if 0 // FIXME: This will have to be included for 3D
    case 2: {
      using entity_type = 
        typename find_entity_<MT, get_dim_(MT::dimension, 2), M>::type;
      auto entity = new entity_type;
      entity->ids_[M] = id;
      return entity;
    }
#endif
      default:
        assert(false && "invalid topology dim");
    }
  }

  template <class MT>
  friend class mesh_topology_t;

 protected:

  template <size_t M>
  void set_info(uint16_t info)
  {
    ids_[M] = (uint64_t(info) << 48) | ids_[M];
  } // set_info

 private:

  std::array<id_t, N> ids_;

}; // class mesh_entity_base_t

/*----------------------------------------------------------------------------*
 * class mesh_entity_t
 *----------------------------------------------------------------------------*/

/*!
  \class mesh_entity_t mesh_types.h
  \brief mesh_entity_t parameterizes a mesh entity base with its dimension and
    number of domains

  \tparam D The dimension of the entity.
  \tparam N The number of domains.
 */

template <size_t D, size_t N>
class mesh_entity_t : public mesh_entity_base_t<N>
{
 public:
  static const size_t dimension = D;

  mesh_entity_t() {}
  virtual ~mesh_entity_t() {}
}; // class mesh_entity_t

/*!
  Define the vector type for storing entities.

  \tparam N The number of domains.
 */
template <size_t N>
using entity_vector_t = std::vector<mesh_entity_base_t<N> *>;

/*----------------------------------------------------------------------------*
 * class domain_entity_t
 *----------------------------------------------------------------------------*/

/*!
  \class domain_entity mesh_types.h

  \brief domain_entity is a simple wrapper to mesh entity that associates with
    it a domain id

  \tparam M Domain
  \tparam E Entity type
 */

template <size_t M, class E>
class domain_entity
{
 public:
  domain_entity(E * entity) : entity_(entity) {}
  domain_entity & operator=(domain_entity & e)
  {
    entity_ = e.entity_;
    return *this;
  }

  E * entity() { return entity_; }
  operator E *() { return entity_; }
  E * operator->() { return entity_; }
  E * operator*() { return entity_; }
  operator size_t() const { return entity_->template id<M>(); }
  id_t global_id() const { return entity_->template global_id<M>(); }
  size_t id() const { return entity_->template id<M>(); }
  bool operator==(domain_entity e) const { return entity_ == e.entity_; }
  bool operator!=(domain_entity e) const { return entity_ != e.entity_; }
 private:
  E * entity_;
};

/*----------------------------------------------------------------------------*
 * class entity_group
 *----------------------------------------------------------------------------*/

/*!
  \class entity_group mesh_topology.h
  \brief entity_group is an ordered collection of entities. this can be used
    for grouping of entities where global / topology connectivity
    is not required, e.g: "wedges of a corner"
 */

template <class T>
class entity_group
{
 public:
  using vec = std::vector<T *>;

  /*--------------------------------------------------------------------------*
   * class iterator_
   *--------------------------------------------------------------------------*/

  class iterator_
  {
   public:
    iterator_(const vec & entities, size_t index)
        : entities_(&entities), index_(index)
    {
    }

    iterator_ & operator++()
    {
      ++index_;
      return *this;
    }

    iterator_ & operator=(const iterator_ & itr)
    {
      index_ = itr.index_;
      entities_ = itr.entities_;
      return *this;
    }

    T * operator*() { return (*entities_)[index_]; }
    T * operator->() { return (*entities_)[index_]; }
    bool operator==(const iterator_ & itr) const
    {
      return index_ == itr.index_;
    }

    bool operator!=(const iterator_ & itr) const
    {
      return index_ != itr.index_;
    }

   private:
    const vec * entities_;
    size_t index_;
  }; // class iterator_

  //! Constructor
  entity_group() {}
  entity_group(vec && entities) : entities_(std::move(entities)) {}
  void add(T * ent) { entities_.push_back(ent); }
  const vec & get_entities() const { return entities_; }
  static constexpr size_t dim() { return T::dimension; }
  iterator_ begin() { return iterator_(entities_, 0); }
  iterator_ end() { return iterator_(entities_, entities_.size()); }

  auto operator[](size_t i) const
  { return entities_[i]; }

  auto front() const
  { return entities_.front(); }

  auto back() const 
  { return entities_.back(); }

  auto size() const 
  { return entities_.size(); }

 private:
  vec entities_;

}; // class entity_group

/*----------------------------------------------------------------------------*
 * class connectivity_t
 *----------------------------------------------------------------------------*/

/*!
  \class connectivity_t mesh_topology.h
  \brief connectivity_t provides basic connectivity information in a
    compressed storage format.
 */
class connectivity_t
{
 public:
  //! Constructor.
  connectivity_t() {}
  /*!
    Clear the storage arrays for this instance.
   */
  void clear()
  {
    to_id_vec_.clear();
    from_index_vec_.clear();
  } // clear

  /*!
    Initialize the offset array.
   */
  void init() { from_index_vec_.push_back(0); }
  /*!
    Initialize the connectivity information from a given connectivity
    vector.

    \param cv The connectivity information.
   */
  void init(const connection_vector_t & cv)
  {
    assert(to_id_vec_.empty() && from_index_vec_.empty());

    // the first offset is always 0
    from_index_vec_.push_back(0);

    // populate the to id's and add from offsets for each connectivity group

    size_t n = cv.size();

    for (size_t i = 0; i < n; ++i) {
      const id_vector_t & iv = cv[i];

      for (id_t id : iv) {
        to_id_vec_.push_back(id);
      } // for

      from_index_vec_.push_back(to_id_vec_.size());
    } // for
  } // init

  /*!
    Initialize connectivity and create entities.
    Used in build_connectivity() for creating edges/faces.

    \tparam mesh type
    \tparam domain
    \tparam num domains
    \param id vector to populate
    \param entity vector to populate
    \param input connection vector
    \param topological dimension of entities created
   */
  template <class MT, size_t M, size_t N>
  void init_create(id_vector_t & iv, entity_vector_t<N> & ev,
      const connection_vector_t & cv, size_t dim, mesh_topology_base_t & mesh)
  {
    assert(to_id_vec_.empty() && from_index_vec_.empty());

    // the first offset is always 0
    from_index_vec_.push_back(0);

    size_t n = cv.size();

    size_t max_id = 0;

    // cv is organized into groups of from entity to entity

    for (size_t i = 0; i < n; ++i) {
      const id_vector_t & iv = cv[i];

      for (id_t id : iv) {
        max_id = std::max(max_id, id.entity());
        to_id_vec_.push_back(id);
      } // for

      from_index_vec_.push_back(to_id_vec_.size());
    } // for

    size_t start_id = ev.size();

    ev.reserve(max_id + 1);

    for (size_t local_id = start_id; local_id <= max_id; ++local_id) {
      id_t global_id = id_t::make<M>(dim, local_id);
      ev.push_back(
          mesh_entity_base_t<N>::template create_<MT, M>(dim, global_id, mesh));
      iv.push_back(global_id);
    } // for
  } // init_create

  /*!
    Resize a connection.

    \param num_conns Number of connections for each group
   */
  void resize(index_vector_t & num_conns)
  {
    clear();

    size_t n = num_conns.size();
    from_index_vec_.resize(n + 1);

    uint64_t size = 0;

    for (size_t i = 0; i < n; ++i) {
      from_index_vec_[i] = size;
      size += num_conns[i];
    } // for

    from_index_vec_[n] = size;

    to_id_vec_.resize(size);
    std::fill(to_id_vec_.begin(), to_id_vec_.end(), 0);
  } // resize

  /*!
    End a from entity group by setting the end offset in the
    from connection vector.
   */
  void end_from() { from_index_vec_.push_back(to_id_vec_.size()); } // end_from
  /*!
    Push a single id into the current from group.
   */
  void push(id_t id) { to_id_vec_.push_back(id); } // push
  /*!
    Debugging method. Dump the raw vectors of the connection.
   */
  void dump()
  {
    for (size_t i = 1; i < from_index_vec_.size(); ++i) {
      for (size_t j = from_index_vec_[i - 1]; j < from_index_vec_[i]; ++j) {
        std::cout << to_id_vec_[j].entity() << std::endl;
        // std::cout << to_id_vec_[j] << std::endl;
      }
      std::cout << std::endl;
    }

    std::cout << "=== id_vec" << std::endl;
    for (id_t id : to_id_vec_) {
      std::cout << id.entity() << std::endl;
    } // for

    std::cout << "=== group_vec" << std::endl;
    for (size_t index : from_index_vec_) {
      std::cout << index << std::endl;
    } // for
  } // dump

  /*!
    Get the from index vector.
   */
  const index_vector_t & get_from_index_vec() const { return from_index_vec_; }
  /*!
    Get the to id's vector.
   */
  const id_vector_t & get_entities() const { return to_id_vec_; }
  /*!
    Get the entities of the specified from index.
   */
  id_t * get_entities(size_t index)
  {
    assert(index < from_index_vec_.size() - 1);
    return to_id_vec_.data() + from_index_vec_[index];
  }

  /*!
    Get the entities of the specified from index and return the count.
   */
  id_t * get_entities(size_t index, size_t & endIndex)
  {
    assert(index < from_index_vec_.size() - 1);
    uint64_t start = from_index_vec_[index];
    endIndex = from_index_vec_[index + 1] - start;
    return to_id_vec_.data() + start;
  }

  /*!
    True if the connectivity is empty (hasn't been populated).
   */
  bool empty() const { return to_id_vec_.empty(); }
  /*!
    Set a single connection.
   */
  void set(id_t from_local_id, id_t to_id, size_t pos)
  {
    to_id_vec_[from_index_vec_[from_local_id] + pos] = to_id;
  }

  /*!
    Return the number of from entities.
   */
  size_t from_size() const { return from_index_vec_.size() - 1; }
  /*!
    Return the number of to entities.
   */
  size_t to_size() const { return to_id_vec_.size(); }
  /*!
    Set/init the connectivity use by compute topology methods like transpose.
   */
  template <size_t M, size_t N>
  void set(entity_vector_t<N> & ev, connection_vector_t & conns)
  {
    clear();

    size_t n = conns.size();
    from_index_vec_.resize(n + 1);

    size_t size = 0;

    for (size_t i = 0; i < n; i++) {
      from_index_vec_[i] = size;
      size += conns[i].size();
    }

    from_index_vec_[n] = size;

    to_id_vec_.reserve(size);

    for (size_t i = 0; i < n; ++i) {
      const id_vector_t & conn = conns[i];
      uint64_t m = conn.size();

      for (size_t j = 0; j < m; ++j) {
        to_id_vec_.push_back(ev[conn[j]]->template global_id<M>());
      }
    }
  }

  id_vector_t to_id_vec_;
  index_vector_t from_index_vec_;

}; // class connectivity_t

template <size_t D, size_t NM>
struct mesh_storage_t {

  /*!
    Defines a type for storing entity instances as an array of
    \ref entity_vector_t, which is a std::vector of \ref mesh_entity_base_t.
   */
  using entities_t = std::array<entity_vector_t<NM>, D + 1>;

  /*!
    Defines a type for storing connectivity information.
   */
  using topology_t = std::array<std::array<connectivity_t, D + 1>, D + 1>;

  using id_vecs_t = std::array<id_vector_t, D + 1>;

  // array of array of vector of mesh_entity_base_t *
  std::array<entities_t, NM> entities;

  // array of array of connectivity_t
  std::array<std::array<topology_t, NM>, NM> topology;

  // array of array of vector of id_t
  std::array<id_vecs_t, NM> id_vecs;

}; // struct mesh_storage_t

/*----------------------------------------------------------------------------*
 * class mesh_topology_base_t
 *----------------------------------------------------------------------------*/

/*!
  \class mesh_topology_base_t mesh_topology.h
  \brief contains methods and data about the mesh topology that do not depend
    on type parameterization, e.g: entity types, domains, etc.
 */

class mesh_topology_base_t : public data_client_t
{
public:

  /*!
    Return the number of entities in for a specific domain and topology dim.
   */
  virtual size_t num_entities(size_t domain, size_t dim) const = 0;

  /*!
    Return the topological dimension of the mesh.
   */
  virtual size_t topological_dimension() const = 0;

  /*!
    Get the normal (non-binding) connectivity of a domain.
   */
  virtual const connectivity_t & get_connectivity(
      size_t domain, size_t from_dim, size_t to_dim) const = 0;

  /*!
    Get the normal (non-binding) connectivity of a domain.
   */
  virtual connectivity_t & get_connectivity(
      size_t domain, size_t from_dim, size_t to_dim) = 0;

  /*!
    Get the binding connectivity of specified domains.
   */
  virtual const connectivity_t & get_connectivity(size_t from_domain,
      size_t to_domain, size_t from_dim, size_t to_dim) const = 0;

  /*!
    Get the binding connectivity of specified domains.
   */
  virtual connectivity_t & get_connectivity(
      size_t from_domain, size_t to_domain, size_t from_dim, size_t to_dim) = 0;

}; // mesh_topology_base_t

} // namespace flecsi

#endif // flecsi_mesh_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
