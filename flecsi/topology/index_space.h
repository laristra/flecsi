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

#include <type_traits>

namespace flecsi {
namespace topology {

template<class T>
struct default_predicate__{
  bool operator()(const T& v) const{
    return true;
  }
};

template<class T,
         bool STORAGE = false,
         bool OWNED = true,
         bool SORTED = false,
         class F = default_predicate__<T>>
class index_space{
public:
  using id_t = typename std::remove_pointer<T>::type::id_t;
  
  using id_vector_t = std::vector<id_t>;
  using item_vector_t = std::vector<T>;

  using item_t = typename std::remove_pointer<T>::type;

  class id_range_{
  public:
    id_range_(const id_range_& r)
    : items_(r.items_), begin_(r.begin_), end_(r.end_){}

    id_range_(const id_vector_t& items)
    : items_(&items), begin_(0), end_(items_->size()){}

    id_range_(const id_vector_t& items, size_t begin, size_t end)
    : items_(&items), begin_(begin), end_(end){}

    typename id_vector_t::const_iterator begin() const{ 
      return items_->begin() + begin_;
    }

    typename id_vector_t::const_iterator end() const{ 
      return items_->begin() + end_;
    }

  private:
    const id_vector_t* items_;
    size_t begin_;
    size_t end_;
  };

  template<class S>
  class iterator_{
   public:
    using MS = typename std::remove_const<S>::type;

    iterator_(const iterator_& itr)
    : items_(itr.items_), index_(itr.index_), end_(itr.end_), s_(itr.s_){}

    iterator_(item_vector_t* s, const id_vector_t& items, size_t index, size_t end)
    : items_(&items), index_(index), end_(end), s_(s){}

    iterator_(const item_vector_t* s, const id_vector_t& items, size_t index, size_t end)
    : items_(&items), index_(index), end_(end), s_(const_cast<item_vector_t*>(s)){}

    iterator_& operator++(){
      ++index_;
      return *this;
    }

    iterator_& operator=(const iterator_ & itr){
      index_ = itr.index_;
      end_ = itr.end_;
      items_ = itr.items_;
      s_ = itr.s_;
      return *this;
    }

    S& operator*(){
      while(index_ < end_){
        T& item = get_(index_);
        if(F()(item)){
          return item;
        }
        ++index_;
      }

      assert(false && "end of range");
    }

    S* operator->(){
      while(index_ < end_){
        T& item = get_(index_);
        if(F()(item)){
          return &item;
        }
        ++index_;
      }

      assert(false && "end of range");
    }

    bool operator==(const iterator_& itr) const{
      return index_ == itr.index_;
    }

    bool operator!=(const iterator_& itr) const{
      return index_ != itr.index_;
    }

    T& get_(size_t index){
      return (*s_)[(*items_)[index].index_space_index()];
    }

   private:
    const id_vector_t* items_;
    size_t index_;
    size_t end_;
    item_vector_t* s_;
  };

  using filter_function = std::function<bool(T&)>;

  using apply_function = std::function<void(T&)>;

  template<typename S>
  using map_function = std::function<S(T&)>;

  template<typename S>
  using reduce_function = std::function<void(T&, S&)>;

  index_space(bool storage = true)
  : v_(new id_vector_t), begin_(0), end_(0), owned_(true),
    sorted_(SORTED), s_(storage ? new item_vector_t : nullptr){
    assert((STORAGE || !storage) && "invalid instantiation");
    assert(OWNED && "invalid instantiation");
  }

  template<class S, bool STORAGE2, bool OWNED2, bool SORTED2, class F2> 
  index_space(const index_space<S, STORAGE2, OWNED2, SORTED2, F2>& is,
              size_t begin, size_t end)
  : v_(is.v_), begin_(begin), end_(end), owned_(false),
    sorted_(is.sorted_), s_(reinterpret_cast<item_vector_t*>(is.s_)){

    static_assert(std::is_convertible<T,S>::value,
                  "invalid index space construction");

    assert(!STORAGE && "invalid instantiation");
    assert(!OWNED && "invalid instantiation");
  }

  index_space(const index_space& is)
  : v_(OWNED ? new id_vector_t(*is.v_) : is.v_),
    begin_(is.begin_), end_(is.end_), owned_(OWNED), sorted_(is.sorted_), 
    s_(is.s_){
    assert(!STORAGE && "invalid instantiation");
  }

  ~index_space(){
    if(OWNED || owned_){
      delete v_;
    }

    if(STORAGE){
      delete s_;
    }
  }
  
  template<class S, bool STORAGE2 = STORAGE, bool OWNED2 = OWNED,
    bool SORTED2 = SORTED, class F2 = F>
  auto& cast(){
    static_assert(std::is_convertible<S,T>::value,
                  "invalid index space cast");

    return *reinterpret_cast<index_space<S,STORAGE2,OWNED2,SORTED2,F2>*>(this);
  }

  template<class S, bool STORAGE2 = STORAGE, bool OWNED2 = OWNED, bool SORTED2 = SORTED, class F2 = F>
  auto& cast() const{
    static_assert(std::is_convertible<S,T>::value,
                  "invalid index space cast");

    return *reinterpret_cast<const index_space<S,STORAGE2,OWNED2,SORTED2,F2>*>(this);
  }

  index_space& operator=(const index_space& r){
    assert(!STORAGE && "invalid assignment");
    assert(!OWNED && "invalid assignment");
    
    v_ = r.v_;
    begin_ = r.begin_;
    end_ = r.end_;
    owned_ = false;
    sorted_ = r.sorted_;
    s_ = r.s_;
    
    return *this;
  }

  iterator_<T> begin(){ 
    return iterator_<T>(s_, *v_, begin_, end_);
  }

  iterator_<const T> begin() const{ 
    return iterator_<const T>(s_, *v_, begin_, end_);
  }

  iterator_<T> end(){
    return iterator_<T>(s_, *v_, end_, end_); 
  }

  iterator_<const T> end() const{
    return iterator_<const T>(s_, *v_, end_, end_); 
  }

  id_range_ ids() const{
    return id_range_(*v_, begin_, end_);
  }

  id_range_ ids(size_t begin, size_t end) const{
    return id_range_(*v_, begin, end);
  }

  template<class S = T>
  auto slice(size_t begin, size_t end) const{
    return index_space<S, false, false, SORTED>(*this, begin, end);
  }

  template<class S = T>
  auto slice() const{
    return index_space<S, false, false, SORTED>(*this, begin_, end_);
  }

  template<class S = T>
  auto slice(size_t begin, size_t end){
    return index_space<S, false, false, SORTED>(*this, begin, end);
  }

  template<class S = T>
  auto slice(){
    return index_space<S, false, false, SORTED>(*this, begin_, end_);
  }

  T& get_(size_t offset){
    return (*s_)[(*v_)[begin_ + offset].index_space_index()];
  }

  const T& get_(size_t offset) const{
    return (*s_)[(*v_)[begin_ + offset].index_space_index()];
  }

  T& get_end_(size_t offset){
    return (*s_)[(*v_)[end_ - 1 - offset].index_space_index()];
  }

  const T& get_end_(size_t offset) const{
    return (*s_)[(*v_)[end_ - 1 - offset].index_space_index()];
  }

  T& operator[](size_t i){
    return get_(i);
  }

  const T& operator[](size_t i) const{
    return get_(i);
  }

  const id_t& operator()(size_t i) const{
    return (*v_)[begin_ + i];
  }

  id_t& operator()(size_t i){
    return (*v_)[begin_ + i];
  }

  T& front(){
    return get_(0);
  }

  const T& front() const{
    return get_(0);
  }

  T& back(){
    return get_end_(0);
  }

  const T& back() const{
    return get_end_(0);
  }

  size_t size() const { return end_ - begin_; }
  
  bool empty() const { return begin_ == end_; }

  void clear(){
    begin_ = 0;
    end_ = 0;

    if(OWNED || owned_){
      v_->clear();
      sorted_ = SORTED;
    }
  }

  template<bool STORAGE2, bool OWNED2, bool SORTED2, class F2>
  void set_master(const index_space<T, STORAGE2, OWNED2, SORTED2, F2>& master){
    set_master(const_cast<index_space<T, STORAGE2, OWNED2, SORTED2, F2>&>(master));
  }

  template<bool STORAGE2, bool OWNED2, bool SORTED2, class F2>
  void set_master(index_space<T, STORAGE2, OWNED2, SORTED2, F2>& master){
    s_ = reinterpret_cast<item_vector_t*>(master.s_);
  }

  std::vector<const T> to_vec() const{
    std::vector<const T> ret;
    size_t n = end_ - begin_;
    ret.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      ret.push_back((*this)[i]);
    } // for
    return ret;
  }

  std::vector<T> to_vec(){
    std::vector<T> ret;
    size_t n = end_ - begin_;
    ret.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      ret.push_back((*this)[i]);
    } // for
    return ret;
  }

  const id_vector_t& id_vec() const{
    return *v_;
  }

  id_t* id_array() const{
    return v_->data();
  }

  auto filter(filter_function f) const {
    index_space<T, false, true, false> is(false);
    is.set_master(*this);

    for (auto item : *this) {
      if (f(item)) {
        is.push_back(item);
      }
    }

    return is;
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
    if(!OWNED && !owned_){
      v_ = new id_vector_t(*v_);
      owned_ = true;
    }

    if(!SORTED && !sorted_){
      auto vc = const_cast<id_vector_t*>(v_);
      std::sort(vc->begin(), vc->end());
      sorted_ = true;
    }
  }

  index_space& operator&=(const index_space& r){
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

    *v_ = std::move(ret);

    begin_ = 0;
    end_ = v_->size();

    return *this;
  }

  index_space operator&(const index_space& r) const{
    index_space ret(*this);
    ret &= r;
    return ret;
  }

  index_space& operator|=(const index_space& r){
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

    *v_ = std::move(ret);

    begin_ = 0;
    end_ = v_->size();

    return *this;
  }

  index_space operator|(const index_space& r) const{
    index_space ret(*this);
    ret |= r;
    return ret;
  }

  index_space& operator-=(const index_space& r){
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

    *v_ = std::move(ret);

    begin_ = 0;
    end_ = v_->size();

    return *this;
  }

  index_space operator-(const index_space& r) const{
    index_space ret(*this);
    ret -= r;
    return ret;
  }

  void push_back(const T& item){
    if(!OWNED && !owned_){
      v_ = new id_vector_t(*v_);
      owned_ = true;
    }

    if(STORAGE){
      s_->push_back(item);
    }

    if(SORTED || sorted_){
      auto id = id_(item);
      auto itr = std::upper_bound(v_->begin(), v_->end(), id);
      v_->insert(itr, id);
    }
    else{
      v_->push_back(id_(item));
    }

    ++end_;
  }

  void push_back(id_t index){
    if(!OWNED && !owned_){
      v_ = new id_vector_t(*v_);
      owned_ = true;
    }

    if(SORTED || sorted_){
      auto itr = std::upper_bound(v_->begin(), v_->end(), index);
      v_->insert(itr, index);
    }
    else{
      v_->push_back(index);
    }

    ++end_;
  }

  index_space& operator<<(T item){
    push_back(item);
    return *this;
  }

  void append_(std::vector<T>& ents, std::vector<id_t>& ids){
    assert(STORAGE);
    assert(OWNED);
    assert(ents.size() == ids.size());
    
    s_->insert(s_->begin(), ents.begin(), ents.end());

    size_t n = ents.size();
    
    if(SORTED || sorted_){
      v_->reserve(n);

      for(id_t id : ids){
        auto itr = std::upper_bound(v_->begin(), v_->end(), id);
        v_->insert(itr, id);
      }
    }
    else{
      v_->insert(v_->begin(), ids.begin(), ids.end());
    }

    end_ += n;
  }

  id_t id_(const item_t& item){
    return item.index_space_id();
  }

  id_t id_(item_t* item){
    return item->index_space_id();
  }
  
private:
  template<class, bool, bool, bool, class> 
  friend class index_space;

  friend class connectivity_t;
  
  template<class>
  friend class tree_topology;

  id_vector_t* v_;
  size_t begin_;
  size_t end_;
  bool owned_;
  bool sorted_;
  item_vector_t* s_;

  size_t begin_push_(){
    assert(OWNED);
    return v_->size();
  }

  void begin_push_(size_t n){
    assert(OWNED);
    assert(begin_ == 0);
    size_t m = v_->size();
    v_->reserve(m + n);
    end_ += n;
  }

  void batch_push_(id_t index){
    assert(OWNED);
    v_->push_back(index);
  }

  void push_(id_t index){
    assert(OWNED);
    v_->push_back(index);
    ++end_;
  }

  void end_push_(size_t n){
    assert(OWNED);
    assert(begin_ == 0);
    end_ += v_->size() - n;
  }

  void resize_(size_t n){
    assert(OWNED);
    assert(begin_ == 0);
    v_->resize(n);
    end_ = v_->size();
  }

  void fill_(id_t index){
    assert(OWNED);
    std::fill(v_->begin(), v_->end(), index);
  }

  id_vector_t& id_vec_(){
    return *v_;
  }

  typename id_vector_t::iterator index_begin_(){
    return v_->begin();
  }

  typename id_vector_t::iterator index_end_(){
    return v_->end();
  }
};

} // namespace topology
} // namespace flecsi
