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

namespace flecsi {
namespace topology {

template<class T>
struct default_predicate__{
  bool operator()(const T& v) const{
    return true;
  }
};

template<class T>
struct index_space_storage{
  using index_t = typename T::index_t;

  T* index_space_get(index_t i){
    return items_[i];
  }

  void index_space_add(T* item){
    items_.push_back(item);
  }

  std::vector<T*> items_;
};

template<class T,
         bool OWNED = false,
         bool STORAGE = false,
         class S = index_space_storage<T>,
         class F = default_predicate__<T*>>
class index_space_{
public:

  class iterator_{
   public:
    using index_t = typename T::index_t;
    using index_vector_t = std::vector<index_t>;

    iterator_(const iterator_& itr)
    : store_(itr.store_), items_(itr.items_), index_(itr.index_),
    count_(items_->size()){}

    iterator_(S& store, const index_vector_t& items, size_t index)
    : store_(&store), items_(&items), index_(index), count_(items_->size()){}

    iterator_& operator++(){
      ++index_;
      return *this;
    }

    iterator_& operator=(const iterator_ & itr){
      store_ = itr.store_;
      index_ = itr.index_;
      items_ = itr.items_;
      return *this;
    }

    T* operator*(){
      while(index_ < count_){
        auto item = store_->index_space_get((*items_)[index_]);
        if(F()(item)){
          return item;
        }
        ++index_;
      }

      return nullptr;
    }

    T* operator->(){
      while(index_ < count_){
        auto item = store_->index_space_get((*items_)[index_]);
        if(F()(item)){
          return item;
        }
        ++index_;
      }
    }

    bool operator==(const iterator_& itr) const{
      return index_ == itr.index_;
    }

    bool operator!=(const iterator_& itr) const{
      return index_ != itr.index_;
    }

   private:
    S* store_;
    const index_vector_t* items_;
    size_t index_;
    size_t count_;
  };

  using index_vector_t = typename iterator_::index_vector_t;

  using filter_function = std::function<bool(T&)>;

  using apply_function = std::function<void(T&)>;

  template<typename R>
  using map_function = std::function<R(T&)>;

  template<typename R>
  using reduce_function = std::function<void(T&, R&)>;

  index_space_(S& store, const index_vector_t& v, bool sorted = false)
  : store_(&store), v_(&v), begin_(0), end_(v_->size()),
  owned_(false), sorted_(sorted){}

  index_space_(S& store, index_vector_t&& v, bool sorted)
  : store_(&store), v_(new index_vector_t(std::move(v))),
  begin_(0), end_(v_->size()), owned_(true), sorted_(sorted){}

  index_space_(const index_vector_t& v, bool sorted = false)
  : store_(new S), v_(&v), begin_(0), end_(v_->size()),
  owned_(false), sorted_(sorted){
    assert(STORAGE && "invalid instantiation");
  }

  index_space_(index_vector_t&& v, bool sorted)
  : store_(new S), v_(std::move(v)), begin_(0), end_(v_->size()),
  owned_(true), sorted_(sorted){
    assert(STORAGE && "invalid instantiation");
  }

  index_space_()
  : store_(new S), begin_(0), end_(0),
  owned_(true), sorted_(true){
    assert(STORAGE && "invalid instantiation");
    assert(OWNED && "invalid instantiation");
  }

  ~index_space_(){
    if(STORAGE){
      delete store_;
    }

    if(OWNED || owned_){
      delete v_;
    }
  }

  index_space_ & operator=(const index_space_& r) = default;

  iterator_ begin() const { return iterator_(*store_, *v_, begin_); }

  iterator_ end() const { return iterator_(*store_, *v_, end_); }

  T* operator[](size_t i) const{
    return store_->template index_space_get((*v_)[begin_ + i]);
  }

  T* front() const{
    return store_->template index_space_get((*v_)[begin_]);
  }

  T* back() const{
    return store_->template index_space_get((*v_)[end_ - 1]);
  }

  size_t size() const { return end_ - begin_; }

  index_space_ filter(filter_function f) const {
    index_vector_t v;

    for (auto item : *this) {
      if (f(*item)) {
        v.push_back(item->id());
      }
    }

    return index_space_(*store_, std::move(v), sorted_);
  }

  template<typename R>
  std::vector<index_space_> scatter(map_function<T> f) const {

    std::map<R, index_vector_t> id_map;
    for (auto item : *this)
      id_map[f(*item)].push_back(item->id());

    std::vector<index_space_> ent_map;
    for ( auto entry : id_map )
      ent_map.emplace_back(
        std::move(index_space_(*store_, std::move(entry.second), sorted_) )
      );

    return ent_map;
  }

  void apply(apply_function f) const {
    for (auto ent : *this) {
      f(ent);
    }
  }

  template<typename R>
  std::vector<R> map(map_function<T> f) const {
    std::vector<S> ret;
    ret.reserve(v_->size());

    for (auto item : *this) {
      ret.push_back(f(*item));
    }
    return ret;
  }

  template<typename R>
  R reduce(T start, reduce_function<T> f) const {
    T r = start;

    for (auto item : *this) {
      f(*item, r);
    }

    return r;
  }

  void prepare_(){
    if(!OWNED && !owned_){
      v_ = new index_vector_t(*v_);
      owned_ = true;
    }

    if(!sorted_){
      auto vc = const_cast<index_vector_t*>(v_);
      std::sort(vc->begin(), vc->end());
      sorted_ = true;
    }
  }

  index_space_& operator&=(const index_space_& r){
    prepare_();

    index_vector_t ret;

    if(r.sorted_){
      ret.resize(std::min(v_->size(), r.v_->size()));

      auto itr = std::set_intersection(v_->begin(), v_->end(),
                                       r.v_->begin(), r.v_->end(), ret.begin());

      ret.resize(itr - ret.begin());
    }
    else{
      index_vector_t v2(*r.v_);
      std::sort(v2.begin(), v2.end());

      ret.resize(std::min(v_->size(), v2.size()));

      auto itr = std::set_intersection(v_->begin(), v_->end(),
                                       v2.begin(), v2.end(), ret.begin());

      ret.resize(itr - ret.begin());
    }

    delete v_;
    v_ = new index_vector_t(std::move(ret));

    begin_ = 0;
    end_ = v_->size();

    return *this;
  }

  index_space_ operator&(const index_space_& r) const{
    index_space_ ret(*this);
    ret &= r;
    return ret;
  }

  index_space_& operator|=(const index_space_& r){
    prepare_();

    index_vector_t ret;

    if(r.sorted_){
      ret.resize(v_->size() + r.v_->size());

      auto itr = std::set_union(v_->begin(), v_->end(),
                                r.v_->begin(), r.v_->end(), ret.begin());

     ret.resize(itr - ret.begin());
    }
    else{
      index_vector_t v2(*r.v_);

      std::sort(v2.begin(), v2.end());

      ret.resize(v_->size() + v2.size());

      auto itr = std::set_union(v_->begin(), v_->end(),
                                v2.begin(), v2.end(), ret.begin());

      ret.resize(itr - ret.begin());
    }

    delete v_;
    v_ = new index_vector_t(std::move(ret));

    begin_ = 0;
    end_ = v_->size();

    return *this;
  }

  index_space_ operator|(const index_space_& r) const{
    index_space_ ret(*this);
    ret |= r;
    return ret;
  }

  index_space_& operator-=(const index_space_& r){
    prepare_();

    index_vector_t ret(v_->size());

    if(r.sorted_){
      auto itr = std::set_difference(v_->begin(), v_->end(),
                                     r.v_->begin(), r.v_->end(), ret.begin());

      ret.resize(itr - ret.begin());
    }
    else{
      index_vector_t v2(*r.v_);

      std::sort(v2.begin(), v2.end());

      auto itr = std::set_difference(v_->begin(), v_->end(),
                                     v2.begin(), v2.end(), ret.begin());

      ret.resize(itr - ret.begin());
    }

    delete v_;
    v_ = new index_vector_t(std::move(ret));

    begin_ = 0;
    end_ = v_->size();

    return *this;
  }

  index_space_ operator-(const index_space_& r) const{
    index_space_ ret(*this);
    ret -= r;
    return ret;
  }

  void add(T* item){
    if(!OWNED && !owned_){
      v_ = new index_vector_t(*v_);
      owned_ = true;
    }

    auto vc = const_cast<index_vector_t*>(v_);

    if(STORAGE || store_ == this){
      index_space_add(item);
    }

    if(sorted_){
      auto id = item->id();
      auto itr = std::upper_bound(vc->begin(), vc->end(), id);
      vc->insert(itr, id);
    }
    else{
      vc->push_back(item->id());
    }
  }

  index_space_& operator<<(T* item){
    add(item);
    return *this;
  }

private:
  S* store_ = nullptr;
  const index_vector_t* v_ = nullptr;
  size_t begin_ = 0;
  size_t end_ = 0;
  bool owned_ = false;
  bool sorted_ = true;
};

} // namespace topology
} // namespace flecsi
