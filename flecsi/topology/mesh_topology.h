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

#ifndef flecsi_topology_mesh_topology_h
#define flecsi_topology_mesh_topology_h

/*!
  \file mesh_topology.h
  \authors nickm@lanl.gov, bergen@lanl.gov
  \date Initial file creation: Sep 23, 2015
 */

/*

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

*/

#include <algorithm>
#include <iostream>
#include <array>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <functional>
#include <map>
#include <cstring>
#include <type_traits>

#include "flecsi/utils/common.h"
#include "flecsi/utils/set_intersection.h"
#include "flecsi/utils/static_verify.h"
#include "flecsi/topology/mesh_types.h"

namespace flecsi {
namespace topology {
namespace verify_mesh {

///
// \tparam N
///
template<
  size_t N
>
class mesh_policy
{
public:

  template<
    size_t M,
    size_t D
  >
  static
  mesh_entity_base_t<N> *
  create_entity(
    mesh_topology_base_t * mesh,
    size_t num_vertices
 )
  {
    return nullptr;
  } // create_entity

}; // class mesh_policy

///
//
///
template<
  size_t N
>
class mesh_entity
{
public:

  mesh_entity() {}

  mesh_entity(mesh_topology_base_t &) {}

  std::vector<size_t>
  create_entities(
    flecsi::utils::id_t cell_id,
    size_t dim,
    domain_connectivity<N> & c,
    flecsi::utils::id_t * e
  )
  {
    return std::vector<size_t>();
  } // create_entities

  index_vector_t
  create_bound_entities(
    size_t from_domain,
    size_t to_domain,
    size_t create_dim,
    flecsi::utils::id_t cell_id,
    domain_connectivity<N>& primal_conn,
    domain_connectivity<N>& domain_conn, 
    flecsi::utils::id_t *c
  )
  {
    return index_vector_t();
  } // create_bound_entities

}; // class mesh_entity

FLECSI_MEMBER_CHECKER(num_dimensions);
FLECSI_MEMBER_CHECKER(num_domains);
FLECSI_MEMBER_CHECKER(entity_types);
FLECSI_MEMBER_CHECKER(connectivities);
FLECSI_MEMBER_CHECKER(bindings);
FLECSI_MEMBER_CHECKER(create_entity);

} // namespace verify_mesh

/*----------------------------------------------------------------------------*
 * class mesh_topology_t
 *----------------------------------------------------------------------------*/

/*!
  \class mesh_topology_t mesh_topology.h
  \brief mesh_topology_t is parameterized on a class (MT) which gives
    information about its entity types, connectivities and more. the mesh
    topology is responsibly for computing connectivity info between entities
    of different topological dimension, e.g: vertex -> cell,
    cell -> edge, etc. and provides methods for traversing these adjancies.
    It also holds vectors containing the entity instances.
 */
template<
  class MT
>
class mesh_topology_t : public mesh_topology_base_t
{
  // static verification of mesh policy

  static_assert(verify_mesh::has_member_num_dimensions<MT>::value,
                "mesh policy missing num_dimensions size_t");
  
  static_assert(std::is_convertible<decltype(MT::num_dimensions),
    size_t>::value, "mesh policy num_dimensions must be size_t");



  static_assert(verify_mesh::has_member_num_domains<MT>::value,
                "mesh policy missing num_domains size_t");
  
  static_assert(std::is_convertible<decltype(MT::num_domains),
    size_t>::value, "mesh policy num_domains must be size_t");



  static_assert(verify_mesh::has_member_entity_types<MT>::value,
                "mesh policy missing entity_types tuple");
  
  static_assert(utils::is_tuple<typename MT::entity_types>::value,
                "mesh policy entity_types is not a tuple");



  static_assert(verify_mesh::has_member_connectivities<MT>::value,
                "mesh policy missing connectivities tuple");

  static_assert(utils::is_tuple<typename MT::connectivities>::value,
                "mesh policy connectivities is not a tuple");


  
  static_assert(verify_mesh::has_member_bindings<MT>::value,
                "mesh policy missing bindings tuple");

  static_assert(utils::is_tuple<typename MT::bindings>::value,
                "mesh policy bindings is not a tuple");



  static_assert(verify_mesh::has_member_create_entity<MT>::value,
                "mesh policy missing create_entity()");

public:

  using id_t = utils::id_t;
  
  // used to find the entity type of topological dimension D and domain M
  template<size_t D, size_t M = 0>
  using entity_type = typename find_entity_<MT, D, M>::type;

  // Don't allow the mesh to be copied or copy constructed

  mesh_topology_t(const mesh_topology_t &) = delete;

  mesh_topology_t & operator=(const mesh_topology_t &) = delete;

  // Allow move operations
  mesh_topology_t(mesh_topology_t && o) = default;


  //! override default move assignement
  mesh_topology_t & operator=(mesh_topology_t && o) = default;

  //! Constructor
  mesh_topology_t()
  {
    for (size_t from_domain = 0; from_domain < MT::num_domains; ++from_domain) {
      for (size_t to_domain = 0; to_domain < MT::num_domains; ++to_domain) {
        ms_.topology[from_domain][to_domain].init_(from_domain, to_domain);
      }
    }

    // initialize all lower connectivities because the user might 
    // specify different combinations of connections
    for (size_t i = 1; i < MT::num_dimensions+1; ++i)
      for (size_t j = 0; j < i; ++j)
        get_connectivity_(0, i, j).init();


    for (size_t to_domain = 0; to_domain < MT::num_domains; ++to_domain) {
      for (size_t to_dim = 0; to_dim <= MT::num_dimensions; ++to_dim) {
        auto& master = ms_.index_spaces[to_domain][to_dim];

        for (size_t from_domain = 0; from_domain < MT::num_domains;
             ++from_domain) {
          for (size_t from_dim = 0; from_dim <= MT::num_dimensions; 
               ++from_dim) {
            get_connectivity_(from_domain, to_domain, from_dim, to_dim).
              get_index_space().set_master(master);            
          }
        }
      }
    }

  } // mesh_topology_t()

  // The mesh retains ownership of the entities and deletes them
  // upon mesh destruction
  virtual
  ~mesh_topology_t()
  {
    for (size_t m = 0; m < MT::num_domains; ++m) {
      for (size_t d = 0; d <= MT::num_dimensions; ++d) {
        auto & is = ms_.index_spaces[m][d];
        for (auto ent : is) {
          delete ent;
        }
      }
    }
  }

  // Add and entity to a mesh domain and assign its id per domain
  template<
    size_t D,
    size_t M = 0
  >
  void
  add_entity(
    mesh_entity_base_t<MT::num_domains> * ent,
    size_t partition_id=0
 )
  {
    using etype = entity_type<D, M>;
    using dtype = domain_entity<M, etype>;

    auto & is = ms_.index_spaces[M][D].template cast<dtype>();

    id_t global_id = id_t::make<D, M>(is.size(), partition_id);

    ent->template set_global_id<M>(global_id);
    is.push_back(dtype(static_cast<etype*>(ent)));
  } // add_entity

  // A mesh is constructed by creating cells and vertices and associating
  // vertices with cells as in this method.
  template<
    size_t M,
    class C,
    typename V
  >
  void
  init_cell(
    C * cell,
    V && verts
 )
  {
    init_cell_<M>(cell, std::forward<V>(verts));
  } // init_cell

  template<
    size_t M,
    class C,
    typename V
  >
  void
  init_cell(
    C * cell,
    std::initializer_list<V *> verts
 )
  {
    init_cell_<M>(cell, verts);
  } // init_cell

  // Initialize an entities connectivity with a subset of another
  template<
    size_t M,
    size_t D1,
    size_t D2,
    class E1,
    class E2 
  >
  void
  init_entity(
    E1 * super,
    E2 && subs
 )
  {
    init_entity_<M,D1,D2>(super, std::forward<E2>(subs));
  } // init_entity

  template<
    size_t M,
    size_t D1,
    size_t D2,
    class E1,
    class E2
  >
  void
  init_entity(
    E1 * super,
    std::initializer_list<E2*> subs
 )
  {
    init_entity_<M,D1,D2>(super, subs);
  } // init_entity

  // Virtual method of num_entities_()
  size_t
  num_entities(
    size_t dim,
    size_t domain=0
 ) const override
  {
    return num_entities_(dim, domain);
  } // num_entities

  /*!
    The init method builds entities as edges/faces and computes adjacencies
    and bindings.
   */
  template<
    size_t M = 0
  >
  void init()
  {
    // Compute mesh connectivity
    using TP = typename MT::connectivities;
    compute_connectivity_<M, std::tuple_size<TP>::value, TP>::compute(*this);

    using BT = typename MT::bindings;
    compute_bindings_<M, std::tuple_size<BT>::value, BT>::compute(*this);
  } // init

  /*!
    Similar to init(), but only compute bindings. This method should be called
    when a domain is sparse, i.e: missing certain entity types such as cells
    and it is not possible to compute connectivities.
   */
  template<
    size_t M = 0
  >
  void init_bindings()
  {
    using BT = typename MT::bindings;
    compute_bindings_<M, std::tuple_size<BT>::value, BT>::compute(*this);
  } // init

  /*!
   Return the number of entities contained in specified topological dimension
   and domain.
   */
  template<
    size_t D,
    size_t M = 0
    >
  decltype(auto)
  num_entities() const
  {
    return ms_.index_spaces[M][D].size();
  } // num_entities

  /*!
   Get the connectivity of the specified from/to domain and from/to topological
   dimensions.
   */
  const connectivity_t &
  get_connectivity(
    size_t from_domain,
    size_t to_domain,
    size_t from_dim,
    size_t to_dim) const override
  {
    return get_connectivity_(from_domain, to_domain, from_dim, to_dim);
  } // get_connectivity

  /*!
   Get the connectivity of the specified from/to domain and from/to topological
   dimensions.
   */
  connectivity_t &
  get_connectivity(
    size_t from_domain,
    size_t to_domain,
    size_t from_dim,
    size_t to_dim) override
  {
    return get_connectivity_(from_domain, to_domain, from_dim, to_dim);
  } // get_connectivity

  /*!
   Get the connectivity of the specified domain and from/to topological
   dimensions.
   */
  const connectivity_t &
  get_connectivity(
    size_t domain,
    size_t from_dim,
    size_t to_dim) const override
  {
    return get_connectivity_(domain, domain, from_dim, to_dim);
  } // get_connectivity

  /*!
   Get the connectivity of the specified domain and from/to topological
   dimensions.
   */
  connectivity_t &
  get_connectivity(
    size_t domain,
    size_t from_dim,
    size_t to_dim) override
  {
    return get_connectivity_(domain, domain, from_dim, to_dim);
  } // get_connectivity

  size_t
  topological_dimension() const override
  { 
    return MT::num_dimensions; 
  }
  
  template<
    size_t M = 0
  >
  const auto &
  get_index_space_(
    size_t dim
  ) const
  {
    return ms_.index_spaces[M][dim];
  } // get_entities_

  template<
    size_t M = 0
  >
  auto &
  get_index_space_(
    size_t dim
  )
  {
    return ms_.index_spaces[M][dim];
  } // get_entities_

  /*!
    Get an entity in domain M of topological dimension D with specified id.
  */
  template<
    size_t D,
    size_t M = 0
  >
  auto
  get_entity(
    id_t global_id
  ) const
  {
    using etype = entity_type<D, M>;
    return static_cast<etype *>(ms_.index_spaces[M][D][global_id.entity()]);
  } // get_entity

  /*!
    Get an entity in domain M of topological dimension D with specified id.
  */
  template<
    size_t M = 0
  >
  auto
  get_entity(
    size_t dim,
    id_t global_id
  )
  {
    return ms_.index_spaces[M][dim][global_id.entity()];
  } // get_entity

  /*!
    Get the entities of topological dimension D connected to another entity
    by specified connectivity from domain FM and to domain TM.
  */
  template<
    size_t D,
    size_t FM,
    size_t TM = FM,
    class E
  >
  const auto
  entities(
    const E * e
  ) const
  {

    const connectivity_t & c = get_connectivity(FM, TM, E::dimension, D);
    assert(!c.empty() && "empty connectivity");
    const index_vector_t & fv = c.get_from_index_vec();

    using etype = entity_type<D, TM>;
    using dtype = domain_entity<TM, etype>;
    
    auto ents = c.get_index_space().slice<dtype>(
      fv[e->template id<FM>()], fv[e->template id<FM>() + 1]);
    return ents;
  } // entities

  /*!
    Get the entities of topological dimension D connected to another entity
    by specified connectivity from domain FM and to domain TM.
  */
  template<
    size_t D,
    size_t FM,
    size_t TM = FM,
    class E
  >
  auto
  entities(
    E * e
  )
  {
    connectivity_t & c = get_connectivity(FM, TM, E::dimension, D);
    assert(!c.empty() && "empty connectivity");
    const index_vector_t & fv = c.get_from_index_vec();

    using etype = entity_type<D, TM>;
    using dtype = domain_entity<TM, etype>;
    
    return c.get_index_space().slice<dtype>(
      fv[e->template id<FM>()], fv[e->template id<FM>() + 1]);
  } // entities

  /*!
    Get the entities of topological dimension D connected to another entity
    by specified connectivity from domain FM and to domain TM.
  */
  template<
    size_t D,
    size_t FM = 0,
    size_t TM = FM,
    class E
  >
  decltype(auto)
  entities(
    domain_entity<FM, E> & e
  ) const
  {
    return entities<D, FM, TM>(e.entity());
  } // entities

  /*!
    Get the entities of topological dimension D connected to another entity
    by specified connectivity from domain FM and to domain TM.
  */
  template<
    size_t D,
    size_t FM = 0,
    size_t TM = FM,
    class E
  >
  decltype(auto)
  entities(
    domain_entity<FM, E> & e
  )
  {
    return entities<D, FM, TM>(e.entity());
  } // entities

  /*!
    Get the top-level entities of topological dimension D of the specified
    domain M. e.g: cells of the mesh.
  */
  template<
    size_t D,
    size_t M = 0
  >
  auto
  entities() const
  {
    using etype = entity_type<D, M>;
    using dtype = domain_entity<M, etype>;
    return ms_.index_spaces[M][D].template slice<dtype>();
  } // entities

  /*!
    Get the top-level entity id's of topological dimension D of the specified
    domain M. e.g: cells of the mesh.
  */
  template<
    size_t D,
    size_t M = 0
  >
  auto
  entity_ids() const
  {
    return ms_.index_spaces[M][D].ids();
  } // entity_ids

  /*!
    Get the entity id's of topological dimension D connected to another entity
    by specified connectivity from domain FM and to domain TM.
  */
  template<
    size_t D,
    size_t FM = 0,
    size_t TM = FM,
    class E
  >
  decltype(auto)
  entity_ids(
    domain_entity<FM, E> & e
  )
  {
    return entity_ids<D, FM, TM>(e.entity());
  } // entities

  /*!
    Get the entity id's of topological dimension D connected to another entity
    by specified connectivity from domain FM and to domain TM.
  */
  template<
    size_t D,
    size_t FM = 0,
    size_t TM = FM,
    class E
  >
  auto
  entity_ids(
    const E * e
  ) const
  {
    const connectivity_t & c = get_connectivity(FM, TM, E::dimension, D);
    assert(!c.empty() && "empty connectivity");
    const index_vector_t & fv = c.get_from_index_vec();
    return c.get_index_space().ids(
      fv[e->template id<FM>()], fv[e->template id<FM>() + 1]);
  } // entities

  /*!
    Get the entities of topological dimension D connected to another entity
    by specified connectivity from domain FM and to domain TM.
  */
  template<
    size_t D,
    size_t FM,
    size_t TM = FM,
    class E
  >
  void
  reverse_entities(
    E * e
  )
  {
    auto & c = get_connectivity(FM, TM, E::dimension, D);
    assert(!c.empty() && "empty connectivity");
    c.reverse_entities(e->template id<FM>());
  } // entities

  /*!
    Get the entities of topological dimension D connected to another entity
    by specified connectivity from domain FM and to domain TM.
  */
  template<
    size_t D,
    size_t FM = 0,
    size_t TM = FM,
    class E
  >
  void
  reverse_entities(
    domain_entity<FM, E> & e
  )
  {
    return reverse_entities<D, FM, TM>(e.entity());
  } // entities


  /*!
    Get the entities of topological dimension D connected to another entity
    by specified connectivity from domain FM and to domain TM.
  */
  template<
    size_t D,
    size_t FM,
    size_t TM = FM,
    class E, 
    class U
  >
  void
  reorder_entities(
    E * e,
    U && order
  )
  {
    auto & c = get_connectivity(FM, TM, E::dimension, D);
    assert(!c.empty() && "empty connectivity");
    c.reorder_entities(e->template id<FM>(), std::forward<U>(order));
  } // entities

  /*!
    Get the entities of topological dimension D connected to another entity
    by specified connectivity from domain FM and to domain TM.
  */
  template<
    size_t D,
    size_t FM = 0,
    size_t TM = FM,
    class E, 
    class U
  >
  void
  reverse_entities(
    domain_entity<FM, E> & e,
    U && order
  )
  {
    return reorder_entities<D, FM, TM>(e.entity(), std::forward<U>(order));
  } // entities

  template<
    typename I
  >
  void
  compute_graph_partition(
    size_t domain,
    size_t dim,
    const std::vector<I>& partition_sizes,
    std::vector<mesh_graph_partition<I>>& partitions
  ){

    using int_t = I;

    partitions.reserve(partition_sizes.size());

    int_t total_size = 0;
    for(auto pi : partition_sizes){
      total_size += pi;
    }

    size_t n = num_entities_(dim, domain);
    size_t pn = n / total_size;

    size_t to_dim;

    if (dim == 0) {
      // vertex -> vertex via shared edge.
      to_dim = 1;
    } else {
      // edge -> edge via shared vertex, cell -> cell via shared edge/face etc.
      to_dim = dim - 1;
    }

    const connectivity_t& c1 = get_connectivity(domain, dim, to_dim);
    assert(!c1.empty() && "empty connectivity c1");
    const index_vector_t& fv1 = c1.get_from_index_vec();

    const connectivity_t& c2 = get_connectivity(domain, to_dim, dim);
    assert(!c2.empty() && "empty connectivity c2");
    const index_vector_t& fv2 = c2.get_from_index_vec();

    mesh_graph_partition<int_t> cp;
    cp.offset.reserve(pn);

    size_t offset = 0;
    size_t pi = 0;

    std::vector<int_t> partition;
    partition.push_back(0);

    for(size_t from_id = 0; from_id < n; ++from_id){
      auto to_ids = c1.get_index_space().ids(fv1[from_id], fv1[from_id + 1]);
      cp.offset.push_back(offset);
      
      for(auto to_id : to_ids){
        auto ret_ids = 
          c2.get_index_space().ids(fv2[to_id.entity()], fv2[to_id.entity() + 1]);
        
        for(auto ret_id : ret_ids){
          if(ret_id.entity() != from_id){
            cp.index.push_back(ret_id.local_id());
            ++offset;
          }
        }
      }

      size_t m = cp.offset.size();

      if(m >= pn * partition_sizes[pi]){
        partitions.emplace_back(std::move(cp));
        partition.push_back(m + partition.back());
        offset = 0;
        ++pi;
      }
    }

    for(auto& pi : partitions){
      pi.partition = partition;
    }
  }

  /*!
    Debug method to dump the connectivity of the mesh over all domains and
    topological dimensions.
  */

  std::ostream &
  dump(
    std::ostream & stream
  )
  {
    for (size_t from_domain = 0; from_domain < MT::num_domains; ++from_domain) {
      stream << "=================== from domain: " << from_domain
                << std::endl;
      for (size_t to_domain = 0; to_domain < MT::num_domains; ++to_domain) {
        stream << "========== to domain: " << to_domain << std::endl;
        ms_.topology[from_domain][to_domain].dump(stream);
      }
    }
    return stream;
  } // dump

  void dump()
  {
    dump(std::cout);
  } // dump

  template<
    typename A
  >
  void
  save(
    A & archive
  ) const {
    size_t size;
    char* data = serialize_(size);
    archive.saveBinary(&size, sizeof(size));
    
    archive.saveBinary(data, size);
    free(data);
  } // save

  template<
    typename A>
  void
  load(
    A & archive
  ){
    size_t size;
    archive.loadBinary(&size, sizeof(size));

    char* data = (char*)malloc(size);
    archive.loadBinary(data, size);
    unserialize_(data);
    free(data);
  } // load

  char*
  serialize_(
    uint64_t& size
  ) const 
  {
    const size_t alloc_size = 1048576;
    size = alloc_size;

    char* buf = (char*)std::malloc(alloc_size);
    uint64_t pos = 0;
    
    uint32_t num_domains = MT::num_domains;
    std::memcpy(buf + pos, &num_domains, sizeof(num_domains));
    pos += sizeof(num_domains);

    uint32_t num_dimensions = MT::num_dimensions;
    std::memcpy(buf + pos, &num_dimensions, sizeof(num_dimensions));
    pos += sizeof(num_dimensions);

    for(size_t domain = 0; domain < MT::num_domains; ++domain){
      for(size_t dimension = 0; dimension <= MT::num_dimensions; ++dimension){
        uint64_t num_entities = ms_.entities[domain][dimension].size();
        std::memcpy(buf + pos, &num_entities, sizeof(num_entities));
        pos += sizeof(num_entities);
      }
    }

    for(size_t from_domain = 0; from_domain < MT::num_domains; ++from_domain){
      for(size_t to_domain = 0; to_domain < MT::num_domains; ++to_domain){

        auto& dc = ms_.topology[from_domain][to_domain];

        for(size_t from_dim = 0; from_dim <= MT::num_dimensions; ++from_dim){
          for(size_t to_dim = 0; to_dim <= MT::num_dimensions; ++to_dim){
            const connectivity_t& c = dc.get(from_dim, to_dim);

            auto& tv = c.to_id_vec();
            uint64_t num_to = tv.size();
            std::memcpy(buf + pos, &num_to, sizeof(num_to));
            pos += sizeof(num_to);

            size_t bytes = num_to * sizeof(id_vector_t::value_type);

            if(size - pos < bytes){
              size += bytes + alloc_size;
              buf = (char*)std::realloc(buf, size);
            }

            std::memcpy(buf + pos, tv.data(), bytes);
            pos += bytes;

            auto& fv = c.from_index_vec();
            uint64_t num_from = fv.size();
            std::memcpy(buf + pos, &num_from, sizeof(num_from));
            pos += sizeof(num_from);

            bytes = num_from * sizeof(index_vector_t::value_type);

            if(size - pos < bytes){
              size += bytes + alloc_size;
              buf = (char*)std::realloc(buf, size);
            }

            std::memcpy(buf + pos, fv.data(), bytes);
            pos += bytes;
          }
        }
      }
    }

    size = pos;

    return buf;
  }

  void
  unserialize_(
    char* buf
  )
  {
    uint64_t pos = 0;

    uint32_t num_domains;
    std::memcpy(&num_domains, buf + pos, sizeof(num_domains));
    pos += sizeof(num_domains);
    assert(num_domains == MT::num_domains && "domain size mismatch");

    uint32_t num_dimensions;
    std::memcpy(&num_dimensions, buf + pos, sizeof(num_dimensions));
    pos += sizeof(num_dimensions);
    assert(num_dimensions == MT::num_dimensions && "dimension size mismatch");

    unserialize_domains_<MT, MT::num_domains, MT::num_dimensions, 0>::
      unserialize(*this, buf, pos);

    for(size_t from_domain = 0; from_domain < MT::num_domains; ++from_domain){
      for(size_t to_domain = 0; to_domain < MT::num_domains; ++to_domain){

        auto& dc = ms_.topology[from_domain][to_domain];

        for(size_t from_dim = 0; from_dim <= MT::num_dimensions; ++from_dim){
          for(size_t to_dim = 0; to_dim <= MT::num_dimensions; ++to_dim){
            connectivity_t& c = dc.get(from_dim, to_dim);
    
            auto& tv = c.to_id_vec();
            uint64_t num_to;
            std::memcpy(&num_to, buf + pos, sizeof(num_to));
            pos += sizeof(num_to);
            auto ta = (id_vector_t::value_type*)(buf + pos); 
            tv.resize(num_to);
            tv.assign(ta, ta + num_to);
            pos += num_to * sizeof(id_vector_t::value_type);

            auto& fv = c.from_index_vec();
            uint64_t num_from;
            std::memcpy(&num_from, buf + pos, sizeof(num_from));
            pos += sizeof(num_from);
            auto fa = (index_vector_t::value_type*)(buf + pos); 
            fv.resize(num_from);
            fv.assign(fa, fa + num_from);
            pos += num_from * sizeof(index_vector_t::value_type);            
          }
        }
      }
    }
  }

  void
  append_to_index_space_(
    size_t domain,
    size_t dim,
    std::vector<mesh_entity_base_*>& ents,
    std::vector<id_t>& ids) override
  {
    auto& is =  ms_.index_spaces[domain][dim];
    is.append_(ents, ids);
  }

private:

  mesh_storage_t<MT::num_dimensions, MT::num_domains> ms_;

  template<size_t DM, size_t I, class TS>
  friend class compute_connectivity_;

  template<size_t DM, size_t I, class TS>
  friend class compute_bindings_;

  template<
    size_t M,
    typename V>
  void
  init_cell_(
    entity_type<MT::num_dimensions, M> * cell,
    V && verts
  )
  {
    auto & c = get_connectivity_(M, MT::num_dimensions, 0);

    assert(cell->template id<M>() == c.from_size() && "id mismatch");

    for (entity_type<0, M> * v : std::forward<V>(verts)) {
      c.push(v->template global_id<M>());
    } // for

    c.end_from();
  } // init_cell

  template<
    size_t M,
    size_t D1,
    size_t D2,
    class E2
  >
  void
  init_entity_(
    entity_type<D1, M> * super,
    E2 && subs
  )
  {
    auto & c = get_connectivity_(M, D1, D2);

    assert(super->template id<M>() == c.from_size() && "id mismatch");

    for (auto e : subs) {
      c.push(e->template global_id<M>());
    } // for

    c.end_from();
  } // init_entity

  // Get the number of entities in a given domain and topological dimension
  size_t
  num_entities_(
    size_t dim,
    size_t domain=0
  ) const
  {
    return ms_.index_spaces[domain][dim].size();
  } // num_entities_

  /*!
    Build connectivity informaiton and add entities to the mesh for the
    given dimension.
    
    \remark this is the general one that gets instantiated even though
            it may never get called
  */
  template<
    size_t Domain,
    size_t DimensionToBuild,
    size_t UsingDimension>
    typename std::enable_if< (UsingDimension <= 1 ||
      UsingDimension > MT::num_dimensions) >::type
  build_connectivity()
  {
    assert(false && "shouldn't be in here");
  }

  /*!
    Build connectivity informaiton and add entities to the mesh for the
    given dimension.
    
    \remark This one is enable_if'd so it never gets instantiated in certain cases,
            otherwise we would need create_entities in wedges and vertices
   */
  template<
    size_t Domain,
    size_t DimensionToBuild,
    size_t UsingDimension>
    typename std::enable_if< (UsingDimension > 1 && 
      UsingDimension <= MT::num_dimensions) >::type
  build_connectivity()
  {
    // std::cerr << "build: " << DimensionToBuild << " using " << UsingDimension << std::endl;

    // Sanity check
    static_assert(
      DimensionToBuild <= MT::num_dimensions,
      "DimensionToBuild must be <= total number of dimensions"      
   );
    static_assert(
      UsingDimension <= MT::num_dimensions, 
      "UsingDimension must be <= total number of dimensions"
   );
    static_assert(
      Domain < MT::num_domains, 
      "Domain must be < total number of domains"
   );

    // Reference to storage from cells to the entity (to be created here).
    connectivity_t & cell_to_entity =
      get_connectivity_(Domain, UsingDimension, DimensionToBuild);

    // Storage for entity-to-vertex connectivity information.
    connection_vector_t entity_vertex_conn;

    // Helper variables
    size_t entity_id = 0;
    size_t max_cell_entity_conns = 1;

    domain_connectivity<MT::num_dimensions> & dc = ms_.topology[Domain][Domain];

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

    auto& is = ms_.index_spaces[Domain][DimensionToBuild].template cast<
      domain_entity<Domain, entity_type>>();
    auto& cis = ms_.index_spaces[Domain][UsingDimension];

    for (size_t c = 0; c < _num_cells; ++c) {
      // Get the cell object

      auto cell = static_cast<cell_type*>(cis[c]);

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
      auto sv = cell->template create_entities(cell_id,
        DimensionToBuild, dc, entity_vertices.data());

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

          max_cell_entity_conns = 
            std::max(max_cell_entity_conns, conns.size());

          id_t global_id = id_t::make<Domain>(DimensionToBuild, entity_id);
          
          auto ent =
            MT::template create_entity<Domain, DimensionToBuild>(this, m);
          ent->template set_global_id<Domain>(global_id);
          
          is.push_back(static_cast<entity_type*>(ent));

          // A new entity was added, so we advance the id counter.
          ++entity_id;
        } // if
      } // for
    } // for

    // Set the connectivity information from the created entities to
    // the vertices.
    connectivity_t & entity_to_vertex = dc.template get<DimensionToBuild>(0);
    entity_to_vertex.init(entity_vertex_conn);
    cell_to_entity.init(cell_entity_conn);
  } // build_connectivity

  /*!
     used internally to compute connectivity information for
     topological dimension
       FD -> TD where FD < TD
   */
  template<
    size_t FM,
    size_t TM,
    size_t FD,
    size_t TD
  >
  void
  transpose()
  {
    //std::cerr << "transpose: " << FD << " -> " << TD << std::endl;

    // The connectivity we will be populating
    auto & out_conn = get_connectivity_(FM, TM, FD, TD);
    if (!out_conn.empty()) {
      return;
    } // if

    index_vector_t pos(num_entities_(FD, FM), 0);

    for (auto to_entity : entities<TD, TM>()) {
      for (id_t from_id : entity_ids<FD, TM, FM>(to_entity)) {
        ++pos[from_id.entity()];
      }
    }

    out_conn.resize(pos);

    std::fill(pos.begin(), pos.end(), 0);

    for (auto to_entity : entities<TD, TM>()) {
      for (id_t from_id : entity_ids<FD, TM, FM>(to_entity)) {
        out_conn.set(from_id.entity(), to_entity->template global_id<TM>(),
            pos[from_id.entity()]++);
      }
    }

  } // transpose

  /*!
     Used internally to compute connectivity information for
     topological dimension
       FD -> TD using FD -> D' and D' -> TD
   */
  template<
    size_t FM,
    size_t TM,
    size_t FD,
    size_t TD,
    size_t D
  >
  void
  intersect()
  {
    // std::cerr << "intersect: " << FD << " -> " << TD << std::endl;

    // The connectivity we will be populating
    connectivity_t & out_conn = get_connectivity_(FM, TM, FD, TD);
    if (!out_conn.empty()) {
      return;
    } // if

    // the number of each entity type
    auto num_from_ent = num_entities_(FD, FM);
    auto num_to_ent = num_entities_(TD, FM);

    // Temporary storage for connection id's
    connection_vector_t conns(num_from_ent);

    // Keep track of which to id's we have visited
    using visited_vec = std::vector<bool>;
    visited_vec visited(num_to_ent);

    size_t max_size = 1;

    // Read connectivities
    connectivity_t & c = get_connectivity_(FM, FD, D);
    assert(!c.empty());

    connectivity_t & c2 = get_connectivity_(TM, TD, D);
    assert(!c2.empty());

    // Iterate through entities in from topological dimension
    for (auto from_entity : entities<FD, FM>()) {
      id_t from_id = from_entity->template global_id<FM>();
      id_vector_t & ents = conns[from_id.entity()];
      ents.reserve(max_size);

      size_t count;
      id_t * ep = c.get_entities(from_id.entity(), count);

      // Create a copy of to vertices so they can be sorted
      id_vector_t from_verts(ep, ep+count);
      // sort so we have a unique key for from vertices
      std::sort(from_verts.begin(), from_verts.end());

      // initially set all to id's to unvisited
      for (auto from_ent2 : entities<D, FM>(from_entity)) {
        for (id_t to_id : entity_ids<TD, TM>(from_ent2)) {
          visited[to_id.entity()] = false;
        }
      }

      // Loop through each from entity again
      for (auto from_ent2 : entities<D, FM>(from_entity)) {
        for (id_t to_id : entity_ids<TD, TM>(from_ent2)) {

          // If we have already visited, skip
          if (visited[to_id.entity()]) {
            continue;
          } // if

          visited[to_id.entity()] = true;

          // If the topological dimensions are the same, always add to id
          if (FD == TD) {
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
            if (D < TD) {
              if (std::includes(from_verts.begin(), from_verts.end(),
                                  to_verts.begin(), to_verts.end()))
                ents.emplace_back(to_id); 
            } 
            // If we are going through a higher level, then set
            // intersection is sufficient. i.e. one set does not need to 
            // be a subset of the other
            else {
              if (utils::intersects(from_verts.begin(), from_verts.end(),
                                      to_verts.begin(), to_verts.end()))
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

  /*!
     Used to compute connectivity information for topological dimension
       D1 -> D2
   */
  template<
    size_t M,
    size_t FD,
    size_t TD
  >
  void
  compute_connectivity()
  {
    // std::cerr << "compute: " << FD << " -> " << TD << std::endl;

    // Get the output connectivity
    connectivity_t & out_conn = get_connectivity_(M, FD, TD);

    // Check if we have already computed it
    if (!out_conn.empty()) {
      return;
    } // if

    // if we don't have cell -> vertex connectivities, then
    // try building cell -> vertex connectivity through the
    // faces (3d) or edges(2d)
    static_assert(MT::num_dimensions <= 3, 
                   "this needs to be re-thought for higher dimensions");

    if (get_connectivity_(M, MT::num_dimensions, 0).empty()) {
      assert(!get_connectivity_(M, MT::num_dimensions-1, 0).empty() && 
              " need at least edges(2d)/faces(3) -> vertex connectivity");
      // assume we have cell -> faces, so invert it to get faces -> cells
      transpose<M, M, MT::num_dimensions-1, MT::num_dimensions>();
      // invert faces -> vertices to get vertices -> faces
      transpose<M, M, 0, MT::num_dimensions-1>();
      // build cells -> vertices via intersections with faces
      intersect<M, M, MT::num_dimensions, 0, MT::num_dimensions-1>();
    }

    // Check if we need to build entities, e.g: edges or faces
    if (num_entities_(FD, M) == 0) {
      if (get_connectivity_(M, FD+1, 0).empty())
        build_connectivity<M, FD, MT::num_dimensions>();
      else
        build_connectivity<M, FD, FD+1>();
    } // if

    if (num_entities_(TD, M) == 0) {
      if (get_connectivity_(M, TD+1, 0).empty())
        build_connectivity<M, TD, MT::num_dimensions>();
      else
        build_connectivity<M, TD, TD+1>();
    } // if

    if (num_entities_(FD, M) == 0 && num_entities_(TD, M) == 0) {
      return;
    } // if

    // Depending on the corresponding topological dimensions, call transpose
    // or intersect as need
     if (FD < TD) {
      compute_connectivity<M, TD, FD>();
      transpose<M, M, FD, TD>();
    } else {
       if (FD == 0 && TD == 0) {
         // compute vertex to vertex connectivities through shared cells.
         compute_connectivity<M, FD, MT::num_dimensions>();
         compute_connectivity<M, MT::num_dimensions, TD>();
         intersect<M, M, FD, TD, MT::num_dimensions>();
       } else {
         // computer connectivities through shared vertices.
         compute_connectivity<M, FD, 0>();
         compute_connectivity<M, 0, TD>();
         intersect<M, M, FD, TD, 0>();
       }
    } // if
  } // compute_connectivity

  /*!
    if the to-dimension is larger than the from-dimension, build the bindings 
    using the create_bound_entities functionality
  */
  template<
    size_t FM,
    size_t TM,
    size_t FD,
    size_t TD,
    typename std::enable_if< (FM < TM) >::type* = nullptr
  >
  void
  _compute_bindings()
  {

    // if the connectivity for a transpose exists, do it
    if(!get_connectivity_(TM, FM, TD, FD).empty())
      transpose<FM, TM, FD, TD>();

    // otherwise try building the connectivity directly
    else if (num_entities_(TD, TM) == 0)
      build_bindings<FM, TM, TD>();

  } // compute_bindings

  /*!
    if the from-dimension is larger than the to-dimension, we want
    to transpose.  So make sure the opposite connectivity exists first
  */
  template<
    size_t FM,
    size_t TM,
    size_t FD,
    size_t TD,
    typename = typename std::enable_if< (FM > TM) >::type
  >
  void
  _compute_bindings()
  {

    // build the opposite connectivity first
    _compute_bindings< TM, FM, TD, FD>();

    // now apply a transpose to get the requested connectivity
    transpose<FM, TM, FD, TD>();

  } // compute_bindings

  /*!
    in the odd case the from-dimension matches the to-dimension, try and 
    build the connectivity between the two
  */
  template<
    size_t FM,
    size_t TM,
    size_t FD,
    size_t TD
  >
  typename std::enable_if< (FM == TM) >::type
  _compute_bindings()
  {

    // compute connectivities through shared vertices at the at the lowest
    // dimension (doesn't matter which one really)
    _compute_bindings<0, TM, 0, FD>();
    _compute_bindings<0, TM, 0, TD>();
    
    // now try and transpose it
    auto & trans_conn = get_connectivity_(TM, FM, TD, FD);
    if(!trans_conn.empty())
      transpose<FM, TM, FD, TD>();

  } // compute_bindings


  /*!
    Main driver for computing bindings
  */
  template<
    size_t FM,
    size_t TM,
    size_t FD,
    size_t TD>
  void
  compute_bindings() 
  {
    // std::cerr << "compute: , dom " << FM << " -> " << TM 
    //           <<  ", dim " << FD << " -> " << TD << std::endl;

    // check if requested connectivity is already there, nothing to do
    connectivity_t & out_conn = get_connectivity_(FM, TM, FD, TD);
    
    if (!out_conn.empty()) return;
    
    _compute_bindings< FM, TM, FD, TD >();

  }

  /*!
    Build bindings associated with a from/to domain and topological dimension.
    compute_bindings will call this on each binding found in the tuple of
    bindings specified in the mesh type/traits mesh specialization.
   */
  template<
    size_t FM,
    size_t TM,
    size_t TD
  >
  void
  build_bindings()
  {

    // std::cerr << "build bindings: dom " << FM << " -> " << TM 
    //           << " dim " << TD << std::endl;

    // Sanity check
    static_assert(TD <= MT::num_dimensions, "invalid dimension");

    // Helper variables
    size_t entity_id = 0;
    const size_t _num_cells = num_entities<MT::num_dimensions, FM>();

    // Storage for cell connectivity information
    connection_vector_t cell_conn(_num_cells);

    // Get cell definitions from domain 0
    auto & cells = ms_.index_spaces[FM][MT::num_dimensions];

    static constexpr size_t M0 = 0;

    for (size_t i = 0; i < MT::num_dimensions; ++i) {
      get_connectivity_<TM, FM, TD>(i).init();
    }
    for (size_t i = 0; i < TD; ++i) {
      get_connectivity_(TM, TM, TD, i).init();
    }

    // This buffer should be large enough to hold all entities
    // that potentially need to be created
    std::array<id_t, 4096> entity_ids;

    using to_entity_type = entity_type<TD, TM>;

    // Iterate over cells
    for (auto c : cells) {
      // Map used to ensure unique entity creation
      id_vector_map_t entity_ids_map;

      // Get a cell object.
      auto cell = static_cast<entity_type<MT::num_dimensions, M0> *>(c);
      id_t cell_id = cell->template global_id<FM>();

      domain_connectivity<MT::num_dimensions> & primal_conn =
        ms_.topology[FM][FM];
      domain_connectivity<MT::num_dimensions> & domain_conn =
        ms_.topology[FM][TM];

      // p.first:   The number of entities per cell.
      // p.second:  A std::vector of id_t containing the ids of the
      //            entities that define the bound entity.

      auto sv = cell->create_bound_entities(
        FM, TM, TD, cell_id, primal_conn, domain_conn, entity_ids.data());

      size_t n = sv.size();

      // Iterate over the newly-defined entities
      id_vector_t & conns = cell_conn[cell_id.entity()];

      conns.reserve(n);

      size_t pos = 0;

      auto& is = ms_.index_spaces[TM][TD].template cast<
        domain_entity<TM, to_entity_type>>();

      for (size_t i = 0; i < n; ++i) {
        size_t m = sv[i];

        id_t create_id = id_t::make<TD, TM>(entity_id, cell_id.partition());

        // Add this id to the cell entity connections
        conns.push_back(create_id);

        uint32_t dim_flags = 0;
        uint32_t dom_flags = 0;
        size_t num_vertices = 0;

        for (size_t k = 0; k < m; ++k) {
          id_t global_id = entity_ids[pos + k];
          auto dim = global_id.dimension();
          auto dom = global_id.domain();
          get_connectivity_(TM, dom, TD, dim).push(global_id);
          if (dom == FM) {
            dim_flags |= 1U << dim;
            num_vertices += dim == 0 ? 1 : 0;
          }
          else 
            dom_flags |= 1U << dim;
        }

        for (size_t i = 0; i < MT::num_dimensions; ++i) {
          if (dim_flags & (1U << i)) {
            get_connectivity_<TM, FM, TD>(i).end_from();
          }
        }

        for (size_t i = 0; i < TD; ++i) {
          if (dom_flags & (1U << i)) {
            get_connectivity_(TM, TM, TD, i).end_from();
          }
        }

        id_t global_id = id_t::make<TM>(TD, entity_id);
        
        auto ent = MT::template create_entity<TM, TD>(this, num_vertices);
        ent->template set_global_id<TM>(global_id);
        
        is.push_back(static_cast<to_entity_type*>(ent));

        ++entity_id;

        pos += m;
      } // for
    } // for

    // Reference to storage from cells to the entity (to be created here).
    connectivity_t & cell_out =
      get_connectivity_(FM, TM, MT::num_dimensions, TD);
    cell_out.init(cell_conn);

  } // build_bindings

  /*!
   Implementation of get_connectivity for various get_connectivity convenience
   methods.
   */
  const connectivity_t &
  get_connectivity_(
    size_t from_domain,
    size_t to_domain,
    size_t from_dim,
    size_t to_dim) const
  {
    assert(from_domain < MT::num_domains && "invalid from domain");
    assert(to_domain < MT::num_domains && "invalid to domain");
    return ms_.topology[from_domain][to_domain].get(from_dim, to_dim);
  } // get_connectivity

  /*!
   Implementation of get_connectivity for various get_connectivity convenience
   methods.
   */
  connectivity_t &
  get_connectivity_(
    size_t from_domain,
    size_t to_domain,
    size_t from_dim,
    size_t to_dim)
  {
    assert(from_domain < MT::num_domains && "invalid from domain");
    assert(to_domain < MT::num_domains && "invalid to domain");
    return ms_.topology[from_domain][to_domain].get(from_dim, to_dim);
  } // get_connectivity

  /*!
   Implementation of get_connectivity for various get_connectivity convenience
   methods.
   */
  template<
    size_t FM,
    size_t TM,
    size_t FD
  >
  connectivity_t &
  get_connectivity_(
    size_t to_dim
  )
  {
    return ms_.topology[FM][TM].template get<FD>(to_dim);
  } // get_connectivity

  /*!
   Implementation of get_connectivity for various get_connectivity convenience
   methods.
   */
  template<
    size_t FM,
    size_t TM,
    size_t FD,
    size_t TD
  >
  connectivity_t &
  get_connectivity_()
  {
    return ms_.topology[FM][TM].template get<FD, TD>();
  } // get_connectivity

  /*!
   Implementation of get_connectivity for various get_connectivity convenience
   methods.
   */
  const connectivity_t &
  get_connectivity_(
    size_t domain,
    size_t from_dim,
    size_t to_dim) const
  {
    return get_connectivity_(domain, domain, from_dim, to_dim);
  } // get_connectivity

  connectivity_t &
  get_connectivity_(
    size_t domain,
    size_t from_dim,
    size_t to_dim)
  {
    return get_connectivity_(domain, domain, from_dim, to_dim);
  } // get_connectivity

}; // class mesh_topology_t

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_mesh_topology_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
