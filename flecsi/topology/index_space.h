/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*! @file */

#include <algorithm>
#include <functional>
#include <map>
#include <type_traits>
#include <vector>

namespace flecsi {
namespace topology {

/*----------------------------------------------------------------------------*
 * class index_space__
 *----------------------------------------------------------------------------*/

template<size_t, class E>
class domain_entity__;

//! helper classes for resolving types
template<typename T>
struct index_space_ref_type__ {
  using type = T &;
};

//! helper classes for resolving types
template<typename S>
struct index_space_ref_type__<S *> {
  using type = S *;
};

//! helper classes for resolving types
template<size_t M, class E>
struct index_space_ref_type__<domain_entity__<M, E>> {
  using type = domain_entity__<M, E>;
};

//----------------------------------------------------------------------------//
//! index_space__ provides a compile-time
//! configurable and iterable container of objects, e.g. mesh/tree topology
//! entities and their id's. Index space defines the concept of STORAGE -
//! whether the actual entities referenced are stored within this index space
//! OR contained in a 'master' index space. OWNERSHIP - whether its set of id's
//! are owned by this index space or aliased to another index space and then
//! must be copied before this index space can then modify them. SORTED - refers
//! to if the id's are sorted and can then have set operations directly applied
//! to them, else the index space must first be sorted. To make operations on
//! index spaces faster, the index space is parameterized on a number of these
//! parameters and can be efficiently recast depending on how it is to be used.
//!
//! @tparam STORAGE if true then this is a 'master' index space with its own
//!   storage.
//!
//! @tparam OWNED if true then id ownership is definitely true, else must check
//!   owned_ at runtime
//!
//! @tparam SORTED if true then id's are definitely sorted and shall
//!   be maintained in sorted order.
//!
//! @tparam F iterator predicate/filter function
//!
//! @ingroup topology
//----------------------------------------------------------------------------//
template<
    class T,
    bool STORAGE = false,
    bool OWNED = true,
    bool SORTED = false,
    class F = void,
    template<typename, typename...> class ID_STORAGE_TYPE = std::vector,
    template<typename, typename...> class STORAGE_TYPE = ID_STORAGE_TYPE>
class index_space__ {
public:
  //! ID type
  using id_t = typename std::remove_pointer<T>::type::id_t;

  //! ID storage type
  using id_storage_t = ID_STORAGE_TYPE<id_t>;

  //! Storage type
  using storage_t = STORAGE_TYPE<T>;

  //! item, e.g. entity type
  using item_t = typename std::remove_pointer<T>::type;

  //! Reference type
  using ref_t = typename index_space_ref_type__<T>::type;

  //! reference casting type
  using cast_t = std::decay_t<ref_t>;

  //! filter predicate function signature
  using filter_function = std::function<bool(T &)>;

  //! apply function signature
  using apply_function = std::function<void(T &)>;

  //! map function signature
  template<typename S>
  using map_function = std::function<S(T &)>;

  //! Reduce function signature
  template<typename S>
  using reduce_function = std::function<void(T &, S &)>;

  //--------------------------------------------------------------------------//
  //! Iterable id range.
  //!
  //! @ingroup topology
  //--------------------------------------------------------------------------//
  class id_range_ {
  public:
    //-----------------------------------------------------------------//
    //! Copy constructor
    //-----------------------------------------------------------------//
    id_range_(const id_range_ & r)
        : items_(r.items_), begin_(r.begin_), end_(r.end_) {}

    //-----------------------------------------------------------------//
    //! Initialize range from items
    //-----------------------------------------------------------------//
    id_range_(const id_storage_t & items)
        : items_(&items), begin_(0), end_(items_->size()) {}

    //-----------------------------------------------------------------//
    //! Initialize range from items with beginning and end
    //-----------------------------------------------------------------//
    id_range_(const id_storage_t & items, size_t begin, size_t end)
        : items_(&items), begin_(begin), end_(end) {}

    //-----------------------------------------------------------------//
    //! Assignment operator
    //-----------------------------------------------------------------//
    id_range_ & operator=(const id_range_ & r) {
      items_ = r.items_;
      begin_ = r.begin_;
      end_ = r.end_;
    }

    //-----------------------------------------------------------------//
    //! Get begin iterator
    //-----------------------------------------------------------------//
    typename id_storage_t::const_iterator begin() const {
      return items_->begin() + begin_;
    }

    //-----------------------------------------------------------------//
    //! Get end iterator
    //-----------------------------------------------------------------//
    typename id_storage_t::const_iterator end() const {
      return items_->begin() + end_;
    }

  private:
    const id_storage_t * items_;
    size_t begin_;
    size_t end_;
  };

  //------------------------------------------------------------------//
  //! Iterator base, const be parameterized with 'T' or 'const T'
  //------------------------------------------------------------------//
  template<class S>
  class iterator_base_ {
  public:
    using MS = typename std::remove_const<S>::type;

    //-----------------------------------------------------------------//
    //! Copy constructor
    //-----------------------------------------------------------------//
    iterator_base_(const iterator_base_ & itr)
        : items_(itr.items_), index_(itr.index_), end_(itr.end_), s_(itr.s_) {}

    //-----------------------------------------------------------------//
    //! Initialize iterator from items and range
    //-----------------------------------------------------------------//
    iterator_base_(
        storage_t * s,
        const id_storage_t & items,
        size_t index,
        size_t end)
        : items_(&items), index_(index), end_(end), s_(s) {}

    //-----------------------------------------------------------------//
    //! Initialize iterator from items and range
    //-----------------------------------------------------------------//
    iterator_base_(
        const storage_t * s,
        const id_storage_t & items,
        size_t index,
        size_t end)
        : items_(&items), index_(index), end_(end),
          s_(const_cast<storage_t *>(s)) {}

    //-----------------------------------------------------------------//
    //! Assignment operator
    //-----------------------------------------------------------------//
    iterator_base_ & operator=(const iterator_base_ & itr) {
      index_ = itr.index_;
      end_ = itr.end_;
      items_ = itr.items_;
      s_ = itr.s_;
      return *this;
    }

    //-----------------------------------------------------------------//
    //! Equality operator
    //-----------------------------------------------------------------//
    bool operator==(const iterator_base_ & itr) const {
      return index_ == itr.index_;
    }

    //-----------------------------------------------------------------//
    //! Inequality operator
    //-----------------------------------------------------------------//
    bool operator!=(const iterator_base_ & itr) const {
      return index_ != itr.index_;
    }

    //-----------------------------------------------------------------//
    //! Helper method. Get item at index
    //-----------------------------------------------------------------//
    auto get_(size_t index) {
      return static_cast<cast_t>((*s_)[(*items_)[index].index_space_index()]);
    }

  protected:
    const id_storage_t * items_;
    size_t index_;
    size_t end_;
    storage_t * s_;
  };

  //------------------------------------------------------------------------//
  //! Predicated iterator.
  //!
  //! @tparam S reference return type
  //! @tparam P predicate functor
  //!
  //! @ingroup topology
  //------------------------------------------------------------------------//
  template<class S, class P>
  class iterator_ : public iterator_base_<S> {
  public:
    using B = iterator_base_<S>;

    //-----------------------------------------------------------------//
    //! Copy constructors
    //-----------------------------------------------------------------//
    iterator_(const iterator_ & itr) : B(itr) {}

    //-----------------------------------------------------------------//
    //! Initialize iterator from item storage and index range
    //-----------------------------------------------------------------//
    iterator_(
        storage_t * s,
        const id_storage_t & items,
        size_t index,
        size_t end)
        : B(s, items, index, end) {}

    //-----------------------------------------------------------------//
    //! Initialize iterator from item storage and index range
    //-----------------------------------------------------------------//
    iterator_(
        const storage_t * s,
        const id_storage_t & items,
        size_t index,
        size_t end)
        : B(s, items, index, end) {}

    //-----------------------------------------------------------------//
    //! Increment operator
    //-----------------------------------------------------------------//
    iterator_ & operator++() {
      ++B::index_;
      return *this;
    }

    //-----------------------------------------------------------------//
    //! Dereference operator
    //-----------------------------------------------------------------//
    S & operator*() {
      while (B::index_ < B::end_) {
        T & item = B::get_(B::index_);
        if (P()(item)) {
          return item;
        }
        ++B::index_;
      }

      assert(false && "end of range");
    }

    //-----------------------------------------------------------------//
    //! Arrow operator
    //-----------------------------------------------------------------//
    S * operator->() {
      while (B::index_ < B::end_) {
        T & item = B::get_(B::index_);
        if (P()(item)) {
          return &item;
        }
        ++B::index_;
      }

      assert(false && "end of range");
    }
  };

  //------------------------------------------------------------------------//
  //! Non-predicated iterator.
  //!
  //! @tparam S reference return type
  //!
  //! @ingroup topology
  //------------------------------------------------------------------------//
  template<class S>
  class iterator_<S, void> : public iterator_base_<S> {
  public:
    using B = iterator_base_<S>;

    using ref_t = index_space_ref_type__<S>;

    //-----------------------------------------------------------------//
    //! Copy constructor
    //-----------------------------------------------------------------//
    iterator_(const iterator_ & itr) : B(itr) {}

    //-----------------------------------------------------------------//
    //! Initialize iterator from items and range
    //-----------------------------------------------------------------//
    iterator_(
        storage_t * s,
        const id_storage_t & items,
        size_t index,
        size_t end)
        : B(s, items, index, end) {}

    //-----------------------------------------------------------------//
    //! Initialize iterator from items and range
    //-----------------------------------------------------------------//
    iterator_(
        const storage_t * s,
        const id_storage_t & items,
        size_t index,
        size_t end)
        : B(s, items, index, end) {}

    //-----------------------------------------------------------------//
    //! Increment operator
    //-----------------------------------------------------------------//
    iterator_ & operator++() {
      ++B::index_;
      return *this;
    }

    //-----------------------------------------------------------------//
    //! Dereference operator
    //-----------------------------------------------------------------//
    S operator*() {
      return B::get_(B::index_);
    }

    //-----------------------------------------------------------------//
    //! Arrow operator
    //-----------------------------------------------------------------//
    S * operator->() {
      return &B::get_(B::index_);
    }
  };

  //-----------------------------------------------------------------//
  //! Constructor. If storage is true then allocate storage type,
  //! else this index space will index into a separate storage.
  //-----------------------------------------------------------------//
  index_space__(bool storage = STORAGE)
      : v_(new id_storage_t), begin_(0), end_(0), owned_(true), sorted_(SORTED),
        s_(storage ? new storage_t : nullptr) {
    assert((STORAGE || !storage) && "invalid instantiation");
  }

  //-----------------------------------------------------------------//
  //! Slice constructor. A slice is a view on existing index space and
  //! narrower range.
  //-----------------------------------------------------------------//
  template<
      class S,
      bool STORAGE2,
      bool OWNED2,
      bool SORTED2,
      class F2,
      template<typename, typename...> class ID_STORAGE_TYPE2,
      template<typename, typename...> class STORAGE_TYPE2>
  index_space__(
      const index_space__<
          S,
          STORAGE2,
          OWNED2,
          SORTED2,
          F2,
          ID_STORAGE_TYPE2,
          STORAGE_TYPE2> & is,
      size_t begin,
      size_t end)
      : v_(is.v_), begin_(begin), end_(end), owned_(false), sorted_(is.sorted_),
        s_(reinterpret_cast<storage_t *>(is.s_)) {

    static_assert(
        std::is_convertible<T, S>::value, "invalid index space construction");
    static_assert(!STORAGE, "expected !STORAGE");
    static_assert(!OWNED, "expected !OWNED");
  }

  //-----------------------------------------------------------------//
  //! Constructor to alias an existing index space unless OWNED.
  //-----------------------------------------------------------------//
  index_space__(const index_space__ & is)
      : v_(OWNED ? new id_storage_t(*is.v_) : is.v_), begin_(is.begin_),
        end_(is.end_), owned_(OWNED), sorted_(is.sorted_), s_(is.s_) {
    assert(s_ && "no storage");
    static_assert(!STORAGE, "expected !STORAGE");
  }

  //-----------------------------------------------------------------//
  //! Move constructor.
  //-----------------------------------------------------------------//
  index_space__(index_space__ && is)
      : v_(std::move(is.v_)), begin_(std::move(is.begin_)),
        end_(std::move(is.end_)), sorted_(std::move(is.sorted_)),
        s_(std::move(is.s_)) {

    if (OWNED || is.owned_) {
      is.v_ = nullptr;
      owned_ = true;
    } else {
      owned_ = false;
    }

    if (STORAGE) {
      is.s_ = nullptr;
    }

    is.begin_ = 0;
    is.end_ = 0;
  }

  //-----------------------------------------------------------------//
  //! Destructor.
  //-----------------------------------------------------------------//
  ~index_space__() {
    if (OWNED || owned_) {
      delete v_;
    }

    if (STORAGE) {
      delete s_;
    }
  }

  //-----------------------------------------------------------------//
  //! Return the storage object.
  //-----------------------------------------------------------------//
  storage_t * storage() {
    return s_;
  }

  //-----------------------------------------------------------------//
  //! Return the storage object.
  //-----------------------------------------------------------------//
  const storage_t * storage() const {
    return s_;
  }

  //-----------------------------------------------------------------//
  //! Set the storage object.
  //-----------------------------------------------------------------//
  void set_storage(storage_t * s) {
    s_ = s;
  }

  //-----------------------------------------------------------------//
  //! Assignment operator. Alias an existing index space unless OWNED.
  //-----------------------------------------------------------------//
  index_space__ & operator=(const index_space__ & is) {
    assert(!STORAGE && "invalid assignment");

    if (OWNED) {
      delete v_;
      v_ = new id_storage_t(*is.v_);
      owned_ = true;
    } else {
      v_ = is.v_;
      owned_ = false;
    }

    begin_ = is.begin_;
    end_ = is.end_;
    sorted_ = is.sorted_;
    s_ = is.s_;

    return *this;
  }

  //-----------------------------------------------------------------//
  //! Move assignment operator.
  //-----------------------------------------------------------------//
  index_space__ & operator=(index_space__ && is) {
    v_ = std::move(is.v_);

    if (OWNED || is.owned_) {
      is.v_ = nullptr;
      owned_ = true;
    } else {
      owned_ = false;
    }

    begin_ = std::move(is.begin_);
    end_ = std::move(is.end_);
    sorted_ = std::move(is.sorted_);
    s_ = std::move(is.s_);

    if (STORAGE) {
      is.s_ = nullptr;
    }

    is.begin_ = 0;
    is.end_ = 0;

    return *this;
  }

  //-----------------------------------------------------------------//
  //! Cast an index space by re-specifying template parameters such
  //! as OWNED or SORTED.
  //-----------------------------------------------------------------//
  template<
      class S,
      bool STORAGE2 = STORAGE,
      bool OWNED2 = OWNED,
      bool SORTED2 = SORTED,
      class F2 = F,
      template<typename, typename...> class ID_STORAGE_TYPE2 = ID_STORAGE_TYPE,
      template<typename, typename...> class STORAGE_TYPE2 = STORAGE_TYPE>
  auto & cast() {
    static_assert(std::is_convertible<S, T>::value, "invalid index space cast");

    auto res = reinterpret_cast<index_space__<
        S, STORAGE2, OWNED2, SORTED2, F2, ID_STORAGE_TYPE2, STORAGE_TYPE2> *>(
        this);
    assert(res != nullptr && "invalid cast");
    return *res;
  }

  //-----------------------------------------------------------------//
  //! Cast an index space by re-specifying template parameters such
  //! as OWNED or SORTED.
  //-----------------------------------------------------------------//
  template<
      class S,
      bool STORAGE2 = STORAGE,
      bool OWNED2 = OWNED,
      bool SORTED2 = SORTED,
      class F2 = F,
      template<typename, typename...> class ID_STORAGE_TYPE2 = ID_STORAGE_TYPE,
      template<typename, typename...> class STORAGE_TYPE2 = STORAGE_TYPE>
  auto & cast() const {
    static_assert(std::is_convertible<S, T>::value, "invalid index space cast");

    auto res = reinterpret_cast<index_space__<
        S, STORAGE2, OWNED2, SORTED2, F2, ID_STORAGE_TYPE2, STORAGE_TYPE2> *>(
        this);
    assert(res != nullptr && "invalid cast");
    return *res;
  }

  //-----------------------------------------------------------------//
  //! Return begin iterator
  //-----------------------------------------------------------------//
  auto begin() {
    return iterator_<T, F>(s_, *v_, begin_, end_);
  }

  //-----------------------------------------------------------------//
  //! Return begin iterator
  //-----------------------------------------------------------------//
  auto begin() const {
    return iterator_<const T, F>(s_, *v_, begin_, end_);
  }

  //-----------------------------------------------------------------//
  //! Return end iterator
  //-----------------------------------------------------------------//
  auto end() {
    return iterator_<T, F>(s_, *v_, end_, end_);
  }

  //-----------------------------------------------------------------//
  //! Return end iterator
  //-----------------------------------------------------------------//
  auto end() const {
    return iterator_<const T, F>(s_, *v_, end_, end_);
  }

  //-----------------------------------------------------------------//
  //! Return begin offset
  //-----------------------------------------------------------------//
  size_t begin_offset() const {
    return begin_;
  }

  //-----------------------------------------------------------------//
  //! Return end offset
  //-----------------------------------------------------------------//
  size_t end_offset() const {
    return end_;
  }

  //-----------------------------------------------------------------//
  //! Get offset
  //-----------------------------------------------------------------//
  ref_t get_offset(size_t offset) {
    return (*s_)[(*v_)[offset].index_space_index()];
  }

  //-----------------------------------------------------------------//
  //! Get offset
  //-----------------------------------------------------------------//
  const ref_t get_offset(size_t offset) const {
    return (*s_)[(*v_)[offset].index_space_index()];
  }

  //-----------------------------------------------------------------//
  //! Get all IDs in range
  //-----------------------------------------------------------------//
  id_range_ ids() const {
    return id_range_(*v_, begin_, end_);
  }

  //-----------------------------------------------------------------//
  //! Get all IDs in range
  //-----------------------------------------------------------------//
  id_range_ ids(size_t begin, size_t end) const {
    return id_range_(*v_, begin, end);
  }

  //-----------------------------------------------------------------//
  //! Get all IDs in range
  //-----------------------------------------------------------------//
  id_range_ ids(const std::pair<size_t, size_t> & p) const {
    return id_range_(*v_, p.first, p.second);
  }

  //-----------------------------------------------------------------//
  //! Slice and cast an index space. A slice aliases the current index
  //! space, definining a new iteration range of offsets to indices.
  //!
  //! @param begin begin offset
  //! @param end end offset
  //-----------------------------------------------------------------//
  template<class S = T>
  auto slice(size_t begin, size_t end) const {
    return index_space__<
        S, false, false, SORTED, F, ID_STORAGE_TYPE, STORAGE_TYPE>(
        *this, begin, end);
  }

  //-----------------------------------------------------------------//
  //! Slice and cast an index space. A slice aliases the current index
  //! space, definining a new iteration range of offsets to indices.
  //!
  //! @tparam S class to cast to
  //!
  //! @param r offset range
  //-----------------------------------------------------------------//
  template<class S = T>
  auto slice(const std::pair<size_t, size_t> & range) const {
    return index_space__<
        S, false, false, SORTED, F, ID_STORAGE_TYPE, STORAGE_TYPE>(
        *this, range.first, range.second);
  }

  //-----------------------------------------------------------------//
  //! Cast an index space to use a new entity type. The entity type
  //! must be binary compatible with the cast type, i.e. an entity
  //! wrapper or derived pointer type.
  //!
  //! @tparam S class to cast to
  //-----------------------------------------------------------------//
  template<class S = T>
  auto slice() const {
    return index_space__<
        S, false, false, SORTED, F, ID_STORAGE_TYPE, STORAGE_TYPE>(
        *this, begin_, end_);
  }

  //-----------------------------------------------------------------//
  //! Helper method. Get item at offset.
  //-----------------------------------------------------------------//
  ref_t get_(size_t offset) {
    return static_cast<cast_t>(
        (*s_)[(*v_)[begin_ + offset].index_space_index()]);
  }

  //-----------------------------------------------------------------//
  //! Helper method. Get item at offset.
  //-----------------------------------------------------------------//
  const ref_t get_(size_t offset) const {
    return static_cast<cast_t>(
        (*s_)[(*v_)[begin_ + offset].index_space_index()]);
  }

  //-----------------------------------------------------------------//
  //! Helper method. Get item at offset from end.
  //-----------------------------------------------------------------//
  ref_t get_end_(size_t offset) {
    return static_cast<cast_t>(
        (*s_)[(*v_)[end_ - 1 - offset].index_space_index()]);
  }

  //-----------------------------------------------------------------//
  //! Helper method. Get item at offset from end.
  //-----------------------------------------------------------------//
  const ref_t get_end_(size_t offset) const {
    return static_cast<cast_t>(
        (*s_)[(*v_)[end_ - 1 - offset].index_space_index()]);
  }

  //-----------------------------------------------------------------//
  //! Get item at offset.
  //-----------------------------------------------------------------//
  ref_t operator[](size_t offset) {
    return get_(offset);
  }

  //-----------------------------------------------------------------//
  //! Get item at offset.
  //-----------------------------------------------------------------//
  const ref_t operator[](size_t i) const {
    return get_(i);
  }

  //-----------------------------------------------------------------//
  //! Index into the index space and return reference to the entity id
  //! at index
  //-----------------------------------------------------------------//
  const id_t & operator()(size_t i) const {
    return (*v_)[begin_ + i];
  }

  //-----------------------------------------------------------------//
  //! Index into the index space and return reference to the entity id
  //! at index
  //-----------------------------------------------------------------//
  id_t & operator()(size_t i) {
    return (*v_)[begin_ + i];
  }

  //-----------------------------------------------------------------//
  //! Get the entity at the front of the index space
  //-----------------------------------------------------------------//
  ref_t front() {
    return get_(0);
  }

  //-----------------------------------------------------------------//
  //! Get the entity at the front of the index space
  //-----------------------------------------------------------------//
  const ref_t front() const {
    return get_(0);
  }

  //-----------------------------------------------------------------//
  //! Get the entity at the back of the index space
  //-----------------------------------------------------------------//
  ref_t back() {
    return get_end_(0);
  }

  //-----------------------------------------------------------------//
  //! Get the entity at the back of the index space
  //-----------------------------------------------------------------//
  const ref_t back() const {
    return get_end_(0);
  }

  //-----------------------------------------------------------------//
  //! Get the size of the index space
  //-----------------------------------------------------------------//
  size_t size() const {
    return end_ - begin_;
  }

  //-----------------------------------------------------------------//
  //! Return if the index space is empty, i.e. has no indices
  //-----------------------------------------------------------------//
  bool empty() const {
    return begin_ == end_;
  }

  //-----------------------------------------------------------------//
  //! Clear all indices and entities
  //-----------------------------------------------------------------//
  void clear() {
    begin_ = 0;
    end_ = 0;

    if (OWNED || owned_) {
      v_->clear();
      sorted_ = SORTED;
    }

    if (STORAGE) {
      s_->clear();
    }
  }

  //-----------------------------------------------------------------//
  //! Set the master entity storage. The master owns the actual
  //! entities that the index space references.
  //-----------------------------------------------------------------//
  template<
      bool STORAGE2,
      bool OWNED2,
      bool SORTED2,
      class F2,
      template<typename, typename...> class INDEX_STORAGE_TYPE2,
      template<typename, typename...> class STORAGE_TYPE2>
  void set_master(const index_space__<
                  T,
                  STORAGE2,
                  OWNED2,
                  SORTED2,
                  F2,
                  INDEX_STORAGE_TYPE2,
                  STORAGE_TYPE2> & master) {
    set_master(const_cast<index_space__<
                   T, STORAGE2, OWNED2, SORTED2, F2, INDEX_STORAGE_TYPE2,
                   STORAGE_TYPE2> &>(master));
  }

  //-----------------------------------------------------------------//
  //! Set the master entity storage. The master owns the actual
  //! entities that the index space references.
  //-----------------------------------------------------------------//
  template<
      bool STORAGE2,
      bool OWNED2,
      bool SORTED2,
      class F2,
      template<typename, typename...> class INDEX_STORAGE_TYPE2,
      template<typename, typename...> class STORAGE_TYPE2>
  void set_master(index_space__<
                  T,
                  STORAGE2,
                  OWNED2,
                  SORTED2,
                  F2,
                  INDEX_STORAGE_TYPE2,
                  STORAGE_TYPE2> & master) {
    s_ = reinterpret_cast<storage_t *>(master.s_);
  }

  //-----------------------------------------------------------------//
  //! Construct and return a vector containing the entities
  //! associated with this index space
  //-----------------------------------------------------------------//
  std::vector<const T> to_vec() const {
    return to_vec_<const T>();
  }

  //-----------------------------------------------------------------//
  //! Construct and return a vector containing the entities associated
  //! with this index space
  //-----------------------------------------------------------------//
  std::vector<T> to_vec() {
    return to_vec_<T>();
  }

  //-----------------------------------------------------------------//
  //! Helper method. Construct and return a vector containing the
  //! entities associated with this index space.
  //!
  //! @tparam S entity type
  //-----------------------------------------------------------------//
  template<class S>
  std::vector<S> to_vec_() {
    std::vector<S> ret;
    size_t n = end_ - begin_;
    ret.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      ret.push_back((*this)[i]);
    } // for
    return ret;
  }

  //-----------------------------------------------------------------//
  //! Return the index storage object.
  //-----------------------------------------------------------------//
  const id_storage_t & id_storage() const {
    return *v_;
  }

  //-----------------------------------------------------------------//
  //! Return the index storage object.
  //-----------------------------------------------------------------//
  id_storage_t & id_storage() {
    return *v_;
  }

  //-----------------------------------------------------------------//
  //! Set the index storage object.
  //-----------------------------------------------------------------//
  void set_id_storage(id_storage_t * v) {
    if (owned_) {
      delete v_;
    }

    v_ = v;
    owned_ = false;
  }

  //-----------------------------------------------------------------//
  //! Return the index space IDs as an array.
  //-----------------------------------------------------------------//
  const id_t * id_array() const {
    return v_->data();
  }

  id_t * id_array() {
    return v_->data();
  }

  //-----------------------------------------------------------------//
  //! Walk the index space and apply the predicate f, returning a new
  //! index space of those entities for which the predicate returned true.
  //!
  //! @tparam Predicate predicate callable object type
  //-----------------------------------------------------------------//
  template<typename Predicate>
  auto filter(Predicate && f) const {
    index_space__<T, false, true, false> is;
    is.set_master(*this);

    for (auto item : *this) {
      if (std::forward<Predicate>(f)(item)) {
        is.push_back(item);
      }
    }

    return is;
  }

  //-----------------------------------------------------------------//
  //! Apply a function f to each indexed entity in the index space,
  //! mutating its state.
  //-----------------------------------------------------------------//
  void apply(apply_function f) const {
    for (auto ent : *this) {
      f(ent);
    }
  }

  //-----------------------------------------------------------------//
  //! Non mutating method to apply a function f over each entity an
  //! index face, returning a new index space in the process
  //-----------------------------------------------------------------//
  template<class S>
  auto map(map_function<S> f) const {
    index_space__<S, false, true, false> is;
    is.set_master(*this);

    is.begin_push_(v_->size());

    for (auto item : *this) {
      is.batch_push_(f(*item));
    }
    return is;
  }

  //-----------------------------------------------------------------//
  //! Apply a reduction function to each entity in the index space
  //! and return the reduced result.
  //!
  //! @param start start value, e.g. 0 for sum, 1 for product
  //-----------------------------------------------------------------//
  template<typename S>
  S reduce(T start, reduce_function<T> f) const {
    T r = start;

    for (auto item : *this) {
      f(*item, r);
    }

    return r;
  }

  //-----------------------------------------------------------------//
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
  //-----------------------------------------------------------------//
  template<typename Predicate>
  auto bin(Predicate && f) const {
    return bin_as_map(std::forward<Predicate>(f));
  }

  //-----------------------------------------------------------------//
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
  //-----------------------------------------------------------------//
  template<typename Predicate>
  auto bin_as_map(Predicate && f) const {

    // If the list was sorted beforehand, the result will also be
    // sorted.  Own the index_vector_t (i.e., create storage for it)
    using new_index_space_t = index_space__<T, false, true, SORTED>;

    // get the predicate result type
    using result_t =
        std::decay_t<decltype(std::forward<Predicate>(f)(operator[](0)))>;

    // bin the data, and use a map to sort the keys
    std::map<result_t, new_index_space_t> bins;
    for (auto ent : *this)
      bins[std::forward<Predicate>(f)(ent)].push_back(ent);

    // now go and set the master
    for (auto & entry : bins)
      entry.second.set_master(*this);

    return bins;
  }

  //-----------------------------------------------------------------//
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
  //-----------------------------------------------------------------//
  template<typename Predicate>
  auto bin_as_vector(Predicate && f) const {

    // get the bins as a map
    auto bins_map = bin_as_map(std::forward<Predicate>(f));

    // If the list was sorted beforehand, the result will also be
    // sorted.  Own the index_vector_t (i.e., create storage for it)
    using new_index_space_t =
        typename std::decay_t<decltype(bins_map)>::mapped_type;

    // now extract the results and store them in a vector.
    // should be faster for access.
    std::vector<new_index_space_t> bins_vec;
    bins_vec.reserve(bins_map.size());

    for (auto && entry : bins_map)
      bins_vec.emplace_back(std::move(entry.second));

    return bins_vec;
  }

  //-----------------------------------------------------------------//
  //! Helper method, for write operations.
  //! If the containers are not owned then make a copy of them
  //! internally and set ownership.
  //-----------------------------------------------------------------//
  void prepare_() {
    if (!OWNED && !owned_) {
      v_ = new id_storage_t(*v_);
      owned_ = true;
    }

    if (!SORTED && !sorted_) {
      auto vc = const_cast<id_storage_t *>(v_);
      std::sort(vc->begin(), vc->end());
      sorted_ = true;
    }
  }

  //-----------------------------------------------------------------//
  //! In-place set intersection operation. This method will sort
  //! the ID storage if not already sorted and will create a copy
  //! of its aliased indices if OWNED is false.
  //-----------------------------------------------------------------//
  index_space__ & operator&=(const index_space__ & r) {
    prepare_();

    id_storage_t ret;

    if (r.sorted_) {
      ret.resize(std::min(v_->size(), r.v_->size()));

      auto itr = std::set_intersection(
          v_->begin(), v_->end(), r.v_->begin(), r.v_->end(), ret.begin());

      ret.resize(itr - ret.begin());
    } else {
      id_storage_t v2(*r.v_);
      std::sort(v2.begin(), v2.end());

      ret.resize(std::min(v_->size(), v2.size()));

      auto itr = std::set_intersection(
          v_->begin(), v_->end(), v2.begin(), v2.end(), ret.begin());

      ret.resize(itr - ret.begin());
    }

    *v_ = std::move(ret);

    begin_ = 0;
    end_ = v_->size();

    return *this;
  }

  //-----------------------------------------------------------------//
  //! Return r-value of set intersection of passed index spaces
  //-----------------------------------------------------------------//
  index_space__ operator&(const index_space__ & r) const {
    index_space__ ret(*this);
    ret &= r;
    return ret;
  }

  //-----------------------------------------------------------------//
  //! In-place set union operation. This method will sort
  //! the ID storage if not already sorted and will create a copy
  //! of its aliased indices if OWNED is false.
  //-----------------------------------------------------------------//
  index_space__ & operator|=(const index_space__ & r) {
    prepare_();

    id_storage_t ret;

    if (r.sorted_) {
      ret.resize(v_->size() + r.v_->size());

      auto itr = std::set_union(
          v_->begin(), v_->end(), r.v_->begin(), r.v_->end(), ret.begin());

      ret.resize(itr - ret.begin());
    } else {
      id_storage_t v2(*r.v_);

      std::sort(v2.begin(), v2.end());

      ret.resize(v_->size() + v2.size());

      auto itr = std::set_union(
          v_->begin(), v_->end(), v2.begin(), v2.end(), ret.begin());

      ret.resize(itr - ret.begin());
    }

    *v_ = std::move(ret);

    begin_ = 0;
    end_ = v_->size();

    return *this;
  }

  //-----------------------------------------------------------------//
  //! Return r-value of set union of passed index spaces.
  //-----------------------------------------------------------------//
  index_space__ operator|(const index_space__ & r) const {
    index_space__ ret(*this);
    ret |= r;
    return ret;
  }

  //-----------------------------------------------------------------//
  //! In-place set complement operator. This method will sort
  //! the ID storage if not already sorted and will create a copy
  //! of its aliased indices if OWNED is false.
  //-----------------------------------------------------------------//
  index_space__ & operator-=(const index_space__ & r) {
    prepare_();

    id_storage_t ret(v_->size());

    if (r.sorted_) {
      auto itr = std::set_difference(
          v_->begin(), v_->end(), r.v_->begin(), r.v_->end(), ret.begin());

      ret.resize(itr - ret.begin());
    } else {
      id_storage_t v2(*r.v_);

      std::sort(v2.begin(), v2.end());

      auto itr = std::set_difference(
          v_->begin(), v_->end(), v2.begin(), v2.end(), ret.begin());

      ret.resize(itr - ret.begin());
    }

    *v_ = std::move(ret);

    begin_ = 0;
    end_ = v_->size();

    return *this;
  }

  //-----------------------------------------------------------------//
  //! Return r-value of set difference of passed index spaces.
  //-----------------------------------------------------------------//
  index_space__ operator-(const index_space__ & r) const {
    index_space__ ret(*this);
    ret -= r;
    return ret;
  }

  //-----------------------------------------------------------------//
  //! Add an entity to the index space. If the index space does not
  //! have storage then only the index is pushed. It will create a copy
  //! of its aliased indices if OWNED is false.
  //-----------------------------------------------------------------//
  void push_back(const T & item) {
    if (!OWNED && !owned_) {
      v_ = new id_storage_t(*v_);
      owned_ = true;
    }

    if (STORAGE) {
      s_->push_back(item);
    }

    if (SORTED || sorted_) {
      auto id = id_(item);
      auto itr = std::upper_bound(v_->begin(), v_->end(), id);
      v_->insert(itr, id);
    } else {
      v_->push_back(id_(item));
    }

    ++end_;
  }

  //-----------------------------------------------------------------//
  //! Add an entity to the index space. If the index space does not
  //! have storage then only the index is pushed. It will create a copy
  //! of its aliased indices if OWNED is false.
  //-----------------------------------------------------------------//
  void push_back(id_t index) {
    if (!OWNED && !owned_) {
      v_ = new id_storage_t(*v_);
      owned_ = true;
    }

    if (SORTED || sorted_) {
      auto itr = std::upper_bound(v_->begin(), v_->end(), index);
      v_->insert(itr, index);
    } else {
      v_->push_back(index);
    }

    ++end_;
  }

  //-----------------------------------------------------------------//
  //! Called if an index was added to storage internally.
  //-----------------------------------------------------------------//
  void pushed() {
    ++end_;
  }

  //-----------------------------------------------------------------//
  //! Append another index space’s entities to the index space.
  //! If the index space does not have storage then only the indices
  //! are appended. It will create a copy of its aliased indices if
  //! OWNED is false.
  //-----------------------------------------------------------------//
  void append(const index_space__ & is) {
    if (!OWNED && !owned_) {
      v_ = new id_storage_t(*v_);
      owned_ = true;
    }

    if (STORAGE) {
      s_->insert(s_->begin(), is.s_->begin(), is.s_->end());
    }

    size_t n = is.size();

    if (SORTED || sorted_) {
      v_->reserve(n);

      for (auto ent : is) {
        id_t id = id_(ent);
        auto itr = std::upper_bound(v_->begin(), v_->end(), id);
        v_->insert(itr, id);
      }
    } else {
      v_->insert(v_->begin(), is.v_->begin(), is.v_->end());
    }

    end_ += n;
  }

  //-----------------------------------------------------------------//
  //! Shortcut for push_back()
  //-----------------------------------------------------------------//
  index_space__ & operator<<(T item) {
    push_back(item);
    return *this;
  }

  //-----------------------------------------------------------------//
  //! Append helper method. Do not call directly.
  //-----------------------------------------------------------------//
  void append_(const std::vector<T> & ents, const std::vector<id_t> & ids) {
    static_assert(STORAGE, "expected STORAGE");
    static_assert(OWNED, "expected OWNED");

    assert(ents.size() == ids.size());

    s_->insert(s_->begin(), ents.begin(), ents.end());

    size_t n = ents.size();

    if (SORTED || sorted_) {
      v_->reserve(n);

      for (id_t id : ids) {
        auto itr = std::upper_bound(v_->begin(), v_->end(), id);
        v_->insert(itr, id);
      }
    } else {
      v_->insert(v_->begin(), ids.begin(), ids.end());
    }

    end_ += n;
  }

  //-----------------------------------------------------------------//
  //! Helper method to get ID.
  //-----------------------------------------------------------------//
  id_t id_(const item_t & item) {
    return item.index_space_id();
  }

  //-----------------------------------------------------------------//
  //! Helper method to get ID.
  //-----------------------------------------------------------------//
  id_t id_(const item_t * item) {
    return item->index_space_id();
  }

  //-----------------------------------------------------------------//
  //! Set to begin index of contained IDs.
  //-----------------------------------------------------------------//
  void set_begin(size_t begin) {
    begin_ = begin;
  }

  //-----------------------------------------------------------------//
  //! Set the end index of contained IDs.
  //-----------------------------------------------------------------//
  void set_end(size_t end) {
    end_ = end;
  }

private:
  template<
      class,
      bool,
      bool,
      bool,
      class,
      template<class, class...> class,
      template<class, class...> class>
  friend class index_space__;

  friend class connectivity_t;

  id_storage_t * v_;
  size_t begin_;
  size_t end_;
  bool owned_;
  bool sorted_;
  storage_t * s_;

  //-----------------------------------------------------------------//
  //! Private methods for efficiently populating an index space.
  //-----------------------------------------------------------------//
  size_t begin_push_() {
    static_assert(OWNED, "expected OWNED");
    return v_->size();
  }

  //-----------------------------------------------------------------//
  //! Private methods for efficiently populating an index space.
  //-----------------------------------------------------------------//
  void reserve_(size_t n) {
    static_assert(OWNED, "expected OWNED");
    return v_->reserve(n);
  }

  //-----------------------------------------------------------------//
  //! Private methods for efficiently populating an index space.
  //-----------------------------------------------------------------//
  void begin_push_(size_t n) {
    static_assert(OWNED, "expected OWNED");
    assert(begin_ == 0);
    size_t m = v_->size();
    v_->reserve(m + n);
    end_ += n;
  }

  //-----------------------------------------------------------------//
  //! Private methods for efficiently populating an index space.
  //-----------------------------------------------------------------//
  void batch_push_(id_t index) {
    static_assert(OWNED, "expected OWNED");
    v_->push_back(index);
  }

  //-----------------------------------------------------------------//
  //! Private methods for efficiently populating an index space.
  //-----------------------------------------------------------------//
  void push_(id_t index) {
    static_assert(OWNED, "expected OWNED");
    v_->push_back(index);
    ++end_;
  }

  //-----------------------------------------------------------------//
  //! Private methods for efficiently populating an index space.
  //-----------------------------------------------------------------//
  void end_push_(size_t n) {
    static_assert(OWNED, "expected OWNED");
    assert(begin_ == 0);
    end_ += v_->size() - n;
  }

  //-----------------------------------------------------------------//
  //! Private methods for efficiently populating an index space.
  //-----------------------------------------------------------------//
  void resize_(size_t n) {
    static_assert(OWNED, "expected OWNED");
    assert(begin_ == 0);
    v_->resize(n);
    end_ = v_->size();
  }

  //-----------------------------------------------------------------//
  //! Private methods for efficiently populating an index space.
  //-----------------------------------------------------------------//
  void fill_(id_t index) {
    static_assert(OWNED, "expected OWNED");
    std::fill(v_->begin(), v_->end(), index);
  }

  //-----------------------------------------------------------------//
  //! Private methods for efficiently populating an index space.
  //-----------------------------------------------------------------//
  id_storage_t & id_storage_() {
    return *v_;
  }

  //-----------------------------------------------------------------//
  //! Private methods for efficiently populating an index space.
  //-----------------------------------------------------------------//
  typename id_storage_t::iterator index_begin_() {
    return v_->begin();
  }

  //-----------------------------------------------------------------//
  //! Private methods for efficiently populating an index space.
  //-----------------------------------------------------------------//
  typename id_storage_t::iterator index_end_() {
    return v_->end();
  }
};

//----------------------------------------------------------------------------//
//! this class provides a simple ID which is simply a size_t and the index is
//! the ID as opposed to more complex IDs which might need to store different
//! pieces of data such as partition, topological dimension, etc.
//----------------------------------------------------------------------------//
class simple_id {
public:
  //-----------------------------------------------------------------//
  //! Constructor
  //-----------------------------------------------------------------//
  simple_id(size_t id) : id_(id) {}

  //-----------------------------------------------------------------//
  //! Conversion the size_t
  //-----------------------------------------------------------------//
  operator size_t() const {
    return id_;
  }

  //-----------------------------------------------------------------//
  //! Comparison operator
  //-----------------------------------------------------------------//
  bool operator<(const simple_id & eid) const {
    return id_ < eid.id_;
  }

  //-----------------------------------------------------------------//
  //! Define the index space ID
  //-----------------------------------------------------------------//
  size_t index_space_index() const {
    return id_;
  }

private:
  size_t id_;
};

//----------------------------------------------------------------------------//
//! a convenience class which associates a simple ID with type T
//----------------------------------------------------------------------------//
template<typename T>
class simple_entry__ {
public:
  using id_t = simple_id;

  //-----------------------------------------------------------------//
  //! Constructor to associate an id with an entry
  //-----------------------------------------------------------------//
  simple_entry__(id_t id, const T & entry) : id_(id), entry_(entry) {}

  //-----------------------------------------------------------------//
  //! Conversion operator
  //-----------------------------------------------------------------//
  operator T() const {
    return entry_;
  }

  //-----------------------------------------------------------------//
  //! Get the entry ID
  //-----------------------------------------------------------------//
  auto entry_id() const {
    return entry_;
  }

  //-----------------------------------------------------------------//
  //! Get the index space ID
  //-----------------------------------------------------------------//
  id_t index_space_id() const {
    return id_;
  }

private:
  id_t id_;
  T entry_;
};

} // namespace topology
} // namespace flecsi
