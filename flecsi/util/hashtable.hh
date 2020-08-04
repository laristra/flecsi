
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

#if !defined(__FLECSI_PRIVATE__)
#error Do not include this file directly!
#endif

#include <utility>
#include <flecsi/util/array_ref.hh>


namespace flecsi {
namespace util {

template<class KEY, class TYPE, class HASH = std::hash<KEY>>
class hashtable;

template<class KEY, class TYPE, class HASH>
class hashtableIterator
{

private:

    typename hashtable<KEY,TYPE,HASH>::pair_t* ptr_;
    friend class hashtable<KEY,TYPE,HASH>;
    explicit hashtableIterator(typename hashtable<KEY,TYPE,HASH>::pair_t* p) : ptr_(p) {}

public:

    typename hashtable<KEY,TYPE,HASH>::pair_t& operator*() const {
        return *ptr_;
    }

    hashtableIterator& operator++() {
        while((ptr_+1)->first == key_t{}){
          ++ptr_; 
        }
        return *this; 
    }

    bool operator==(const hashtableIterator& iter) const {
        return this->ptr_ == iter.ptr_;
    }

    bool operator!=(const hashtableIterator& iter) const {
        return this->ptr_ != iter.ptr_;
    }
};


//----------------------------------------------------------------------------//
//! Hash table class for the tree topology. This is a generic representation
//! based on an index space implementing find insert and find functions
//----------------------------------------------------------------------------//
template<class KEY, class TYPE, class HASH>
struct hashtable {

public: 
  using key_t = KEY;
  using type_t = TYPE;
  using pair_t = std::pair<key_t,type_t>;
  using hash_f = HASH; 

  using pointer = pair_t*; 
  using const_pointer = const pair_t*; 
  using reference = pair_t&; 
  using const_reference = const pair_t&; 
  
  //using iterator = pointer; 
  //using const_iterator = const_pointer; 

  using iterator = hashtableIterator<KEY,TYPE,HASH>;
  using const_iterator = const iterator; 

private: 

  pointer p, q; 

  std::size_t nelements_; 
  const std::size_t modulo_ = 334214459;
  util::span<pair_t> acc_; 
  std::size_t hash_capacity_; 

  // Max number of search before crash 
  const std::size_t max_find_ = 10; 

public:

  hashtable() = default; 

  hashtable(const util::span<pair_t>& acc){
    acc_ = acc; 
    hash_capacity_ = acc_.size();
    init_(); 
  }

  void set_acc(const util::span<pair_t>& acc){
    acc_ = acc; 
    hash_capacity_ = acc_.size(); 
    init_(); 
  }

  /**
   * @brief Find a value in the hashtable
   * While the value or a null key is not found we keep looping
   */
  iterator find(const key_t & key) {
    std::size_t h = hash_f()(key) % hash_capacity_;
    pointer ptr = acc_.data() + h;
    std::size_t iter = 0; 
    std::cout<<"Search for: "<<key<<std::endl;
    std::cout<<"On case: "<<ptr->first<<" ptr: "<<ptr<<std::endl;
    while(ptr->first != key && ptr->first != key_t{} && iter != max_find_) {
      std::cout<<"Nothing"<<std::endl;
      h = (h + modulo_) % hash_capacity_;
      ptr = acc_.data() + h;
      ++iter; 
    }
    std::cout<<"Found: "<<ptr->first<<" - "<<ptr->second<<std::endl;
    if(ptr->first != key || iter == max_find_) {
      return iterator(nullptr);
    }
    return iterator(ptr);
  }

  /**
   * @brief Insert an object in the hash map at a defined position
   * This function tries to find the first available position in case of
   * conflict using modulo method.
   */
  template<typename... ARGS>
  iterator insert(const key_t & key, ARGS &&... args) {
    std::size_t h = hash_f()(key) % hash_capacity_; 
    pointer ptr = acc_.data() + h;
    std::size_t iter = 0; 
    while(ptr->first != key && ptr->first != key_t{} && iter != max_find_) {
      h = (h + modulo_) % hash_capacity_;
      ptr = acc_.data() + h;
      ++iter; 
    }
    // Update first and last pointer
    if(p != nullptr){
      p = p > ptr ? ptr : p;  
    }else{
      p = ptr; 
    }
    if(q != nullptr){
      q = q < ptr ? ptr : q; 
    }else{
      q = ptr; 
    }
    if(iter == max_find_){
      flog(error)<<"Max iteration reached, couldn't insert element: "<<key<<std::endl;
      return iterator(nullptr); 
    }
    ++nelements_; 
    ptr = new(ptr) pair_t(key,{std::forward<ARGS>(args)...}); 

    //ptr->first = key; 
    //ptr->second = type_t(std::forward<ARGS>(args)...);
    return iterator(ptr);
  }

  // Clear all keys
  void clear() {
    for(auto a : *this) {
      a.first = key_t{};
    }
  }

  constexpr iterator begin() const noexcept{ 
    return iterator(p); 
  }

  constexpr iterator end() const noexcept {
    if(q == nullptr)
      return iterator(nullptr);
    return iterator(q+1); 
  }

  constexpr const_iterator cbegin() const noexcept {
    return begin();
  }
  
  constexpr const_iterator cend() const noexcept {
    return end();
  }

  constexpr reference front() const {
    return *begin();
  }
  constexpr reference back() const {
    return end()[-1];
  }

  constexpr reference operator[](std::size_t i) const {
    return begin()[i];
  }

  constexpr pointer data() const noexcept {
    return begin();
  }

  constexpr std::size_t size() const noexcept {
    return nelements_;
  }

  constexpr bool empty() const noexcept {
    return begin() == end();
  }

private: 
  void init_(){
    p = nullptr;  
    for(auto a: acc_){
      if(a.first != key_t{}){
        p = &a;
        break;  
      }
    }
    q = nullptr; 
    for(auto a = acc_.rbegin(); a != acc_.rend(); ++a){
      if(a->first != key_t{}){
        q = &(*a); 
        break;
      }
    }
    nelements_ = 0; 
    for(auto a: *this){
      ++nelements_; 
    }
  }


}; // class hashtable

} // namespace util
} // namespace flecsi
