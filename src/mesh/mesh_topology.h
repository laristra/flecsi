/*~------------------------------------------------------------------------~*
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
 *~------------------------------------------------------------------------~*/

#ifndef flexi_mesh_topology_h
#define flexi_mesh_topology_h

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

  class MeshEntityBase{
  public:
    size_t id() const{
      return id_;
    }

    static constexpr size_t getDim_(size_t meshDim, size_t dim){
      return dim > meshDim ? meshDim : dim;
    }

    template<class MT>
    static MeshEntityBase* create_(size_t dim, size_t id){
      switch(dim){
      case 0:{
        using EntityType = 
          typename std::tuple_element<getDim_(MT::dimension, 0),
                                      typename MT::EntityTypes>::type;
        auto entity = new EntityType;
        entity->id_ = id;
        return entity;
      }
      case 1:{
        using EntityType = 
          typename std::tuple_element<getDim_(MT::dimension, 1),
                                      typename MT::EntityTypes>::type;
        auto entity = new EntityType;
        entity->id_ = id;
        return entity;
      }
      case 2:{
        using EntityType = 
          typename std::tuple_element<getDim_(MT::dimension, 2),
                                      typename MT::EntityTypes>::type;
        auto entity = new EntityType;
        entity->id_ = id;
        return entity;
      }
      case 3:{
        using EntityType = 
          typename std::tuple_element<getDim_(MT::dimension, 3),
                                      typename MT::EntityTypes>::type;
        auto entity = new EntityType;
        entity->id_ = id;
        return entity;
      }
      default:
        assert(false && "invalid topology dim");
      }
    }

    template<class MT>
    friend class MeshTopology;

  private:
    size_t id_;
  };

  template<size_t D>
  class MeshEntity : public MeshEntityBase{
  public:
    static const size_t dimension = D;
    
    MeshEntity(){}
  };

  using EntityVec = std::vector<MeshEntityBase*>;

  template<class T>
  class EntityGroup{
  public:
    using Vec = std::vector<T*>;

    class iterator_{
    public:

      iterator_(const Vec& entities, size_t index)
        : entities_(&entities),
          index_(index){}
      
      iterator_& operator++(){
        ++index_;
        return *this;
      }

      iterator_& operator=(const iterator_& itr){
        index_ = itr.index_;
        entities_ = itr.entities_;
        return *this;
      }

      T* operator*(){
        return (*entities_)[index_];
      }

      T* operator->(){
        return (*entities_)[index_];
      }

      bool operator==(const iterator_& itr) const{
        return index_ == itr.index_;
      }

      bool operator!=(const iterator_& itr) const{
        return index_ != itr.index_;
      }

    private:
      const Vec* entities_;
      size_t index_;
    };

    EntityGroup(){}

    EntityGroup(Vec&& entities)
      : entities_(std::move(entities)){}
    
    void add(T* ent){
      entities_.push_back(ent);
    }
    
    const Vec& getEntities() const{
      return entities_;
    }

    static constexpr size_t dim(){
      return T::dimension;
    }

    iterator_ begin(){
      return iterator_(entities_, 0);
    }
    
    iterator_ end(){
      return iterator_(entities_, entities_.size());
    }
    
  private:
    Vec entities_;
  };
    
  class MeshTopologyBase{
  public:

    using IdVec = std::vector<id_t>;
  
    using ConnVec = std::vector<IdVec>;

    struct IdVecHash{
      size_t operator()(const IdVec& v) const{
        size_t h = 0;
        for(id_t id : v){
          h |= id;
        }
        return h;
      }
    };
  
    using IdVecMap = std::unordered_map<IdVec, id_t, IdVecHash>;
  
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
          
          for(id_t id : iv){
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

        id_t maxId = 0;

        for(size_t i = 0; i < n; ++i){
          const IdVec& iv = cv[i];
          
          for(id_t id : iv){
            maxId = std::max(maxId, id);
            entityVec_.push_back(MeshEntityBase::create_<MT>(dim, id));
          }
          
          fromIndexVec_.push_back(entityVec_.size());
        }

        size_t m = entityVec_.size();
        ev.resize(maxId + 1);

        for(MeshEntityBase* ent : entityVec_){
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
    
      void push(MeshEntityBase* ent){
        entityVec_.push_back(ent);
      }
    
      void dump(){
        std::cout << "=== idVec" << std::endl;
        for(MeshEntityBase* ent : entityVec_){
          std::cout << ent->id() << std::endl;
        }
      
        std::cout << "=== groupVec" << std::endl;
        for(id_t id : fromIndexVec_){
          std::cout << id << std::endl;
        }
      }

      const EntityVec& getEntities() const{
        return entityVec_;
      }

      const IdVec& getFromIndexVec() const{
        return fromIndexVec_;
      }
    
      MeshEntityBase** getEntities(size_t index){
        assert(index < fromIndexVec_.size() - 1);
        return entityVec_.data() + fromIndexVec_[index];
      }

      MeshEntityBase** getEntities(size_t index, size_t& endIndex){
        assert(index < fromIndexVec_.size() - 1);
        uint64_t start = fromIndexVec_[index];
        endIndex = fromIndexVec_[index + 1] - start;
        return entityVec_.data() + start;
      }
        
      bool empty(){
        return entityVec_.empty();
      }
    
      void set(size_t fromId, MeshEntityBase* ent, size_t pos){
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
  class MeshTopology : public MeshTopologyBase{
  public:
    using VertexType = 
      typename std::tuple_element<0, typename MT::EntityTypes>::type;
    
    using EdgeType = 
      typename std::tuple_element<1, typename MT::EntityTypes>::type;  

    using FaceType = 
      typename std::tuple_element<MT::dimension - 1,
                                  typename MT::EntityTypes>::type;

    using CellType = 
      typename std::tuple_element<MT::dimension,
                                  typename MT::EntityTypes>::type;

    class Iterator{
    public:
      Iterator(MeshTopology& mesh, size_t dim)
        : mesh_(mesh),
          entities_(&mesh.getEntities_(dim)),
          dim_(dim),
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
      
      MeshEntityBase& get(){
        return *(*entities_)[index_];
      }

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

      MeshEntityBase** getEntities(size_t dim){
        Connectivity& c = mesh_.getConnectivity_(dim_, dim);
        assert(!c.empty());
        return c.getEntities(index_);
      }

    protected:

      const EntityVec* getEntities_(){
        return entities_;
      }

    private:

      MeshTopology& mesh_;
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

      using EntityTypeVec = std::vector<EntityType*>;

      EntityIterator(MeshTopology& mesh)
        : Iterator(mesh, D){}

      EntityIterator(Iterator& itr)
        : Iterator(itr, D){}

      EntityType& operator*(){
        return static_cast<EntityType&>(Iterator::get());
      }

      EntityType* operator->(){
        return &static_cast<EntityType&>(Iterator::get());
      }

      typename EntityTypeVec::const_iterator begin(){
        return reinterpret_cast<const EntityTypeVec*>(
          Iterator::getEntities_())->begin();
      }

      typename EntityTypeVec::const_iterator end(){
        return reinterpret_cast<const EntityTypeVec*>(
          Iterator::getEntities_())->end();
      }
    };

    using VertexIterator = EntityIterator<0>;
    using EdgeIterator = EntityIterator<1>;
    using FaceIterator = EntityIterator<MT::dimension - 1>;
    using CellIterator = EntityIterator<MT::dimension>;

    template<size_t D>
    class iterator{
    public:
      using EntityType = 
        typename std::tuple_element<D, typename MT::EntityTypes>::type;

      using EntityTypeVec = std::vector<EntityType*>;

      iterator(const iterator& itr)
        : entities_(itr.entities_),
          index_(itr.index_){}

      iterator(const EntityVec& entities, size_t index)
        : entities_(&reinterpret_cast<const EntityTypeVec&>(entities)),
          index_(index){}

      iterator(const EntityTypeVec& entities, size_t index)
        : entities_(&entities),
          index_(index){}
      
      iterator& operator++(){
        ++index_;
        return *this;
      }

      iterator& operator=(const iterator& itr){
        index_ = itr.index_;
        entities_ = itr.entities_;
        return *this;
      }

      EntityType* operator*(){
        return (*entities_)[index_];
      }

      EntityType* operator->(){
        return (*entities_)[index_];
      }

      bool operator==(const iterator& itr) const{
        return index_ == itr.index_;
      }

      bool operator!=(const iterator& itr) const{
        return index_ != itr.index_;
      }

    private:
      const EntityTypeVec* entities_;
      size_t index_;
    };

    using vertex_iterator = iterator<0>;
    using edge_iterator = iterator<1>;
    using face_iterator = iterator<MT::dimension - 1>;
    using cell_iterator = iterator<MT::dimension>;

    template<size_t D>
    class EntityRange{
    public:
      using iterator_t = iterator<D>;
      using EntityType = typename iterator_t::EntityType;
      using EntityTypeVec = typename iterator_t::EntityTypeVec;

      EntityRange(const EntityVec& v)
        : v_(reinterpret_cast<const EntityTypeVec&>(v)),
          begin_(0),
          end_(v_.size()){}

      EntityRange(const EntityTypeVec& v)
        : v_(v),
          begin_(0),
          end_(v_.size()){}

      EntityRange(const EntityVec& v, size_t begin, size_t end)
        : v_(reinterpret_cast<const EntityTypeVec&>(v)),
          begin_(begin),
          end_(end){}

      EntityRange(const EntityTypeVec& v, size_t begin, size_t end)
        : v_(v),
          begin_(begin),
          end_(end){}

      EntityRange(const EntityRange& r)
        : v_(r.v_),
          begin_(0),
          end_(v_.size()){}

      iterator_t begin(){
	return iterator_t(v_, begin_);
      }

      iterator_t end(){
        return iterator_t(v_, end_);
      }

      EntityTypeVec toVec(){
        return EntityTypeVec(v_.begin() + begin_, v_.begin() + end_);
      }

    private:
      const EntityTypeVec& v_;
      size_t begin_;
      size_t end_;
    };

    MeshTopology(){
      getConnectivity_(MT::dimension, 0).init();
      std::fill(nextIds_.begin(), nextIds_.end(), 0);
    }
  
    void addVertex(VertexType* vertex){
      entities_[0].push_back(vertex);
    }
  
    void addCell(CellType* cell,
                 std::initializer_list<VertexType*> verts){

      assert(verts.size() == 
             MT::numVerticesPerEntity(MT::dimension) &&
             "invalid number of vertices per cell");
    
      auto& c = getConnectivity_(MT::dimension, 0);

      assert(cell->id() == c.fromSize() && "id mismatch"); 

      for(VertexType* v : verts){
        c.push(v);
      }

      c.endFrom();

      entities_[MT::dimension].push_back(cell);
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

      auto& c = getConnectivity_(MT::dimension - 1, 0);
      if(c.empty()){
        c.init();
      }
      
      assert(face->id() == c.fromSize() && "id mismatch"); 

      for(VertexType* v : verts){
        c.push(v);
      }

      c.endFrom();
      entities_[MT::dimension - 1].push_back(face);
    }
    
    void addCellEdges(CellType* cell, std::initializer_list<EdgeType*> edges){
      assert(edges.size() == 
             MT::numEntitiesPerCell(1) &&
             "invalid number of edges per cell");

      auto& c = getConnectivity_(MT::dimension, 1);
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
             MT::numEntitiesPerCell(MT::dimension - 1) &&
             "invalid number of face per cell");

      auto& c = getConnectivity_(MT::dimension,
                                 MT::dimension - 1);
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

      assert(dim <= MT::dimension);

      size_t verticesPerEntity = MT::numVerticesPerEntity(dim);
      size_t entitiesPerCell =  MT::numEntitiesPerCell(dim);
    
      Connectivity& entityToVertex = getConnectivity_(dim, 0);
    
      IdVec entityVertices(entitiesPerCell * verticesPerEntity);

      Connectivity& cellToEntity =
        getConnectivity_(MT::dimension, dim);

      ConnVec entityVertexConn;

      size_t entityId = 0;
      size_t maxCellEntityConns = 1;

      Connectivity& cellToVertex =
        getConnectivity_(MT::dimension, 0);
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
          id_t* a = &entityVertices[i * verticesPerEntity];
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
      std::cerr << "transpose: " << fromDim << " -> " << 
         toDim << std::endl;
    
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
      std::cerr << "intersect: " << fromDim << " -> " << 
        toDim << std::endl;

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

        MeshEntityBase** ep = fromEntity.getEntities(0);

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
              MeshEntityBase** ep = toItr.getEntities(0);

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
      int d = MT::dimension;
      for(int i = d; i >= 0; --i){
        for(int j = 0; j <= d; ++j){
          if(i != j){
            compute(i, j);
          }
        }
      }
    }

    decltype(auto) numCells() const {
      return entities_[MT::dimension].size();
    }
  
    decltype(auto) numVertices() const {
      return entities_[0].size();
    }
  
    decltype(auto) numEdges() const {
      return entities_[1].size();
    }
  
    decltype(auto) numFaces() const {
      return entities_[MT::dimension - 1].size();
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
      return MT::dimension;
    }  

    template<class T, class... S>
    T* make(S&&... args){
      T* entity = new T(std::forward<S>(args)...);
      entity->id_ = nextIds_[T::dimension]++;
      return entity;
    }

    template<class T>
    static T* makeWithId(id_t id){
      T* entity = new T;
      entity->id_ = id;
      return entity;
    }

    const EntityVec& getEntities_(size_t dim) const{
      return entities_[dim];
    }

    EntityRange<0> vertices(){
      return EntityRange<0>(entities_[0]);
    }

    template<size_t D, class E>
    EntityRange<D> entities(E* e){
      Connectivity& c = getConnectivity(E::dimension, D);
      if(c.empty()){
        compute(E::dimension, D);
      }

      const IdVec& fv = c.getFromIndexVec();

      return EntityRange<D>(c.getEntities(), fv[e->id()], fv[e->id() + 1]);
    }

    template<class E>
    EntityRange<0> vertices(E* e){
      return entities<0>(e);
    }

    EntityRange<1> edges(){
      if(entities_[1].empty()){
        build(1);
      }
      
      return EntityRange<1>(entities_[1]);
    }

    template<class E>
    EntityRange<1> edges(E* e){
      return entities<1>(e);
    }

    EntityRange<MT::dimension - 1> faces(){
      return EntityRange<MT::dimension - 1>(entities_[MT::dimension - 1]);
    }

    template<class E>
    EntityRange<MT::dimension - 1> faces(E* e){
      return entities<MT::dimension - 1>(e);
    }

    EntityRange<MT::dimension> cells(){
      return EntityRange<MT::dimension>(entities_[MT::dimension]);
    }

    template<class E>
    EntityRange<MT::dimension> cells(E* e){
      return entities<MT::dimension>(e);
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
      std::array<EntityVec, MT::dimension + 1>;

    using Topology_ =
      std::array<std::array<Connectivity, MT::dimension + 1>,
      MT::dimension + 1>;

    using NextIds_ = 
      std::array<id_t, MT::dimension + 1>;

    Entities_ entities_;
    Topology_ topology_;
    NextIds_ nextIds_;
  };

} // flexi

#endif // flexi_mesh_topology_h

/*~-----------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-----------------------------------------------------------------------~-*/

