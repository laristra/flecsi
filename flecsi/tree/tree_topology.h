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

#pragma once

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
#include <mutex>

#include "flecsi/geometry/point.h"
#include "flecsi/concurrency/thread_pool.h"
#include "flecsi/data/data_client.h"

#define np(X)                                                             \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << (X) << std::endl

#define hp(X)                                                             \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << std::hex << (X) << std::endl

namespace flecsi{
namespace tree{

template<typename T, size_t D>
struct tree_geometry{};

template<typename T>
struct tree_geometry<T, 1>{
  using point_t = point<T, 1>;
  using element_t = T;

  static bool within(const point_t& origin,
                     const point_t& center,
                     element_t radius){
    return distance(origin, center) < radius;
  }

  static bool within_box(const point_t& origin,
                         const point_t& min,
                         const point_t& max){
    return origin[0] < max[0] && origin[0] > min[0];
  }

  // initial attempt to get this working, needs to be optimized

  static bool intersects(const point_t& origin,
                         element_t size,
                         const point_t& scale,
                         const point_t& center,
                         element_t radius){
    
    return center[0] > origin[0] - radius &&
           center[0] < origin[0] + size * scale[0] + radius;
  }

  static bool intersects_box(const point_t& origin,
                             element_t size,
                             const point_t& scale,
                             const point_t& min2,
                             const point_t& max2){
    point_t max1 = origin;
    max1[0] += size * scale[0];
    return intersects_box_(origin, max1, min2, max2);
  }

  static bool intersects_box_(const point_t& min1,
                              const point_t& max1,
                              const point_t& min2,
                              const point_t& max2){
    
    if(max1[0] < min2[0]){
      return false;
    }
    
    if(min1[0] > max2[0]){
      return false;
    }

    return true;
  }

  static bool intersect_true(const point_t& origin,
                             element_t size,
                             const point_t& scale,
                             const point_t& center,
                             element_t radius){
    return true;
  }
};

template<typename T>
struct tree_geometry<T, 2>{
  using point_t = point<T, 2>;
  using element_t = T;

  static bool within(const point_t& origin,
                     const point_t& center,
                     element_t radius){
    return distance(origin, center) < radius;
  }

  static bool within_box(const point_t& origin,
                         const point_t& min,
                         const point_t& max){
    return origin[0] < max[0] && origin[0] > min[0] &&
           origin[1] < max[1] && origin[1] > min[1];
  }

  // initial attempt to get this working, needs to be optimized

  static bool intersects(const point_t& origin,
                         element_t size,
                         const point_t& scale,
                         const point_t& center,
                         element_t radius){
    return center[0] > origin[0] - radius &&
           center[0] < origin[0] + size * scale[0] + radius &&
           center[1] > origin[1] - radius &&
           center[1] < origin[1] + size * scale[1] + radius;
  }

  static bool intersects_box(const point_t& origin,
                             element_t size,
                             const point_t& scale,
                             const point_t& min2,
                             const point_t& max2){
    point_t max1 = origin;

    for(size_t d = 0; d < 2; ++d){
      max1[d] += size * scale[d];
    }
    
    return intersects_box_(origin, max1, min2, max2);
  }

  static bool intersects_box_(const point_t& min1,
                              const point_t& max1,
                              const point_t& min2,
                              const point_t& max2){
    
    for(size_t d = 0; d < 2; ++d){
      if(max1[d] < min2[d]){
        return false;
      }
      
      if(min1[d] > max2[d]){
        return false;
      }
    }
    
    return true;
  }

  static bool intersect_true(const point_t& origin,
                             element_t size,
                             const point_t& scale,
                             const point_t& center,
                             element_t radius){
    return true;
  }
};

template<typename T>
struct tree_geometry<T, 3>{
  using point_t = point<T, 3>;
  using element_t = T;

  static bool within(const point_t& origin,
                     const point_t& center,
                     element_t radius){
    return distance(origin, center) < radius;
  }

  static bool within_box(const point_t& origin,
                         const point_t& min,
                         const point_t& max){
    return origin[0] < max[0] && origin[0] > min[0] &&
           origin[1] < max[1] && origin[1] > min[1] &&
           origin[2] < max[2] && origin[2] > min[2];
  }

  static bool intersects(const point_t& origin,
                         element_t size,
                         const point_t& scale,
                         const point_t& center,
                         element_t radius){

    return center[0] > origin[0] - radius &&
           center[0] < origin[0] + size * scale[0] + radius &&
           center[1] > origin[1] - radius &&
           center[1] < origin[1] + size * scale[1] + radius &&
           center[2] > origin[2] - radius &&
           center[2] < origin[2] + size * scale[2] + radius;
  }

  static bool intersects_box(const point_t& origin,
                             element_t size,
                             const point_t& scale,
                             const point_t& min2,
                             const point_t& max2){
    point_t max1 = origin;

    for(size_t d = 0; d < 3; ++d){
      max1[d] += size * scale[d];
    }

    return intersects_box_(origin, max1, min2, max2);
  }

  static bool intersects_box_(const point_t& min1,
                              const point_t& max1,
                              const point_t& min2,
                              const point_t& max2){
    
    for(size_t d = 0; d < 3; ++d){
      if(max1[d] < min2[d]){
        return false;
      }
      
      if(min1[d] > max2[d]){
        return false;
      }
    }

    return true;
  }

  static bool intersect_true(const point_t& origin,
                             element_t size,
                             const point_t& scale,
                             const point_t& center,
                             element_t radius){
    return true;
  }
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

  template<typename S>
  branch_id(const std::array<point<S, dimension>, 2>& range,
            const point<S, dimension>& p)
  : id_(int_t(1) << bits - 1){  
    
    std::array<int_t, dimension> coords;

    for(size_t i = 0; i < dimension; ++i){
      assert(p[i] >= 0 && p[i] <= 1 && "invalid coordinates");
      S min = range[0][i];
      S scale = range[1][i] - min;
      coords[i] = (p[i] - min)/scale * (int_t(1) << (bits - 1)/dimension);
    }

    for(size_t i = 0; i < max_depth; ++i){
      for(size_t j = 0; j < dimension; ++j){
        id_ |= (coords[j] & int_t(1) << i) << i + j;
      }
    }
  }

  template<typename S>
  branch_id(const std::array<point<S, dimension>, 2>& range,
            const point<S, dimension>& p, size_t depth)
  : id_(int_t(1) << depth * dimension + 1){  
    std::array<int_t, dimension> coords;

    for(size_t i = 0; i < dimension; ++i){
      S min = range[0][i];
      S scale = range[1][i] - min;
      coords[i] = (p[i] - min)/scale * (int_t(1) << (bits - 1)/dimension);
    }

    size_t k = 0;
    for(size_t i = max_depth - depth; i < max_depth; ++i){
      for(size_t j = 0; j < dimension; ++j){
        int_t bit = (coords[j] & int_t(1) << i) >> i;
        id_ |= bit << k * dimension + j;
      }
      ++k;
    }
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

  void set_value_(int_t value){
    id_ = value;
  }

  bool operator<(const branch_id& bid) const{
    return id_ < bid.id_;
  }

  template<typename S>
  void coordinates(const std::array<point<S, dimension>, 2>& range,
                   point<S, dimension>& p) const{
    std::array<int_t, dimension> coords;
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

    constexpr int_t m = (int_t(1) << max_depth) - 1;

    for(size_t j = 0; j < dimension; ++j){
      S min = range[0][j];
      S scale = range[1][j] - min;

      coords[j] <<= max_depth - d;
      p[j] = min + scale * S(coords[j])/m;
    }
  }

private:
  int_t id_;

  constexpr branch_id(int_t id)
  : id_(id){}
};

class entity_id_t{
public:

  entity_id_t(){}

  entity_id_t(const entity_id_t& id)
  : id_(id.id_){}

  entity_id_t(size_t id)
  : id_(id){}

  operator size_t() const{
    return id_;
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
struct branch_id_hasher__{
  size_t operator()(const branch_id<T, D>& k) const{
    return std::hash<T>()(k.value_());
  }
};

enum class action : uint8_t{
  none = 0b00,
  refine = 0b01,
  coarsen = 0b10
};

template<class P>
class tree_topology : public P, public data_client_t{
public:
  using Policy = P;

  static const size_t dimension = Policy::dimension;

  using element_t = typename Policy::element_t;
  
  using point_t = point<element_t, dimension>;

  using range_t = std::pair<element_t, element_t>;

  using branch_int_t = typename Policy::branch_int_t;

  using branch_id_t = branch_id<branch_int_t, dimension>;

  using branch_id_vector_t = std::vector<branch_id_t>;


  using branch_t = typename Policy::branch_t;

  using branch_vector_t = std::vector<branch_t*>;


  using entity_t = typename Policy::entity_t;

  using entity_vector_t = std::vector<entity_t*>;
  
  using apply_function = std::function<void(branch_t&)>;

  using entity_id_vector_t = std::vector<entity_id_t>;

  using geometry_t = tree_geometry<element_t, dimension>;

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

  tree_topology(){
    branch_id_t bid = branch_id_t::root();
    root_ = new branch_t;
    root_->set_id_(bid);
    branch_map_.emplace(bid, root_);

    max_depth_ = 0;
    max_scale_ = element_t(1);

    for(size_t d = 0; d < dimension; ++d){
      range_[0][d] = element_t(0);
      range_[1][d] = element_t(1);
      scale_[d] = element_t(1);
    }
  }

  tree_topology(const point<element_t, dimension>& start,
                const point<element_t, dimension>& end){
    branch_id_t bid = branch_id_t::root();
    root_ = new branch_t;
    root_->set_id_(bid);
    branch_map_.emplace(bid, root_);

    max_depth_ = 0;

    for(size_t d = 0; d < dimension; ++d){
      scale_[d] = end[d] - start[d];
      max_scale_ = std::max(max_scale_, scale_[d]);
      range_[0][d] = start[d];
      range_[1][d] = end[d];
    }
  }

  branch_t* child(branch_t* b, size_t ci){
    return b->template child_<branch_t>(ci);
  }

  const std::vector<entity_t*>& entities() const{
    return entities_;
  }

  void insert(entity_t* ent){
    insert(ent, max_depth_);
  }

  void update(entity_t* ent){
    branch_id_t bid = ent->get_branch_id();
    branch_id_t nid = to_branch_id(ent->coordinates(), bid.depth());

    if(bid == nid){
      return;
    }

    remove(ent);
    insert(ent, max_depth_);  
  }

  void remove(entity_t* ent){
    assert(!ent->get_branch_id().is_null());

    auto itr = branch_map_.find(ent->get_branch_id());
    assert(itr != branch_map_.end());
    branch_t* b = itr->second;
    
    b->remove(ent);
    ent->set_branch_id_(branch_id_t::null());

    switch(b->requested_action_()){
      case action::none:
        break;
      case action::coarsen:{
        auto p = static_cast<branch_t*>(b->parent());
        if(p && Policy::should_coarsen(p)){
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

  point_t unit_coordinates(const point_t& p){
    point_t pn;

    for(size_t d = 0; d < dimension; ++d){
      pn[d] = (p[d] - range_[0][d]) / scale_[d];
    }

    return pn;
  }

  entity_set_t find_in_radius(const point_t& center, element_t radius){
    entity_id_vector_t ents;

    auto ef = 
    [&](entity_t* ent, const point_t& center, element_t radius) -> bool{
      return geometry_t::within(ent->coordinates(), center, radius);
    };

    size_t depth;
    element_t size;
    branch_t* b = find_start_(center, radius, depth, size);

    find_(b, size, ents, ef, geometry_t::intersects, center, radius);

    return entity_set_t(*this, std::move(ents), false);
  }

  entity_set_t find_in_radius(thread_pool& pool,
                              const point_t& center,
                              element_t radius){
    
    size_t queue_depth = get_queue_depth(pool);
    size_t m = branch_int_t(1) << queue_depth * P::dimension;

    entity_id_vector_t entity_ids;

    auto ef = 
    [&](entity_t* ent, const point_t& center, element_t radius) -> bool{
      return geometry_t::within(ent->coordinates(), center, radius);
    };

    virtual_semaphore sem(1 - int(m));
    std::mutex mtx;

    entity_id_vector_t ents;

    size_t depth;
    element_t size;
    branch_t* b = find_start_(center, radius, depth, size);
    queue_depth += depth;

    find_(pool, sem, mtx, queue_depth, depth, b, size, ents, ef,
          geometry_t::intersects, center, radius);

    sem.acquire();

    return entity_set_t(*this, std::move(ents), false);
  }

  entity_set_t find_in_box(const point_t& min, const point_t& max){
    entity_id_vector_t ents;

    auto ef = 
    [&](entity_t* ent, const point_t& min, const point_t& max) -> bool{
      return geometry_t::within_box(ent->coordinates(), min, max);
    };

    element_t radius = 0;
    for(size_t d = 0; d < dimension; ++d){
      radius = std::max(radius, max[d] - min[d]);
    }
    
    constexpr element_t c = std::sqrt(element_t(2))/element_t(2);
    radius *= c;
    
    point_t center = min;
    center += radius;

    size_t depth;
    element_t size;
    branch_t* b = find_start_(center, radius, depth, size);

    find_(b, size, ents, ef, geometry_t::intersects_box, min, max);

    return entity_set_t(*this, std::move(ents), false);
  }

  entity_set_t find_in_box(thread_pool& pool,
                           const point_t& min,
                           const point_t& max){
    
    size_t queue_depth = get_queue_depth(pool);
    size_t m = branch_int_t(1) << queue_depth * P::dimension;

    entity_id_vector_t entity_ids;

    auto ef = 
    [&](entity_t* ent, const point_t& min, const point_t& max) -> bool{
      return geometry_t::within_box(ent->coordinates(), min, max);
    };

    element_t radius = 0;
    for(size_t d = 0; d < dimension; ++d){
      radius = std::max(radius, max[d] - min[d]);
    }

    constexpr element_t c = std::sqrt(element_t(2))/element_t(2);
    radius *= c;
    
    point_t center = min;
    center += radius;

    size_t depth;
    element_t size;
    branch_t* b = find_start_(center, radius, depth, size);

    entity_id_vector_t ents;

    queue_depth += depth;

    virtual_semaphore sem(1 - int(m));
    std::mutex mtx;

    find_(pool, sem, mtx, queue_depth, depth, b, size, ents, ef,
          geometry_t::intersects_box, min, max);

    sem.acquire();

    return entity_set_t(*this, std::move(ents), false);
  }

  template<typename EF, typename... ARGS>
  void apply_in_radius(const point_t& center,
                       element_t radius,
                       EF&& ef,
                       ARGS&&... args){

    auto f = [&](entity_t* ent, const point_t& center, element_t radius){
      if(geometry_t::within(ent->coordinates(), center, radius)){
        ef(ent, std::forward<ARGS>(args)...);
      }
    };

    size_t depth;
    element_t size;
    branch_t* b = find_start_(center, radius, depth, size);

    apply_(b, size, f, geometry_t::intersects, center, radius);
  }

  template<typename EF, typename... ARGS>
  void apply_in_radius(thread_pool& pool,
                       const point_t& center,
                       element_t radius,
                       EF&& ef,
                       ARGS&&... args){
    
    size_t queue_depth = get_queue_depth(pool);
    size_t m = branch_int_t(1) << queue_depth * P::dimension;

    auto f = [&](entity_t* ent, const point_t& center, element_t radius){
      if(geometry_t::within(ent->coordinates(), center, radius)){
        ef(ent, std::forward<ARGS>(args)...);
      }
    };

    size_t depth;
    element_t size;
    branch_t* b = find_start_(center, radius, depth, size);
    queue_depth += depth;

    virtual_semaphore sem(1 - int(m));

    apply_(pool, sem, queue_depth, depth, b, size,
           f, geometry_t::intersects, center, radius);

    sem.acquire();
  }

  template<typename EF, typename... ARGS>
  void apply_in_box(thread_pool& pool,
                    const point_t& min,
                    const point_t& max,
                    EF&& ef,
                    ARGS&&... args){
    
    size_t queue_depth = get_queue_depth(pool);
    size_t m = branch_int_t(1) << queue_depth * P::dimension;

    auto f = [&](entity_t* ent, const point_t& min, const point_t& max){
      if(geometry_t::within_box(ent->coordinates(), min, max)){
        ef(ent, std::forward<ARGS>(args)...);
      }
    };

    element_t radius = 0;
    for(size_t d = 0; d < dimension; ++d){
      radius = std::max(radius, max[d] - min[d]);
    }

    constexpr element_t c = std::sqrt(element_t(2))/element_t(2);
    radius *= c;
    
    point_t center = min;
    center += radius;

    size_t depth;
    element_t size;
    branch_t* b = find_start_(center, radius, depth, size);
    queue_depth += depth;

    virtual_semaphore sem(1 - int(m));

    apply_(pool, sem, queue_depth, depth, b, size,
           f, geometry_t::intersects_box, min, max);

    sem.acquire();
  }

  template<typename EF, typename... ARGS>
  void apply_in_box(const point_t& min,
                    const point_t& max,
                    EF&& ef,
                    ARGS&&... args){
    auto f = [&](entity_t* ent, const point_t& min, const point_t& max){
      if(geometry_t::within_box(ent->coordinates(), min, max)){
        ef(ent, std::forward<ARGS>(args)...);
      }
    };

    element_t radius = 0;
    for(size_t d = 0; d < dimension; ++d){
      radius = std::max(radius, max[d] - min[d]);
    }

    constexpr element_t c = std::sqrt(element_t(2))/element_t(2);
    radius *= c;
    
    point_t center = min;
    center += radius;

    size_t depth;
    element_t size;
    branch_t* b = find_start_(center, radius, depth, size);

    apply_(b, size, f, geometry_t::intersects_box, min, max);
  }

  template<class... Args>
  entity_t* make_entity(Args&&... args){
    auto ent = new entity_t(std::forward<Args>(args)...);
    ent->set_id_(entities_.size());
    entities_.push_back(ent);
    return ent;
  }

  size_t max_depth() const{
    return max_depth_;
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

  branch_t* root(){
    return root_;
  }

  template<typename F, typename... ARGS>
  void visit(branch_t* b, F&& f, ARGS&&... args){
    visit_(b, 0, std::forward<F>(f), std::forward<ARGS>(args)...);
  }

  template<typename F, typename... ARGS>
  void visit(thread_pool& pool, branch_t* b, F&& f, ARGS&&... args){
    size_t queue_depth = get_queue_depth(pool);
    size_t m = branch_int_t(1) << queue_depth * P::dimension;

    virtual_semaphore sem(1 - int(m));

    visit_(pool, sem, b, 0, queue_depth,
           std::forward<F>(f), std::forward<ARGS>(args)...);

    sem.acquire();
  }

  template<typename F, typename... ARGS>
  void visit_children(branch_t* b, F&& f, ARGS&&... args){
    if(b->is_leaf()){
      for(auto ent : *b){
        f(ent, std::forward<ARGS>(args)...);
      }
      return;
    }

    for(size_t i = 0; i < branch_t::num_children; ++i){
      branch_t* bi = b->template child_<branch_t>(i);
      visit_children(bi, std::forward<F>(f), std::forward<ARGS>(args)...);
    }  
  }

  template<typename F, typename... ARGS>
  void visit_children(thread_pool& pool, branch_t* b, F&& f, ARGS&&... args){
    size_t queue_depth = get_queue_depth(pool);
    size_t m = branch_int_t(1) << queue_depth * P::dimension;

    virtual_semaphore sem(1 - int(m));

    visit_children_(pool, sem, 0, queue_depth, b,
                    std::forward<F>(f), std::forward<ARGS>(args)...);

    sem.acquire(); 
  }

  char* serialize(uint64_t& size){
    uint64_t num_entities = entities_.size();

    const size_t alloc_size = 
      sizeof(num_entities) + num_entities * sizeof(branch_id_t);
    
    size = alloc_size;

    char* buf = (char*)std::malloc(alloc_size);
    uint64_t pos = 0;

    std::memcpy(buf + pos, &num_entities, sizeof(num_entities));
    pos += sizeof(num_entities);

    for(size_t entity_id = 0; entity_id < num_entities; ++entity_id){
      entity_t* ent = entities_[entity_id];
      branch_int_t bid = ent->get_branch_id().value_();
      std::memcpy(buf + pos, &bid, sizeof(bid));
      pos += sizeof(bid);
    }

    return buf;
  }

  void unserialize(char* buf){
    uint64_t pos = 0;

    uint64_t num_entities;
    std::memcpy(&num_entities, buf + pos, sizeof(num_entities));
    pos += sizeof(num_entities);

    entities_.resize(num_entities);

    for(size_t entity_id = 0; entity_id < num_entities; ++entity_id){
      entity_t* ent = new entity_t;
      entities_[entity_id] = ent;

      ent->set_id_(entity_id);

      branch_int_t bi;
      std::memcpy(&bi, buf + pos, sizeof(bi));
      pos += sizeof(bi);
      
      branch_id_t bid;
      bid.set_value_(bi);

      insert(ent, bid);
    }
  }
    
private:
  using branch_map_t = std::unordered_map<branch_id_t, branch_t*,
    branch_id_hasher__<branch_int_t, dimension>>;

    branch_id_t to_branch_id(const point_t& p, size_t max_depth){
      return branch_id_t(range_, p, max_depth);
    }

    branch_id_t to_branch_id(const point_t& p){
      return branch_id_t(range_, p);
    }

    void insert(entity_t* ent, size_t max_depth){
      branch_id_t bid = to_branch_id(ent->coordinates(), max_depth);
      branch_t* b = find_parent(bid, max_depth);
      ent->set_branch_id_(b->id());

      b->insert(ent);

      switch(b->requested_action_()){
        case action::none:
          break;
        case action::refine:
          refine_(b);
          break;
        default:
          assert(false && "invalid action");
      }
    }

    void insert(entity_t* ent, branch_id_t bid){
      branch_t* b = find_parent(bid, max_depth_);
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

    void refine_(branch_t* b){
      branch_id_t pid = b->id();
      size_t depth = pid.depth() + 1;

      if(!b->template into_branch_<branch_t>()){
        return;
      }
      
      for(size_t i = 0; i < branch_t::num_children; ++i){
        branch_t* ci = b->template child_<branch_t>(i);
        branch_map_.emplace(ci->id(), ci);
      }

      max_depth_ = std::max(max_depth_, depth);

      for(auto ent : *b){
        insert(ent, depth);
      }

      b->clear();
      b->reset();
    }

    // helper method in coarsening
    // insert into p, coarsen all recursive children of b

    void coarsen_(branch_t* p, branch_t* b){
      if(b->is_leaf()){
        return;
      }

      for(size_t i = 0; i < branch_t::num_children; ++i){
        branch_t* ci = b->template child_<branch_t>(i);
        
        for(auto ent : *ci){
          p->insert(ent);
          ent->set_branch_id_(p->id());
        }

        coarsen_(p, ci);
        branch_map_.erase(ci->id());
      }
    }

    void coarsen_(branch_t* p){    
      coarsen_(p, p);
      p->template into_leaf_<branch_t>();
      p->reset();
    }

    size_t get_queue_depth(thread_pool& pool){
      size_t n = pool.num_threads();
      constexpr size_t rb = branch_int_t(1) << P::dimension;
      constexpr double bn = std::log2(double(rb));
      return std::log2(double(n))/bn + 1;
    }

    branch_t* find_start_(const point_t& center,
                          element_t radius,
                          size_t& depth,
                          element_t& size){
      
      element_t norm_radius = radius / max_scale_;

      branch_id_t bid = to_branch_id(center, max_depth_);

      int d = -std::log2(norm_radius) - 1;

      while(d > 0){
        branch_t* b = find_parent(bid, d);

        point_t p2;
        b->id().coordinates(range_, p2);

        size = std::pow(element_t(2), -d);

        bool found = true;
        for(size_t dim = 0; dim < dimension; ++dim){
          if(!(center[dim] - radius > p2[dim] &&
               center[dim] + radius < p2[dim] + size)){
            found = false;
            break;
          }
        }

        if(found){
          depth = d;
          return b;
        }

        --d;
      }

      depth = 0;
      size = element_t(1);
      return root_;
    }

    template<typename EF, typename BF, typename... ARGS>
    void apply_(branch_t* b,
                element_t size,
                EF&& ef,
                BF&& bf,
                ARGS&&... args){

      if(b->is_leaf()){
        for(auto ent : *b){
          ef(ent, std::forward<ARGS>(args)...);
        }
        return;      
      }
      
      size /= 2;

      for(size_t i = 0; i < branch_t::num_children; ++i){
        branch_t* ci = b->template child_<branch_t>(i);

        if(bf(ci->coordinates(range_), size, scale_, std::forward<ARGS>(args)...)){
          apply_(ci, size,
                 std::forward<EF>(ef), std::forward<BF>(bf),
                 std::forward<ARGS>(args)...);
        }        
      }
    }


    template<typename EF, typename BF, typename... ARGS>
    void apply_(thread_pool& pool,
                virtual_semaphore& sem,
                size_t queue_depth,
                size_t depth,
                branch_t* b,
                element_t size,
                EF&& ef,
                BF&& bf,
                ARGS&&... args){

      if(b->is_leaf()){
        for(auto ent : *b){
          ef(ent, std::forward<ARGS>(args)...);
        }

        size_t m = branch_int_t(1) << (queue_depth - depth) * P::dimension;

        for(size_t i = 0; i < m; ++i){
          sem.release(); 
        }

        return;   
      }

      size /= 2;
      ++depth;

      for(size_t i = 0; i < branch_t::num_children; ++i){
        branch_t* ci = b->template child_<branch_t>(i);

        if(bf(ci->coordinates(range_), size, scale_, std::forward<ARGS>(args)...)){        
          if(depth == queue_depth){

            auto f = [&, size, ci](){
              apply_(ci, size,
                std::forward<EF>(ef), std::forward<BF>(bf),
                std::forward<ARGS>(args)...);

              sem.release();
            };

            pool.queue(f);
          }
          else{
            apply_(pool, sem, queue_depth, depth, ci, size,
                   std::forward<EF>(ef), std::forward<BF>(bf),
                   std::forward<ARGS>(args)...);
          }
        }
        else{
          if(depth > queue_depth){
            continue;
          }

          size_t m = 
            branch_int_t(1) << (queue_depth - depth) * P::dimension;

          for(size_t i = 0; i < m; ++i){
            sem.release(); 
          }
        }     
      }
    }

    template<typename EF, typename BF, typename... ARGS>
    void find_(branch_t* b,
               element_t size,
               entity_id_vector_t& ents,
               EF&& ef,
               BF&& bf,
               ARGS&&... args){

      if(b->is_leaf()){
        for(auto ent : *b){
          if(ef(ent, std::forward<ARGS>(args)...)){
            ents.push_back(ent->id());
          }
        }
        return;      
      }
      
      size /= 2;

      for(size_t i = 0; i < branch_t::num_children; ++i){
        branch_t* ci = b->template child_<branch_t>(i);

        if(bf(ci->coordinates(range_), size, scale_, std::forward<ARGS>(args)...)){
          find_(ci, size, ents,
                std::forward<EF>(ef), std::forward<BF>(bf),
                std::forward<ARGS>(args)...);
        }        
      }
    }


    template<typename EF, typename BF, typename... ARGS>
    void find_(thread_pool& pool,
               virtual_semaphore& sem,
               std::mutex& mtx,
               size_t queue_depth,
               size_t depth,
               branch_t* b,
               element_t size,
               entity_id_vector_t& ents,
               EF&& ef,
               BF&& bf,
               ARGS&&... args){

      if(b->is_leaf()){
        mtx.lock();
        for(auto ent : *b){
          if(ef(ent, std::forward<ARGS>(args)...)){
            ents.push_back(ent->id());
          }
        }
        mtx.unlock();

        size_t m = branch_int_t(1) << (queue_depth - depth) * P::dimension;

        for(size_t i = 0; i < m; ++i){
          sem.release(); 
        }

        return;
      }

      size /= 2;
      ++depth;

      for(size_t i = 0; i < branch_t::num_children; ++i){
        branch_t* ci = b->template child_<branch_t>(i);

        if(bf(ci->coordinates(range_), size, scale_, std::forward<ARGS>(args)...)){        
          if(depth == queue_depth){

            auto f = [&, size, ci](){
              entity_id_vector_t branch_ents;

              find_(ci, size, branch_ents,
                std::forward<EF>(ef), std::forward<BF>(bf),
                std::forward<ARGS>(args)...);

              mtx.lock();
              ents.insert(ents.end(), branch_ents.begin(), branch_ents.end());
              mtx.unlock();

              sem.release();
            };

            pool.queue(f);
          }
          else{
            find_(pool, sem, mtx, queue_depth, depth, ci, size, ents,
                  std::forward<EF>(ef), std::forward<BF>(bf),
                  std::forward<ARGS>(args)...);
          }
        }
        else{
          if(depth > queue_depth){
            continue;
          }

          size_t m = 
            branch_int_t(1) << (queue_depth - depth) * P::dimension;

          for(size_t i = 0; i < m; ++i){
            sem.release(); 
          }
        }
      }
    }

    template<typename F, typename... ARGS>
    void visit_(branch_t* b, size_t depth, F&& f, ARGS&&... args){
      if(f(b, depth, std::forward<ARGS>(args)...)){
        return;
      }

      if(b->is_leaf()){
        return;
      }

      for(size_t i = 0; i < branch_t::num_children; ++i){
        branch_t* bi = b->template child_<branch_t>(i);
        visit_(bi, depth + 1, std::forward<F>(f), std::forward<ARGS>(args)...);
      }
    }

    template<typename F, typename... ARGS>
    void visit_(thread_pool& pool,
                virtual_semaphore& sem,
                branch_t* b,
                size_t depth,
                size_t queue_depth,
                F&& f,
                ARGS&&... args){

      if(depth == queue_depth){
        auto vf = [&, depth, b](){
          visit_(b, depth, std::forward<F>(f), std::forward<ARGS>(args)...);
          sem.release();
        };

        pool.queue(vf);
        return;
      }

      if(f(b, depth, std::forward<ARGS>(args)...)){
        size_t m = branch_int_t(1) << (queue_depth - depth) * P::dimension;

        for(size_t i = 0; i < m; ++i){
          sem.release(); 
        }

        return;
      }

      if(b->is_leaf()){
        size_t m = branch_int_t(1) << (queue_depth - depth) * P::dimension;

        for(size_t i = 0; i < m; ++i){
          sem.release(); 
        }

        return;      
      }

      for(size_t i = 0; i < branch_t::num_children; ++i){
        branch_t* bi = b->template child_<branch_t>(i);

        visit_(pool, sem, bi, depth + 1, queue_depth,
               std::forward<F>(f), std::forward<ARGS>(args)...);
      }
    }

    template<typename F, typename... ARGS>
    void visit_children_(thread_pool& pool,
                         virtual_semaphore& sem,
                         size_t depth,
                         size_t queue_depth,
                         branch_t* b,
                         F&& f,
                         ARGS&&... args){

      if(depth == queue_depth){
        auto vf = [&, b](){
          visit_children(b, std::forward<F>(f), std::forward<ARGS>(args)...);
          sem.release();
        };

        pool.queue(vf);
        return;
      }
      
      if(b->is_leaf()){
        for(auto ent : *b){
          f(ent, std::forward<ARGS>(args)...);
        }

        size_t m = branch_int_t(1) << (queue_depth - depth) * P::dimension;

        for(size_t i = 0; i < m; ++i){
          sem.release(); 
        }

        return;      
      }

      for(size_t i = 0; i < branch_t::num_children; ++i){
        branch_t* bi = b->template child_<branch_t>(i);
        visit_children_(pool, sem, depth + 1, queue_depth,
                        bi, std::forward<F>(f), std::forward<ARGS>(args)...);
      }  
    }


  branch_map_t branch_map_;
  
  size_t max_depth_;

  branch_t* root_;

  std::vector<entity_t*> entities_;
  std::array<point<element_t, dimension>, 2> range_;
  point<element_t, dimension> scale_;
  element_t max_scale_;
};

template<typename T, size_t D>
class tree_entity{
public:
  using id_t = entity_id_t;

  using branch_id_t = branch_id<T, D>;

  tree_entity()
  : branch_id_(branch_id_t::null()){}

  branch_id_t get_branch_id() const{
    return branch_id_;
  }

  entity_id_t id() const{
    return id_;
  }

private:
  template<class P>
  friend class tree_topology;

  void set_id_(entity_id_t id){
    id_ = id;
  }

  void set_branch_id_(branch_id_t bid){
    branch_id_ = bid;
  }

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
  : action_(action::none),
  parent_(nullptr),
  children_(nullptr){}

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

  tree_branch* parent() const{
    return parent_;
  }

  bool is_leaf() const{
    return !children_;
  }

private:
  template<class P>
  friend class tree_topology;

  void set_id_(branch_id_t id){
    id_ = id;
  }

  action requested_action_(){
    return action_;
  }

  template<class B>
  B* child_(size_t ci) const{
    assert(ci < num_children);
    return static_cast<B*>(children_) + ci;
  }

  template<class B>
  bool into_branch_(){
    if(children_){
      return false;
    }

    auto c = new B[num_children];

    for(branch_int_t bi = 0; bi < num_children; ++bi){
      B& ci = c[bi];
      ci.id_ = id_;
      ci.id_.push(bi);
      ci.parent_ = this;
      ci.children_ = nullptr;
    }

    children_ = c;

    return true;
  }

  template<class B>
  void into_leaf_(){
    if(children_){
      delete[] static_cast<B*>(children_);
      children_ = nullptr;      
    }
  }

  tree_branch* parent_;
  tree_branch* children_;

  branch_id_t id_;
  action action_;
};

} // namespace tree
} // namespace flecsi
