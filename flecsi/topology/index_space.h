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

#include <algorithm>
#include <map>
#include <type_traits>
#include <vector>

namespace flecsi {
namespace topology {

/*----------------------------------------------------------------------------*
 * class index_space
 *----------------------------------------------------------------------------*/

/*! 
 \class index_space
 \brief index_space provides a compile-time
  configurable and iterable container of objects, e.g. mesh/tree topology
  entities and their id's. Index space defines the concept of STORAGE -
  whether the actual entities referenced are stored within this index space
  OR contained in a 'master' index space. OWNERSHIP - whether its set of id's
  are owned by this index space or aliased to another index space and then mustcbe copied before this index space can then modify them. SORTED - refers to if the id's are sorted and can then have set operations directly applied to them, else the index space must first be sorted. To make operations on index spaces faster, the index space is parameterized on a number of these parameters and can be efficiently recast depending on how it is to be used: STORAGE - if true then this is a 'master' index space with its own storage. OWNED - if true then id ownership is definitely true, else must check owned_ at runtime. SORTED - if true then id's are definitely stored and shall remain in sorted order.
*/

template<
  class T,
  bool STORAGE = false,
  bool OWNED = true,
  bool SORTED = false,
  class F = void
>
class index_space
{
public:
  using id_t = typename std::remove_pointer<T>::type::id_t;
  
  using id_vector_t = std::vector<id_t>;
  using item_vector_t = std::vector<T>;

  using item_t = typename std::remove_pointer<T>::type;

  using filter_function = std::function<bool(T&)>;

  using apply_function = std::function<void(T&)>;

  template<typename S>
  using map_function = std::function<S(T&)>;

  template<typename S>
  using reduce_function = std::function<void(T&, S&)>;

  /*!
    Iterable id range.
   */
  class id_range_{
  public:
    id_range_(
      const id_range_& r
    )
    : items_(r.items_), begin_(r.begin_), end_(r.end_)
    {}

    id_range_(
      const id_vector_t& items
    )
    : items_(&items), begin_(0), end_(items_->size())
    {}

    id_range_(
      const id_vector_t& items,
      size_t begin,
      size_t end
      )
    : items_(&items), begin_(begin), end_(end)
    {}

    id_range_&
    operator=(
      const id_range_& r
    )
    {
      items_ = r.items_;
      begin_ = r.begin_;
      end_ = r.end_;
    }

    typename id_vector_t::const_iterator
    begin() const
    { 
      return items_->begin() + begin_;
    }

    typename id_vector_t::const_iterator
    end() const
    { 
      return items_->begin() + end_;
    }

  private:
    const id_vector_t* items_;
    size_t begin_;
    size_t end_;
  };

  /*!
    Iterator base, const be parameterized with 'T' or 'const T'.
   */
  template<
    class S
  >
  class iterator_base_
  {
   public:
    using MS = typename std::remove_const<S>::type;

    iterator_base_(
      const iterator_base_& itr
    )
    : items_(itr.items_), index_(itr.index_), end_(itr.end_), s_(itr.s_)
    {}

    iterator_base_(
      item_vector_t* s,
      const id_vector_t& items,
      size_t index,
      size_t end
    )
    : items_(&items), index_(index), end_(end), s_(s)
    {}

    iterator_base_(
      const item_vector_t* s,
      const id_vector_t& items,
      size_t index,
      size_t end
    )
    : items_(&items), index_(index), end_(end),
    s_(const_cast<item_vector_t*>(s))
    {}

    iterator_base_&
    operator=(
      const iterator_base_ & itr
    )
    {
      index_ = itr.index_;
      end_ = itr.end_;
      items_ = itr.items_;
      s_ = itr.s_;
      return *this;
    }

    bool
    operator==(
      const iterator_base_& itr
      ) const
    {
      return index_ == itr.index_;
    }

    bool
    operator!=(
      const iterator_base_& itr
    ) const
    {
      return index_ != itr.index_;
    }

    T&
    get_(
      size_t index
    )
    {
      return (*s_)[(*items_)[index].index_space_index()];
    }

   protected:
    const id_vector_t* items_;
    size_t index_;
    size_t end_;
    item_vector_t* s_;
  };

  /*!
    Predicated iterator.
   */
  template<
    class S,
    class P
  >
  class iterator_ : public iterator_base_<S>
  {
  public:
    using B = iterator_base_<S>;

    iterator_(
      const iterator_& itr
    )
    : B(itr)
    {}

    iterator_(
      item_vector_t* s,
      const id_vector_t& items,
      size_t index,
      size_t end
    )
    : B(s, items, index, end)
    {}

    iterator_(
      const item_vector_t* s,
      const id_vector_t& items,
      size_t index,
      size_t end
    )
    : B(s, items, index, end)
    {}

    iterator_&
    operator++()
    {
      ++B::index_;
      return *this;
    }

    S&
    operator*()
    {
      while(B::index_ < B::end_)
      {
        T& item = B::get_(B::index_);
        if(P()(item))
        {
          return item;
        }
        ++B::index_;
      }

      assert(false && "end of range");
    }

    S*
    operator->()
    {
      while(B::index_ < B::end_)
      {
        T& item = B::get_(B::index_);
        if(P()(item))
        {
          return &item;
        }
        ++B::index_;
      }

      assert(false && "end of range");
    }
  };

  /*!
    Non-predicated iterator.
   */
  template<
    class S
  >
  class iterator_<S, void> : public iterator_base_<S>{
  public:
    using B = iterator_base_<S>;

    iterator_(
      const iterator_& itr
    )
    : B(itr)
    {}

    iterator_(
      item_vector_t* s,
      const id_vector_t& items,
      size_t index,
      size_t end
    )
    : B(s, items, index, end)
    {}

    iterator_(
      const item_vector_t* s,
      const id_vector_t& items,
      size_t index,
      size_t end
    )
    : B(s, items, index, end)
    {}

    iterator_&
    operator++()
    {
      ++B::index_;
      return *this;
    }

    S&
    operator*()
    {
      return B::get_(B::index_);
    }

    S*
    operator->()
    {
      return &B::get_(B::index_);
    }
  };

  index_space(
    bool storage = STORAGE
  )
  : v_(new id_vector_t), begin_(0), end_(0), owned_(true),
    sorted_(SORTED), s_(storage ? new item_vector_t : nullptr)
  {
    assert((STORAGE || !storage) && "invalid instantiation");
    static_assert(OWNED, "expected OWNED");
  }

  /*!
    Slice an existing index space.
   */
  template<
    class S,
    bool STORAGE2,
    bool OWNED2,
    bool SORTED2,
    class F2
  > 
  index_space(
    const index_space<S, STORAGE2, OWNED2, SORTED2, F2>& is,
    size_t begin,
    size_t end
  )
  : v_(is.v_), begin_(begin), end_(end), owned_(false),
    sorted_(is.sorted_), s_(reinterpret_cast<item_vector_t*>(is.s_))
  {

    static_assert(std::is_convertible<T,S>::value,
                  "invalid index space construction");
    static_assert(!STORAGE, "expected !STORAGE");
    static_assert(!OWNED, "expected !OWNED");
  }

  /*!
    Alias an existing index space unless OWNED.
   */
  index_space(
    const index_space& is
  )
  : v_(OWNED ? new id_vector_t(*is.v_) : is.v_),
    begin_(is.begin_), end_(is.end_), owned_(OWNED), sorted_(is.sorted_), 
    s_(is.s_)
  {
    assert(s_ && "no storage");
    static_assert(!STORAGE, "expected !STORAGE");
  }

  index_space(
    index_space&& is
  )
  : v_(std::move(is.v_)), begin_(std::move(is.begin_)), 
    end_(std::move(is.end_)), sorted_(std::move(is.sorted_)), 
    s_(std::move(is.s_))
  {

    if(OWNED || is.owned_)
    {
      is.v_ = nullptr;
      owned_ = true;
    }
    else{
      owned_ = false;
    }

    if(STORAGE)
    {
      is.s_ = nullptr;
    }

    is.begin_ = 0;
    is.end_ = 0;
  }

  ~index_space()
  {
    if(OWNED || owned_)
    {
      delete v_;
    }

    if(STORAGE)
    {
      delete s_;
    }
  }

  /*!
    Alias an existing index space unless OWNED.
   */
  index_space&
  operator=(
    const index_space& is
  )
  {
    assert(!STORAGE && "invalid assignment");

    if(OWNED)
    {
      delete v_;
      v_ = new id_vector_t(*is.v_);
      owned_ = true;
    }
    else{
      v_ = is.v_;
      owned_ = false;
    }

    begin_ = is.begin_;
    end_ = is.end_;
    sorted_ = is.sorted_;
    s_ = is.s_;
    
    return *this;
  }

  index_space&
  operator=(
    index_space&& is
  )
  {
    v_ = std::move(is.v_);

    if(OWNED || is.owned_)
    {
      is.v_ = nullptr;
      owned_ = true;
    }
    else{
      owned_ = false;
    }

    begin_ = std::move(is.begin_);
    end_ = std::move(is.end_);
    sorted_ = std::move(is.sorted_);
    s_ = std::move(is.s_);

    if(STORAGE)
    {
      is.s_ = nullptr;
    }
    
    is.begin_ = 0;
    is.end_ = 0;

    return *this;
  }

  template<
    class S,
    bool STORAGE2 = STORAGE,
    bool OWNED2 = OWNED,
    bool SORTED2 = SORTED,
    class F2 = F
  >
  auto&
  cast()
  {
    static_assert(std::is_convertible<S,T>::value,
                  "invalid index space cast");

    return *reinterpret_cast<index_space<S,STORAGE2,OWNED2,SORTED2,F2>*>(this);
  }

  template<
    class S,
    bool STORAGE2 = STORAGE,
    bool OWNED2 = OWNED,
    bool SORTED2 = SORTED,
    class F2 = F
  >
  auto&
  cast() const
  {
    static_assert(std::is_convertible<S,T>::value,
                  "invalid index space cast");

    return *reinterpret_cast<const index_space<S,STORAGE2,OWNED2,SORTED2,F2>*>(this);
  }

  auto
  begin()
  { 
    return iterator_<T, F>(s_, *v_, begin_, end_);
  }

  auto
  begin() const
  { 
    return iterator_<const T, F>(s_, *v_, begin_, end_);
  }

  auto
  end()
  {
    return iterator_<T, F>(s_, *v_, end_, end_); 
  }

  auto
  end() const
  {
    return iterator_<const T, F>(s_, *v_, end_, end_); 
  }

  size_t
  begin_offset() const
  {
    return begin_;
  }

  size_t
  end_offset() const
  {
    return end_;
  }

  T&
  get_offset(
    size_t offset
  )
  {
    return (*s_)[(*v_)[offset].index_space_index()];
  }

  const T&
  get_offset(
    size_t offset
  ) const
  {
    return (*s_)[(*v_)[offset].index_space_index()];
  }  

  id_range_
  ids() const
  {
    return id_range_(*v_, begin_, end_);
  }

  id_range_
  ids(
    size_t begin,
    size_t end
  ) const
  {
    return id_range_(*v_, begin, end);
  }

  template<
    class S = T
  >
  auto
  slice(
    size_t begin,
    size_t end
  ) const
  {
    return index_space<S, false, false, SORTED>(*this, begin, end);
  }

  template<
    class S = T
  >
  auto slice() const
  {
    return index_space<S, false, false, SORTED>(*this, begin_, end_);
  }

  T&
  get_(
    size_t offset
  )
  {
    return (*s_)[(*v_)[begin_ + offset].index_space_index()];
  }

  const T&
  get_(
    size_t offset
  ) const
  {
    return (*s_)[(*v_)[begin_ + offset].index_space_index()];
  }

  T&
  get_end_(
    size_t offset
  )
  {
    return (*s_)[(*v_)[end_ - 1 - offset].index_space_index()];
  }

  const T&
  get_end_(
    size_t offset
  ) const
  {
    return (*s_)[(*v_)[end_ - 1 - offset].index_space_index()];
  }

  T&
  operator[](
    size_t i
  )
  {
    return get_(i);
  }

  const T&
  operator[](
    size_t i
  ) const
  {
    return get_(i);
  }

  const id_t&
  operator()(
    size_t i
  ) const
  {
    return (*v_)[begin_ + i];
  }

  id_t&
  operator()(
    size_t i
  )
  {
    return (*v_)[begin_ + i];
  }

  T&
  front()
  {
    return get_(0);
  }

  const T&
  front() const
  {
    return get_(0);
  }

  T&
  back()
  {
    return get_end_(0);
  }

  const T&
  back() const
  {
    return get_end_(0);
  }

  size_t
  size() const
  {
    return end_ - begin_; 
  }
  
  bool
  empty() const
  {
    return begin_ == end_;
  }

  void
  clear()
  {
    begin_ = 0;
    end_ = 0;

    if(OWNED || owned_)
    {
      v_->clear();
      sorted_ = SORTED;
    }

    if(STORAGE)
    {
      s_->clear();
    }
  }

  template<
    bool STORAGE2,
    bool OWNED2,
    bool SORTED2,
    class F2
  >
  void
  set_master(
    const index_space<T, STORAGE2, OWNED2, SORTED2, F2>& master
  )
  {
    set_master(const_cast<index_space<T, STORAGE2, OWNED2, SORTED2, F2>&>(master));
  }

  /*!
    Set storage to point to a master index space.
   */
  template<
    bool STORAGE2,
    bool OWNED2,
    bool SORTED2,
    class F2
  >
  void
  set_master(
    index_space<T, STORAGE2, OWNED2, SORTED2, F2>& master
  )
  {
    s_ = reinterpret_cast<item_vector_t*>(master.s_);
  }

  std::vector<const T>
  to_vec() const
  {
    return to_vec_<const T>();
  }

  std::vector<T>
  to_vec()
  {
    return to_vec_<T>();
  }

  template<
    class S
  >
  std::vector<S>
  to_vec_()
  {
    std::vector<S> ret;
    size_t n = end_ - begin_;
    ret.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      ret.push_back((*this)[i]);
    } // for
    return ret;
  }

  const id_vector_t&
  id_vec() const
  {
    return *v_;
  }

  id_t*
  id_array() const
  {
    return v_->data();
  }

  template<
    typename Predicate
  >
  auto
  filter(
    Predicate && f
  ) const
  {
    index_space<T, false, true, false> is;
    is.set_master(*this);

    for (auto item : *this) {
      if (std::forward<Predicate>(f)(item)) {
        is.push_back(item);
      }
    }

    return is;
  }

  void
  apply(
    apply_function f
  ) const 
  {
    for (auto ent : *this) {
      f(ent);
    }
  }

  template<
    class S
  >
  auto
  map(
    map_function<S> f
  ) const 
  {
    index_space<S, false, true, false> is;
    is.set_master(*this);

    is.begin_push_(v_->size());

    for (auto item : *this) {
      is.batch_push_(f(*item));
    }
    return is;
  }

  template<
    typename S
  >
  S
  reduce(
    T start,
    reduce_function<T> f
  ) const 
  {
    T r = start;

    for (auto item : *this) {
      f(*item, r);
    }

    return r;
  }

  //! \brief Bin entities using a predicate function.
  //! 
  //! The predicate function returns some sortable key that is
  //! used to bin entities.
  //!
  //! \taram Predicate  The type of the predicate function.
  //!
  //! \param f  The predicate function.  Should return a sortable
  //!   bin key that determines the order of the result.
  //! \return a vector of index_space's, with each element corresponding
  //!   to a specific bin.
  //!
  //! \remark This version returns a map
  template<
    typename Predicate
  >
  auto
  bin(
    Predicate && f
  ) const 
  {
    return bin_as_map( std::forward<Predicate>(f) );
  }

  //! \brief Bin entities using a predicate function.
  //! 
  //! The predicate function returns some sortable key that is
  //! used to bin entities.
  //!
  //! \taram Predicate  The type of the predicate function.
  //!
  //! \param f  The predicate function.  Should return a sortable
  //!   bin key that determines the order of the result.
  //! \return a vector of index_space's, with each element corresponding
  //!   to a specific bin.
  //!
  //! \remark This version returns a map, and is less costly to bin,
  //!   but more costly to access.
  template<
    typename Predicate
  >
  auto
  bin_as_map(
    Predicate && f
  ) const 
  {

    // If the list was sorted beforehand, the result will also be
    // sorted.  Own the index_vector_t (i.e., create storage for it)
    using new_index_space_t = index_space<T, false, true, SORTED>;

    // get the predicate result type
    using result_t = 
      std::decay_t< decltype( std::forward<Predicate>(f)(operator[](0))) >;

    // bin the data, and use a map to sort the keys
    std::map<result_t, new_index_space_t> bins;
    for (auto ent : *this)
      bins[std::forward<Predicate>(f)(ent)].push_back(ent);

    // now go and set the master
    for ( auto & entry : bins )
      entry.second.set_master(*this);

    return bins;
  }

  //! \brief Bin entities using a predicate function.
  //! 
  //! The predicate function returns some sortable key that is
  //! used to bin entities.
  //!
  //! \taram Predicate  The type of the predicate function.
  //!
  //! \param f  The predicate function.  Should return a sortable
  //!   bin key that determines the order of the result.
  //! \return a vector of index_space's, with each element corresponding
  //!   to a specific bin.
  //!
  //! \remark This version returns a vector.
  template<
    typename Predicate
  >
  auto
  bin_as_vector(
    Predicate && f
  ) const 
  {

    // get the bins as a map
    auto bins_map = bin_as_map( std::forward<Predicate>(f) );

    // If the list was sorted beforehand, the result will also be
    // sorted.  Own the index_vector_t (i.e., create storage for it)
    using new_index_space_t = 
      typename std::decay_t<decltype(bins_map)>::mapped_type;

    // now extract the results and store them in a vector.
    // should be faster for access.
    std::vector< new_index_space_t > bins_vec;
    bins_vec.reserve( bins_map.size() );

    for ( auto && entry : bins_map )
      bins_vec.emplace_back( std::move(entry.second) );

    return bins_vec;
  }


  void
  prepare_()
  {
    if(!OWNED && !owned_)
    {
      v_ = new id_vector_t(*v_);
      owned_ = true;
    }

    if(!SORTED && !sorted_)
    {
      auto vc = const_cast<id_vector_t*>(v_);
      std::sort(vc->begin(), vc->end());
      sorted_ = true;
    }
  }

  index_space&
  operator&=(
    const index_space& r
  )
  {
    prepare_();

    id_vector_t ret;

    if(r.sorted_)
    {
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

  index_space
  operator&(
    const index_space& r
  ) const
  {
    index_space ret(*this);
    ret &= r;
    return ret;
  }

  index_space&
  operator|=(
    const index_space& r
  )
  {
    prepare_();

    id_vector_t ret;

    if(r.sorted_)
    {
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

  index_space
  operator|(
    const index_space& r
  ) const
  {
    index_space ret(*this);
    ret |= r;
    return ret;
  }

  index_space&
  operator-=(
    const index_space& r
  )
  {
    prepare_();

    id_vector_t ret(v_->size());

    if(r.sorted_)
    {
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

  index_space
  operator-(
    const index_space& r
  ) const
  {
    index_space ret(*this);
    ret -= r;
    return ret;
  }

  void
  push_back(
    const T& item
  )
  {
    if(!OWNED && !owned_)
    {
      v_ = new id_vector_t(*v_);
      owned_ = true;
    }

    if(STORAGE)
    {
      s_->push_back(item);
    }

    if(SORTED || sorted_)
    {
      auto id = id_(item);
      auto itr = std::upper_bound(v_->begin(), v_->end(), id);
      v_->insert(itr, id);
    }
    else{
      v_->push_back(id_(item));
    }

    ++end_;
  }

  void
  push_back(
    id_t index
  )
  {
    if(!OWNED && !owned_)
    {
      v_ = new id_vector_t(*v_);
      owned_ = true;
    }

    if(SORTED || sorted_)
    {
      auto itr = std::upper_bound(v_->begin(), v_->end(), index);
      v_->insert(itr, index);
    }
    else{
      v_->push_back(index);
    }

    ++end_;
  }

  void
  append(
    const index_space& is
  )
  {
    if(!OWNED && !owned_)
    {
      v_ = new id_vector_t(*v_);
      owned_ = true;
    }
    
    if(STORAGE)
    {
      s_->insert(s_->begin(), is.s_->begin(), is.s_->end());
    }

    size_t n = is.size();
    
    if(SORTED || sorted_)
    {
      v_->reserve(n);

      for(auto ent : is)
      {
        id_t id = id_(ent);
        auto itr = std::upper_bound(v_->begin(), v_->end(), id);
        v_->insert(itr, id);
      }
    }
    else{
      v_->insert(v_->begin(), is.v_->begin(), is.v_->end());
    }

    end_ += n;
  }

  index_space&
  operator<<(
    T item
  )
  {
    push_back(item);
    return *this;
  }

  void
  append_(
    const std::vector<T>& ents,
    const std::vector<id_t>& ids
  )
  {
    static_assert(STORAGE, "expected STORAGE");
    static_assert(OWNED, "expected OWNED");

    assert(ents.size() == ids.size());
    
    s_->insert(s_->begin(), ents.begin(), ents.end());

    size_t n = ents.size();
    
    if(SORTED || sorted_)
    {
      v_->reserve(n);

      for(id_t id : ids)
      {
        auto itr = std::upper_bound(v_->begin(), v_->end(), id);
        v_->insert(itr, id);
      }
    }
    else{
      v_->insert(v_->begin(), ids.begin(), ids.end());
    }

    end_ += n;
  }

  id_t
  id_(
    const item_t& item
  )
  {
    return item.index_space_id();
  }

  id_t
  id_(
    const item_t* item
  )
  {
    return item->index_space_id();
  }
  
private:
  template<class, bool, bool, bool, class> 
  friend class index_space;

  friend class connectivity_t;
  
  id_vector_t* v_;
  size_t begin_;
  size_t end_;
  bool owned_;
  bool sorted_;
  item_vector_t* s_;

  /*!
    Private methods for efficiently populating an index space.
   */
  size_t
  begin_push_()
  {
    static_assert(OWNED, "expected OWNED");
    return v_->size();
  }

  /*!
    Private methods for efficiently populating an index space.
   */
  void
  reserve_(
    size_t n
  )
  {
    static_assert(OWNED, "expected OWNED");
    return v_->reserve(n);
  }

  /*!
    Private methods for efficiently populating an index space.
   */
  void
  begin_push_(
    size_t n
  )
  {
    static_assert(OWNED, "expected OWNED");
    assert(begin_ == 0);
    size_t m = v_->size();
    v_->reserve(m + n);
    end_ += n;
  }

  /*!
    Private methods for efficiently populating an index space.
   */
  void
  batch_push_(
    id_t index
  )
  {
    static_assert(OWNED, "expected OWNED");
    v_->push_back(index);
  }

  /*!
    Private methods for efficiently populating an index space.
   */
  void
  push_(
    id_t index
    )
  {
    static_assert(OWNED, "expected OWNED");
    v_->push_back(index);
    ++end_;
  }


  /*!
    Private methods for efficiently populating an index space.
   */
  void
  end_push_(
    size_t n
  )
  {
    static_assert(OWNED, "expected OWNED");
    assert(begin_ == 0);
    end_ += v_->size() - n;
  }

  /*!
    Private methods for efficiently populating an index space.
   */
  void
  resize_(
    size_t n
  )
  {
    static_assert(OWNED, "expected OWNED");
    assert(begin_ == 0);
    v_->resize(n);
    end_ = v_->size();
  }

  /*!
    Private methods for efficiently populating an index space.
   */
  void
  fill_(
    id_t index
  )
  {
    static_assert(OWNED, "expected OWNED");
    std::fill(v_->begin(), v_->end(), index);
  }

  /*!
    Private methods for efficiently populating an index space.
   */
  id_vector_t&
  id_vec_()
  {
    return *v_;
  }

  /*!
    Private methods for efficiently populating an index space.
   */
  typename id_vector_t::iterator
  index_begin_()
  {
    return v_->begin();
  }

  /*!
    Private methods for efficiently populating an index space.
   */
  typename id_vector_t::iterator
  index_end_()
  {
    return v_->end();
  }
};

class simple_id
{
public:
  simple_id(
    size_t id
  )
  : id_(id)
  {}

  operator size_t() const
  {
    return id_;
  }

  bool
  operator<(
    const simple_id& eid
  ) const
  {
    return id_ < eid.id_;
  }

  size_t
  index_space_index() const
  {
    return id_;
  }

private:
  size_t id_;
};

template<
  typename T
>
class simple_entry
{
public:
  using id_t = simple_id;

  simple_entry(
    id_t id,
    const T& entry
  )
  : id_(id),
  entry_(entry)
  {}

  operator T() const
  {
    return entry_;
  }

  auto entry_id() const
  {
    return entry_;
  }

  id_t
  index_space_id() const
  {
    return id_;
  }

private:
  id_t id_;
  T entry_;
};

} // namespace topology
} // namespace flecsi
