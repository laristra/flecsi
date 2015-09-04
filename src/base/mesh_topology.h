/*
 * ###########################################################################
 * Copyright (c) 2015, Los Alamos National Security, LLC.
 * All rights reserved.
 *
 *  Copyright 2015. Los Alamos National Security, LLC. This software was
 *  produced under U.S. Government contract DE-AC52-06NA25396 for Los
 *  Alamos National Laboratory (LANL), which is operated by Los Alamos
 *  National Security, LLC for the U.S. Department of Energy. The
 *  U.S. Government has rights to use, reproduce, and distribute this
 *  software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY,
 *  LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY
 *  FOR THE USE OF THIS SOFTWARE.  If software is modified to produce
 *  derivative works, such modified software should be clearly marked,
 *  so as not to confuse it with the version available from LANL.
 *
 *  Additionally, redistribution and use in source and binary forms,
 *  with or without modification, are permitted provided that the
 *  following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Los Alamos National Security, LLC, Los
 *      Alamos National Laboratory, LANL, the U.S. Government, nor the
 *      names of its contributors may be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
 *  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 * ###########################################################################
 *
 * Notes
 *
 * #####
 */

#ifndef __MESH_TOPOLOGY_H__
#define __MESH_TOPOLOGY_H__

#include <algorithm>
#include <iostream>
#include <array>
#include <vector>
#include <cassert>
#include <unordered_map>

#define ndump(X) std::cout << __FILE__ << ":" << __LINE__ << ": " << \
__PRETTY_FUNCTION__ << ": " << #X << " = " << X << std::endl

#define nlog(X) std::cout << __FILE__ << ":" << __LINE__ << ": " << \
__PRETTY_FUNCTION__ << ": " << X << std::endl

namespace{

  static const uint64_t LAST_FLAG = 1UL << 63;
  static const uint64_t INDEX_MASK = ~LAST_FLAG;

} // namespace

namespace jali{

  class MeshEntity{
  public:
    MeshEntity(size_t id)
      : id_(id){}

    size_t id() const{
      return id_;
    }

  private:
    size_t id_;
  };
   
  template<size_t D, class MT>
  struct CreateEntity{};

  template<class MT>
  struct CreateEntity<1, MT>{
    using VertexType = 
      typename std::tuple_element<0, typename MT::EntityTypes>::type;
    
    using CellType = 
      typename std::tuple_element<MT::topologicalDimension(),
                                  typename MT::EntityTypes>::type;

    static MeshEntity* create(size_t dim, size_t index){
      switch(dim){
      case 0:
        return new VertexType(index);
      case 1:
        return new CellType(index);
      default:
        assert(false && "invalid topological dimension");
      }
    }
  };

  template<class MT>
  struct CreateEntity<2, MT>{
    using VertexType = 
      typename std::tuple_element<0, typename MT::EntityTypes>::type;
    
    using EdgeType = 
      typename std::tuple_element<1, typename MT::EntityTypes>::type;  

    using CellType = 
      typename std::tuple_element<MT::topologicalDimension(),
                                  typename MT::EntityTypes>::type;

    static MeshEntity* create(size_t dim, size_t index){
      switch(dim){
      case 0:
        return new VertexType(index);
      case 1:
        return new EdgeType(index);
      case 2:
        return new CellType(index);
      default:
        assert(false && "invalid topological dimension");
      }
    } 
  };

  template<class MT>
  struct CreateEntity<3, MT>{
    using VertexType = 
      typename std::tuple_element<0, typename MT::EntityTypes>::type;
    
    using EdgeType = 
      typename std::tuple_element<1, typename MT::EntityTypes>::type;  

    using FaceType = 
      typename std::tuple_element<MT::topologicalDimension() - 1,
                                  typename MT::EntityTypes>::type;

    using CellType = 
      typename std::tuple_element<MT::topologicalDimension(),
                                  typename MT::EntityTypes>::type;

    MeshEntity* create(size_t dim, size_t index){
      switch(dim){
      case 0:
        return new VertexType(index);
      case 1:
        return new EdgeType(index);
      case 2:
        return new FaceType(index);
      case 3:
        return new CellType(index);
      default:
        assert(false && "invalid topological dimension");
      }
    }
  };

  class MeshTopologyBase{
  public:
    using Id = uint64_t;

    using IdVec = std::vector<Id>;
  
    using ConnVec = std::vector<IdVec>;
  
    struct IdVecHash{
      size_t operator()(const IdVec& v) const{
        size_t h = 0;
        for(Id id : v){
          h |= id;
        }
        return h;
      }
    };
  
    using IdVecMap = std::unordered_map<IdVec, Id, IdVecHash>;
  
    using IndexVec = std::vector<size_t>;

    class Connectivity{
    public:

      Connectivity(){}
    
      void clear(){
        toIdVec_.clear();
        fromIndexVec_.clear();
      }
    
      void init(){
        fromIndexVec_.push_back(0);
      }
    
      void init(const ConnVec& cv){
        assert(toIdVec_.empty() && fromIndexVec_.empty());
      
        fromIndexVec_.push_back(0);
      
        size_t n = cv.size();
      
        for(size_t i = 0; i < n; ++i){
          const IdVec& iv = cv[i];
          
          for(Id id : iv){
            toIdVec_.push_back(id);
          }
          
          toIdVec_.back() |= LAST_FLAG;
          fromIndexVec_.push_back(toIdVec_.size());
        }
      }

      void init(Id* fromIndices,
                size_t fromCount,
                Id* toIndices,
                size_t toCount){

        toIdVec_.clear();
        toIdVec_.resize(toCount);
        
        fromIndexVec_.clear();
        fromIndexVec_.resize(fromCount);

        std::copy(fromIndices, fromIndices + fromCount,
                  fromIndexVec_.begin());

        std::copy(toIndices, toIndices + toCount,
                  toIdVec_.begin());
      }
    
      void resize(IndexVec& numConns){
        clear();
      
        size_t n = numConns.size();
        fromIndexVec_.resize(n + 1);

        uint64_t size = 0;

        for(size_t i = 0; i < n; ++i){
          fromIndexVec_[i] = size;
          size += numConns[i];
        }

        fromIndexVec_[n] = size;
      
        toIdVec_.resize(size);
        std::fill(toIdVec_.begin(), toIdVec_.end(), 0);
      }
    
      void endFrom(){
        toIdVec_.back() |= LAST_FLAG;
        fromIndexVec_.push_back(toIdVec_.size());
      }
    
      void push(Id id){
        toIdVec_.push_back(id);
      }
    
      void dump(){
        std::cout << "=== idVec" << std::endl;
        for(Id id : toIdVec_){
          std::cout << (INDEX_MASK & id) << "(" << 
            (bool(id & LAST_FLAG)) << ")" << std::endl;
        }
      
        std::cout << "=== groupVec" << std::endl;
        for(Id id : fromIndexVec_){
          std::cout << id << std::endl;
        }
      }
    
      Id* getEntities(size_t index){
        assert(index < fromIndexVec_.size() - 1);
        return toIdVec_.data() + fromIndexVec_[index];
      }

      Id* getEntities(size_t index, size_t& endIndex){
        assert(index < fromIndexVec_.size() - 1);
        uint64_t start = fromIndexVec_[index];
        endIndex = fromIndexVec_[index + 1] - start;
        return toIdVec_.data() + start;
      }
        
      bool empty(){
        return toIdVec_.empty();
      }
    
      void set(size_t fromId, size_t toId, size_t pos){
        toIdVec_[fromIndexVec_[fromId] + pos] = toId;
      }

      void finishSet(){
        size_t n = fromIndexVec_.size();
        for(size_t i = 1; i < n; ++i){
          toIdVec_[fromIndexVec_[i] - 1] |= LAST_FLAG;
        }
      }
    
      size_t fromSize() const{
        return fromIndexVec_.size() - 1;
      }

      size_t toSize() const{
        return toIdVec_.size();
      }

      Id* rawIdVec(){
        return toIdVec_.data();
      }

      Id* rawFromIndexVec(){
        return fromIndexVec_.data();
      }

      Id* rawIdVec(size_t& size){
        size = toIdVec_.size();
        return toIdVec_.data();
      }

      Id* rawFromIndexVec(size_t& size){
        size = fromIndexVec_.size();
        return fromIndexVec_.data();
      }
    
      void set(ConnVec& conns){
        clear();
        
        size_t n = conns.size();      
        fromIndexVec_.resize(n + 1);
      
        size_t size = 0;

        for(size_t i = 0; i <= n; i++){
          fromIndexVec_[i] = size;
          size += conns[i].size();
        }
            
        toIdVec_.reserve(size);

        for(size_t i = 0; i < n; ++i){
          const IdVec& conn = conns[i];
          uint64_t m = conn.size();
          
          for(size_t j = 0; j < m; ++j){
            toIdVec_.push_back(conn[j]);
          }

          toIdVec_.back() |= LAST_FLAG;
        }
      }
    
      IdVec toIdVec_;
      IdVec fromIndexVec_;
    };

    Id* getToIndices(uint32_t fromDim,
                     uint32_t toDim){

      Connectivity& c = getConnectivity(fromDim, toDim);

      if(c.empty()){
        compute(fromDim, toDim);
      }

      return c.rawIdVec();
    }

    Id* getFromIndices(uint32_t fromDim,
                       uint32_t toDim){

      Connectivity& c = getConnectivity(fromDim, toDim);

      if(c.empty()){
        compute(fromDim, toDim);
      }

      return c.rawFromIndexVec();
    }

    virtual size_t numEntities(size_t dim) = 0;

    virtual void build(size_t dim) = 0;
    
    virtual void compute(size_t fromDim, size_t toDim) = 0;  

    virtual void computeAll() = 0;

    virtual size_t topologicalDimension() = 0;
  
    virtual Connectivity&
    getConnectivity(size_t fromDim, size_t toDim) = 0;
  };

  template<class MT>
  class MeshTopology : public MeshTopologyBase{
  public:
    using VertexType = 
      typename std::tuple_element<0, typename MT::EntityTypes>::type;
    
    using EdgeType = 
      typename std::tuple_element<1, typename MT::EntityTypes>::type;  

    using FaceType = 
      typename std::tuple_element<MT::topologicalDimension() - 1,
                                  typename MT::EntityTypes>::type;

    using CellType = 
      typename std::tuple_element<MT::topologicalDimension(),
                                  typename MT::EntityTypes>::type;

    class Entity{
    public:
      Entity(MeshTopology& mesh, size_t dim, size_t index=0)
        : mesh_(mesh),
          ents_(mesh_.getEntities_(dim)),
          dim_(dim),
          index_(index),
          endIndex_(mesh_.numEntities(dim_)){
        assert(index_ < endIndex_);
      }
    
      size_t dim(){
        return dim_;
      }
    
      size_t index(){
        return index_;
      }
    
      Id* getEntities(size_t dim){
        Connectivity& c = mesh_.getConnectivity_(dim_, dim);
        assert(!c.empty());
        return c.getEntities(index_);
      }
    
      Entity& operator++(){
        assert(index_ < endIndex_);
        ++index_;
        return *this;
      }
    
      bool end() const{
        return index_ >= endIndex_;
      }
    
      MeshTopology& mesh(){
        return mesh_;
      }
    
    protected:
      MeshTopology& mesh_;
      const std::vector<MeshEntity*>& ents_;
      size_t dim_;
      size_t index_;
      size_t endIndex_;
    };
  
    class EntityIterator{
    public:
      EntityIterator(Entity& entity, size_t dim, size_t index=0)
        : mesh_(entity.mesh()),
          ents_(mesh_.getEntities_(dim)),
          dim_(dim),
          index_(index){
        Connectivity& c = mesh_.getConnectivity_(entity.dim(), dim_);
        if(c.empty()){
          mesh_.compute(entity.dim(), dim_);
        }
      
        entities_ = c.getEntities(entity.index(), endIndex_);
        assert(index_ < endIndex_);
      }
    
      EntityIterator(EntityIterator& itr, size_t dim, size_t index=0)
        : mesh_(itr.mesh_),
          ents_(mesh_.getEntities_(dim)),
          dim_(dim),
          index_(index){
        Connectivity& c = mesh_.getConnectivity_(itr.dim_, dim_);
        if(c.empty()){
          mesh_.compute(itr.dim_, dim_);
        }
      
        entities_ = c.getEntities(itr.index_, endIndex_);
        assert(index_ < endIndex_);
      }
    
      size_t dim(){
        return dim_;
      }
    
      size_t index(){
        return entities_[index_] & INDEX_MASK;
      }
    
      Id* getEntities(size_t dim){
        Connectivity& c = mesh_.getConnectivity_(dim_, dim);
        assert(!c.empty());
        return c.getEntities(index_);
      }
    
      EntityIterator& operator++(){
        assert(index_ < endIndex_);
        ++index_;
        return *this;
      }
    
      bool end() const{
        return index_ >= endIndex_;
      }
    
    protected:
      MeshTopology& mesh_;
      const std::vector<MeshEntity*>& ents_;
      size_t dim_;
      size_t index_;
      size_t endIndex_;
      Id* entities_;
    };
  
    class Cell : public Entity{
    public:
      Cell(MeshTopology& mesh, size_t index=0)
        : Entity(mesh, MT::topologicalDimension(), index){}

      CellType& operator*(){
        return *static_cast<CellType*>(Entity::ents_[Entity::index_]);
      }
    };
  
    class CellIterator : public EntityIterator{
    public:
      CellIterator(Entity& entity, size_t index=0)
        : EntityIterator(entity, MT::topologicalDimension(), index){}

      CellType& operator*(){
        return *static_cast<CellType*>(
          EntityIterator::ents_[EntityIterator::index_]);
      }
    };
  
    class Vertex : public Entity{
    public:
      Vertex(MeshTopology& mesh, size_t index=0)
        : Entity(mesh, 0, index){}

      VertexType& operator*(){
        return *static_cast<VertexType*>(Entity::ents_[Entity::index_]);
      }
    };
  
    class VertexIterator : public EntityIterator{
    public:
      VertexIterator(Entity& entity, size_t index=0)
        : EntityIterator(entity, 0, index){}

      VertexType& operator*(){
        return *static_cast<CellType*>(
          EntityIterator::ents_[EntityIterator::index_]);
      }
    };
  
    class Edge : public Entity{
    public:
      Edge(MeshTopology& mesh, size_t index=0)
        : Entity(mesh, 1, index){}

      EdgeType& operator*(){
        return *static_cast<EdgeType*>(Entity::ents_[Entity::index_]);
      }
    };
  
    class EdgeIterator : public EntityIterator{
    public:
      EdgeIterator(Entity& entity, size_t index=0)
        : EntityIterator(entity, 1, index){}

      EdgeType& operator*(){
        return *static_cast<EdgeType*>(
          EntityIterator::ents_[EntityIterator::index_]);
      }
    };
  
    class Face : public Entity{
    public:
      Face(MeshTopology& mesh, size_t index=0)
        : Entity(mesh, MT::topologicalDimension() - 1, index){}

      FaceType& operator*(){
        return *static_cast<FaceType*>(Entity::ents_[Entity::index_]);
      }
    };
  
    class FaceIterator : public EntityIterator{
    public:
      FaceIterator(Entity& entity, size_t index=0)
        : EntityIterator(entity, MT::topologicalDimension() - 1, index){}

      FaceType& operator*(){
        return *static_cast<FaceType*>(
          EntityIterator::ents_[EntityIterator::index_]);
      }
    };
  
    MeshTopology(){
      getConnectivity_(MT::topologicalDimension(), 0).init();
    }
  
    void addVertex(VertexType* vertex){
      entities_[0].push_back(vertex);
    }
  
    void addCell(CellType* cell,
                 std::initializer_list<VertexType*> verts){

      assert(verts.size() == 
             MT::numVerticesPerEntity(MT::topologicalDimension()) &&
             "invalid number of vertices per cell");
    
      auto& c = getConnectivity_(MT::topologicalDimension(), 0);

      assert(cell->id() == c.fromSize() && "id mismatch"); 

      for(VertexType* v : verts){
        c.push(v->id());
      }
      c.endFrom();

      entities_[MT::topologicalDimension()].push_back(cell);
    }

    void addEdge(EdgeType* edge, VertexType* vertex1, VertexType* vertex2){
      auto& c = getConnectivity_(1, 0);
      if(c.empty()){
        c.init();
      }

      assert(edge->id() == c.fromSize() && "id mismatch"); 

      c.push(vertex1->id());
      c.push(vertex2->id());
      c.endFrom();

      entities_[1].push_back(edge);
    }

    void addFace(FaceType* face, std::initializer_list<VertexType*> verts){
      assert(verts.size() == 
             MT::numVerticesPerEntity(2) &&
             "invalid number vertices per face");

      auto& c = getConnectivity_(MT::topologicalDimension() - 1, 0);
      if(c.empty()){
        c.init();
      }

      assert(face->id() == c.fromSize() && "id mismatch"); 

      for(Vertex* v : verts){
        c.push(v->id());
      }
      c.endFrom();
      entities_[MT::topologicalDimension() - 1].push_back(face);
    }
    
    void addCellEdges(CellType* cell, std::initializer_list<EdgeType*> edges){
      assert(edges.size() == 
             MT::numEntitiesPerCell(1) &&
             "invalid number of edges per cell");

      auto& c = getConnectivity_(MT::topologicalDimension(), 1);
      if(c.empty()){
        c.init();
      }

      assert(cell->id() == c.fromSize() && "id mismatch"); 
      
      for(Edge* edge : edges){
        c.push(edge->id());
      }
      c.endFrom();
    }

    void addCellFaces(CellType* cell, std::initializer_list<FaceType*> faces){
      assert(faces.size() == 
             MT::numEntitiesPerCell(MT::topologicalDimension() - 1) &&
             "invalid number of face per cell");

      auto& c = getConnectivity_(MT::topologicalDimension(),
                                 MT::topologicalDimension() - 1);
      if(c.empty()){
        c.init();
      }

      assert(cell->id() == c.fromSize() && "id mismatch"); 
      
      for(Face* face : faces){
        c.push(face->id());
      }
      c.endFrom();
    }

    size_t numEntities(size_t dim) override{
      size_t size = entities_[dim].size();
      if(size == 0){
        build(dim);
        return entities_[dim].size();
      }

      return size;
    }

    void build(size_t dim) override{
      //std::cerr << "build: " << dim << std::endl;

      assert(dim <= MT::topologicalDimension());

      size_t verticesPerEntity = MT::numVerticesPerEntity(dim);
      size_t entitiesPerCell =  MT::numEntitiesPerCell(dim);
    
      Connectivity& entityToVertex = getConnectivity_(dim, 0);
    
      IdVec entityVertices(entitiesPerCell * verticesPerEntity);

      Connectivity& cellToEntity =
        getConnectivity_(MT::topologicalDimension(), dim);
    
      ConnVec entityVertexConn;

      size_t entityId = 0;
      size_t maxCellEntityConns = 1;

      Connectivity& cellToVertex =
        getConnectivity_(MT::topologicalDimension(), 0);
      assert(!cellToVertex.empty());

      size_t n = numCells();

      ConnVec cellEntityConn(n);

      IdVecMap entityVerticesMap(n * MT::numEntitiesPerCell(dim)/2);
    
      for(size_t c = 0; c < n; ++c){
        IdVec& conns = cellEntityConn[c]; 
        
        conns.reserve(maxCellEntityConns);
      
        Id* vertices = cellToVertex.getEntities(c);
      
        MT::createEntities(dim, entityVertices, vertices);

        for(size_t i = 0; i < entitiesPerCell; ++i){
          Id* a = &entityVertices[i * verticesPerEntity];
          IdVec ev(a, a + verticesPerEntity);
          std::sort(ev.begin(), ev.end());
          ev.back() |= LAST_FLAG;

          auto itr = entityVerticesMap.emplace(std::move(ev), entityId);
          conns.emplace_back(itr.first->second);
        
          if(itr.second){
            IdVec ev2 = IdVec(a, a + verticesPerEntity);
            ev2.back() |= LAST_FLAG;

            entityVertexConn.emplace_back(std::move(ev2));
          
            maxCellEntityConns =
              std::max(maxCellEntityConns, cellEntityConn[c].size());
          
            ++entityId;
          }
        }
      }

      cellToEntity.init(cellEntityConn);
      entityToVertex.init(entityVertexConn);

      Id* ids = cellToEntity.rawIdVec();
      size_t m = cellToEntity.toSize();
      entities_[dim].reserve(n);

      for(size_t i = 0; i < m; ++i){
        MeshEntity* ent = 
          CreateEntity<MT::topologicalDimension(), MT>::create(dim, ids[i]);

        entities_[dim].push_back(ent);
      }
    }
  
    void transpose(size_t fromDim, size_t toDim){
      //std::cerr << "transpose: " << fromDim << " -> " << 
      //   toDim << std::endl;
    
      IndexVec pos(numEntities(fromDim), 0);
    
      for(Entity toEntity(*this, toDim); !toEntity.end(); ++toEntity){
        for(EntityIterator fromItr(toEntity, fromDim); 
            !fromItr.end(); ++fromItr){
          pos[fromItr.index()]++;
        }
      }
    
      Connectivity& outConn = getConnectivity_(fromDim, toDim);
      outConn.resize(pos);

      std::fill(pos.begin(), pos.end(), 0);
    
      for(Entity toEntity(*this, toDim); !toEntity.end(); ++toEntity){
        for(EntityIterator fromItr(toEntity, fromDim); 
            !fromItr.end(); ++fromItr){
          outConn.set(fromItr.index(), toEntity.index(), 
                      pos[fromItr.index()]++);
        }
      }
      
      outConn.finishSet();
    }
  
    void intersect(size_t fromDim, size_t toDim, size_t dim){
      //std::cerr << "intersect: " << fromDim << " -> " << 
      //  toDim << std::endl;

      Connectivity& outConn = getConnectivity_(fromDim, toDim);
      if(!outConn.empty()){
        return;
      }
    
      ConnVec conns(numEntities(fromDim));
    
      using VisitedVec = std::vector<bool>;
      VisitedVec visited(numEntities(fromDim));
    
      IdVec fromVerts(MT::numVerticesPerEntity(fromDim));
      IdVec toVerts(MT::numVerticesPerEntity(toDim));

      size_t maxSize = 1;    

      for(Entity fromEntity(*this, fromDim); 
          !fromEntity.end(); ++fromEntity){
        IdVec& entities = conns[fromEntity.index()];
        entities.reserve(maxSize);

        Id* ep = fromEntity.getEntities(0);

        std::copy(ep, ep + MT::numVerticesPerEntity(fromDim),
                  fromVerts.begin());
      
        std::sort(fromVerts.begin(), fromVerts.end());
      
        for(EntityIterator fromItr(fromEntity, dim);
            !fromItr.end(); ++fromItr){
          for(EntityIterator toItr(fromItr, toDim);
              !toItr.end(); ++toItr){
            visited[toItr.index()] = false;
          }
        }
      
        for(EntityIterator fromItr(fromEntity, dim);
            !fromItr.end(); ++fromItr){
          for(EntityIterator toItr(fromItr, toDim);
              !toItr.end(); ++toItr){
            if(visited[toItr.index()]){
              continue;
            }
          
            visited[toItr.index()] = true;
          
            if(fromDim == toDim){
              if(fromEntity.index() != toItr.index()){
                entities.push_back(toItr.index());
              }
            }
            else{
              Id* ep = toItr.getEntities(0);

              std::copy(ep, ep + MT::numVerticesPerEntity(toDim),
                        toVerts.begin());
            
              std::sort(toVerts.begin(), toVerts.end());
            
              if(std::includes(fromVerts.begin(), fromVerts.end(),
                               toVerts.begin(), toVerts.end())){
              
                entities.emplace_back(toItr.index());
              }
            }
          }
        }
      
        maxSize = std::max(entities.size(), maxSize);
      }
    
      outConn.init(conns);
    }
  
    void compute(size_t fromDim, size_t toDim) override{
      //std::cerr << "compute: " << fromDim << " -> " << toDim << std::endl;

      Connectivity& outConn = getConnectivity_(fromDim, toDim);
    
      if(!outConn.empty()){
        return;
      }
    
      if(numEntities(fromDim) == 0){
        build(fromDim);
      }
    
      if(numEntities(toDim) == 0){
        build(toDim);
      }
    
      if(numEntities(fromDim) == 0 && numEntities(toDim) == 0){
        return;
      }
    
      if(fromDim == toDim){
        ConnVec connVec(numEntities(fromDim), IdVec(1));
      
        for(Entity entity(*this, fromDim); !entity.end(); ++entity){
          connVec[entity.index()][0] = entity.index();
        }
      
        outConn.set(connVec);
      }
      else if(fromDim < toDim){
        compute(toDim, fromDim);
        transpose(fromDim, toDim);
      }
      else{
        compute(fromDim, 0);
        compute(0, toDim);
        intersect(fromDim, toDim, 0);
      }
    }
    
    void computeAll() override{
      int d = MT::topologicalDimension();
      for(int i = d; i >= 0; --i){
        for(int j = 0; j <= d; ++j){
          if(i != j){
            compute(i, j);
          }
        }
      }
    }

    size_t numCells(){
      return entities_[MT::topologicalDimension()].size();
    }
  
    size_t numVertices(){
      return entities_[0].size();
    }
  
    size_t numEdges(){
      return entities_[1].size();
    }
  
    size_t numFaces(){
      return entities_[MT::topologicalDimension() - 1].size();
    }

    Connectivity& getConnectivity(size_t fromDim, size_t toDim) override{
      return getConnectivity_(fromDim, toDim);
    }

    Connectivity& getConnectivity_(size_t fromDim, size_t toDim){
      assert(fromDim < topology_.size() && "invalid fromDim");
      auto& t = topology_[fromDim];
      assert(toDim < t.size() && "invalid toDim");
      return t[toDim];
    }

    size_t topologicalDimension() override{
      return MT::topologicalDimension();
    }  

    const std::vector<MeshEntity*>& getEntities_(size_t dim) const{
      return entities_[dim];
    }

    void dump(){
      for(size_t i = 0; i < topology_.size(); ++i){
        auto& ci = topology_[i];
        for(size_t j = 0; j < ci.size(); ++j){
          auto& cj = ci[j];
          std::cout << "------------- " << i << " -> " << j << std::endl;
          cj.dump();
        }
      }
    }

 
  private:
    using Topology_ =
      std::array<std::array<Connectivity, MT::topologicalDimension() + 1>,
      MT::topologicalDimension() + 1>;

    using Entities_ = 
      std::array<std::vector<MeshEntity*>, MT::topologicalDimension() + 1>;

    Topology_ topology_;
    Entities_ entities_;
  };

} // jali

#endif // __MESH_TOPOLOGY_H__
