/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_mesh_types_h
#define flexi_mesh_types_h

/*!
 * \file mesh_types.h
 * \authors bergen
 * \date Initial file creation: Dec 23, 2015
 */

#include "flexi/mesh/mesh_utils.h"

namespace flexi {

/*----------------------------------------------------------------------------*
 * class domain_
 *----------------------------------------------------------------------------*/

/*!
  \class domain_ mesh_topology.h
  \brief domain_ allows a domain id to be typeified...
 */

template<size_t M>
class domain_
{
public:

  static const size_t domain = M;

}; // class domain_

/*----------------------------------------------------------------------------*
 * class mesh_entity_base_t
 *----------------------------------------------------------------------------*/

/*!
  \class mesh_entity_base_t mesh_types.h
  \brief mesh_entity_base_t defines a base class that stores the raw info that
    the mesh topology needs, i.e: id and rank data

  \tparam N The number of mesh domains.
 */

template<size_t N>
class mesh_entity_base_t
{
public:

  virtual ~mesh_entity_base_t() {}

  /*!
    Return the id of this entity.

    \return The id of the entity.
   */

  template<size_t M>
  id_t id() const {
    return ids_[M] & 0x0000ffffffffffff;
  } // id

  template<size_t D, size_t M>
  id_t global_id() const {
    return to_global_id<D, M>(id<M>());
  }

  template<size_t M>
  uint16_t info() const { return ids_[M] >> 48; } // info

  /*!
   */

  static constexpr size_t get_dim_(size_t meshDim, size_t dim) {
    return dim > meshDim ? meshDim : dim;
  } // get_dim_

  template <class MT, size_t M> static mesh_entity_base_t
  *create_(size_t dim, size_t id) {
    switch (dim) {
    case 1: {
      using entity_type = 
        typename find_entity_<MT, get_dim_(MT::dimension, 1), M>::type;
      auto entity = new entity_type;
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

  template <class MT> friend class mesh_topology_t;

protected:

  template<size_t M>
  void set_info(uint16_t info) {
    ids_[M] = (uint64_t(info) << 48) | ids_[M];
  } // set_info

private:

  std::array<uint64_t, N> ids_;

}; // class mesh_entity_base_t

/*----------------------------------------------------------------------------*
 * class mesh_entity_t
 *----------------------------------------------------------------------------*/

/*!
  \class mesh_entity_t mesh_types.h
  \brief mesh_entity_t parameterizes a mesh entity base with its dimension and
    number of domains
 */

template <size_t D, size_t N>
class mesh_entity_t : public mesh_entity_base_t<N>
{
public:

  static const size_t dimension = D;

  mesh_entity_t() {}

  virtual ~mesh_entity_t() {}

  template<size_t M>
  id_t global_id() const {
    return mesh_entity_base_t<N>::template global_id<D, M>();
  }
}; // class mesh_entity_t

/*!
  Define the vector type for storing entities.

  \tparam N The number of domains.
 */
template<size_t N>
using entity_vec_t = std::vector<mesh_entity_base_t<N> *>;

/*----------------------------------------------------------------------------*
 * class domain_entity_t
 *----------------------------------------------------------------------------*/

/*!
  \class domain_entity mesh_types.h
  \brief domain_entity is a simple wrapper to mesh entity that associates with
    it a domain id
 */

template<size_t M, class E>
class domain_entity {
public:
  domain_entity(E* entity) : entity_(entity) {}

  domain_entity& operator=(domain_entity& e) {
    entity_ = e.entity_;
    return *this;
  }

  E* entity(){
    return entity_;
  }

  operator E*(){
    return entity_;
  }

  E* operator->(){ 
    return entity_; 
  }

  E* operator*(){ 
    return entity_; 
  }

  operator id_t() {
    return entity_->template id<M>();
  }

  id_t id() {
    return entity_->template id<M>();
  }

  bool operator==(domain_entity e) const {
    return entity_ == e.entity_;
  }

  bool operator!=(domain_entity e) const {
    return entity_ != e.entity_;
  }

private:
  E* entity_;
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

template <class T> class entity_group {
public:
  using vec = std::vector<T *>;

  /*--------------------------------------------------------------------------*
   * class iterator_
   *--------------------------------------------------------------------------*/

  class iterator_ {
  public:
    iterator_(const vec &entities, size_t index)
        : entities_(&entities), index_(index) {}

    iterator_ &operator++() {
      ++index_;
      return *this;
    }

    iterator_ &operator=(const iterator_ &itr) {
      index_ = itr.index_;
      entities_ = itr.entities_;
      return *this;
    }

    T *operator*() { return (*entities_)[index_]; }

    T *operator->() { return (*entities_)[index_]; }

    bool operator==(const iterator_ &itr) const { return index_ == itr.index_; }

    bool operator!=(const iterator_ &itr) const { return index_ != itr.index_; }

  private:
    const vec *entities_;
    size_t index_;
  }; // class iterator_

  //! Constructor
  entity_group() {}

  entity_group(vec &&entities) : entities_(std::move(entities)) {}

  void add(T *ent) { entities_.push_back(ent); }

  const vec &get_entities() const { return entities_; }

  static constexpr size_t dim() { return T::dimension; }

  iterator_ begin() { return iterator_(entities_, 0); }

  iterator_ end() { return iterator_(entities_, entities_.size()); }

private:
  vec entities_;

}; // class entity_group

} // namespace flexi

#endif // flexi_mesh_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
