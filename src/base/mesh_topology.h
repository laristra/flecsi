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

namespace scout{

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

    size_t numEntities(size_t dim){
      size_t size = size_[dim];
      if(size == 0){
        build(dim);
        return size_[dim];
      }

      return size;
    }

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

    virtual void build(size_t dim) = 0;
    
    virtual void compute(size_t fromDim, size_t toDim) = 0;  

    virtual void computeAll() = 0;

    virtual void setConnectivityRaw(size_t fromDim,
                                    size_t toDim,
                                    Id* fromIndices,
                                    size_t fromCount,
                                    Id* toIds,
                                    size_t toCount) = 0;

    virtual void getConnectivityRaw(size_t fromDim,
                                    size_t toDim,
                                    Id*& fromIndices,
                                    size_t& fromCount,
                                    Id*& toIds,
                                    size_t& toCount) = 0;

    virtual size_t topologicalDimension() = 0;
  
    virtual Connectivity&
    getConnectivity(size_t fromDim, size_t toDim) = 0;

  protected:  
    std::vector<size_t> size_;  
  };

  template<class MT>
  class MeshTopology : public MeshTopologyBase{
  public:

    using Float = typename MT::Float;
      
    class Coordinate{
    public:
      Coordinate(){}
    
      Coordinate(std::initializer_list<Float> il){
        static_assert(il.size() == MT::geometricDimension(),
                      "coordinate size mismatch");
        std::copy(il.begin(), il.end(), coordinate_);
      }
    
      template<size_t I>
      Float get(){
        return coordinate_[I];
      }
    
      template<size_t I>
      void set(Float value){
        coordinate_[I] = value;
      }
    
      Coordinate& operator=(std::initializer_list<Float> il){
        size_t i = 0;
        for(Float v : il){
          coordinate_[i++] = v;
        }
      
        return *this;
      }
    
    private:
      using Coordinate_ = std::array<Float, MT::geometricDimension()>;
    
      Coordinate_ coordinate_;
    };
    
    class Geometry{
    public:
      Geometry(){}
    
      void addVertex(Id id, std::initializer_list<Float> il){
        if(id >= coordinates_.size()){
          coordinates_.resize(id + 1);
        }
      
        coordinates_[id] = il;
      }
    
    private:
      using Coordinates_ = std::vector<Coordinate>;
    
      Coordinates_ coordinates_;
    };
  
    class Entity{
    public:
      Entity(MeshTopology& mesh, size_t dim, size_t index=0)
        : mesh_(mesh),
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
    
    private:
      MeshTopology& mesh_;
      size_t dim_;
      size_t index_;
      size_t endIndex_;
    };
  
    class EntityIterator{
    public:
      EntityIterator(Entity& entity, size_t dim, size_t index=0)
        : mesh_(entity.mesh()),
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
    
    private:
      MeshTopology& mesh_;
      size_t dim_;
      size_t index_;
      size_t endIndex_;
      Id* entities_;
    };
  
    class Cell : public Entity{
    public:
      Cell(MeshTopology& mesh, size_t index=0)
        : Entity(mesh, MT::topologicalDimension(), index){}
    };
  
    class CellIterator : public EntityIterator{
    public:
      CellIterator(Entity& entity, size_t index=0)
        : EntityIterator(entity, MT::topologicalDimension(), index){}
    };
  
    class Vertex : public Entity{
    public:
      Vertex(MeshTopology& mesh, size_t index=0)
        : Entity(mesh, 0, index){}
    };
  
    class VertexIterator : public EntityIterator{
    public:
      VertexIterator(Entity& entity, size_t index=0)
        : EntityIterator(entity, 0, index){}
    };
  
    class Edge : public Entity{
    public:
      Edge(MeshTopology& mesh, size_t index=0)
        : Entity(mesh, 1, index){}
    };
  
    class EdgeIterator : public EntityIterator{
    public:
      EdgeIterator(Entity& entity, size_t index=0)
        : EntityIterator(entity, 1, index){}
    };
  
    class Face : public Entity{
    public:
      Face(MeshTopology& mesh, size_t index=0)
        : Entity(mesh, MT::topologicalDimension() - 1, index){}
    };
  
    class FaceIterator : public EntityIterator{
    public:
      FaceIterator(Entity& entity, size_t index=0)
        : EntityIterator(entity, MT::topologicalDimension() - 1, index){}
    };
  
    MeshTopology(){
      getConnectivity_(MT::topologicalDimension(), 0).init();
      for(size_t i = 0; i <= MT::topologicalDimension(); ++i){
        size_.push_back(0);
      }
    }
  
    void addVertex(Id id, std::initializer_list<Float> coord){
      geometry_.addVertex(id, coord);
      ++size_[0];
    }
  
    void addCell(Id cellId, std::initializer_list<Id> verts){
      assert(verts.size() == 
             MT::numVerticesPerEntity(MT::topologicalDimension()) &&
             "invalid number of vertices per cell");
    
      auto& c = getConnectivity_(MT::topologicalDimension(), 0);

      assert(cellId == c.fromSize() && "id mismatch"); 

      for(Id id : verts){
        c.push(id);
      }
      c.endFrom();
      ++size_[MT::topologicalDimension()];
    }

    void addEdge(Id edgeId, Id vertex1, Id vertex2){
      auto& c = getConnectivity_(1, 0);
      if(c.empty()){
        c.init();
      }

      assert(edgeId == c.fromSize() && "id mismatch"); 

      c.push(vertex1);
      c.push(vertex2);
      c.endFrom();
      ++size_[1];
    }

    void addFace(Id faceId, std::initializer_list<Id> verts){
      assert(verts.size() == 
             MT::numVerticesPerEntity(2) &&
             "invalid number vertices per face");

      auto& c = getConnectivity_(MT::topologicalDimension() - 1, 0);
      if(c.empty()){
        c.init();
      }

      assert(faceId == c.fromSize() && "id mismatch"); 

      for(Id id : verts){
        c.push(id);
      }
      c.endFrom();
      ++size_[MT::topologicalDimension() - 1];
    }
    
    void addCellEdges(Id cellId, std::initializer_list<Id> edges){
      assert(edges.size() == 
             MT::numEntitiesPerCell(1) &&
             "invalid number of edges per cell");

      auto& c = getConnectivity_(MT::topologicalDimension(), 1);
      if(c.empty()){
        c.init();
      }

      assert(cellId == c.fromSize() && "id mismatch"); 
      
      for(Id id : edges){
        c.push(id);
      }
      c.endFrom();
    }

    void addCellFaces(Id cellId, std::initializer_list<Id> faces){
      assert(faces.size() == 
             MT::numEntitiesPerCell(MT::topologicalDimension() - 1) &&
             "invalid number of face per cell");

      auto& c = getConnectivity_(MT::topologicalDimension(),
                                 MT::topologicalDimension() - 1);
      if(c.empty()){
        c.init();
      }

      assert(cellId == c.fromSize() && "id mismatch"); 
      
      for(Id id : faces){
        c.push(id);
      }
      c.endFrom();
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

      size_[dim] = entityToVertex.fromSize();
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
      return size_[MT::topologicalDimension()];
    }
  
    size_t numVertices(){
      return size_[0];
    }
  
    size_t numEdges(){
      return size_[1];
    }
  
    size_t numFaces(){
      return size_[MT::topologicalDimension() - 1];
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

    void setConnectivityRaw_(size_t fromDim,
                             size_t toDim,
                             Id* fromIndices,
                             size_t fromCount,
                             Id* toIds,
                             size_t toCount){
      Connectivity& c = getConnectivity_(fromDim, toDim);
      c.init(fromIndices, fromCount, toIds, toCount);

      size_[fromDim] = fromCount - 1;
      size_[toDim] = toCount;
    }

    void setConnectivityRaw(size_t fromDim,
                            size_t toDim,
                            Id* fromIndices,
                            size_t fromCount,
                            Id* toIds,
                            size_t toCount) override{
      setConnectivityRaw_(fromDim, toDim, fromIndices,
                          fromCount, toIds, toCount);
    }

    void getConnectivityRaw(size_t fromDim,
                            size_t toDim,
                            Id*& fromIndices,
                            size_t& fromCount,
                            Id*& toIds,
                            size_t& toCount) override{
      getConnectivityRaw_(fromDim, toDim, fromIndices,
                          fromCount, toIds, toCount);
    }

    void getConnectivityRaw_(size_t fromDim,
                             size_t toDim,
                             Id*& fromIndices,
                             size_t& fromCount,
                             Id*& toIds,
                             size_t& toCount){
      Connectivity& c = getConnectivity_(fromDim, toDim);
      fromIndices = c.rawFromIndexVec(fromCount);
      toIds = c.rawIdVec(toCount);
    }

    size_t topologicalDimension() override{
      return MT::topologicalDimension();
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
    
    Topology_ topology_;
    Geometry geometry_;
  };

  class UniformMesh1dType{
  public:
    using Id = uint64_t;
    using Float = double;

    static constexpr size_t topologicalDimension(){
      return 1;
    }

    static constexpr size_t geometricDimension(){
      return 1;
    }

    static size_t numEntitiesPerCell(size_t dim){
      switch(dim){
      case 0:
        return 2;
      case 1:
        return 1;
      default:
        assert(false && "invalid dimension");
      }
    }

    static constexpr size_t verticesPerCell(){
      return 2;
    }
  
    static size_t numVerticesPerEntity(size_t dim){
      switch(dim){
      case 0:
        return 1;
      case 1:
        return 2;
      default:
        assert(false && "invalid dimension");
      }
    }
  
    static void createEntities(size_t dim, std::vector<Id>& e, Id* v){
      assert(false && "nothing to build");
    }
  
  };

  class UniformMesh2dType{
  public:
    using Id = uint64_t;
    using Float = double;

    static constexpr size_t topologicalDimension(){
      return 2;
    }

    static constexpr size_t geometricDimension(){
      return 2;
    }

    static size_t numEntitiesPerCell(size_t dim){
      switch(dim){
      case 0:
        return 4;
      case 1:
        return 4;
      case 2:
        return 1;
      default:
        assert(false && "invalid dimension");
      }
    }

    static constexpr size_t verticesPerCell(){
      return 4;
    }
  
    static size_t numVerticesPerEntity(size_t dim){
      switch(dim){
      case 0:
        return 1;
      case 1:
        return 2;
      case 2:
        return 4;
      default:
        assert(false && "invalid dimension");
      }
    }
  
    static void createEntities(size_t dim, std::vector<Id>& e, Id* v){
      assert(dim = 1);
      assert(e.size() == 8);
    
      e[0] = v[0];
      e[1] = v[2];

      e[2] = v[1];
      e[3] = v[3];
    
      e[4] = v[0];
      e[5] = v[1];
    
      e[6] = v[2];
      e[7] = v[3];
    }
  
  };

  class UniformMesh3dType{
  public:
    using Id = uint64_t;
    using Float = double;

    static constexpr size_t topologicalDimension(){
      return 3;
    }

    static constexpr size_t geometricDimension(){
      return 3;
    }

    static size_t numEntitiesPerCell(size_t dim){
      switch(dim){
      case 0:
        return 8;
      case 1:
        return 12;
      case 2:
        return 6;
      default:
        assert(false && "invalid dimension");
      }
    }

    static constexpr size_t verticesPerCell(){
      return 4;
    }
  
    static size_t numVerticesPerEntity(size_t dim){
      switch(dim){
      case 0:
        return 1;
      case 1:
        return 2;
      case 2:
        return 4;
      case 3:
        return 8;
      default:
        assert(false && "invalid dimension");
      }
    }
  
    static void createEntities(size_t dim, std::vector<Id>& e, Id* v){
      if(dim == 1){
        assert(e.size() == 24);
    
        e[0] = v[0];
        e[1] = v[2];

        e[2] = v[1];
        e[3] = v[3];
    
        e[4] = v[0];
        e[5] = v[1];
    
        e[6] = v[2];
        e[7] = v[3];

        e[8] = v[4];
        e[9] = v[6];

        e[10] = v[5];
        e[11] = v[7];

        e[12] = v[4];
        e[13] = v[5];
    
        e[14] = v[6];
        e[15] = v[7];

        e[16] = v[1];
        e[17] = v[5];

        e[18] = v[3];
        e[19] = v[7];

        e[20] = v[0];
        e[21] = v[4];

        e[22] = v[2];
        e[23] = v[6];
      }
      else if(dim == 2){
        assert(e.size() == 24);

        e[0] = v[0];
        e[1] = v[1];
        e[2] = v[3];
        e[3] = v[2];

        e[4] = v[4];
        e[5] = v[5];
        e[6] = v[7];
        e[7] = v[6];

        e[8] = v[1];
        e[9] = v[5];
        e[10] = v[7];
        e[11] = v[3];

        e[12] = v[0];
        e[13] = v[4];
        e[14] = v[6];
        e[15] = v[2];

        e[16] = v[0];
        e[17] = v[1];
        e[18] = v[5];
        e[19] = v[4];

        e[20] = v[2];
        e[21] = v[3];
        e[22] = v[7];
        e[23] = v[6];
      }
      else{
        assert(false && "invalid dim");
      }
    }
  
  };

} // scout

#endif // __MESH_TOPOLOGY_H__
