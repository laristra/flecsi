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

#include <map>
#include <unordered_map>
#include <vector>
#include <array>
#include <map>
#include <cmath>
#include <bitset>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <set>
#include <functional>

#define np(X)                                                             \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << (X) << std::endl

#define hp(X)                                                             \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << std::hex << (X) << std::endl

namespace flecsi{
namespace tree_topology_dev{

  template<typename T, size_t D>
  class coordinates{
  public:
    using element_t = T;

    static const size_t dimension = D;

    coordinates(){}

    coordinates(std::initializer_list<element_t> il){
      size_t i = 0;
      for(auto v : il){
        pos_[i++] = v;
      }
    }

    coordinates& operator=(const coordinates& c){
      pos_ = c.pos_;
      return *this;
    }

    bool operator==(const coordinates& c) const{
      for(size_t i = 0; i < dimension; ++i){
        if(pos_[i] != c.pos_[i]){
          return false;
        }
      }

      return true;
    }

    element_t operator[](const size_t i) const{
      return pos_[i];
    }

    element_t& operator[](const size_t i){
      return pos_[i];
    }

    element_t distance(const coordinates& u) const{
      element_t d = 0;
      
      for(size_t i = 0; i < dimension; ++i){
        element_t di = pos_[i] - u.pos_[i];
        d += di * di;
      }
      
      return sqrt(d);
    }

    void output_(std::ostream& ostr) const{
      ostr << "(";
      for(size_t i = 0; i < dimension; ++i){
        if(i > 0){
          ostr << ",";
        }
        ostr << pos_[i];
      }
      ostr << ")";   
    }

  private:
    std::array<element_t, dimension> pos_;
  };

template<typename T, size_t D>
class branch_id{
public:
  using int_t = T;

  static const size_t dimension = D;

  static constexpr size_t bits = sizeof(int_t) * 8;

  static constexpr size_t max_depth = (bits - 1)/dimension;

  branch_id()
  : id_(0){}

  branch_id(const std::array<int_t, dimension>& coords)
  : id_(int_t(1) << bits - 1){  
    
    for(size_t i = 0; i < max_depth; ++i){
      for(size_t j = 0; j < dimension; ++j){
        id_ |= (coords[j] & int_t(1) << i) << i + j;
      }
    }
  }

  constexpr branch_id(int_t x, int_t y){
    id_ = table2d_[y >> 8] << 17 | table2d_[x >> 8] << 16 |
      table2d_[y & 0xFF] << 1 | table2d_[x & 0xFF];
  }

  constexpr branch_id(const branch_id& bid)
  : id_(bid.id_){}

  static constexpr branch_id root(){
    return branch_id(int_t(1) << dimension - 1);
  }

  static constexpr branch_id null(){
    return branch_id(0);
  }

  constexpr bool is_null() const{
    return id_ == int_t(0);
  }

  constexpr size_t depth() const{
    int_t id = id_;
    size_t d = 0;
    
    while(id >>= dimension){
      ++d;
    }

    return d;
  }

  constexpr branch_id& operator=(const branch_id& bid){
    id_ = bid.id_;
    return *this;
  }

  constexpr bool operator==(const branch_id& bid) const{
    return id_ == bid.id_;
  }

  constexpr void push(int_t bits){
    assert(bits < int_t(1) << dimension);

    id_ <<= dimension;
    id_ |= bits;
  }

  constexpr void pop(){
    assert(depth() > 0);
    id_ >>= dimension;
  }

  constexpr void pop(size_t d){
    assert(d >= depth());
    id_ >>= d * dimension;
  }

  constexpr branch_id parent() const{
    return branch_id(id_ >> dimension);
  }

  constexpr void truncate(size_t to_depth){
    size_t d = depth();
    
    if(d < to_depth){
      return;
    }

    id_ >>= (d - to_depth) * dimension; 
  }

  void output_(std::ostream& ostr) const{
    constexpr int_t mask = ((int_t(1) << dimension) - 1) << bits - dimension;

    size_t d = max_depth;

    int_t id = id_;

    while((id & mask) == int_t(0)){
      --d;
      id <<= dimension;
    }

    for(size_t i = 0; i <= d; ++i){
      int_t val = (id & mask) >> (bits - dimension);
      ostr << i << ":" << std::bitset<D>(val) << " ";
      id <<= dimension;
    }
  }

  int_t value_() const{
    return id_;
  }

  bool operator<(const branch_id& bid) const{
    return id_ < bid.id_;
  }

  size_t coordinates(std::array<int_t, dimension>& coords) const{
    coords.fill(int_t(0));

    int_t id = id_;
    size_t d = 0;

    while(id >> dimension != int_t(0)){
      for(size_t j = 0; j < dimension; ++j){
        coords[j] |= (((int_t(1) << j) & id) >> j) << d;
      }

      id >>= dimension;
      ++d;
    }

    for(size_t j = 0; j < dimension; ++j){
      coords[j] <<= max_depth - d;
    }

    return d;
  }

private:
  int_t id_;

  constexpr branch_id(int_t id)
  : id_(id){}

  static constexpr int_t table2d_[256] = {
    0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015, 
    0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055, 
    0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115, 
    0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155, 
    0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415, 
    0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455, 
    0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515, 
    0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555, 
    0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015, 
    0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055, 
    0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115, 
    0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155, 
    0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415, 
    0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455, 
    0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515, 
    0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555, 
    0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015, 
    0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055, 
    0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115, 
    0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155, 
    0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415, 
    0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455, 
    0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515, 
    0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555, 
    0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015, 
    0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055, 
    0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115, 
    0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155, 
    0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415, 
    0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455, 
    0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515, 
    0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555
  };
};

class entity_id_t{
public:

  entity_id_t(){}

  entity_id_t(const entity_id_t& id)
  : id_(id.id_){}

  entity_id_t(size_t id)
  : id_(id){}

  operator size_t() const{
    id_;
  }

  entity_id_t& operator=(const entity_id_t& id){
    id_ = id.id_;
    return *this;
  }

private:
  size_t id_;
};

template<typename T, size_t D>
std::ostream& operator<<(std::ostream& ostr, const branch_id<T,D>& id){
  id.output_(ostr);
  return ostr;
}

template<typename T, size_t D>
std::ostream& operator<<(std::ostream& ostr, const coordinates<T, D>& p){
  p.output_(ostr);
  return ostr;
}

template<typename T, size_t D>
struct branch_id_hasher__{
  size_t operator()(const branch_id<T, D>& k) const{
    return std::hash<T>()(k.value_());
  }
};

double uniform(){
  return double(rand())/RAND_MAX;
}

double uniform(double a, double b){
  return a + (b - a) * uniform();
}

enum class action : uint64_t{
  none = 0b00,
  refine = 0b01,
  coarsen = 0b10
};

template<class P>
class tree_topology : public P{
public:
  using Policy = P;

  static const size_t dimension = Policy::dimension;

  
  using point_t = typename Policy::point_t;

  using element_t = typename Policy::point_t::element_t;


  using branch_int_t = typename Policy::branch_int_t;

  using branch_id_t = branch_id<branch_int_t, dimension>;

  using branch_id_vector_t = std::vector<branch_id_t>;


  using branch_t = typename Policy::branch_t;

  using branch_vector_t = std::vector<branch_t*>;


  using entity_t = typename Policy::entity_t;

  using entity_vector_t = std::vector<entity_t*>;

  using apply_function = std::function<void(branch_t&)>;

  using entity_id_vector_t = std::vector<entity_id_t>;

  template<class T>
  class iterator{
   public:
    using id_t = typename T::id_t;
    using id_vector_t = std::vector<id_t>;

    iterator(const iterator& itr)
    : tree_(itr.tree_), items_(itr.items_), index_(itr.index_){}

    iterator(tree_topology& tree, const id_vector_t& items, size_t index)
    : tree_(tree), items_(&items), index_(index){}

    iterator& operator++(){
      ++index_;
      return *this;
    }

    iterator& operator=(const iterator & itr){
      tree_ = itr.tree_;
      index_ = itr.index_;
      items_ = itr.items_;
      return *this;
    }

    T* operator*(){
      return tree_.get((*items_)[index_]);
    }

    T* operator->(){
      return tree_.get((*items_)[index_]);
    }

    bool operator==(const iterator& itr) const{
      return index_ == itr.index_;
    }

    bool operator!=(const iterator& itr) const{
      return index_ != itr.index_;
    }

   private:
    tree_topology& tree_;
    const id_vector_t* items_;
    size_t index_;
  };

  template<class T>
  class iterable_set{
  public:
    using iterator_t = iterator<T>;

    using id_vector_t = typename iterator_t::id_vector_t;
    
    using filter_function = std::function<bool(T&)>;    
    using apply_function = std::function<void(T&)>;

    template<typename R>
    using map_function = std::function<R(T&)>;

    template<typename R>
    using reduce_function = std::function<void(T&, R&)>;

    iterable_set() = default;

    iterable_set(tree_topology& tree, const id_vector_t& v, bool sorted = false)
    : tree_(&tree), v_(&v), begin_(0), end_(v_->size()),
    owned_(false), sorted_(sorted){}

    iterable_set(tree_topology& tree, id_vector_t&& v, bool sorted)
    : tree_(&tree), v_(new id_vector_t(std::move(v))),
    begin_(0), end_(v_->size()), owned_(true), sorted_(sorted){}

    ~iterable_set(){
      if(owned_){
        delete v_;
      }
    }

    iterable_set & operator=(const iterable_set& r) = default;

    iterator_t begin() const { return iterator_t(*tree_, *v_, begin_); }
    
    iterator_t end() const { return iterator_t(*tree_, *v_, end_); }
       
    T* operator[](size_t i) const{
      return tree_->template get((*v_)[begin_ + i]);
    }
     
    T* front() const{
      return tree_->template get((*v_)[begin_]);
    }

    T* back() const{
      return tree_->template get((*v_)[end_ - 1]);
    }
     
    size_t size() const { return end_ - begin_; }
     
    iterable_set filter(filter_function f) const {
      id_vector_t v;

      for (auto item : *this) {
        if (f(*item)) {
          v.push_back(item->id());
        }
      }

      return iterable_set(*tree_, std::move(v), sorted_);
    }

    template<typename S>
    std::vector<iterable_set> scatter(map_function<T> f) const {

      std::map<S, id_vector_t> id_map;
      for (auto item : *this)
        id_map[f(*item)].push_back(item->id());

      std::vector<iterable_set> ent_map;
      for ( auto entry : id_map )
        ent_map.emplace_back( 
          std::move(iterable_set(*tree_, std::move(entry.second), sorted_) )
        );

      return ent_map;
    }
     
    void apply(apply_function f) const {
      for (auto ent : *this) {
        f(ent);
      }
    }
     
    template<typename S>
    std::vector<S> map(map_function<T> f) const {
      std::vector<S> ret;
      ret.reserve(v_->size());
      
      for (auto item : *this) {
        ret.push_back(f(*item));
      }
      return ret;
    }

    template<typename S>
    S reduce(T start, reduce_function<T> f) const {
      T r = start;
      
      for (auto item : *this) {
        f(*item, r);
      }

      return r;
    }
     
    void prepare_(){
      if(!owned_){
        v_ = new id_vector_t(*v_);
        owned_ = true;
      }

      if(!sorted_){
        auto vc = const_cast<id_vector_t*>(v_);
        std::sort(vc->begin(), vc->end());
        sorted_ = true;
      }
    }
     
    iterable_set& operator&=(const iterable_set& r){
      prepare_();

      id_vector_t ret;

      if(r.sorted_){
        ret.resize(std::min(v_->size(), r.v_->size()));

        auto itr = std::set_intersection(v_->begin(), v_->end(),
                                         r.v_->begin(), r.v_->end(), ret.begin());

        ret.resize(itr - ret.begin());
      }
      else{
        id_vector_t v2(*r.v_);
        std::sort(v2.begin(), v2.end());

        ret.resize(std::min(v_->size(), v2.size()));

        auto itr = std::set_intersection(v_->begin(), v_->end(),
                                         v2.begin(), v2.end(), ret.begin());

        ret.resize(itr - ret.begin());
      }

      delete v_;
      v_ = new id_vector_t(std::move(ret));

      begin_ = 0;
      end_ = v_->size();

      return *this;
    }
     
    iterable_set operator&(const iterable_set& r) const{
      iterable_set ret(*this);
      ret &= r;
      return ret;
    }
     
    iterable_set& operator|=(const iterable_set& r){
      prepare_();

      id_vector_t ret;

      if(r.sorted_){
        ret.resize(v_->size() + r.v_->size());

        auto itr = std::set_union(v_->begin(), v_->end(),
                                  r.v_->begin(), r.v_->end(), ret.begin());

       ret.resize(itr - ret.begin());
      }
      else{
        id_vector_t v2(*r.v_);

        std::sort(v2.begin(), v2.end());

        ret.resize(v_->size() + v2.size());

        auto itr = std::set_union(v_->begin(), v_->end(),
                                  v2.begin(), v2.end(), ret.begin());

        ret.resize(itr - ret.begin());
      }

      delete v_;
      v_ = new id_vector_t(std::move(ret));

      begin_ = 0;
      end_ = v_->size();

      return *this;
    }
     
    iterable_set operator|(const iterable_set& r) const{
      iterable_set ret(*this);
      ret |= r;
      return ret;
    } 
     
    iterable_set& operator-=(const iterable_set& r){
      prepare_();

      id_vector_t ret(v_->size());

      if(r.sorted_){
        auto itr = std::set_difference(v_->begin(), v_->end(),
                                       r.v_->begin(), r.v_->end(), ret.begin());

        ret.resize(itr - ret.begin());
      }
      else{
        id_vector_t v2(*r.v_);

        std::sort(v2.begin(), v2.end());

        auto itr = std::set_difference(v_->begin(), v_->end(),
                                       v2.begin(), v2.end(), ret.begin());

        ret.resize(itr - ret.begin());
      }

      delete v_;
      v_ = new id_vector_t(std::move(ret));

      begin_ = 0;
      end_ = v_->size();

      return *this;
    }
     
    iterable_set operator-(const iterable_set& r) const{
      iterable_set ret(*this);
      ret -= r;
      return ret;
    }
     
    void add(T* item){
      if(!owned_){
        v_ = new id_vector_t(*v_);
        owned_ = true;
      }

      auto vc = const_cast<id_vector_t*>(v_);

      if(sorted_){
        auto id = item->id();
        auto itr = std::upper_bound(vc->begin(), vc->end(), id);
        vc->insert(itr, id);
      }
      else{
        vc->push_back(item->id());
      }
    }
     
    iterable_set& operator<<(T* item){
      add(item);
      return *this;
    }
     
  private:
    tree_topology* tree_ = nullptr;
    const id_vector_t* v_ = nullptr;
    size_t begin_ = 0;
    size_t end_ = 0;
    bool owned_ = false;
    bool sorted_ = true;
  };

  using branch_set_t = iterable_set<branch_t>;
  using entity_set_t = iterable_set<entity_t>;

  tree_topology(std::initializer_list<element_t> bounds){
    assert(bounds.size() == bounds_.size());

    size_t i = 0;
    for(element_t ei : bounds){
      bounds_[i++] = ei;
    }

    branch_id_t bid = branch_id_t::root();
    root_ = make_branch(bid);
    root_->set_parent_(nullptr);
    branch_map_.emplace(bid, root_);

    max_depth_ = 0;
  }

  branch_t* find_parent_(branch_id_t bid){
    for(;;){
      auto itr = branch_map_.find(bid);
      if(itr != branch_map_.end()){
        return itr->second;
      }
      bid.pop();
    }
  }

  branch_t* find_parent(branch_t* b){
    return find_parent(b->id());
  }

  branch_t* find_parent(branch_id_t bid){
    return find_parent(bid, max_depth_);
  }

  branch_t* find_parent(branch_id_t bid, size_t max_depth){
    branch_id_t pid = bid;
    pid.truncate(max_depth);

    return find_parent_(pid);
  }

  void insert(entity_t* ent, size_t max_depth){
    branch_id_t bid = to_branch_id(ent->coordinates());
    point_t p = to_coordinates(bid);

    branch_t* b = find_parent(bid, max_depth);
    ent->set_branch_id_(b->id());

    b->insert(ent);

    switch(b->requested_action()){
      case action::none:
        break;
      case action::refine:
        refine_(b);
        break;
      default:
        assert(false && "invalid action");
    }
  }

  void insert(entity_t* ent){
    insert(ent, max_depth_);
  }

  void remove(entity_t* ent){
    assert(!ent->get_branch_id().is_null());

    auto itr = branch_map_.find(ent->get_branch_id());
    assert(itr != branch_map_.end());
    branch_t* b = itr->second;
    
    b->remove(ent);
    ent->set_branch_id_(branch_id_t::null());

    switch(b->requested_action()){
      case action::none:
        break;
      case action::coarsen:{
        auto p = static_cast<branch_t*>(b->parent());
        if(p){
          coarsen_(p);
        }
        break;
      }
      case action::refine:
        b->reset();
        break;
      default:
        assert(false && "invalid action");
    }
  }

  void refine_(branch_t* b){
    branch_id_t pid = b->id();
    size_t depth = pid.depth() + 1;
    
    for(branch_int_t bi = 0; bi < branch_t::num_children; ++bi){
      branch_id_t bid = pid;
      bid.push(bi);
      auto c = make_branch(bid);
      c->set_parent_(b);
      b->set_child_(bi, c);
      branch_map_.emplace(bid, c);
    }

    for(auto ent : *b){
      insert(ent, depth);
    }

    b->clear();
    b->reset();

    max_depth_ = std::max(max_depth_, depth);
  }

  // helper method in coarsening
  // insert into p, coarsen all recursive children of b

  void coarsen_(branch_t* p, branch_t* b){
    if(b->is_leaf()){
      return;
    }

    for(size_t i = 0; i < branch_t::num_children; ++i){
      branch_t* ci = static_cast<branch_t*>(b->child(i));
      
      for(auto ent : *ci){
        p->insert(ent);
        ent->set_branch_id_(p->id());
      }

      coarsen_(p, ci);
      branch_map_.erase(ci->id());
      delete ci;
    }
  }

  void coarsen_(branch_t* p){    
    coarsen_(p, p);
    p->make_leaf();
    p->reset();
  }

  entity_set_t find(const point_t& p, element_t radius){
    // find the lowest level branch which is guaranteed
    // to contain the point with radius

    branch_id_t bid = to_branch_id(p);

    size_t d = 0;

    element_t dw = radius * element_t(2);
    element_t w = bounds_[1] - bounds_[0];
    
    while(w > dw){
      w /= element_t(2);
      ++d;
    }
    
    branch_t* b = find_parent(bid, d);

    entity_id_vector_t entity_ids;

    find_(b, entity_ids, p, radius, w);

    return entity_set_t(*this, std::move(entity_ids), false);
  }

  static bool intersects(const coordinates<element_t, 2>& r1,
                         element_t w,
                         const coordinates<element_t, 2>& center,
                         element_t radius){
    
    element_t c1x = center[0] + radius;
    element_t c1y = center[1] + radius;

    element_t w2 = w / element_t(2);

    element_t c2x = r1[0] + w2;
    element_t c2y = r1[1] + w2;

    element_t d1x = c1x - c2x;
    element_t d1y = c1y - c2y;

    if(d1x < -w2){
      d1x = -w2;
    }
    else if(d1x > w2){
      d1x = w2;
    }

    if(d1y < -w2){
      d1y = -w2;
    }
    else if(d1y > w2){
      d1y = w2;
    }

    c2x += d1x - c1x;
    c2y += d1y - c1y;

    return sqrt(c2x * c2x + c2y * c2y) < radius;
  }

  void find_(branch_t* b,
             entity_id_vector_t& entity_ids,
             const point_t& p,
             element_t radius,
             element_t w){

    if(b->is_leaf()){
      for(auto ent : *b){
        if(p.distance(ent->coordinates()) < radius){
          entity_ids.push_back(ent->id());
        }
      }

      return;      
    }

    element_t w2 = w / element_t(2);

    for(size_t i = 0; i < branch_t::num_children; ++i){
      branch_t* ci = static_cast<branch_t*>(b->child(i));
      
      point_t p2 = to_coordinates(ci->id());

      if(intersects(p2, w, p, radius)){
        find_(ci, entity_ids, p, radius, w2);
      }
    }
  }

  branch_vector_t neighbors(branch_t* l) const{
    assert(false && "unimplemented");
  }

  branch_id_vector_t neighbors(branch_id_t b) const{
    assert(false && "unimplemented");
  }

  entity_vector_t locality(entity_t* ent, element_t dist){
    assert(false && "unimplemented");
  }

  branch_t* make_branch(branch_id_t id){
    auto b = new branch_t;
    b->set_id_(id);
    return b;
  }

  template<class... Args>
  entity_t* make_entity(Args&&... args){
    auto ent = new entity_t(std::forward<Args>(args)...);
    ent->set_id_(entities_.size());
    entities_.push_back(ent);
    return ent;
  }

  branch_id_t to_branch_id(const point_t& p){    
    std::array<branch_int_t, dimension> coords;
    
    for(size_t i = 0; i < dimension; ++i){
      element_t start = bounds_[i * 2];
      element_t end = bounds_[i * 2 + 1];

      coords[i] = (p[i] - start)/(end - start) * 
        (branch_int_t(1) << (branch_id_t::bits - 1)/dimension);
    }

    return branch_id_t(coords);
  }

  point_t to_coordinates(branch_id_t bid){
    std::array<branch_int_t, dimension> coords;
    bid.coordinates(coords);

    constexpr branch_int_t max = 
      (branch_int_t(1) << branch_id_t::max_depth) - 1;
    
    point_t p;
    for(size_t i = 0; i < dimension; ++i){
      element_t start = bounds_[i * 2];
      element_t end = bounds_[i * 2 + 1];

      p[i] = element_t(coords[i])/max * (end - start) + start;
    }

    return p;
  }

  size_t max_depth() const{
    return max_depth_;
  }

  void apply(apply_function f){
    apply(f, root_);
  }

  void apply(apply_function f, branch_t* b){    
    f(*b);

    branch_id_t bid = b->id();

    constexpr branch_int_t n = branch_int_t(1) << dimension;

    for(branch_int_t ci = 0; ci < n; ++ci){
      branch_id_t cid = bid;
      cid.push(ci);

      auto citr = branch_map_.find(cid);
      if(citr == branch_map_.end()){
        continue;
      }

      auto c = citr->second;

      apply(f, c);
    }
  }

  entity_t* get(entity_id_t id){
    assert(id < entities_.size());
    return entities_[id];
  }

  branch_t* get(branch_id_t id){
    auto itr = branch_map_.find(id);
    assert(itr != branch_map_.end());
    return itr->second;
  }

private:
  using branch_map_t = std::unordered_map<branch_id_t, branch_t*,
    branch_id_hasher__<branch_int_t, dimension>>;

  branch_map_t branch_map_;

  std::array<element_t, dimension * 2> bounds_;
  
  size_t max_depth_;

  branch_t* root_;

  std::vector<entity_t*> entities_;
};

template<typename T, size_t D>
class tree_entity{
public:
  using id_t = entity_id_t;

  using branch_id_t = branch_id<T, D>;

  tree_entity()
  : branch_id_(branch_id_t::null()){}

  void set_branch_id_(branch_id_t bid){
    branch_id_ = bid;
  }

  branch_id_t get_branch_id() const{
    return branch_id_;
  }

  entity_id_t id() const{
    return id_;
  }

  void set_id_(entity_id_t id){
    id_ = id;
  }

private:
  branch_id_t branch_id_;
  entity_id_t id_;
};

template<typename T, size_t D>
class tree_branch{
public:
  using branch_int_t = T;

  static const size_t dimension = D;

  using branch_id_t = branch_id<T, D>;

  using id_t = branch_id_t;

  static constexpr size_t num_children = branch_int_t(1) << dimension;

  tree_branch()
  : action_(action::none){
    children_[0] = nullptr;
  }

  void set_id_(branch_id_t id){
    id_ = id;
  }

  branch_id_t id() const{
    return id_;
  }

  void refine(){
    action_ = action::refine;
  }

  void coarsen(){
    action_ = action::coarsen;
  }

  void reset(){
    action_ = action::none;
  }

  action requested_action(){
    return action_;
  }

  tree_branch* child(size_t ci) const{
    assert(ci < num_children);
    return children_[ci];
  }

  void set_child_(size_t ci, tree_branch* b){
    assert(ci < num_children);
    children_[ci] = b;
  }

  tree_branch* parent() const{
    return parent_;
  }

  void set_parent_(tree_branch* b){
    parent_ = b;
  }

  std::array<tree_branch*, num_children>& children(){
    return children_;
  }

  bool is_leaf() const{
    return !children_[0];
  }

  void make_leaf(){
    children_[0] = nullptr;
  }

private:
  tree_branch* parent_;
  std::array<tree_branch*, num_children> children_;

  branch_id_t id_;
  action action_ : 2;
};

class tree_policy{
public:
  using tree_t = tree_topology<tree_policy>;

  using branch_int_t = uint64_t;

  static const size_t dimension = 2;

  using element_t = double;

  using point_t = coordinates<element_t, dimension>;

  class entity : public tree_entity<branch_int_t, dimension>{
  public:
    entity(const point_t& p)
    : coordinates_(p){}

    const point_t& coordinates() const{
      return coordinates_;
    }

    private:
      point_t coordinates_;
  };

  using entity_t = entity;

  class branch : public tree_branch<branch_int_t, dimension>{
  public:
    branch(){}

    void insert(entity_t* ent){
      ents_.push_back(ent);
      
      if(ents_.size() > 1){
        refine();
      }
    }

    void remove(entity_t* ent){
      auto itr = std::find(ents_.begin(), ents_.end(), ent);
      ents_.erase(itr);
      
      if(ents_.empty()){
        coarsen();
      }
    }

    auto begin(){
      return ents_.begin();
    }

    auto end(){
      return ents_.end();
    }

    void clear(){
      ents_.clear();
    }

    size_t count(){
      return ents_.size();
    }

  private:
    std::vector<entity_t*> ents_;
  };

  using branch_t = branch;
};

} // namespace tree_topology_dev
} // namespace flecsi
