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
#include "flecsi/topology/mesh_utils.h"
#include "flecsi/utils/array_ref.h"
#include "flecsi/utils/reorder.h"
#include "flecsi/topology/index_space.h"

namespace flecsi {
namespace topology {

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

using id_vector_t = std::vector<utils::id_t>;
using connection_vector_t = std::vector<id_vector_t>;

// hash use for mapping in building topology connectivity
struct id_vector_hash_t {
  size_t operator()(const id_vector_t & v) const
  {
    size_t h = 0;
    for (utils::id_t id : v) {
      h |= id.local_id();
    } // for

    return h;
  } // operator()

}; // struct id_vector_hash_t

// used when building the topology connectivities
using id_vector_map_t = 
  std::unordered_map<id_vector_t, utils::id_t, id_vector_hash_t>;

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

class mesh_entity_base_{
public:
  using id_t = flecsi::utils::id_t;
};

template <size_t N>
class mesh_entity_base_t : public mesh_entity_base_
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

  id_t global_id(size_t domain) const
  {
    return ids_[domain];
  } // id

  template <size_t M>
  size_t id() const
  {
    return ids_[M].entity();
  } // id

  size_t id(size_t domain) const
  {
    return ids_[domain].entity();
  } // id

  template <size_t M>
  uint16_t info() const
  {
    return ids_[M] >> 48;
  } // info

  /*!
    Set the id of this entity.
   */
  template <size_t M>
  void set_global_id( const id_t & id )
  {
    ids_[M] = id;
  } // id

  /*!
   */

  static constexpr size_t get_dim_(size_t meshDim, size_t dim)
  {
    return dim > meshDim ? meshDim : dim;
  } // get_dim_

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
  using id_t = typename E::id_t;
  using item_t = E*;

  domain_entity(E * entity) : entity_(entity) {}
  domain_entity & operator=(const domain_entity & e)
  {
    entity_ = e.entity_;
    return *this;
  }

  E * entity() { return entity_; }
  const E * entity() const { return entity_; }
  operator E *() { return entity_; }
  E * operator->() { return entity_; }
  const E * operator->() const { return entity_; }
  E * operator*() { return entity_; }
  const E * operator*() const { return entity_; }
  operator size_t() const { return entity_->template id<M>(); }
  id_t global_id() const { return entity_->template global_id<M>(); }
  size_t id() const { return entity_->template id<M>(); }
  bool operator==(domain_entity e) const { return entity_ == e.entity_; }
  bool operator!=(domain_entity e) const { return entity_ != e.entity_; }
  bool operator<(domain_entity e) const { return entity_ < e.entity_; }
  id_t index_space_id() const { return entity_->template global_id<M>(); }
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
  
  using id_t = flecsi::utils::id_t;

  connectivity_t(const connectivity_t&) = delete;

  connectivity_t& operator=(const connectivity_t&) = delete;

  // allow move operations
  connectivity_t(connectivity_t&&) = default;
  connectivity_t& operator=(connectivity_t&&) = default;

  //! Constructor.
  connectivity_t()
  : index_space_(false){}

  /*!
    Clear the storage arrays for this instance.
   */
  void clear()
  {
    index_space_.clear();
    from_index_vec_.clear();
  } // clear

  /*!
    Initialize the offset array.
   */
  void init() {
    clear();
    from_index_vec_.push_back(0);
  }

  /*!
    Initialize the connectivity information from a given connectivity
    vector.

    \param cv The connectivity information.
   */
  void init(const connection_vector_t & cv) {

    clear();

    // the first offset is always 0
    from_index_vec_.push_back(0);

    // populate the to id's and add from offsets for each connectivity group

    size_t start = index_space_.begin_push_();

    size_t n = cv.size();

    size_t from = 0;

    for (size_t i = 0; i < n; ++i) {
      const id_vector_t & iv = cv[i];

      for (id_t id : iv) {
        index_space_.batch_push_(id);
        ++from;
      } // for

      from_index_vec_.push_back(from);
    } // for

    index_space_.end_push_(start);
  } // init

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

    index_space_.resize_(size);
    index_space_.fill_(id_t(0));
  } // resize

  /*!
    End a from entity group by setting the end offset in the
    from connection vector.
   */
  void end_from() {
    from_index_vec_.push_back(index_space_.size());
  } // end_from

  /*!
    Push a single id into the current from group.
   */
  void push(id_t id) {
    index_space_.push_(id);
  } // push

  /*!
    Debugging method. Dump the raw vectors of the connection.
   */
  std::ostream & dump( std::ostream & stream )
  {
    for (size_t i = 1; i < from_index_vec_.size(); ++i) {
      for (size_t j = from_index_vec_[i - 1]; j < from_index_vec_[i]; ++j) {
        stream << index_space_(j).entity() << std::endl;
        // stream << to_id_vec_[j] << std::endl;
      }
      stream << std::endl;
    }

    stream << "=== id_vec" << std::endl;
    for (id_t id : index_space_.ids()) {
      stream << id.entity() << std::endl;
    } // for

    stream << "=== group_vec" << std::endl;
    for (size_t index : from_index_vec_) {
      stream << index << std::endl;
    } // for
    return stream;
  } // dump

  void dump()
  {
    dump( std::cout );
  } // dump

  /*!
    Get the from index vector.
   */
  const index_vector_t & get_from_index_vec() const { return from_index_vec_; }
  /*!
    Get the to id's vector.
   */
  const id_vector_t & get_entities() const { return index_space_.id_vec(); }
  /*!
    Get the entities of the specified from index.
   */
  id_t * get_entities(size_t index)
  {
    assert(index < from_index_vec_.size() - 1);
    return index_space_.id_array() + from_index_vec_[index];
  }

  /*!
    Get the entities of the specified from index and return the count.
   */
  id_t * get_entities(size_t index, size_t & count)
  {
    assert(index < from_index_vec_.size() - 1);
    uint64_t start = from_index_vec_[index];
    count = from_index_vec_[index + 1] - start;
    return index_space_.id_array() + start;
  }


  /*!
    Get the entities of the specified from index and return the count.
   */
  auto get_entity_vec(size_t index) const
  {
    assert(index < from_index_vec_.size() - 1);
    auto start = from_index_vec_[index];
    auto count = from_index_vec_[index + 1] - start;
    return utils::make_array_ref( index_space_.id_array() + start, count );
  }


  /*!
    Get the entities of the specified from index and return the count.
   */
  void reverse_entities(size_t index)
  {
    assert(index < from_index_vec_.size() - 1);
    auto start = from_index_vec_[index];
    auto end = from_index_vec_[index + 1];
    std::reverse(index_space_.index_begin_() + start,
                 index_space_.index_begin_() + end);
  }


  /*!
    Get the entities of the specified from index and return the count.
   */
  template< class U >
  void reorder_entities(size_t index, U && order)
  {
    assert(index < from_index_vec_.size() - 1);
    auto start = from_index_vec_[index];
    auto count = from_index_vec_[index + 1] - start;
    assert( order.size() == count );
    utils::reorder( order.begin(), order.end(), index_space_.id_array() + start );
  }

  /*!
    True if the connectivity is empty (hasn't been populated).
   */
  bool empty() const { return index_space_.empty(); }
  /*!
    Set a single connection.
   */
  void set(size_t from_local_id, id_t to_id, size_t pos)
  {
    index_space_(from_index_vec_[from_local_id] + pos) = to_id;
  }

  /*!
    Return the number of from entities.
   */
  size_t from_size() const { return from_index_vec_.size() - 1; }
  /*!
    Return the number of to entities.
   */
  size_t to_size() const { return index_space_.size(); }
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

    index_space_.begin_push_(size);

    for (size_t i = 0; i < n; ++i) {
      const id_vector_t & conn = conns[i];
      uint64_t m = conn.size();

      for (size_t j = 0; j < m; ++j) {
        index_space_.batch_push_(ev[conn[j]]->template global_id<M>());
      }
    }
  }

  const index_vector_t & from_index_vec() const
  {
    return from_index_vec_;
  }

  index_vector_t & from_index_vec()
  {
    return from_index_vec_;
  }

  const auto& to_id_vec() const
  {
    return index_space_.id_vec();
  }

  auto& to_id_vec()
  {
    return index_space_.id_vec_();
  }

  auto& get_index_space(){
    return index_space_;
  }

  auto& get_index_space() const{
    return index_space_;
  }

  index_space<mesh_entity_base_*> index_space_; 
  index_vector_t from_index_vec_;

}; // class connectivity_t

/*!
  Holds the connectivities from domain M1 -> M2 for all topological dimensions.
 */
template<size_t D>
class domain_connectivity{
public:

  using id_t = flecsi::utils::id_t;

  void init_(size_t from_domain, size_t to_domain){
    from_domain_ = from_domain;
    to_domain_ = to_domain;
  }

  template<size_t FD, size_t TD>
  connectivity_t& get(){
    static_assert(FD <= D, "invalid from dimension");
    static_assert(TD <= D, "invalid to dimension");
    return conns_[FD][TD];
  }

  template<size_t FD, size_t TD>
  const connectivity_t& get() const{
    static_assert(FD <= D, "invalid from dimension");
    static_assert(TD <= D, "invalid to dimension");
    return conns_[FD][TD];
  }

  template<size_t FD>
  connectivity_t& get(size_t to_dim){
    static_assert(FD <= D, "invalid from dimension");
    assert(to_dim <= D && "invalid to dimension");
    return conns_[FD][to_dim];
  }

  template<size_t FD>
  const connectivity_t& get(size_t to_dim) const{
    static_assert(FD <= D, "invalid from dimension");
    assert(to_dim <= D && "invalid to dimension");
    return conns_[FD][to_dim];
  }

  connectivity_t& get(size_t from_dim, size_t to_dim){
    assert(from_dim <= D && "invalid from dimension");
    assert(to_dim <= D && "invalid to dimension");
    return conns_[from_dim][to_dim];
  }

  const connectivity_t& get(size_t from_dim, size_t to_dim) const{
    assert(from_dim <= D && "invalid from dimension");
    assert(to_dim <= D && "invalid to dimension");
    return conns_[from_dim][to_dim];
  }

  template<size_t FD, size_t ND>
  id_t* get_entities(mesh_entity_t<FD, ND>* from_ent, size_t to_dim){
    return get<FD>(to_dim).get_entities(from_ent->id(from_domain_));
  }

  template<size_t FD, size_t ND>
  id_t* get_entities(mesh_entity_t<FD, ND>* from_ent,
                     size_t to_dim,
                     size_t & count){
    return get<FD>(to_dim).get_entities(from_ent->id(from_domain_), count);
  }

  id_t* get_entities(id_t from_id, size_t to_dim){
    return get(from_id.dimension(), to_dim).get_entities(from_id.entity());
  }

  id_t* get_entities(id_t from_id, size_t to_dim, size_t & count){
    return get(from_id.dimension(), to_dim).get_entities(from_id.entity(), count);
  }

  template<size_t FD, size_t ND>
  auto get_entity_vec(mesh_entity_t<FD, ND>* from_ent, size_t to_dim) const
  {
    auto & conn = get<FD>(to_dim);
    return conn.get_entity_vec( from_ent->id(from_domain_) );
  }

  auto get_entity_vec(id_t from_id, size_t to_dim) const
  {
    auto & conn = get(from_id.dimension(), to_dim);
    return conn.get_entity_vec( from_id.entity() );
  }

  std::ostream & dump( std::ostream & stream )
  {
    for(size_t i = 0; i < conns_.size(); ++i){
      auto & ci = conns_[i];
      for(size_t j = 0; j < ci.size(); ++j){
        auto & cj = ci[j];
        stream << "------------- " << i << " -> " << j << std::endl;
        cj.dump( stream );
      }
    }
    return stream;
  }

  void dump(){
    dump( std::cout );
  }

private:
  using conn_array_t =
    std::array<std::array<connectivity_t, D + 1>, D + 1>;

  conn_array_t conns_;
  size_t from_domain_;
  size_t to_domain_;
};

template <size_t D, size_t NM>
struct mesh_storage_t {

  /*!
    Defines a type for storing entity instances as an array of
    \ref entity_vector_t, which is a std::vector of \ref mesh_entity_base_t.
   */

  using index_spaces_t = 
    std::array<index_space<mesh_entity_base_*, true, true, true>, D + 1>;

  // array of array of domain_connectivity
  std::array<std::array<domain_connectivity<D>, NM>, NM> topology;

  std::array<index_spaces_t, NM> index_spaces;

}; // struct mesh_storage_t

/*----------------------------------------------------------------------------*
 * class mesh_topology_base_t
 *----------------------------------------------------------------------------*/

/*!
  \class mesh_topology_base_t mesh_topology.h
  \brief contains methods and data about the mesh topology that do not depend
    on type parameterization, e.g: entity types, domains, etc.
 */

class mesh_topology_base_t : public data::data_client_t
{
public:

  using id_t = utils::id_t;

  // Default constructor
  mesh_topology_base_t() = default;

  // Don't allow the mesh to be copied or copy constructed
  mesh_topology_base_t(const mesh_topology_base_t &) = delete;
  mesh_topology_base_t & operator=(const mesh_topology_base_t &) = delete;

  /// Allow move operations
  mesh_topology_base_t(mesh_topology_base_t &&) = default;

  //! override default move assignement
  mesh_topology_base_t & operator=(mesh_topology_base_t && o)
  {
    // call base_t move operator
    data::data_client_t::operator=(std::move(o));
    // return a reference to the object
    return *this;
  };


  /*!
    Return the number of entities in for a specific domain and topology dim.
   */
  virtual size_t num_entities(size_t dim, size_t domain) const = 0;

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

  /*!
    This method should be called to construct and entity rather than
    calling the constructor directly. This way, the ability to have
    extra initialization behavior is reserved.
  */
  template <class T, class... S>
  T * make(S &&... args)
  {
    T * entity = new T(std::forward<S>(args)...);
    return entity;
  } // make

  virtual void append_to_index_space_(size_t domain,
    size_t dimension,
    std::vector<mesh_entity_base_*>& ents,
    std::vector<id_t>& ids) = 0;

}; // mesh_topology_base_t

template <class MT, size_t D, size_t M>
using entity_type_ = typename find_entity_<MT, D, M>::type;

template<class MT, size_t NM, size_t M, size_t D>
void unserialize_dimension_(mesh_topology_base_t& mesh,
                            char* buf,
                            uint64_t& pos){
  uint64_t num_entities;
  std::memcpy(&num_entities, buf + pos, sizeof(num_entities));
  pos += sizeof(num_entities);
  
  using id_t = utils::id_t;

  std::vector<mesh_entity_base_*> ents;
  std::vector<id_t> ids;
  ents.reserve(num_entities);
  ids.reserve(num_entities);

  // TODO - fix
  size_t partition_id = 0;

  for(size_t local_id = 0; local_id < num_entities; ++local_id){
    id_t global_id = id_t::make<D, M>(local_id, partition_id);

    auto ent = new entity_type_<MT, D, M>();
    ent->template set_global_id<M>(global_id);
    ents.push_back(ent);
    ids.push_back(global_id);
  }

  mesh.append_to_index_space_(M, D, ents, ids);
}

template<class MT, size_t NM, size_t ND, size_t M, size_t D>
struct unserialize_dimensions_{

  static void unserialize(mesh_topology_base_t& mesh,
                          char* buf,
                          uint64_t& pos){
    unserialize_dimension_<MT, NM, M, D>(mesh, buf, pos);
    unserialize_dimensions_<MT, NM, ND, M, D + 1>::unserialize(mesh, buf, pos);
  }

};

template<class MT, size_t NM, size_t ND, size_t M>
struct unserialize_dimensions_<MT, NM, ND, M, ND>{

  static void unserialize(mesh_topology_base_t& mesh,
                          char* buf,
                          uint64_t& pos){
    unserialize_dimension_<MT, NM, M, ND>(mesh, buf, pos);
  }

};

template<class MT, size_t NM, size_t ND, size_t M>
struct unserialize_domains_{

  static void unserialize(mesh_topology_base_t& mesh,
                          char* buf,
                          uint64_t& pos){
    unserialize_dimensions_<MT, NM, ND, M, 0>::unserialize(mesh, buf, pos);
    unserialize_domains_<MT, NM, ND, M + 1>::unserialize(mesh, buf, pos);
  }

};

template<class MT, size_t NM, size_t ND>
struct unserialize_domains_<MT, NM, ND, NM>{

  static void unserialize(mesh_topology_base_t& mesh,
                          char* buf,
                          uint64_t& pos){
    return;
  }

};

} // namespace topology
} // namespace flecsi

#endif // flecsi_mesh_types_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
