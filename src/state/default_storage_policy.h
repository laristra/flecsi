/*~--------------------------------------------------------------------------~*
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
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_default_storage_policy_h
#define flexi_default_storage_policy_h

#include <unordered_map>
#include <vector>
#include <memory>
#include <typeinfo>

#include "../utils/index_space.h"
#include "../utils/const_string.h"

/*!
 * \file default_storage_policy.h
 * \authors bergen
 * \date Initial file creation: Oct 27, 2015
 */

namespace flexi {

/*----------------------------------------------------------------------------*
 * class default_state_storage_policy_t
 *----------------------------------------------------------------------------*/

/*!
  \class default_state_storage_policy_t state.h
  \brief default_state_storage_policy_t provides...
 */

template<typename user_meta_data_t>
class default_state_storage_policy_t
{
protected:

  //! Constructor
  default_state_storage_policy_t() {}

  //! Desctructor
  virtual ~default_state_storage_policy_t() {}

  /*!
   */
  struct meta_data_t {
    std::string label;
    user_meta_data_t user_data;
    size_t size;
    size_t type_size;

    struct type_info_t {
      type_info_t(const std::type_info & type_info_)
        : type_info(type_info_) {}
      const std::type_info & type_info;
    }; // struct type_info_t

    std::shared_ptr<type_info_t> rtti;

    std::vector<uint8_t> data;
  }; // struct meta_data_t

  /*--------------------------------------------------------------------------*
   * class accessor_t
   *--------------------------------------------------------------------------*/

  template<typename T>
  class accessor_t
  {
  public:

    using iterator_t = index_space_t::iterator_t;

    accessor_t(const std::string & label, const size_t size,
      T * data, const user_meta_data_t & meta)
      : label_(label), size_(size), data_(data), meta_(meta), is_(size_) {}

    const std::string & label() { return label_; }
    size_t size() const { return size_; }

    template<typename E>
    const T & operator [] (E * e) const {
      return this->operator [] (e->id());
    } // operator

    template<typename E>
    T & operator [] (E * e) {
      return this->operator [] (e->id());
    } // operator

    const T & operator [] (size_t index) const {
      assert(index < size_ && "index out of range");
      return data_[index];
    } // operator

    T & operator [] (size_t index) {
      assert(index < size_ && "index out of range");
      return data_[index];
    } // operator

    const user_meta_data_t & meta() const { return meta_; }

    iterator_t begin() { return { is_, 0 }; }
    iterator_t end() { return { is_, size_ }; }

  private:
    
    std::string label_;
    size_t size_;
    T * data_;
    const user_meta_data_t & meta_;
    index_space_t is_;

  }; // struct accessor_t

  /*!
    Return an accessor to the data for a given variable.

    \param key The name of the data to return.
   */
  template<typename T, size_t NS>
  accessor_t<T> accessor(const const_string_t & key) {
    return { meta_[NS][key.hash()].label, meta_[NS][key.hash()].size,
      reinterpret_cast<T *>(&meta_[NS][key.hash()].data[0]),
      meta_[NS][key.hash()].user_data };
  } // accessor

  /*!
    Return an accessor to the data for a given variable.

    \param hash A hash key for the data to return.
   */
  template<typename T, size_t NS>
  accessor_t<T> accessor(const_string_t::hash_type_t hash) {
    return { meta_[NS][hash].label, meta_[NS][hash].size,
      reinterpret_cast<T *>(&meta_[NS][hash].data[0]),
      meta_[NS][hash].user_data };
  } // accessor

  /*!
    Register a new state variable.

    \param key The key of the variable to add.
    \param indices The number of indices that parameterize the state.
   */
  template<typename T, size_t NS, typename ... Args>
  decltype(auto) register_state(const const_string_t & key, size_t indices,
    Args && ... args) {
    
    // add space as we need it
    if(meta_.size() < NS+1) {
      meta_.resize(NS+1);
    } // if

    // keys must be unique within a given namespace
    assert(meta_[NS].find(key.hash()) == meta_[NS].end() &&
      "key already exists");

    // user meta data
    meta_[NS][key.hash()].user_data.initialize(std::forward<Args>(args) ...);
    
    // store the type size and allocate storage
    meta_[NS][key.hash()].label = key.c_str();
    meta_[NS][key.hash()].size = indices;
    meta_[NS][key.hash()].type_size = sizeof(T);
    meta_[NS][key.hash()].rtti.reset(
      new typename meta_data_t::type_info_t(typeid(T)));
    meta_[NS][key.hash()].data.resize(indices*sizeof(T));

    return accessor<T,NS>(key.hash());
  } // register

  /*!
    Return an accessor to all data for a given type.
   */
  template<typename T, size_t NS>
  std::vector<accessor_t<T>> accessors() {
    std::vector<accessor_t<T>> v;

    for(auto entry_pair: meta_[NS]) {
      if(entry_pair.second.rtti->type_info == typeid(T)) {
        v.push_back(accessor<T,NS>(entry_pair.first));
      } // if
    } // for

    return v;
  } // accessors

  /*!
    Return an accessor to all data for a given type and predicate.
   */
  template<typename T, size_t NS, typename P>
  std::vector<accessor_t<T>> accessors(P && predicate) {
    std::vector<accessor_t<T>> v;

    for(auto entry_pair: meta_[NS]) {
      // create an accessor
      auto a = accessor<T,NS>(entry_pair.first);

      if(entry_pair.second.rtti->type_info == typeid(T) &&
        predicate(a)) {
        v.push_back(a);
      } // if
    } // for

    return v;
  } // accessors_predicate

  /*!
    Return accessors to all data.
   */
  template<size_t NS>
  std::vector<accessor_t<uint8_t>> accessors() {
    std::vector<accessor_t<uint8_t>> v;

    for(auto entry_pair: meta_[NS]) {
      v.push_back(accessor<uint8_t,NS>(entry_pair.first));
    } // for

    return v;
  } // accessors

  /*!
    Return an accessor to all data for a given type and predicate.
   */
  template<size_t NS, typename P>
  std::vector<accessor_t<uint8_t>> accessors(P && predicate) {
    std::vector<accessor_t<uint8_t>> v;

    for(auto entry_pair: meta_[NS]) {
      // create an accessor
      auto a = accessor<uint8_t,NS>(entry_pair.first);

      if(predicate(a)) {
        v.push_back(a);
      } // if
    } // for

    return v;
  } // accessors_predicate

  /*!
    Return user meta data for a given variable.

    \param key The name of the data to return.
   */
  template<size_t NS>
  user_meta_data_t & meta_data(const const_string_t & key) {
    return meta_[NS][key.hash()].user_data;
  } // meta_data

  /*!
    Return pointer to raw data.

    \param name The name of the data to return.
   */
  template<typename T, size_t NS>
  std::shared_ptr<T> & data(const const_string_t & key) {
    std::shared_ptr<T> sp { new T[meta_[NS][key.hash()].size] };
    memcpy(*sp, &meta_[NS][key.hash()].data[0],
      meta_[NS][key.hash()].data.size());
    return sp;
  } // data

private:

  std::vector<std::unordered_map<const_string_t::hash_type_t,
    meta_data_t>> meta_;
  
}; // class default_state_storage_policy_t

} // namespace flexi

#endif // flexi_default_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
