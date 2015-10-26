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

#include <algorithm>
#include <iostream>
#include <array>
#include <vector>
#include <cassert>
#include <unordered_map>

#include "flexi/utils/common.h"

#define ndump(X)                                                               \
  std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__      \
            << ": " << #X << " = " << X << std::endl

#define nlog(X)                                                                \
  std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__      \
            << ": " << X << std::endl

namespace flexi {

/*----------------------------------------------------------------------------*
 * class MeshEntityBase
 *----------------------------------------------------------------------------*/

/*!
  \class MeshEntityBase mesh_topology.h
  \brief MeshEntityBase defines a base class for...
 */

class MeshEntityBase {
public:
  /*!
    Return the id of this entity.

    \return The id of the entity.
   */

  id_t id() const { return id_; } // id

  uint16_t info() const { return info_; } // info

  /*!
   */

  static constexpr size_t getDim_(size_t meshDim, size_t dim) {
    return dim > meshDim ? meshDim : dim;
  } // getDim_

  template <class MT> static MeshEntityBase *create_(size_t dim, size_t id) {
    switch (dim) {
    case 0: {
      using EntityType = typename std::tuple_element<getDim_(MT::dimension, 0),
          typename MT::EntityTypes>::type;

      auto entity = new EntityType;
      entity->id_ = id;
      return entity;
    }
    case 1: {
      using EntityType = typename std::tuple_element<getDim_(MT::dimension, 1),
          typename MT::EntityTypes>::type;
      auto entity = new EntityType;
      entity->id_ = id;
      return entity;
    }
    case 2: {
      using EntityType = typename std::tuple_element<getDim_(MT::dimension, 2),
          typename MT::EntityTypes>::type;
      auto entity = new EntityType;
      entity->id_ = id;
      return entity;
    }
    case 3: {
      using EntityType = typename std::tuple_element<getDim_(MT::dimension, 3),
          typename MT::EntityTypes>::type;
      auto entity = new EntityType;
      entity->id_ = id;
      return entity;
    }
    default:
      assert(false && "invalid topology dim");
    }
  }

  template <class MT> friend class MeshTopology;

protected:

  void setInfo(uint16_t info){
    info_ = info;
  }

private:
  id_t id_ : 48;
  uint16_t info_ : 16;
}; // class MeshEntityBase

/*----------------------------------------------------------------------------*
 * class MeshEntity
 *----------------------------------------------------------------------------*/

/*!
  \class MeshEntity mesh_topology.h
  \brief ...
 */

template <size_t D> class MeshEntity : public MeshEntityBase {
public:
  static const size_t dimension = D;

  MeshEntity() {}
}; // class MeshEntity

using EntityVec = std::vector<MeshEntityBase *>;

/*----------------------------------------------------------------------------*
 * class EntityGroup
 *----------------------------------------------------------------------------*/

/*!
  \class MeshGroup mesh_topology.h
  \brief ...
 */

template <class T> class EntityGroup {
public:
  using Vec = std::vector<T *>;

  /*--------------------------------------------------------------------------*
   * class iterator_
   *--------------------------------------------------------------------------*/

  class iterator_ {
  public:
    iterator_(const Vec &entities, size_t index)
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
    const Vec *entities_;
    size_t index_;
  }; // class iterator_

  //! Constructor
  EntityGroup() {}

  EntityGroup(Vec &&entities) : entities_(std::move(entities)) {}

  void add(T *ent) { entities_.push_back(ent); }

  const Vec &getEntities() const { return entities_; }

  static constexpr size_t dim() { return T::dimension; }

  iterator_ begin() { return iterator_(entities_, 0); }

  iterator_ end() { return iterator_(entities_, entities_.size()); }

private:
  Vec entities_;

}; // class EntityGroup

/*----------------------------------------------------------------------------*
 * class MeshTopologyBase
 *----------------------------------------------------------------------------*/

/*!
  \class MeshTopologyBase mesh_topology.h
  \brief...
 */

template<size_t I>
struct compute_connectivity_{
  template<class M, class... TS>
  static int compute(M& mesh, std::tuple<TS...> t){
    using T = typename std::tuple_element<I, decltype(t)>::type;
    using T1 = typename std::tuple_element<0, T>::type;
    using T2 = typename std::tuple_element<1, T>::type;

    mesh.compute(T1::dimension, T2::dimension);
    return compute_connectivity_<I - 1>::compute(mesh, t);
  }
};

template<>
struct compute_connectivity_<0>{
  template<class M, class... TS>
  static int compute(M&, std::tuple<TS...>){
    return 0;
  }
};

class MeshTopologyBase {
public:
  using IdVec = std::vector<id_t>;

  using ConnVec = std::vector<IdVec>;

  struct IdVecHash {
    size_t operator()(const IdVec &v) const {
      size_t h = 0;
      for (id_t id : v) {
        h |= id;
      }
      return h;
    }
  }; // struct IdVecHash

  using IdVecMap = std::unordered_map<IdVec, id_t, IdVecHash>;

  using IndexVec = std::vector<size_t>;

  /*--------------------------------------------------------------------------*
   * class Connectivity
   *--------------------------------------------------------------------------*/

  class Connectivity {
  public:
    Connectivity() {}

    void clear() {
      toIdVec_.clear();
      fromIndexVec_.clear();
    }

    void init() { fromIndexVec_.push_back(0); }

    void init(const ConnVec &cv) {
      assert(toIdVec_.empty() && fromIndexVec_.empty());

      fromIndexVec_.push_back(0);

      size_t n = cv.size();

      for (size_t i = 0; i < n; ++i) {
        const IdVec &iv = cv[i];

        for (id_t id : iv) {
          toIdVec_.push_back(id);
        }

        fromIndexVec_.push_back(toIdVec_.size());
      }
    } // init

    template <class MT>
    void initCreate(IdVec &iv, EntityVec &ev, const ConnVec &cv, size_t dim) {
      assert(toIdVec_.empty() && fromIndexVec_.empty());

      fromIndexVec_.push_back(0);

      size_t n = cv.size();

      id_t maxId = 0;

      for (size_t i = 0; i < n; ++i) {
        const IdVec &iv = cv[i];

        for (id_t id : iv) {
          maxId = std::max(maxId, id);
          toIdVec_.push_back(id);
        } // for

        fromIndexVec_.push_back(toIdVec_.size());
      } // for

      id_t startId = ev.size();

      ev.reserve(maxId + 1);

      for(id_t id = startId; id <= maxId; ++id){
        ev.push_back(MeshEntityBase::create_<MT>(dim, id));
        iv.push_back(id);
      }

    } // initCreate

    void resize(IndexVec &numConns) {
      clear();

      size_t n = numConns.size();
      fromIndexVec_.resize(n + 1);

      uint64_t size = 0;

      for (size_t i = 0; i < n; ++i) {
        fromIndexVec_[i] = size;
        size += numConns[i];
      }

      fromIndexVec_[n] = size;

      toIdVec_.resize(size);
      std::fill(toIdVec_.begin(), toIdVec_.end(), 0);
    } // resize

    void endFrom() { fromIndexVec_.push_back(toIdVec_.size()); } // endFrom

    void push(id_t id) { toIdVec_.push_back(id); } // push

    void dump() {
      for(size_t i = 1; i < fromIndexVec_.size(); ++i){
        for(size_t j = fromIndexVec_[i - 1]; j < fromIndexVec_[i]; ++j){
          std::cout << toIdVec_[j] << std::endl;
        }
        std::cout << std::endl;
      }

      std::cout << "=== idVec" << std::endl;
      for (id_t id : toIdVec_) {
        std::cout << id << std::endl;
      } // for

      std::cout << "=== groupVec" << std::endl;
      for (id_t id : fromIndexVec_) {
        std::cout << id << std::endl;
      } // for
    }   // dump

    const IdVec &getFromIndexVec() const { return fromIndexVec_; }

    const IdVec& getEntities() const {
      return toIdVec_;
    }

    id_t *getEntities(size_t index) {
      assert(index < fromIndexVec_.size() - 1);
      return toIdVec_.data() + fromIndexVec_[index];
    }

    id_t *getEntities(size_t index, size_t &endIndex) {
      assert(index < fromIndexVec_.size() - 1);
      uint64_t start = fromIndexVec_[index];
      endIndex = fromIndexVec_[index + 1] - start;
      return toIdVec_.data() + start;
    }

    bool empty() const { return toIdVec_.empty(); }

    void set(id_t fromId, id_t toId, size_t pos) {
      toIdVec_[fromIndexVec_[fromId] + pos] = toId;
    }

    size_t fromSize() const { return fromIndexVec_.size() - 1; }

    size_t toSize() const { return toIdVec_.size(); }

    void set(EntityVec &ev, ConnVec &conns) {
      clear();

      size_t n = conns.size();
      fromIndexVec_.resize(n + 1);

      size_t size = 0;

      for (size_t i = 0; i < n; i++) {
        fromIndexVec_[i] = size;
        size += conns[i].size();
      }

      fromIndexVec_[n] = size;

      toIdVec_.reserve(size);

      for (size_t i = 0; i < n; ++i) {
        const IdVec &conn = conns[i];
        uint64_t m = conn.size();

        for (size_t j = 0; j < m; ++j) {
          toIdVec_.push_back(ev[conn[j]]->id());
        }
      }
    }

    IdVec toIdVec_;
    IdVec fromIndexVec_;
  }; // class Connectivity

  virtual size_t numEntities(size_t dim) const = 0;

  virtual void build(size_t dim) = 0;

  virtual void compute(size_t fromDim, size_t toDim) = 0;

  virtual void init() = 0;

  virtual size_t topologicalDimension() const = 0;

  virtual const Connectivity &getConnectivity(
      size_t fromDim, size_t toDim) const = 0;

  virtual Connectivity &getConnectivity(size_t fromDim, size_t toDim) = 0;

}; // MeshTopologyBase

/*----------------------------------------------------------------------------*
 * class MeshTopology
 *----------------------------------------------------------------------------*/

/*!
  \class MeshTopology mesh_topology.h
  \brief ...
 */

template <class MT> class MeshTopology : public MeshTopologyBase {
public:
  using VertexType =
      typename std::tuple_element<0, typename MT::EntityTypes>::type;

  using EdgeType =
      typename std::tuple_element<1, typename MT::EntityTypes>::type;

  using FaceType = typename std::tuple_element<MT::dimension - 1,
      typename MT::EntityTypes>::type;

  using CellType = typename std::tuple_element<MT::dimension,
      typename MT::EntityTypes>::type;

  /*--------------------------------------------------------------------------*
   * class Iterator
   *--------------------------------------------------------------------------*/

  class index_iterator {
  public:
    index_iterator(MeshTopology &mesh, size_t dim)
      : mesh_(mesh), entities_(&mesh.getIdVec_(dim)), dim_(dim), index_(0),
        endIndex_(mesh.numEntities(dim)), level_(0) {}

    index_iterator(index_iterator &itr, size_t dim)
        : mesh_(itr.mesh_), dim_(dim), level_(itr.level_ + 1) {

      Connectivity &c = mesh_.getConnectivity(itr.dim_, dim_);
      assert(!c.empty());

      entities_ = &c.getEntities();

      const IdVec &fv = c.getFromIndexVec();
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

    id_t *getEntities(size_t dim) {
      Connectivity &c = mesh_.getConnectivity_(dim_, dim);
      assert(!c.empty());
      return c.getEntities(index_);
    }

  protected:
    const IdVec &getEntities_() { return *entities_; }

  private:
    MeshTopology &mesh_;
    const IdVec* entities_;
    size_t dim_;
    size_t index_;
    size_t endIndex_;
    size_t level_;
  }; // class index_iterator

  /*--------------------------------------------------------------------------*
   * class entity_index_iterator
   *--------------------------------------------------------------------------*/

  template <size_t D> class entity_index_iterator : public index_iterator {
  public:
    entity_index_iterator(MeshTopology &mesh) : index_iterator(mesh, D) {}

    entity_index_iterator(index_iterator &itr) : index_iterator(itr, D) {}

  }; // class entity_index_iterator

  using vertex_index_iterator = entity_index_iterator<0>;
  using edge_index_iterator = entity_index_iterator<1>;
  using face_index_iterator = entity_index_iterator<MT::dimension - 1>;
  using cell_index_iterator = entity_index_iterator<MT::dimension>;

  /*--------------------------------------------------------------------------*
   * class iterator
   *--------------------------------------------------------------------------*/

  template <size_t D> class iterator {
  public:
    using EntityType =
        typename std::tuple_element<D, typename MT::EntityTypes>::type;

    iterator(const iterator &itr)
      : mesh_(itr.mesh_), entities_(itr.entities_), index_(itr.index_) {}

    iterator(MeshTopology &mesh, const IdVec &entities, size_t index)
        : mesh_(mesh), 
          entities_(&entities),
          index_(index) {}

    iterator &operator++() {
      ++index_;
      return *this;
    }

    iterator &operator=(const iterator &itr) {
      index_ = itr.index_;
      entities_ = itr.entities_;
      return *this;
    }

    EntityType *operator*() { return mesh_.getEntity<D>((*entities_)[index_]); }

    EntityType *operator->() { return mesh_.getEntity<D>((*entities_)[index_]); }

    bool operator==(const iterator &itr) const { return index_ == itr.index_; }

    bool operator!=(const iterator &itr) const { return index_ != itr.index_; }

  private:
    MeshTopology &mesh_;
    const IdVec *entities_;
    size_t index_;

  }; // class iterator

  /*--------------------------------------------------------------------------*
   * class const_iterator
   *--------------------------------------------------------------------------*/

  template <size_t D> class const_iterator {
  public:
    using EntityType =
        typename std::tuple_element<D, typename MT::EntityTypes>::type;

    const_iterator(const const_iterator &itr)
      : mesh_(itr.mesh_), entities_(itr.entities_), index_(itr.index_) {}

    const_iterator(const MeshTopology &mesh, const IdVec &entities, size_t index)
        : mesh_(mesh), 
          entities_(&entities),
          index_(index) {}

    const_iterator &operator++() {
      ++index_;
      return *this;
    }

    const_iterator &operator=(const const_iterator &itr) {
      index_ = itr.index_;
      entities_ = itr.entities_;
      return *this;
    }

    const EntityType *operator*() { return mesh_.getEntity<D>((*entities_)[index_]); }

    const EntityType *operator->() { return mesh_.getEntity<D>((*entities_)[index_]); }

    bool operator==(const const_iterator &itr) const { return index_ == itr.index_; }

    bool operator!=(const const_iterator &itr) const { return index_ != itr.index_; }

  private:
    const MeshTopology &mesh_;
    const IdVec *entities_;
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
   * class EntityRange
   *--------------------------------------------------------------------------*/

  /*!
    \class EntityRange mesh_topology.h
    \brief ...
   */

  template <size_t D> class EntityRange {
  public:
    using iterator_t = iterator<D>;
    using EntityType = typename iterator_t::EntityType;
    using EntityVec = std::vector<EntityType*>;

    EntityRange(MeshTopology &mesh, const IdVec &v)
      : mesh_(mesh), v_(v), begin_(0),
        end_(v_.size()) {}

    EntityRange(MeshTopology &mesh, const IdVec &v, size_t begin, size_t end)
      : mesh_(mesh), v_(v), begin_(begin),
        end_(end) {}

    EntityRange(const EntityRange &r) : mesh_(r.mesh_), v_(r.v_),
                                        begin_(0), end_(v_.size()) {}

    iterator_t begin() const { return iterator_t(mesh_, v_, begin_); }

    iterator_t end() const { return iterator_t(mesh_, v_, end_); }

    EntityVec toVec() const{
      EntityVec ret;
      for(size_t i = begin_; i < end_; ++i){
        ret.push_back(mesh_.getEntity<D>(v_[i]));
      }
      return ret;
    }

  private:

    MeshTopology &mesh_;
    const IdVec &v_;
    size_t begin_;
    size_t end_;

  }; // class EntityRange

  /*--------------------------------------------------------------------------*
   * class ConstEntityRange
   *--------------------------------------------------------------------------*/

  /*!
    \class ConstEntityRange mesh_topology.h
    \brief ...
   */

  template <size_t D> class ConstEntityRange {
  public:
    using const_iterator_t = const_iterator<D>;
    using EntityType = typename const_iterator_t::EntityType;
    using EntityVec = std::vector<const EntityType*>;

    ConstEntityRange(const MeshTopology &mesh, const IdVec &v)
      : mesh_(mesh), v_(v), begin_(0),
        end_(v_.size()) {}

    ConstEntityRange(const MeshTopology &mesh, const IdVec &v, size_t begin, size_t end)
      : mesh_(mesh), v_(v), begin_(begin),
        end_(end) {}

    ConstEntityRange(const ConstEntityRange &r) : mesh_(r.mesh_), v_(r.v_),
                                                  begin_(0), end_(v_.size()) {}

    const_iterator_t begin() const { return const_iterator_t(mesh_, v_, begin_); }

    const_iterator_t end() const { return const_iterator_t(mesh_, v_, end_); }

    EntityVec toVec() const{
      EntityVec ret;
      for(size_t i = begin_; i < end_; ++i){
        ret.push_back(mesh_.getEntity<D>(v_[i]));
      }
      return ret;
    }

  private:

    const MeshTopology &mesh_;
    const IdVec &v_;
    size_t begin_;
    size_t end_;

  }; // class ConstEntityRange

  //! Constructor
  MeshTopology() {
    getConnectivity_(MT::dimension, 0).init();
  } // MeshTopology()

  template<size_t D>
  void addEntity(MeshEntityBase *ent) {
    auto &ents = entities_[D];
    id_t id = ent->id();
    if(ents.size() <= id){
      ents.resize(id + 1);
    }
    ents[id] = ent;
  } // addEntity

  void addVertex(VertexType *vertex) {
    addEntity<0>(vertex);
  } // addVertex

  void addEdge(EdgeType *edge) {
    addEntity<1>(edge);
  } // addEdge

  void addFace(FaceType *face) {
    addEntity<MT::dimension - 1>(face);
  } // addFace

  void addCell(CellType *cell) {
    addEntity<MT::dimension>(cell);
  } // addCell

  void initCell(CellType *cell, std::initializer_list<VertexType *> verts) {
    assert(verts.size() == MT::numVerticesPerEntity(MT::dimension) &&
        "invalid number of vertices per cell");

    auto &c = getConnectivity_(MT::dimension, 0);

    assert(cell->id() == c.fromSize() && "id mismatch");

    for (VertexType *v : verts) {
      c.push(v->id());
    } // for

    c.endFrom();
  } // initCell

  void initEdge(EdgeType *edge, const VertexType * vertex1,
    const VertexType * vertex2) {

    auto &c = getConnectivity_(1, 0);
    if (c.empty()) {
      c.init();
    } // if

    assert(edge->id() == c.fromSize() && "id mismatch");

    c.push(vertex1->id());
    c.push(vertex2->id());

    c.endFrom();
  } // initEdge

  void initFace(FaceType *face, std::initializer_list<VertexType *> verts) {
    assert(verts.size() == MT::numVerticesPerEntity(2) &&
        "invalid number vertices per face");

    auto &c = getConnectivity_(MT::dimension - 1, 0);
    if (c.empty()) {
      c.init();
    }

    assert(face->id() == c.fromSize() && "id mismatch");

    for (VertexType *v : verts) {
      c.push(v);
    }

    c.endFrom();
  } // initFace

  void initCellEdges(CellType *cell, std::initializer_list<EdgeType *> edges) {
    assert(edges.size() == MT::numEntitiesPerCell(1) &&
        "invalid number of edges per cell");

    auto &c = getConnectivity_(MT::dimension, 1);
    if (c.empty()) {
      c.init();
    }

    assert(cell->id() == c.fromSize() && "id mismatch");

    for (EdgeType *edge : edges) {
      c.push(edge);
    }

    c.endFrom();
  } // initCellEdges

  void initCellFaces(CellType *cell, std::initializer_list<FaceType *> faces) {

    assert(faces.size() == MT::numEntitiesPerCell(MT::dimension - 1) &&
        "invalid number of face per cell");

    auto &c = getConnectivity_(MT::dimension, MT::dimension - 1);
    if (c.empty()) {
      c.init();
    }

    assert(cell->id() == c.fromSize() && "id mismatch");

    for (FaceType *face : faces) {
      c.push(face);
    }

    c.endFrom();
  } // initCellFaces

  size_t numEntities(size_t dim) const override {
    return entities_[dim].size();
  } // numEntities

  size_t numEntities_(size_t dim) const {
    return entities_[dim].size();
  } // numEntities_

  void build(size_t dim) override {
    // std::cerr << "build: " << dim << std::endl;

    assert(dim <= MT::dimension);

    size_t verticesPerEntity = MT::numVerticesPerEntity(dim);
    size_t entitiesPerCell = MT::numEntitiesPerCell(dim);

    Connectivity &entityToVertex = getConnectivity_(dim, 0);

    IdVec entityVertices(entitiesPerCell * verticesPerEntity);

    Connectivity &cellToEntity = getConnectivity_(MT::dimension, dim);

    ConnVec entityVertexConn;

    size_t entityId = 0;
    size_t maxCellEntityConns = 1;

    Connectivity &cellToVertex = getConnectivity_(MT::dimension, 0);
    assert(!cellToVertex.empty());

    size_t n = numCells();

    ConnVec cellEntityConn(n);

    IdVecMap entityVerticesMap(n * MT::numEntitiesPerCell(dim) / 2);

    using TestVec = std::vector<std::vector<VertexType*>>;

    for (size_t c = 0; c < n; ++c) {
      IdVec &conns = cellEntityConn[c];

      conns.reserve(maxCellEntityConns);

      id_t *vertices = cellToVertex.getEntities(c);

      MT::createEntities(dim, entityVertices, vertices);

      for (size_t i = 0; i < entitiesPerCell; ++i) {
        id_t *a = &entityVertices[i * verticesPerEntity];
        IdVec ev(a, a + verticesPerEntity);
        std::sort(ev.begin(), ev.end());

        auto itr = entityVerticesMap.emplace(std::move(ev), entityId);
        conns.emplace_back(itr.first->second);

        if (itr.second) {
          IdVec ev2 = IdVec(a, a + verticesPerEntity);

          entityVertexConn.emplace_back(std::move(ev2));

          maxCellEntityConns =
              std::max(maxCellEntityConns, cellEntityConn[c].size());

          ++entityId;
        }
      }
    }

    cellToEntity.initCreate<MT>(idVecs_[dim], entities_[dim], cellEntityConn, dim);
    entityToVertex.init(entityVertexConn);
  } // build

  void transpose(size_t fromDim, size_t toDim) {
    //std::cerr << "transpose: " << fromDim << " -> " << toDim << std::endl;

    IndexVec pos(numEntities_(fromDim), 0);

    for (index_iterator toEntity(*this, toDim); !toEntity.end(); ++toEntity) {
      for (index_iterator fromItr(toEntity, fromDim); !fromItr.end(); ++fromItr) {
	//std::cerr << "size: " << pos.size() << std::endl;
	//std::cerr << "fromItr: " << *fromItr << std::endl;
        pos[*fromItr]++;
      }
    }

    Connectivity &outConn = getConnectivity_(fromDim, toDim);
    outConn.resize(pos);

    std::fill(pos.begin(), pos.end(), 0);

    for (index_iterator toEntity(*this, toDim); !toEntity.end(); ++toEntity) {
      for (index_iterator fromItr(toEntity, fromDim); !fromItr.end(); ++fromItr) {
        outConn.set(*fromItr, *toEntity, pos[*fromItr]++);
      }
    }
  } // transpose

  void intersect(size_t fromDim, size_t toDim, size_t dim) {
    //std::cerr << "intersect: " << fromDim << " -> " << toDim << std::endl;

    Connectivity &outConn = getConnectivity_(fromDim, toDim);
    if (!outConn.empty()) {
      return;
    }

    ConnVec conns(numEntities_(fromDim));

    using VisitedVec = std::vector<bool>;
    VisitedVec visited(numEntities_(fromDim));

    IdVec fromVerts(MT::numVerticesPerEntity(fromDim));
    IdVec toVerts(MT::numVerticesPerEntity(toDim));

    size_t maxSize = 1;

    for (index_iterator fromEntity(*this, fromDim); !fromEntity.end();
         ++fromEntity) {
      IdVec &entities = conns[*fromEntity];
      entities.reserve(maxSize);

      id_t *ep = fromEntity.getEntities(0);

      std::copy(ep, ep + MT::numVerticesPerEntity(fromDim), fromVerts.begin());

      std::sort(fromVerts.begin(), fromVerts.end());

      for (index_iterator fromItr(fromEntity, dim); !fromItr.end(); ++fromItr) {
        for (index_iterator toItr(fromItr, toDim); !toItr.end(); ++toItr) {
          visited[*toItr] = false;
        }
      }

      for (index_iterator fromItr(fromEntity, dim); !fromItr.end(); ++fromItr) {
        for (index_iterator toItr(fromItr, toDim); !toItr.end(); ++toItr) {
          if (visited[*toItr]) {
            continue;
          }

          visited[*toItr] = true;

          if (fromDim == toDim) {
            if (*fromEntity != *toItr) {
              entities.push_back(*toItr);
            }
          } else {
            id_t *ep = toItr.getEntities(0);

            std::copy(
                ep, ep + MT::numVerticesPerEntity(toDim), toVerts.begin());

            std::sort(toVerts.begin(), toVerts.end());

            if (std::includes(fromVerts.begin(), fromVerts.end(),
                    toVerts.begin(), toVerts.end())) {

              entities.emplace_back(*toItr);
            }
          }
        }
      }

      maxSize = std::max(entities.size(), maxSize);
    }

    outConn.init(conns);
  } // intersect

  void compute(size_t fromDim, size_t toDim) override {
    // std::cerr << "compute: " << fromDim << " -> " << toDim << std::endl;

    Connectivity &outConn = getConnectivity_(fromDim, toDim);

    if (!outConn.empty()) {
      return;
    }

    if (numEntities_(fromDim) == 0) {
      build(fromDim);
    }

    if (numEntities_(toDim) == 0) {
      build(toDim);
    }

    if (numEntities_(fromDim) == 0 && numEntities_(toDim) == 0) {
      return;
    }

    if (fromDim == toDim) {
      ConnVec connVec(numEntities_(fromDim), IdVec(1));

      for (index_iterator entity(*this, fromDim); !entity.end(); ++entity) {
        connVec[*entity][0] = *entity;
      }

      outConn.set(entities_[toDim], connVec);
    } else if (fromDim < toDim) {
      compute(toDim, fromDim);
      transpose(fromDim, toDim);
    } else {
      compute(fromDim, 0);
      compute(0, toDim);
      intersect(fromDim, toDim, 0);
    }
  } // compute

  void init() override {
    using TP = typename MT::TraversalPairs;

    compute_connectivity_<std::tuple_size<TP>::value - 1>::compute(*this, TP());
  } // init

  decltype(auto) numCells() const {
    return entities_[MT::dimension].size();
  } // numCells

  decltype(auto) numVertices() const {
    return entities_[0].size();
  } // numVertices

  decltype(auto) numEdges() const { return entities_[1].size(); } // numEdges

  decltype(auto) numFaces() const {
    return entities_[MT::dimension - 1].size();
  } // numFaces

  const Connectivity &getConnectivity(
      size_t fromDim, size_t toDim) const override {
    return getConnectivity_(fromDim, toDim);
  } // getConnectivity

  Connectivity &getConnectivity(size_t fromDim, size_t toDim) override {
    return getConnectivity_(fromDim, toDim);
  } // getConnectivity

  const Connectivity &getConnectivity_(size_t fromDim, size_t toDim) const {
    assert(fromDim < topology_.size() && "invalid fromDim");
    auto &t = topology_[fromDim];
    assert(toDim < t.size() && "invalid toDim");
    return t[toDim];
  } // getConnectivity

  Connectivity &getConnectivity_(size_t fromDim, size_t toDim) {
    assert(fromDim < topology_.size() && "invalid fromDim");
    auto &t = topology_[fromDim];
    assert(toDim < t.size() && "invalid toDim");
    return t[toDim];
  } // getConnectivity

  size_t topologicalDimension() const override { return MT::dimension; }

  template <class T, class... S> T *make(S &&... args) {
    T *entity = new T(std::forward<S>(args)...);

    auto &ents = entities_[T::dimension];    
    entity->id_ = ents.size();
    ents.push_back(entity);

    auto &idVec = idVecs_[T::dimension];
    idVec.push_back(idVec.size());

    return entity;
  }

  const EntityVec &getEntities_(size_t dim) const { return entities_[dim]; }

  const IdVec &getIdVec_(size_t dim) const { return idVecs_[dim]; }

  template<size_t D>
  typename std::tuple_element<D, typename MT::EntityTypes>::type*
  getEntity(id_t id){
    using Type = typename std::tuple_element<D, typename MT::EntityTypes>::type;

    return static_cast<Type*>(entities_[D][id]);
  }
  
  MeshEntityBase*
  getEntity(size_t dim, id_t id){
    return entities_[dim][id];
  }

  EntityRange<0> vertices() { return EntityRange<0>(*this, idVecs_[0]); }

  ConstEntityRange<0> vertices() const { return ConstEntityRange<0>(*this, idVecs_[0]); }

  template <size_t D, class E> ConstEntityRange<D> entities(const E *e) const {
    const Connectivity &c = getConnectivity(E::dimension, D);
    assert(!c.empty() && "empty connectivity");
    const IdVec &fv = c.getFromIndexVec();
    return ConstEntityRange<D>(*this, c.getEntities(), fv[e->id()], fv[e->id() + 1]);
  } // entities

  template <size_t D, class E> EntityRange<D> entities(const E *e) {
    const Connectivity &c = getConnectivity(E::dimension, D);
    assert(!c.empty() && "empty connectivity");
    const IdVec &fv = c.getFromIndexVec();
    return EntityRange<D>(*this, c.getEntities(), fv[e->id()], fv[e->id() + 1]);
  } // entities

  template <class E> decltype(auto) vertices(const E *e) const {
    return entities<0>(e);
  }

  template <class E> decltype(auto) vertices(E *e) {
    return entities<0>(e);
  } // vertices

  EntityRange<1> edges() {
    assert(!idVecs_[1].empty());
    return EntityRange<1>(*this, idVecs_[1]);
  } // edges

  template <class E> EntityRange<1> edges(E *e) {
    return entities<1>(e);
  } // edges

  ConstEntityRange<1> edges() const {
    assert(!idVecs_[1].empty());
    return ConstEntityRange<1>(*this, idVecs_[1]);
  } // edges

  template <class E> EntityRange<1> edges(E *e) const {
    return entities<1>(e);
  } // edges

  EntityRange<MT::dimension - 1> faces() {
    return EntityRange<MT::dimension - 1>(*this, idVecs_[MT::dimension - 1]);
  } // faces

  template <class E> EntityRange<MT::dimension - 1> faces(E *e) {
    return entities<MT::dimension - 1>(e);
  } // faces

  ConstEntityRange<MT::dimension - 1> faces() const {
    return ConstEntityRange<MT::dimension - 1>(*this, idVecs_[MT::dimension - 1]);
  } // faces

  template <class E> ConstEntityRange<MT::dimension - 1> faces(E *e) const {
    return entities<MT::dimension - 1>(e);
  } // faces

  EntityRange<MT::dimension> cells() {
    return EntityRange<MT::dimension>(*this, idVecs_[MT::dimension]);
  } // cells

  template <class E> EntityRange<MT::dimension> cells(E *e) {
    return entities<MT::dimension>(e);
  } // cells

  ConstEntityRange<MT::dimension> cells() const {
    return EntityRange<MT::dimension>(*this, idVecs_[MT::dimension]);
  } // cells

  template <class E> ConstEntityRange<MT::dimension> cells(E *e) const {
    return entities<MT::dimension>(e);
  } // cells

  void dump() {
    for (size_t i = 0; i < topology_.size(); ++i) {
      auto &ci = topology_[i];
      for (size_t j = 0; j < ci.size(); ++j) {
        auto &cj = ci[j];
        std::cout << "------------- " << i << " -> " << j << std::endl;
        cj.dump();
      }
    }
  } // dump

private:
  using Entities_ = std::array<EntityVec, MT::dimension + 1>;

  using Topology_ = std::array<std::array<Connectivity, MT::dimension + 1>,
      MT::dimension + 1>;

  using IdVecs_ = std::array<IdVec, MT::dimension + 1>;

  Entities_ entities_;
  Topology_ topology_;
  IdVecs_ idVecs_;
}; // class MeshTopology

} // flexi

#endif // flexi_mesh_topology_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
