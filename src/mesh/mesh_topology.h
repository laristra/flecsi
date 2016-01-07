/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@ @@       @@@@@@@@ @@     @@ @@
 * /@@///// /@@      /@@///// //@@   @@ /@@
 * /@@      /@@      /@@       //@@ @@  /@@
 * /@@@@@@@ /@@      /@@@@@@@   //@@@   /@@
 * /@@////  /@@      /@@////     @@/@@  /@@
 * /@@      /@@      /@@        @@ //@@ /@@
 * /@@      /@@@@@@@@/@@@@@@@@ @@   //@@/@@
 * //       //////// //////// //     // // 
 * 
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_mesh_topology_h
#define flexi_mesh_topology_h

/*!
  \file mesh_topology.h
  \authors nickm@lanl.gov, bergen@lanl.gov
  \date Initial file creation: Sep 23, 2015
 */

#include <algorithm>
#include <iostream>
#include <array>
#include <vector>
#include <cassert>
#include <unordered_map>

#include "flexi/utils/common.h"
#include "flexi/mesh/mesh_types.h"

namespace flexi {

/*----------------------------------------------------------------------------*
 * class mesh_topology_base_t
 *----------------------------------------------------------------------------*/

/*!
  \class mesh_topology_base_t mesh_topology.h
  \brief contains methods and data about the mesh topology that do not depend
    on type parameterization, e.g: entity types, domains, etc.
 */

class mesh_topology_base_t
{
public:

  using id_vector_t = std::vector<id_t>;

  using connection_vector_t = std::vector<id_vector_t>;

  struct id_vector_hash_t {
    size_t operator()(const id_vector_t &v) const {
      size_t h = 0;
      for (id_t id : v) {
        h |= id;
      }
      return h;
    }
  }; // struct id_vector_hash_t

  using id_vector_map_t =
    std::unordered_map<id_vector_t, id_t, id_vector_hash_t>;

  using index_vector_t = std::vector<size_t>;

  /*--------------------------------------------------------------------------*
   * class connectivity_t
   *--------------------------------------------------------------------------*/

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
    void clear() {
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
    void init(const connection_vector_t & cv) {
      assert(to_id_vec_.empty() && from_index_vec_.empty());

      from_index_vec_.push_back(0);

      size_t n = cv.size();

      for (size_t i = 0; i < n; ++i) {
        const id_vector_t &iv = cv[i];

        for (id_t id : iv) {
          to_id_vec_.push_back(id);
        } // for

        from_index_vec_.push_back(to_id_vec_.size());
      } // for
    } // init

    /*!
      FIXME: Need description...

      \tparam MT
      \tparam M
      \tparam N

      \param iv
      \param ev
      \param cv
      \param dim
     */
    template <class MT, size_t M, size_t N>
    void init_create(id_vector_t & iv, entity_vector_t<N> & ev,
      const connection_vector_t & cv, size_t dim) {
      assert(to_id_vec_.empty() && from_index_vec_.empty());

      from_index_vec_.push_back(0);

      size_t n = cv.size();

      id_t maxId = 0;

      for (size_t i = 0; i < n; ++i) {
        const id_vector_t &iv = cv[i];

        for (id_t id : iv) {
          maxId = std::max(maxId, id);
          to_id_vec_.push_back(id);
        } // for

        from_index_vec_.push_back(to_id_vec_.size());
      } // for

      id_t startId = ev.size();

      ev.reserve(maxId + 1);

      for(id_t id = startId; id <= maxId; ++id){
        ev.push_back(mesh_entity_base_t<N>::template create_<MT, M>(dim, id));
        iv.push_back(id);
      } // for
    } // init_create

    /*!
      FIXME: Need description...

      \param num_conns
     */
    void resize(index_vector_t & num_conns) {
      clear();

      size_t n = num_conns.size();
      from_index_vec_.resize(n + 1);

      uint64_t size = 0;

      for(size_t i = 0; i < n; ++i) {
        from_index_vec_[i] = size;
        size += num_conns[i];
      } // for

      from_index_vec_[n] = size;

      to_id_vec_.resize(size);
      std::fill(to_id_vec_.begin(), to_id_vec_.end(), 0);
    } // resize

    void end_from() {
      from_index_vec_.push_back(to_id_vec_.size());
    } // end_from

    void push(id_t id) { to_id_vec_.push_back(id); } // push

    void dump() {
      for(size_t i = 1; i < from_index_vec_.size(); ++i){
        for(size_t j = from_index_vec_[i - 1]; j < from_index_vec_[i]; ++j){
          std::cout << to_id_vec_[j] << std::endl;
        }
        std::cout << std::endl;
      }

      std::cout << "=== idVec" << std::endl;
      for (id_t id : to_id_vec_) {
        std::cout << id << std::endl;
      } // for

      std::cout << "=== groupVec" << std::endl;
      for (id_t id : from_index_vec_) {
        std::cout << id << std::endl;
      } // for
    }   // dump

    const id_vector_t &get_from_index_vec() const { return from_index_vec_; }

    const id_vector_t& get_entities() const {
      return to_id_vec_;
    }

    id_t *get_entities(size_t index) {
      assert(index < from_index_vec_.size() - 1);
      return to_id_vec_.data() + from_index_vec_[index];
    }

    id_t *get_entities(size_t index, size_t &endIndex) {
      assert(index < from_index_vec_.size() - 1);
      uint64_t start = from_index_vec_[index];
      endIndex = from_index_vec_[index + 1] - start;
      return to_id_vec_.data() + start;
    }

    bool empty() const { return to_id_vec_.empty(); }

    void set(id_t fromId, id_t toId, size_t pos) {
      to_id_vec_[from_index_vec_[fromId] + pos] = toId;
    }

    size_t from_size() const { return from_index_vec_.size() - 1; }

    size_t to_size() const { return to_id_vec_.size(); }

    template<size_t M, size_t N>
    void set(entity_vector_t<N> &ev, connection_vector_t &conns) {
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
        const id_vector_t &conn = conns[i];
        uint64_t m = conn.size();

        for (size_t j = 0; j < m; ++j) {
          to_id_vec_.push_back(ev[conn[j]]->template id<M>());
        }
      }
    }

    id_vector_t to_id_vec_;
    id_vector_t from_index_vec_;

  }; // class connectivity_t

  /*!
   */
  virtual size_t num_entities(size_t domain, size_t dim) const = 0;

  /*!
   */
  virtual size_t topological_dimension() const = 0;

  /*!
   */
  virtual const connectivity_t & get_connectivity(size_t domain,
    size_t from_dim, size_t to_dim) const = 0;

  /*!
   */
  virtual connectivity_t &get_connectivity(size_t domain, size_t from_dim,
    size_t to_dim) = 0;

    /*!
   */
  virtual const connectivity_t & get_connectivity(size_t from_domain,
    size_t to_domain, size_t from_dim, size_t to_dim) const = 0;

  /*!
   */
  virtual connectivity_t &get_connectivity(size_t from_domain,
    size_t to_domain, size_t from_dim, size_t to_dim) = 0;

}; // mesh_topology_base_t

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
template <class MT> class mesh_topology_t : public mesh_topology_base_t
{
public:

  template<size_t M>
  using vertex_type = typename find_entity_<MT, 0, M>::type;

  template<size_t M>
  using edge_type = typename find_entity_<MT, 1, M>::type;

  template<size_t M>
  using face_type = typename find_entity_<MT, MT::dimension - 1, M>::type;

  template<size_t M>
  using cell_type = typename find_entity_<MT, MT::dimension, M>::type;

  /*--------------------------------------------------------------------------*
   * class Iterator
   *--------------------------------------------------------------------------*/

  template<size_t M>
  class index_iterator {
  public:

    index_iterator(mesh_topology_t &mesh, size_t dim)
      : mesh_(mesh), entities_(&mesh.get_id_vec_(dim)), dim_(dim), index_(0),
        endIndex_(mesh.num_entities(M, dim)), level_(0) {}

    index_iterator(index_iterator &itr, size_t dim)
        : mesh_(itr.mesh_), dim_(dim), level_(itr.level_ + 1) {

      connectivity_t &c = mesh_.get_connectivity(M, itr.dim_, dim_);
      assert(!c.empty());

      entities_ = &c.get_entities();

      const id_vector_t &fv = c.get_from_index_vec();
      if (level_ > 1) {
        size_t id = (*itr.entities_)[itr.index_];

        index_ = fv[id];
        endIndex_ = fv[id + 1];
      } else {
        index_ = fv[itr.index_];
        endIndex_ = fv[itr.index_ + 1];
      }
    }

    bool end() const { return index_ >= endIndex_; }

    index_iterator &operator++() {
      assert(index_ < endIndex_);
      ++index_;
      return *this;
    }

    size_t operator*() const { return (*entities_)[index_]; }

    id_t *get_entities(size_t dim) {
      connectivity_t &c = mesh_.get_connectivity_(M, dim_, dim);
      assert(!c.empty());
      return c.get_entities(index_);
    }

    id_t *get_entities(size_t dim, size_t &count) {
      connectivity_t &c = mesh_.get_connectivity_(M, dim_, dim);
      assert(!c.empty());
      return c.get_entities(index_, count);
    }

  protected:
    const id_vector_t &get_entities_() { return *entities_; }

  private:
    mesh_topology_t &mesh_;
    const id_vector_t *entities_;
    size_t dim_;
    size_t index_;
    size_t endIndex_;
    size_t level_;
  }; // class index_iterator

  /*--------------------------------------------------------------------------*
   * class entity_index_iterator
   *--------------------------------------------------------------------------*/

  template <size_t D, size_t M=0>
  class entity_index_iterator : public index_iterator<M> {
  public:

    entity_index_iterator(mesh_topology_t &mesh) : index_iterator<M>(mesh, D) {}

    entity_index_iterator(index_iterator<M> &itr) : index_iterator<M>(itr, D) {}

  }; // class entity_index_iterator

  using vertex_index_iterator = entity_index_iterator<0>;
  using edge_index_iterator = entity_index_iterator<1>;
  using face_index_iterator = entity_index_iterator<MT::dimension - 1>;
  using cell_index_iterator = entity_index_iterator<MT::dimension>;

  /*--------------------------------------------------------------------------*
   * class iterator
   *--------------------------------------------------------------------------*/

  template <size_t D, size_t M=0>
  class iterator
  {
  public:

    using entity_type = typename find_entity_<MT, D, M>::type;

    iterator(const iterator &itr)
      : mesh_(itr.mesh_), entities_(itr.entities_), index_(itr.index_) {}

    iterator(mesh_topology_t &mesh, const id_vector_t &entities, size_t index)
      : mesh_(mesh), entities_(&entities), index_(index) {}

    iterator &operator++() {
      ++index_;
      return *this;
    } // operator ++

    iterator &operator=(const iterator &itr) {
      index_ = itr.index_;
      entities_ = itr.entities_;
      return *this;
    } // operator =

    domain_entity<M, entity_type> operator*() {
      return mesh_.get_entity<D, M>((*entities_)[index_]); 
    } // operator *

    entity_type *operator->() { 
      return mesh_.get_entity<D, M>((*entities_)[index_]); 
    } // operator ->

    bool operator==(const iterator &itr) const {
      return index_ == itr.index_;
    } // operator ==

    bool operator!=(const iterator &itr) const {
      return index_ != itr.index_;
    } // operator !=

  private:

    mesh_topology_t &mesh_;
    const id_vector_t *entities_;
    size_t index_;

  }; // class iterator

  /*--------------------------------------------------------------------------*
   * class const_iterator
   *--------------------------------------------------------------------------*/

  template <size_t D, size_t M=0> class const_iterator {
  public:
    using entity_type = typename find_entity_<MT, D, M>::type;

    const_iterator(const const_iterator &itr)
      : mesh_(itr.mesh_), entities_(itr.entities_), index_(itr.index_) {}

    const_iterator(const mesh_topology_t &mesh, const id_vector_t &entities,
      size_t index)
      : mesh_(mesh), entities_(&entities), index_(index) {}

    const_iterator &operator++() {
      ++index_;
      return *this;
    } // operator ++

    const_iterator &operator=(const const_iterator &itr) {
      index_ = itr.index_;
      entities_ = itr.entities_;
      return *this;
    } // operator =

    domain_entity<M, entity_type> operator*() {
      return mesh_.get_entity<D, M>((*entities_)[index_]); 
    } // operator *

    const entity_type *operator->() const {
      return mesh_.get_entity<D, M>((*entities_)[index_]); 
    } // operator ->

    bool operator==(const const_iterator &itr) const { 
      return index_ == itr.index_; 
    } // operator ==

    bool operator!=(const const_iterator &itr) const { 
      return index_ != itr.index_; 
    } // operator !=

  private:

    const mesh_topology_t &mesh_;
    const id_vector_t *entities_;
    size_t index_;

  }; // class iterator

  using vertex_iterator = iterator<0>;
  using edge_iterator = iterator<1>;
  using face_iterator = iterator<MT::dimension - 1>;
  using cell_iterator = iterator<MT::dimension>;

  using const_vertex_iterator = const_iterator<0>;
  using const_edge_iterator = const_iterator<1>;
  using const_face_iterator = const_iterator<MT::dimension - 1>;
  using const_cell_iterator = const_iterator<MT::dimension>;

  /*--------------------------------------------------------------------------*
   * class entity_range_t
   *--------------------------------------------------------------------------*/

  /*!
    \class entity_range_t mesh_topology.h
    \brief used to enable range-based for iteration
   */
  template <size_t D, size_t M=0>
  class entity_range_t
  {
  public:

    using iterator_t = iterator<D, M>;
    using entity_type = typename iterator_t::entity_type;
    using domain_entity_vector_t = std::vector<domain_entity<M, entity_type>>;

    entity_range_t(mesh_topology_t &mesh, const id_vector_t &v)
      : mesh_(mesh), v_(v), begin_(0), end_(v_.size()) {}

    entity_range_t(mesh_topology_t &mesh, const id_vector_t &v, size_t begin,
      size_t end)
      : mesh_(mesh), v_(v), begin_(begin), end_(end) {}

    entity_range_t(const entity_range_t &r)
      : mesh_(r.mesh_), v_(r.v_), begin_(0), end_(v_.size()) {}

    iterator_t begin() const {
      return iterator_t(mesh_, v_, begin_);
    } // begin

    iterator_t end() const {
      return iterator_t(mesh_, v_, end_);
    } // end

    /*!
      convert this range to a vector
     */
    domain_entity_vector_t to_vec() const {
      domain_entity_vector_t ret;
      for(size_t i = begin_; i < end_; ++i){
        ret.push_back(mesh_.get_entity<D>(v_[i]));
      } // for

      return ret;
    } // to_vec

    size_t size() const {
      return end_ - begin_;
    } // size

  private:

    mesh_topology_t &mesh_;
    const id_vector_t &v_;
    size_t begin_;
    size_t end_;

  }; // class entity_range_t

  /*--------------------------------------------------------------------------*
   * class const_entity_range_t
   *--------------------------------------------------------------------------*/

  /*!
    \class const_entity_range_t mesh_topology.h
    \brief used to enable range-based for iteration on a const mesh
   */

  template <size_t D, size_t M=0>
  class const_entity_range_t
  {
  public:

    using const_iterator_t = const_iterator<D>;
    using entity_type = typename const_iterator_t::entity_type;
    using domain_entity_vector_t =
      std::vector<domain_entity<M, const entity_type>>;

    const_entity_range_t(const mesh_topology_t &mesh, const id_vector_t &v)
      : mesh_(mesh), v_(v), begin_(0), end_(v_.size()) {}

    const_entity_range_t(const mesh_topology_t &mesh, const id_vector_t &v,
      size_t begin, size_t end)
      : mesh_(mesh), v_(v), begin_(begin), end_(end) {}

    const_entity_range_t(const const_entity_range_t &r)
      : mesh_(r.mesh_), v_(r.v_), begin_(0), end_(v_.size()) {}

    const_iterator_t begin() const {
      return const_iterator_t(mesh_, v_, begin_); 
    } // begin

    const_iterator_t end() const {
      return const_iterator_t(mesh_, v_, end_);
    } // end

    /*!
      convert this range to a vector
     */

    domain_entity_vector_t to_vec() const {
      domain_entity_vector_t ret;

      for(size_t i = begin_; i < end_; ++i){
        ret.push_back(mesh_.get_entity<D>(v_[i]));
      } // for

      return ret;
    } // to_vec

    size_t size() const {
      return end_ - begin_;
    } // size

  private:

    const mesh_topology_t &mesh_;
    const id_vector_t &v_;
    size_t begin_;
    size_t end_;

  }; // class const_entity_range_t

  /*--------------------------------------------------------------------------*
   * class iterator
   *--------------------------------------------------------------------------*/

  class id_iterator
  {
  public:
    id_iterator(const id_iterator &itr)
      : entities_(itr.entities_), index_(itr.index_) {}

    id_iterator(const id_vector_t &entities, size_t index)
      : entities_(&entities), index_(index) {}

    id_iterator & operator++() {
      ++index_;
      return *this;
    } // operator ++

    id_iterator & operator=(const id_iterator &itr) {
      index_ = itr.index_;
      entities_ = itr.entities_;
      return *this;
    } // oerator =

    id_t operator*() { return (*entities_)[index_]; }

    bool operator==(const id_iterator &itr) const {
      return index_ == itr.index_;
    } // operator ==

    bool operator!=(const id_iterator &itr) const {
      return index_ != itr.index_;
    } // operator !=

  private:

    const id_vector_t *entities_;
    size_t index_;

  }; // class iterator

  /*--------------------------------------------------------------------------*
   * class id_range
   *--------------------------------------------------------------------------*/

  /*!
    \class id_range mesh_topology.h
    \brief ...
   */

  class id_range {
  public:

    id_range(const id_vector_t &v)
      : v_(v), begin_(0), end_(v_.size()) {}

    id_range(const id_vector_t &v, id_t begin, id_t end)
      : v_(v), begin_(begin), end_(end) {}

    id_range(const id_range &r)
      : v_(r.v_), begin_(0), end_(v_.size()) {}

    id_iterator begin() const { return id_iterator(v_, begin_); }

    id_iterator end() const { return id_iterator(v_, end_); }

    id_vector_t to_vec() const {
      id_vector_t ret;

      for(size_t i = begin_; i < end_; ++i){
        ret.push_back(v_[i]);
      } // for

      return ret;
    } // to_vec

    size_t size() const {
      return end_ - begin_;
    } // size

  private:

    const id_vector_t &v_;
    id_t begin_;
    id_t end_;

  }; // class id_range

  //! Constructor
  mesh_topology_t() {
    for(size_t d = 0; d < MT::num_domains; ++d){
      get_connectivity_(d, MT::dimension, 0).init();
    }
  } // mesh_topology_t()

  virtual ~mesh_topology_t(){
    for(size_t d = 0; d < MT::num_domains; ++d){
      for(auto& ev : entities_[d]){
        for(auto ent : ev){
          delete ent;
        } 
      }
    }  
  }

//
//
// This needs to be documented
//
//

  template<size_t D, size_t M>
  void add_entity(mesh_entity_base_t<MT::num_domains> * ent) {
    auto &ents = entities_[M][D];
    ent->ids_[M] = ents.size();
    ents.push_back(ent);

    auto &idVec = id_vecs_[M][D];
    idVec.push_back(idVec.size());
  } // add_entity

#if 0
  template<size_t M, class T>
  void add_vertex(T *vertex) {
    add_vertex_<M>(vertex);
  } // add_vertex

  template<size_t M>
  void add_vertex_(vertex_type<M> *vertex) {
    add_entity<0, M>(vertex);
  } // add_vertex
#endif

  template<size_t M, class T>
  void add_edge(T *edge) {
    add_edge_<M>(edge);
  } // add_vertex

  template<size_t M>
  void add_edge_(edge_type<M> *edge) {
    add_entity<1, M>(edge);
  } // add_edge, size_t M

  template<size_t M, class T>
  void add_face(T *face) {
    add_face_<M>(face);
  } // add_face

  template<size_t M>
  void add_face_(face_type<M> *face) {
    add_entity<MT::dimension - 1, M>(face);
  } // add_face

  template<size_t M, class T>
  void add_cell(T *cell) {
    add_cell_<M>(cell);
  } // add_cell

  template<size_t M>
  void add_cell_(cell_type<M> *cell) {
    add_entity<MT::dimension, M>(cell);
  } // add_cell

  template<size_t M, class C, class V>
  void init_cell(C *cell, std::initializer_list<V *> verts) {
    init_cell_<M>(cell, verts);
  } // init_cell

//
// This seems problematic, should be more general
//
  template<size_t M>
  void init_cell_(cell_type<M> *cell,
    std::initializer_list<vertex_type<M> *> verts) {
    auto &c = get_connectivity_(M, MT::dimension, 0);

    assert(cell->template id<M>() == c.from_size() && "id mismatch");

    for(vertex_type<M> *v : verts) {
      c.push(v->template id<M>());
    } // for

    c.end_from();
  } // init_cell

  template<size_t M>
  void init_edge(edge_type<M> *edge, const vertex_type<M> * vertex1,
    const vertex_type<M> * vertex2) {

    auto &c = get_connectivity_(1, 0);
    if (c.empty()) {
      c.init();
    } // if

    assert(edge->template id<M>() == c.from_size() && "id mismatch");

    c.push(vertex1->template id<M>());
    c.push(vertex2->template id<M>());

    c.end_from();
  } // init_edge

  template<size_t M>
  void init_face(face_type<M> *face,
                 std::initializer_list<vertex_type<M> *> verts) {
    assert(verts.size() == MT::num_vertices_per_entity(2) &&
        "invalid number vertices per face");

    auto &c = get_connectivity_(MT::dimension - 1, 0);
    if (c.empty()) {
      c.init();
    }

    assert(face->template id<M>() == c.from_size() && "id mismatch");

    for (vertex_type<M> *v : verts) {
      c.push(v);
    }

    c.end_from();
  } // init_face

  template<size_t M>
  void init_cell_edges(cell_type<M> *cell,
                       std::initializer_list<edge_type<M> *> edges) {
    assert(edges.size() == MT::num_entities_per_cell(1) &&
        "invalid number of edges per cell");

    auto &c = get_connectivity_(MT::dimension, 1);
    if (c.empty()) {
      c.init();
    }

    assert(cell->template id<M>() == c.from_size() && "id mismatch");

    for (edge_type<M> *edge : edges) {
      c.push(edge);
    }

    c.end_from();
  } // init_cell_edges

  template<size_t M>
  void init_cell_faces(cell_type<M> *cell,
                       std::initializer_list<face_type<M> *> faces) {

    assert(faces.size() == MT::num_entities_per_cell(MT::dimension - 1) &&
        "invalid number of face per cell");

    auto &c = get_connectivity_(MT::dimension, MT::dimension - 1);
    if (c.empty()) {
      c.init();
    }

    assert(cell->template id<M>() == c.from_size() && "id mismatch");

    for (face_type<M> *face : faces) {
      c.push(face);
    }

    c.end_from();
  } // init_cell_faces

//
//
//
//
//
//

  size_t num_entities(size_t domain, size_t dim) const override {
    return entities_[domain][dim].size();
  } // num_entities

  size_t num_entities_(size_t domain, size_t dim) const {
    return entities_[domain][dim].size();
  } // num_entities_

  /*!
    Build connectivity informaiton and add entities to the mesh for the
    given dimension.
   */
  template<size_t M>
  void build_connectivity(size_t dim) {
    // std::cerr << "build: " << dim << std::endl;

    // Sanity check
    assert(dim <= MT::dimension);

    // Reference to storage from cells to the entity (to be created here).
    connectivity_t & cell_to_entity = get_connectivity_(M, MT::dimension, dim);

    // Storage for entity-to-vertex connectivity information.
    connection_vector_t entity_vertex_conn;

    // Helper variables
    size_t entity_id = 0;
    size_t max_cell_entity_conns = 1;

    // Get connectivity for cells to vertices.
    connectivity_t & cell_to_vertex = get_connectivity_(M, MT::dimension, 0);
    assert(!cell_to_vertex.empty());

    const size_t _num_cells = num_entities<MT::dimension,M>();

    // Storage for cell-to-entity connectivity information.
    connection_vector_t cell_entity_conn(_num_cells);

    // This map is primarily used to make sure that entities are
    // created multiple times, i.e., that they are unique.  The
    // emplace method of the map is used to only define a new entity
    // if it does not already exist in the map.
    id_vector_map_t entity_vertices_map;

    // This will be filled with the vertices that define each cell
    // and passed to the 'create_entities' method as input information.
    id_vector_t entity_vertices;

    for (size_t c = 0; c < _num_cells; ++c) {

      // Get the cell object
      auto cell = static_cast<cell_type<M>*>(entities_[M][MT::dimension][c]);

      // Get storage reference.
      id_vector_t & conns = cell_entity_conn[c];

      // Try to optimize storage.
      conns.reserve(max_cell_entity_conns);

      // Get the vertices that define the current cell
      size_t endIndex;
      id_t * vertices = cell_to_vertex.get_entities(c, endIndex);

      // This call allows the users specialization to create
      // whatever entities are needed to complete the mesh.
      //
      // p.first:   The number of entities per cell.
      // p.second:  A std::vector of id_t containing the ids of the
      //            vertices that define the entity.
      auto p = cell->create_entities(dim, entity_vertices, vertices, endIndex);

      // iterate over the newly-defined entities
      for (size_t i = 0; i < p.first; ++i) {

        // Get the vertices that define this entity by getting
        // a pointer to the vector-of-vector data and then constructing 
        // a vector of ids for only this entity.
        id_t * a = &entity_vertices[i * p.second[i]];
        id_vector_t ev(a, a + p.second[i]);

        // Sort the ids for the current entity so that they are
        // monotonically increasing. This insures that entities are
        // created uniquely (using emplace_back below) because the ids
        // will always occur in the same order for the same entity.
        std::sort(ev.begin(), ev.end());

        // Emplace the sorted vertices into the entity map
        auto itr = entity_vertices_map.emplace(std::move(ev), entity_id);
        
        // Add this id to the cell to entity connections
        conns.push_back(itr.first->second);

        // If the insertion took place
        if(itr.second) {
          // what does this do?
          id_vector_t ev2 = id_vector_t(a, a + p.second[i]);
          entity_vertex_conn.emplace_back(std::move(ev2));

          max_cell_entity_conns =
              std::max(max_cell_entity_conns, cell_entity_conn[c].size());

          // A new entity was added, so we advance the id counter.
          ++entity_id;
        } // if
      } // for
    } // for

    // This call will create the entity objects in the mesh (The above
    // logic only defines the indices and connectivity.)
    cell_to_entity.init_create<MT, M>(id_vecs_[M][dim], entities_[M][dim],
      cell_entity_conn, dim);
    
    // Set the connectivity information from the created entities to
    // the vertices.
    connectivity_t & entity_to_vertex = get_connectivity_(M, dim, 0);
    entity_to_vertex.init(entity_vertex_conn);
  } // build_connectivity

  /*!
     used internally to compute connectivity information for
     topological dimension
       D1 -> D2 where D1 < D2
   */
  template<size_t M>
  void transpose(size_t from_dim, size_t to_dim) {
    //std::cerr << "transpose: " << fromDim << " -> " << toDim << std::endl;

    index_vector_t pos(num_entities_(M, from_dim), 0);

    for (index_iterator<M> to_entity(*this, to_dim);
      !to_entity.end(); ++to_entity) {
      for (index_iterator<M> from_itr(to_entity, from_dim);
        !from_itr.end(); ++from_itr) {
        pos[*from_itr]++;
      } // for
    } // for

    connectivity_t &out_conn = get_connectivity_(M, from_dim, to_dim);
    out_conn.resize(pos);

    std::fill(pos.begin(), pos.end(), 0);

    for (index_iterator<M> to_entity(*this, to_dim);
      !to_entity.end(); ++to_entity) {
      for (index_iterator<M> from_itr(to_entity, from_dim);
        !from_itr.end(); ++from_itr) {
        out_conn.set(*from_itr, *to_entity, pos[*from_itr]++);
      } // for
    } // for
  } // transpose

  /*!
     used internally to compute connectivity information for
     topological dimension
       D1 -> D2 where D1 > D2
   */
  template<size_t M>
  void intersect(size_t from_dim, size_t to_dim, size_t dim) {
    //std::cerr << "intersect: " << fromDim << " -> " << toDim << std::endl;

    connectivity_t &out_conn = get_connectivity_(M, from_dim, to_dim);
    if (!out_conn.empty()) {
      return;
    } // if

    connection_vector_t conns(num_entities_(M, from_dim));

    using visited_vec = std::vector<bool>;
    visited_vec visited(num_entities_(M, from_dim));

    id_vector_t from_verts;
    id_vector_t to_verts;

    size_t max_size = 1;

    for (index_iterator<M> from_entity(*this, from_dim); !from_entity.end();
         ++from_entity) {
      id_vector_t &entities = conns[*from_entity];
      entities.reserve(max_size);

      size_t count;
      id_t *ep = from_entity.get_entities(0, count);

      std::copy(ep, ep + count, from_verts.begin());

      std::sort(from_verts.begin(), from_verts.end());

      for (index_iterator<M> from_itr(from_entity, dim);
        !from_itr.end(); ++from_itr) {
        for (index_iterator<M> to_itr(from_itr, to_dim);
          !to_itr.end(); ++to_itr) {
          visited[*to_itr] = false;
        } // for
      } // for

      for (index_iterator<M> from_itr(from_entity, dim);
        !from_itr.end(); ++from_itr) {
        for (index_iterator<M> to_itr(from_itr, to_dim);
          !to_itr.end(); ++to_itr) {
          if (visited[*to_itr]) {
            continue;
          } // if

          visited[*to_itr] = true;

          if (from_dim == to_dim) {
            if (*from_entity != *to_itr) {
              entities.push_back(*to_itr);
            } // if
          }
          else {
            size_t count;
            id_t *ep = to_itr.get_entities(0, count);

            std::copy(ep, ep + count, to_verts.begin());

            std::sort(to_verts.begin(), to_verts.end());

            if (std::includes(from_verts.begin(), from_verts.end(),
              to_verts.begin(), to_verts.end())) {
              entities.emplace_back(*to_itr);
            } // if
          } // if
        } // for
      } // for

      max_size = std::max(entities.size(), max_size);
    } // for

    out_conn.init(conns);
  } // intersect

  /*!
     used to compute connectivity information for topological dimension
       D1 -> D2
   */
  template<size_t M>
  void compute_connectivity(size_t from_dim, size_t to_dim) {
    //std::cerr << "compute: " << from_dim << " -> " << to_dim << std::endl;

    connectivity_t & out_conn = get_connectivity_(M, from_dim, to_dim);

    if(!out_conn.empty()) {
      return;
    } // if

    if(num_entities_(M, from_dim) == 0) {
      build_connectivity<M>(from_dim);
    } // if

    if(num_entities_(M, to_dim) == 0) {
      build_connectivity<M>(to_dim);
    } // if

    if(num_entities_(M, from_dim) == 0 && num_entities_(M, to_dim) == 0) {
      return;
    } // if

    if(from_dim == to_dim) {
      connection_vector_t conn_vec(num_entities_(M, from_dim), id_vector_t(1));

      for(index_iterator<M> entity(*this, from_dim); !entity.end(); ++entity) {
        conn_vec[*entity][0] = *entity;
      } // for

      out_conn.set<M, MT::num_domains>(entities_[M][to_dim], conn_vec);
    }
    else if(from_dim < to_dim) {
      compute_connectivity<M>(to_dim, from_dim);
      transpose<M>(from_dim, to_dim);
    }
    else {
      compute_connectivity<M>(from_dim, 0);
      compute_connectivity<M>(0, to_dim);
      intersect<M>(from_dim, to_dim, 0);
    } // if
  } // compute_connectivity

  template<size_t FM, size_t TM>
  void compute_bindings(size_t from_dim, size_t to_dim) {
    connectivity_t & out_conn = get_connectivity_(FM, TM, from_dim, to_dim);

    if(!out_conn.empty()) {
      return;
    } // if

    if(num_entities_(FM, from_dim) == 0) {
      build_bindings<FM>(from_dim);
    } // if

    if(num_entities_(TM, to_dim) == 0) {
      build_bindings<TM>(to_dim);
    } // if

    if(num_entities_(FM, from_dim) == 0 && num_entities_(TM, to_dim) == 0) {
      return;
    } // if

    // FIXME: Need to complete

  } // compute_bindings

  template<size_t M>
  void build_bindings(size_t dim) {

    // Sanity check
    assert(dim <= MT::dimension);

    // Much of the connectivity informaiton used in this method
    // is derived from the "primal" mesh, i.e., domain 0
    static constexpr size_t M0 = 0;

    // Helper variables
    size_t entity_id(0);
    size_t max_cell_entity_conns = 1;
    const size_t _num_cells = num_entities<MT::dimension,M0>();

    // Storage for cell-to-entity connectivity information
    connection_vector_t cell_entity_conn(_num_cells);

    // Map used to insure unique entity creation
    id_vector_map_t entity_ids_map;

    // Get cell definitions from domain 0
    using ent_vec_t = entity_vector_t<MT::num_domains>;
    ent_vec_t & cells = entities_[M0][MT::dimension];

    // Iterate over cells
    for(auto c: cells) {

      // Get a cell object.
      auto cell = static_cast<cell_type<M0> *>(c);
      const size_t cell_id = cell->template id<M0>();

      // Get ids of entities with at least this dimension
      connection_vector_t primal_ids(MT::dimension+1);
      for(size_t dim(0); dim<MT::dimension; ++dim) {

        // Get domain 0 mesh connectivity information
        connectivity_t & conn = get_connectivity_(M0, MT::dimension, dim);

        size_t count;
        id_t * ids = conn.get_entities(cell_id, count);

        for(size_t i(0); i<count; ++i) {
          primal_ids[dim].push_back(to_global_id<M0>(dim, ids[i]));
        } // for
      } // for

      // Add the cell id
      primal_ids[MT::dimension].push_back(
        to_global_id<M0>(MT::dimension, cell_id));

      // p.first:   The number of entities per cell.
      // p.second:  A std::vector of id_t containing the ids of the
      //            entities that define the bound entity.
      id_vector_t entity_ids;
      auto p = cell->create_bound_entities(dim, primal_ids, entity_ids);

      // Iterate over the newly-defined entities
      id_vector_t & conns = cell_entity_conn[cell_id];
      for(size_t i(0); i<p.first; ++i) {

        // Get the id range for this entity
        id_t * a = &entity_ids[i * p.second[i]];        
        id_vector_t ev(a, a + p.second[i]);

        // Sort the id range so that it is ascending (insures uniqueness)
        std::sort(ev.begin(), ev.end());

        // Emplace will only create a new entry if it doesn't
        // already exist.
        auto itr = entity_ids_map.emplace(std::move(ev), entity_id);

        // Add this id to the cell entity connections
        conns.push_back(itr.first->second);

        // Increment
        if(itr.second) {
          max_cell_entity_conns =
            std::max(max_cell_entity_conns, cell_entity_conn[cell_id].size());
          ++entity_id;
        } // if
      } // for
    } // for

    // Reference to storage from cells to the entity (to be created here).
    connectivity_t & cell_to_entity =
      get_connectivity_(M0, M, MT::dimension, dim);

    // Create the entity objects
    cell_to_entity.init_create<MT, M>(id_vecs_[M][dim], entities_[M][dim],
      cell_entity_conn, dim);
  } // build_bindings

  template<size_t M = 0>
  void init() {
    std::cout << "init called for domain " << M << std::endl;

    // Compute mesh connectivity
    using TP = typename MT::connectivities;
    compute_connectivity_<M, std::tuple_size<TP>::value, TP>::compute(*this);

    using BT = typename MT::bindings;
    compute_bindings_<M, std::tuple_size<BT>::value, BT>::compute(*this);
  } // init

//
//
// What should we do with these?
//
//

  template<size_t D, size_t M=0>
  decltype(auto) num_entities() const {
    return entities_[M][D].size();
  } // num_entities

//
//
//
//

  const connectivity_t & get_connectivity(size_t from_domain,
      size_t to_domain, size_t from_dim, size_t to_dim) const override {
    return get_connectivity_(from_domain, to_domain, from_dim, to_dim);
  } // get_connectivity

  connectivity_t & get_connectivity(size_t from_domain,
      size_t to_domain, size_t from_dim,
    size_t to_dim) override {
    return get_connectivity_(from_domain, to_domain, from_dim, to_dim);
  } // get_connectivity

  const connectivity_t & get_connectivity(size_t domain,
      size_t from_dim, size_t to_dim) const override {
    return get_connectivity_(domain, domain, from_dim, to_dim);
  } // get_connectivity

  connectivity_t & get_connectivity(size_t domain, size_t from_dim,
    size_t to_dim) override {
    return get_connectivity_(domain, domain, from_dim, to_dim);
  } // get_connectivity

  const connectivity_t & get_connectivity_(size_t from_domain,
    size_t to_domain,
    size_t from_dim,
    size_t to_dim) const {
    assert(from_dim < topology_[from_domain][to_domain].size() && "invalid fromDim");
    auto & t = topology_[from_domain][to_domain][from_dim];
    assert(to_dim < t.size() && "invalid toDim");
    return t[to_dim];
  } // get_connectivity

  connectivity_t & get_connectivity_(size_t from_domain,
    size_t to_domain,
    size_t from_dim,
    size_t to_dim) {
    assert(from_dim < topology_[from_domain][to_domain].size() && "invalid fromDim");
    auto & t = topology_[from_domain][to_domain][from_dim];
    assert(to_dim < t.size() && "invalid toDim");
    return t[to_dim];
  } // get_connectivity

    const connectivity_t & get_connectivity_(size_t domain,
    size_t from_dim,
    size_t to_dim) const {
      return get_connectivity_(domain, domain, from_dim, to_dim);
  } // get_connectivity

  connectivity_t & get_connectivity_(size_t domain,
    size_t from_dim,
    size_t to_dim) {
      return get_connectivity_(domain, domain, from_dim, to_dim);
  } // get_connectivity

  size_t topological_dimension() const override { return MT::dimension; }

  template <class T, class... S> T * make(S &&... args) {
    T *entity = new T(std::forward<S>(args)...);
    return entity;
  } // make

  template<size_t M=0>
  const entity_vector_t<MT::num_domains> & get_entities_(size_t dim) const { 
    return entities_[M][dim]; 
  } // get_entities_

  template<size_t M=0>
  const id_vector_t & get_id_vec_(size_t dim) const { 
    return id_vecs_[M][dim]; 
  } // get_id_vec_

  template<size_t D, size_t M=0>
  auto get_entity(id_t id) const {
    using entity_type = typename find_entity_<MT, D, M>::type;
    return static_cast<entity_type*>(entities_[M][D][id]);
  } // get_entity
  
  template<size_t M=0>
  auto get_entity(size_t dim, id_t id) {
    return entities_[M][dim][id];
  } // get_entity

  /*--------------------------------------------------------------------------*
   * Vertex Interface
   *--------------------------------------------------------------------------*/

  template<size_t M=0>
  entity_range_t<0, M> vertices() { 
    return entity_range_t<0, M>(*this, id_vecs_[M][0]); 
  } // vertices

  template<size_t M=0>
  const_entity_range_t<0, M> vertices() const { 
    return const_entity_range_t<0, M>(*this, id_vecs_[M][0]);
  } // vertices

  template<size_t M=0>
  id_range vertex_ids() const {
    assert(!id_vecs_[M][0].empty());
    return id_range(id_vecs_[M][0]);
  } // vertex_ids

  template <size_t D, size_t FM, size_t TM=FM, class E>
  const_entity_range_t<D, TM>
  entities(const E *e) const {
    const connectivity_t &c = get_connectivity(FM, TM, E::dimension, D);
    assert(!c.empty() && "empty connectivity");
    const id_vector_t &fv = c.get_from_index_vec();
    return const_entity_range_t<D, TM>(*this, c.get_entities(),
      fv[e->template id<FM>()], fv[e->template id<FM>() + 1]);
  } // entities

  template <size_t D, size_t FM, size_t TM=FM, class E>
  entity_range_t<D, TM>
  entities(E *e) {
    const connectivity_t &c = get_connectivity(FM, TM, E::dimension, D);
    assert(!c.empty() && "empty connectivity");
    const id_vector_t &fv = c.get_from_index_vec();
    return entity_range_t<D, TM>(*this, c.get_entities(),
      fv[e->template id<FM>()], fv[e->template id<FM>() + 1]);
  } // entities

//
//
// We need to get rid of these
//
//

  template<size_t D, size_t M, class E>
  decltype(auto) entities(domain_entity<M,E> & e) const {
    return entities<D,M>(e.entity());
  } // entities

  template<size_t D, size_t M, class E>
  decltype(auto) entities(domain_entity<M,E> & e) {
    return entities<D,M>(e.entity());
  } // entities

#if 0
  template <size_t M, class E>
  decltype(auto) vertices(const E *e) const {
    return entities<0, M>(e);
  } // vertices

  template <size_t M, class E>
  decltype(auto) vertices(E *e) {
    return entities<0, M>(e);
  } // vertices

#endif
  template<size_t M, class E>
  decltype(auto) vertices(domain_entity<M, E>& e) const {
    return entities<0, M>(e.entity());
  } // vertices

  template<size_t M, class E>
  decltype(auto) vertices(domain_entity<M, E>& e){
    return entities<0, M>(e.entity());
  } // vertices

  template<size_t M, class E>
  decltype(auto) edges(domain_entity<M, E>& e) const {
    return entities<1, M>(e.entity());
  } //edges

  template<size_t M, class E>
  decltype(auto) edges(domain_entity<M, E>& e){
    return entities<1, M>(e.entity());
  } // edges

  template<size_t M, class E>
  decltype(auto) faces(domain_entity<M, E>& e) const {
    return entities<MT::dimension - 1, M>(e.entity());
  } // faces

  template<size_t M, class E>
  decltype(auto) faces(domain_entity<M, E>& e){
    return entities<MT::dimension - 1, M>(e.entity());
  } // faces

  template<size_t M, class E>
  decltype(auto) cells(domain_entity<M, E>& e) const {
    return entities<MT::dimension, M>(e.entity());
  } // cells

  template<size_t M, class E>
  decltype(auto) cells(domain_entity<M, E>& e){
    return entities<MT::dimension, M>(e.entity());
  } // cells

//
//
//
//
//

  /*--------------------------------------------------------------------------*
   * Edge Interface
   *--------------------------------------------------------------------------*/

  template<size_t M>
  entity_range_t<1, M> edges() {
    assert(!id_vecs_[M][1].empty());
    return entity_range_t<1, M>(*this, id_vecs_[M][1]);
  } // edges

  template <size_t M, class E> entity_range_t<1> edges(E *e) {
    return entities<1, M>(e);
  } // edges

  template <size_t M, class E> decltype(auto) edges(const E *e) const {
    return entities<1, M>(e);
  }

  template<size_t M>
  const_entity_range_t<1, M> edges() const {
    assert(!id_vecs_[M][1].empty());
    return const_entity_range_t<1>(*this, id_vecs_[M][1]);
  } // edges

  template<size_t M>
  id_range edge_ids() const {
    assert(!id_vecs_[M][1].empty());
    return id_range(id_vecs_[M][1]);
  } // edges

  template <size_t M, class E> entity_range_t<1> edges(E *e) const {
    return entities<1, M>(e);
  } // edges

  /*--------------------------------------------------------------------------*
   * Face Interface
   *--------------------------------------------------------------------------*/

  template<size_t M>
  entity_range_t<MT::dimension - 1, M> faces() {
    return entity_range_t<MT::dimension - 1, M>(*this,
      id_vecs_[M][MT::dimension - 1]);
  } // faces

  template <size_t M, class E> entity_range_t<MT::dimension - 1> faces(E *e) {
    return entities<MT::dimension - 1, M>(e);
  } // faces

  template <size_t M, class E>
  decltype(auto) faces(const E *e) const {
    return entities<MT::dimension - 1, M>(e);
  }

  template<size_t M>
  const_entity_range_t<MT::dimension - 1, M> faces() const {
    return const_entity_range_t<MT::dimension - 1, M>(*this,
      id_vecs_[M][MT::dimension - 1]);
  } // faces

  template<size_t M>
  id_range face_ids() const {
    assert(!id_vecs_[MT::dimension - 1].empty());
    return id_range(id_vecs_[M][MT::dimension - 1]);
  } // face_ids

  template <size_t M, class E>
  const_entity_range_t<MT::dimension - 1> faces(E *e) const {
    return entities<MT::dimension - 1, M>(e);
  } // faces

  /*--------------------------------------------------------------------------*
   * Cell Interface
   *--------------------------------------------------------------------------*/

  template<size_t M>
  entity_range_t<MT::dimension, M> cells() {
    return entity_range_t<MT::dimension, M>(*this, id_vecs_[M][MT::dimension]);
  } // cells

  template <size_t M, class E> entity_range_t<MT::dimension, M> cells(E *e) {
    return entities<MT::dimension, M>(e);
  } // cells

  template<size_t M>
  const_entity_range_t<MT::dimension, M> cells() const {
    return entity_range_t<MT::dimension, M>(*this, id_vecs_[M][MT::dimension]);
  } // cells

  template<size_t M>
  id_range cell_ids() const {
    assert(!id_vecs_[M][MT::dimension].empty());
    return id_range(id_vecs_[M][MT::dimension]);
  } // cell_ids

  template <size_t M, class E>
  const_entity_range_t<MT::dimension> cells(E *e) const {
    return entities<MT::dimension, M>(e);
  } // cells

  void dump() {
    for(size_t d = 0; d < MT::num_domains; ++d){
      for (size_t i = 0; i < topology_[d].size(); ++i) {
        auto &ci = topology_[d][d][i];
        for (size_t j = 0; j < ci.size(); ++j) {
          auto &cj = ci[j];
          std::cout << "------------- " << i << " -> " << j << std::endl;
          cj.dump();
        }
      }
    }
  } // dump

private:

  /*!
    Defines a type for storing entity instances as an array of
    \ref entity_vector_t, which is a std::vector of \ref mesh_entity_base_t.
   */
  using entities_t =
    std::array<entity_vector_t<MT::num_domains>, MT::dimension + 1>;
  
  /*!
    Defines a type for storing connectivity information.
   */
  using topology_t = std::array<std::array<connectivity_t, MT::dimension + 1>,
      MT::dimension + 1>;

  using id_vecs_t = std::array<id_vector_t, MT::dimension + 1>;

  // array of array of vector of mesh_entity_base_t *
  std::array<entities_t, MT::num_domains> entities_;

  // array of array of connectivity_t
  std::array<std::array<topology_t, MT::num_domains>, MT::num_domains>
    topology_;

  // array of array of vector of id_t
  std::array<id_vecs_t, MT::num_domains> id_vecs_;

}; // class mesh_topology_t

} // flexi

#endif // flexi_mesh_topology_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
