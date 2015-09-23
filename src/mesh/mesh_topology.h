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

namespace flexi{

  class MeshEntity{
  public:
    MeshEntity(size_t id)
      : id_(id){}

    size_t id() const{
      return id_;
    }

    static constexpr size_t getDim_(size_t meshDim, size_t dim){
      return dim > meshDim ? meshDim : dim;
    }

    template<class MT>
    static MeshEntity* create_(size_t dim, size_t id){
      switch(dim){
      case 0:{
        using EntityType = 
          typename std::tuple_element<getDim_(MT::Dimension, 0),
                                      typename MT::EntityTypes>::type;
        return new EntityType(id);
      }
      case 1:{
        using EntityType = 
          typename std::tuple_element<getDim_(MT::Dimension, 1),
                                      typename MT::EntityTypes>::type;
        return new EntityType(id);
      }
      case 2:{
        using EntityType = 
          typename std::tuple_element<getDim_(MT::Dimension, 2),
                                      typename MT::EntityTypes>::type;
        return new EntityType(id);
      }
      case 3:{
        using EntityType = 
          typename std::tuple_element<getDim_(MT::Dimension, 3),
                                      typename MT::EntityTypes>::type;
        return new EntityType(id);
      }
      default:
        assert(false && "invalid topology dim");
      }
    }

  private:
    size_t id_;
  };

  using EntityVec = std::vector<MeshEntity*>;

  template<size_t D>
  class EntityGroup{
  public:
    EntityGroup(){}
    
    void add(MeshEntity* ent){
      entities_.push_back(ent);
    }
    
    const EntityVec& getEntities() const{
      return entities_;
    }

    static constexpr size_t dim(){
      return D;
    }
    
  private:
    EntityVec entities_;
  };
    
  class MeshBase{
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
        entityVec_.clear();
        fromIndexVec_.clear();
      }
    
      void init(){
        fromIndexVec_.push_back(0);
      }
    
      void init(EntityVec& ev, const ConnVec& cv){
        assert(entityVec_.empty() && fromIndexVec_.empty());
      
        fromIndexVec_.push_back(0);
      
        size_t n = cv.size();
      
        for(size_t i = 0; i < n; ++i){
          const IdVec& iv = cv[i];
          
          for(Id id : iv){
            entityVec_.push_back(ev[id]);
          }
          
          fromIndexVec_.push_back(entityVec_.size());
        }
      }

      template<class MT>
      void initCreate(EntityVec& ev, const ConnVec& cv, size_t dim){
        assert(entityVec_.empty() && fromIndexVec_.empty());
      
        fromIndexVec_.push_back(0);
      
        size_t n = cv.size();

        Id maxId = 0;

        for(size_t i = 0; i < n; ++i){
          const IdVec& iv = cv[i];
          
          for(Id id : iv){
            maxId = std::max(maxId, id);
            entityVec_.push_back(MeshEntity::create_<MT>(dim, id));
          }
          
          fromIndexVec_.push_back(entityVec_.size());
        }

        size_t m = entityVec_.size();
        ev.resize(maxId + 1);

        for(MeshEntity* ent : entityVec_){
          ev[ent->id()] = ent;
        }
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
      
        entityVec_.resize(size);
        std::fill(entityVec_.begin(), entityVec_.end(), nullptr);
      }
    
      void endFrom(){
        fromIndexVec_.push_back(entityVec_.size());
      }
    
      void push(MeshEntity* ent){
        entityVec_.push_back(ent);
      }
    
      void dump(){
        std::cout << "=== idVec" << std::endl;
        for(MeshEntity* ent : entityVec_){
          std::cout << ent->id() << std::endl;
        }
      
        std::cout << "=== groupVec" << std::endl;
        for(Id id : fromIndexVec_){
          std::cout << id << std::endl;
        }
      }

      const EntityVec& getEntities() const{
        return entityVec_;
      }

      const IdVec& getFromIndexVec() const{
        return fromIndexVec_;
      }
    
      MeshEntity** getEntities(size_t index){
        assert(index < fromIndexVec_.size() - 1);
        return entityVec_.data() + fromIndexVec_[index];
      }

      MeshEntity** getEntities(size_t index, size_t& endIndex){
        assert(index < fromIndexVec_.size() - 1);
        uint64_t start = fromIndexVec_[index];
        endIndex = fromIndexVec_[index + 1] - start;
        return entityVec_.data() + start;
      }
        
      bool empty(){
        return entityVec_.empty();
      }
    
      void set(size_t fromId, MeshEntity* ent, size_t pos){
        entityVec_[fromIndexVec_[fromId] + pos] = ent;
      }
    
      size_t fromSize() const{
        return fromIndexVec_.size() - 1;
      }

      size_t toSize() const{
        return entityVec_.size();
      }
    
      void set(EntityVec& ev, ConnVec& conns){
        clear();
        
        size_t n = conns.size();      
        fromIndexVec_.resize(n + 1);
      
        size_t size = 0;

        for(size_t i = 0; i < n; i++){
          fromIndexVec_[i] = size;
          size += conns[i].size();
        }

        fromIndexVec_[n] = size;

        entityVec_.reserve(size);

        for(size_t i = 0; i < n; ++i){
          const IdVec& conn = conns[i];
          uint64_t m = conn.size();
          
          for(size_t j = 0; j < m; ++j){
            entityVec_.push_back(ev[conn[j]]);
          }
        }
      }
    
      EntityVec entityVec_;
      IdVec fromIndexVec_;
    };

    virtual size_t numEntities(size_t dim) = 0;

    virtual void build(size_t dim) = 0;
    
    virtual void compute(size_t fromDim, size_t toDim) = 0;  

    virtual void computeAll() = 0;

    virtual size_t topologicalDimension() = 0;
  
    virtual Connectivity&
    getConnectivity(size_t fromDim, size_t toDim) = 0;
  };

  template<class MT>
  class Mesh : public MeshBase{
  public:
    using VertexType = 
      typename std::tuple_element<0, typename MT::EntityTypes>::type;
    
    using EdgeType = 
      typename std::tuple_element<1, typename MT::EntityTypes>::type;  

    using FaceType = 
      typename std::tuple_element<MT::Dimension - 1,
                                  typename MT::EntityTypes>::type;

    using CellType = 
      typename std::tuple_element<MT::Dimension,
                                  typename MT::EntityTypes>::type;

    class Iterator{
    public:
      Iterator(Mesh& mesh, size_t dim)
        : mesh_(mesh),
          entities_(&mesh.getEntities_(dim)),
          dim_(dim),
          index_(0),
          endIndex_(entities_->size()),
          level_(0){}

      template<class M, size_t D>
      Iterator(M& mesh, EntityGroup<D>& group)
        : mesh_(mesh),
          entities_(&group.getEntities()),
          dim_(D),
          index_(0),
          endIndex_(entities_->size()),
          level_(0){}

      Iterator(Iterator& itr, size_t dim)
        : mesh_(itr.mesh_), 
          dim_(dim),
          level_(itr.level_ + 1){
        
        Connectivity& c = mesh_.getConnectivity(itr.dim_, dim_);
        if(c.empty()){
          mesh_.compute(itr.dim_, dim_);
        }

        entities_ = &c.getEntities();

        const IdVec& fv = c.getFromIndexVec();
        if(level_ > 1){
          size_t id = (*itr.entities_)[itr.index_]->id();

          index_ = fv[id];
          endIndex_ = fv[id + 1];
        }
        else{
          index_ = fv[itr.index_];
          endIndex_ = fv[itr.index_ + 1];          
        }
      }
      
      MeshEntity& get(){
        return *(*entities_)[index_];
      }

		EntityVec::const_iterator begin() const {
			return entities_->begin();
		} // auto

		EntityVec::const_iterator end() const {
			return entities_->end();
		} // auto

      bool isend() const{
        return index_ >= endIndex_;
      }

      Iterator& operator++(){
        assert(index_ < endIndex_);
        ++index_;
        return *this;
      }

      size_t id() const{
        return (*entities_)[index_]->id();
      }

      MeshEntity** getEntities(size_t dim){
        Connectivity& c = mesh_.getConnectivity_(dim_, dim);
        assert(!c.empty());
        return c.getEntities(index_);
      }

    private:

      Mesh& mesh_;
      const EntityVec* entities_;
      size_t dim_;
      size_t index_;
      size_t endIndex_;
      size_t level_;
    };

    template<size_t D>
    class EntityIterator : public Iterator{
    public:
      using EntityType = 
        typename std::tuple_element<D, typename MT::EntityTypes>::type;

      EntityIterator(Mesh& mesh)
        : Iterator(mesh, D){}

      EntityIterator(Iterator& itr)
        : Iterator(itr, D){}

      template<class M, size_t TD>
      EntityIterator(M& mesh, EntityGroup<TD>& group)
        : Iterator(mesh, group){}

      EntityType& operator*(){
        return static_cast<EntityType&>(Iterator::get());
      }

      EntityType* operator->(){
        return &static_cast<EntityType&>(Iterator::get());
      }
    };

    using VertexIterator = EntityIterator<0>;
    using EdgeIterator = EntityIterator<1>;
    using FaceIterator = EntityIterator<MT::Dimension - 1>;
    using CellIterator = EntityIterator<MT::Dimension>;

    Mesh(){
      getConnectivity_(MT::Dimension, 0).init();
    }
  
    void addVertex(VertexType* vertex){
      entities_[0].push_back(vertex);
    }
  
    void addCell(CellType* cell,
                 std::initializer_list<VertexType*> verts){

      assert(verts.size() == 
             MT::numVerticesPerEntity(MT::Dimension) &&
             "invalid number of vertices per cell");
    
      auto& c = getConnectivity_(MT::Dimension, 0);

      assert(cell->id() == c.fromSize() && "id mismatch"); 

      for(VertexType* v : verts){
        c.push(v);
      }

      c.endFrom();

      entities_[MT::Dimension].push_back(cell);
    }

    void addEdge(EdgeType* edge, VertexType* vertex1, VertexType* vertex2){
      auto& c = getConnectivity_(1, 0);
      if(c.empty()){
        c.init();
      }

      assert(edge->id() == c.fromSize() && "id mismatch"); 

      c.push(vertex1);
      c.push(vertex2);

      c.endFrom();

      entities_[1].push_back(edge);
    }

    void addFace(FaceType* face, std::initializer_list<VertexType*> verts){
      assert(verts.size() == 
             MT::numVerticesPerEntity(2) &&
             "invalid number vertices per face");

      auto& c = getConnectivity_(MT::Dimension - 1, 0);
      if(c.empty()){
        c.init();
      }

      assert(face->id() == c.fromSize() && "id mismatch"); 

      for(VertexType* v : verts){
        c.push(v);
      }

      c.endFrom();
      entities_[MT::Dimension - 1].push_back(face);
    }
    
    void addCellEdges(CellType* cell, std::initializer_list<EdgeType*> edges){
      assert(edges.size() == 
             MT::numEntitiesPerCell(1) &&
             "invalid number of edges per cell");

      auto& c = getConnectivity_(MT::Dimension, 1);
      if(c.empty()){
        c.init();
      }

      assert(cell->id() == c.fromSize() && "id mismatch"); 
      
      for(EdgeType* edge : edges){
        c.push(edge);
      }

      c.endFrom();
    }

    void addCellFaces(CellType* cell, std::initializer_list<FaceType*> faces){
      assert(faces.size() == 
             MT::numEntitiesPerCell(MT::Dimension - 1) &&
             "invalid number of face per cell");

      auto& c = getConnectivity_(MT::Dimension,
                                 MT::Dimension - 1);
      if(c.empty()){
        c.init();
      }

      assert(cell->id() == c.fromSize() && "id mismatch"); 
      
      for(FaceType* face : faces){
        c.push(face);
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

    size_t numEntities_(size_t dim){
      return entities_[dim].size();
    }

    void build(size_t dim) override{
      //std::cerr << "build: " << dim << std::endl;

      assert(dim <= MT::Dimension);

      size_t verticesPerEntity = MT::numVerticesPerEntity(dim);
      size_t entitiesPerCell =  MT::numEntitiesPerCell(dim);
    
      Connectivity& entityToVertex = getConnectivity_(dim, 0);
    
      IdVec entityVertices(entitiesPerCell * verticesPerEntity);

      Connectivity& cellToEntity =
        getConnectivity_(MT::Dimension, dim);

      ConnVec entityVertexConn;

      size_t entityId = 0;
      size_t maxCellEntityConns = 1;

      Connectivity& cellToVertex =
        getConnectivity_(MT::Dimension, 0);
      assert(!cellToVertex.empty());

      size_t n = numCells();

      ConnVec cellEntityConn(n);

      IdVecMap entityVerticesMap(n * MT::numEntitiesPerCell(dim)/2);
    
      for(size_t c = 0; c < n; ++c){
        IdVec& conns = cellEntityConn[c]; 
        
        conns.reserve(maxCellEntityConns);
      
        VertexType** vertices = 
          reinterpret_cast<VertexType**>(cellToVertex.getEntities(c));
      
        MT::createEntities(dim, entityVertices, vertices);

        for(size_t i = 0; i < entitiesPerCell; ++i){
          Id* a = &entityVertices[i * verticesPerEntity];
          IdVec ev(a, a + verticesPerEntity);
          std::sort(ev.begin(), ev.end());

          auto itr = entityVerticesMap.emplace(std::move(ev), entityId);
          conns.emplace_back(itr.first->second);
        
          if(itr.second){
            IdVec ev2 = IdVec(a, a + verticesPerEntity);

            entityVertexConn.emplace_back(std::move(ev2));
          
            maxCellEntityConns =
              std::max(maxCellEntityConns, cellEntityConn[c].size());
          
            ++entityId;
          }
        }
      }

      cellToEntity.initCreate<MT>(entities_[dim], cellEntityConn, dim);
      entityToVertex.init(entities_[0], entityVertexConn);
    }
  
    void transpose(size_t fromDim, size_t toDim){
      //std::cerr << "transpose: " << fromDim << " -> " << 
      //   toDim << std::endl;
    
      IndexVec pos(numEntities_(fromDim), 0);
    
      for(Iterator toEntity(*this, toDim); !toEntity.isend(); ++toEntity){
        for(Iterator fromItr(toEntity, fromDim); 
            !fromItr.isend(); ++fromItr){
          pos[fromItr.id()]++;
        }
      }
    
      Connectivity& outConn = getConnectivity_(fromDim, toDim);
      outConn.resize(pos);

      std::fill(pos.begin(), pos.end(), 0);
    
      for(Iterator toEntity(*this, toDim); !toEntity.isend(); ++toEntity){
        for(Iterator fromItr(toEntity, fromDim); 
            !fromItr.isend(); ++fromItr){
          outConn.set(fromItr.id(), &toEntity.get(), 
                      pos[fromItr.id()]++);
        }
      }
    }
  
    void intersect(size_t fromDim, size_t toDim, size_t dim){
      //std::cerr << "intersect: " << fromDim << " -> " << 
      //  toDim << std::endl;

      Connectivity& outConn = getConnectivity_(fromDim, toDim);
      if(!outConn.empty()){
        return;
      }
    
      ConnVec conns(numEntities_(fromDim));
    
      using VisitedVec = std::vector<bool>;
      VisitedVec visited(numEntities_(fromDim));

      EntityVec fromVerts(MT::numVerticesPerEntity(fromDim));
      EntityVec toVerts(MT::numVerticesPerEntity(toDim));

      size_t maxSize = 1;    

      for(Iterator fromEntity(*this, fromDim); 
          !fromEntity.isend(); ++fromEntity){
        IdVec& entities = conns[fromEntity.id()];
        entities.reserve(maxSize);

        MeshEntity** ep = fromEntity.getEntities(0);

        std::copy(ep, ep + MT::numVerticesPerEntity(fromDim),
                  fromVerts.begin());
      
        std::sort(fromVerts.begin(), fromVerts.end());
      
        for(Iterator fromItr(fromEntity, dim);
            !fromItr.isend(); ++fromItr){
          for(Iterator toItr(fromItr, toDim);
              !toItr.isend(); ++toItr){
            visited[toItr.id()] = false;
          }
        }
      
        for(Iterator fromItr(fromEntity, dim);
            !fromItr.isend(); ++fromItr){
          for(Iterator toItr(fromItr, toDim);
              !toItr.isend(); ++toItr){
            if(visited[toItr.id()]){
              continue;
            }
          
            visited[toItr.id()] = true;
          
            if(fromDim == toDim){
              if(fromEntity.id() != toItr.id()){
                entities.push_back(toItr.id());
              }
            }
            else{
              MeshEntity** ep = toItr.getEntities(0);

              std::copy(ep, ep + MT::numVerticesPerEntity(toDim),
                        toVerts.begin());
            
              std::sort(toVerts.begin(), toVerts.end());
            
              if(std::includes(fromVerts.begin(), fromVerts.end(),
                               toVerts.begin(), toVerts.end())){
              
                entities.emplace_back(toItr.id());
              }
            }
          }
        }
      
        maxSize = std::max(entities.size(), maxSize);
      }
      
      outConn.init(entities_[toDim], conns);
    }
  
    void compute(size_t fromDim, size_t toDim) override{
      //std::cerr << "compute: " << fromDim << " -> " << toDim << std::endl;

      Connectivity& outConn = getConnectivity_(fromDim, toDim);
    
      if(!outConn.empty()){
        return;
      }
    
      if(numEntities_(fromDim) == 0){
        build(fromDim);
      }
    
      if(numEntities_(toDim) == 0){
        build(toDim);
      }
    
      if(numEntities_(fromDim) == 0 && numEntities_(toDim) == 0){
        return;
      }
    
      if(fromDim == toDim){
        ConnVec connVec(numEntities_(fromDim), IdVec(1));
      
        for(Iterator entity(*this, fromDim); !entity.isend(); ++entity){
          connVec[entity.id()][0] = entity.id();
        }
      
        outConn.set(entities_[toDim], connVec);
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
      int d = MT::Dimension;
      for(int i = d; i >= 0; --i){
        for(int j = 0; j <= d; ++j){
          if(i != j){
            compute(i, j);
          }
        }
      }
    }

    size_t numCells(){
      return entities_[MT::Dimension].size();
    }
  
    size_t numVertices(){
      return entities_[0].size();
    }
  
    size_t numEdges(){
      return entities_[1].size();
    }
  
    size_t numFaces(){
      return entities_[MT::Dimension - 1].size();
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
      return MT::Dimension;
    }  

    const EntityVec& getEntities_(size_t dim) const{
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
    using Entities_ = 
      std::array<EntityVec, MT::Dimension + 1>;

    using Topology_ =
      std::array<std::array<Connectivity, MT::Dimension + 1>,
      MT::Dimension + 1>;

    Entities_ entities_;
    Topology_ topology_;
  };

} // flexi

#endif // __MESH_TOPOLOGY_H__
