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

#ifndef flecsi_default_storage_policy_h
#define flecsi_default_storage_policy_h

#include <unordered_map>
#include <vector>
#include <memory>
#include <typeinfo>
#include <cassert>

#include "flecsi/utils/humble.h"
#include "flecsi/utils/index_space.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/data/data_constants.h"

/*!
 * \file default_storage_policy.h
 * \authors bergen
 * \date Initial file creation: Oct 27, 2015
 */

namespace flecsi {
namespace data {

namespace default_storage_policy {

  /*!
    \struct storage_type_t
   */
  template<size_t DT, typename storage_t> struct storage_type_t {};

  /*!
    FIXME: Scalar storage type.
   */
  template<typename storage_t>
  struct storage_type_t<global, storage_t> {
  }; // struct storage_type_t

  /*!
    FIXME: Dense storage type.
   */
  template<typename storage_t>
  struct storage_type_t<dense, storage_t> {
  }; // struct storage_type_t

  /*!
    FIXME: Sparse storage type.
   */
  template<typename storage_t>
  struct storage_type_t<sparse, storage_t> {
  }; // struct storage_type_t

  /*!
    FIXME: Scoped storage type.
   */
  template<typename storage_t>
  struct storage_type_t<scoped, storage_t> {
  }; // struct storage_type_t

  /*!
    FIXME: Bundle storage type.
   */
  template<typename storage_t>
  struct storage_type_t<tuple, storage_t> {
  }; // struct storage_type_t

} // namespace default_storage_policy

template<typename user_meta_data_t>
struct default_storage_policy_t {

  /*--------------------------------------------------------------------------*
   * struct meta_data_t
   *--------------------------------------------------------------------------*/

  /*!
    \brief meta_data_t provides storage for extra information that is
      used to interpret data variable information at different points
      in the low-level runtime.
   */
  struct meta_data_t {
    std::string label;
    user_meta_data_t user_data;
    size_t size;
    size_t type_size;

    /*!
      \brief type_info_t allows creation of reference information
        to the user-specified type of the data data.

      The std::type_info type requires dynamic initialization.  The
        type_info_t type is designed to allow construction without
        needing a non-trivial default constructor for the
        meta_data_t type.
     */
    struct type_info_t {
      type_info_t(const std::type_info & type_info_) : type_info(type_info_) {}
      const std::type_info & type_info;
    }; // struct type_info_t

    std::shared_ptr<type_info_t> rtti;

    std::unordered_map<size_t, std::vector<uint8_t>> data;
  }; // struct meta_data_t

  // Define the data storage container
  using storage_t = std::unordered_map<size_t,
    std::unordered_map<utils::const_string_t::hash_type_t, meta_data_t>>;

  // Define the storage type
  template<size_t data_type_t>
  using storage_type_t =
    default_storage_policy::storage_type_t<data_type_t, storage_t>;

  // Storage container instance
  storage_t storage_;
}; // struct default_storage_policy_t

/*----------------------------------------------------------------------------*
 * class default_data_storage_policy_t
 *----------------------------------------------------------------------------*/

/*!
  \class default_data_storage_policy_t default_storage_policy.h
  \brief default_data_storage_policy_t provides a serial/local storage
    policy for the \e flecsi data model.

  This storage policy is probably adequate for serial or MPI-based
  runtimes.  This implementation should not be used as a template
  for creating new storage policies.  If you are developing a new
  policy, look at the interface in \ref data.h.  This is the interface
  that needs to be implemented by new storage policies.
 */
template <typename user_meta_data_t>
class default_data_storage_policy_t
{
 protected:

  //! Constructor
  default_data_storage_policy_t() {}

  //! Desctructor
  virtual ~default_data_storage_policy_t() {}

  char* serialize_(uint64_t& size) const {
    const size_t alloc_size = 1048576;
    size = alloc_size;

    char* buf = (char*)std::malloc(alloc_size);
    uint64_t pos = 4;

    uint32_t num_entries = 0;

    for(auto& itr : meta_){
      uint64_t ns = itr.first;

      auto& mm = itr.second;

      for(auto& mitr : mm){
        std::memcpy(buf + pos, &ns, sizeof(ns));
        pos += sizeof(ns);

        uint64_t rs = mitr.first;
        std::memcpy(buf + pos, &rs, sizeof(rs));
        pos += sizeof(rs);

        const meta_data_t& md = mitr.second;
        auto& data = md.data;

        uint64_t data_size = md.size * md.type_size;
        std::memcpy(buf + pos, &data_size, sizeof(data_size));
        pos += sizeof(data_size);

        if(size - pos < data_size){
          size += data_size + alloc_size;
          buf = (char*)std::realloc(buf, size);
        }

        std::memcpy(buf + pos, data.data(), data_size);
        pos += data_size;

        ++num_entries;
      }
    }

    std::memcpy(buf, &num_entries, sizeof(num_entries));

    size = pos;

    return buf;
  }

  void unserialize_(char* buf){
    uint64_t pos = 0;

    uint32_t num_entries;
    std::memcpy(&num_entries, buf + pos, sizeof(num_entries));
    pos += sizeof(num_entries);

    for(size_t i = 0; i < num_entries; ++i){
      uint64_t ns;
      std::memcpy(&ns, buf + pos, sizeof(ns));
      pos += sizeof(ns);

      uint64_t rs;
      std::memcpy(&rs, buf + pos, sizeof(rs));
      pos += sizeof(rs);

      auto itr = meta_.find(ns);
      assert(itr != meta_.end() && "invalid namespace");

      auto& m = itr->second;

      auto mitr = m.find(rs);
      assert(mitr != m.end() && "invalid namespace");

      meta_data_t& md = mitr->second;
      auto& data = md.data;

      uint64_t data_size;
      std::memcpy(&data_size, buf + pos, sizeof(data_size));
      pos += sizeof(data_size);

      data.reserve(data_size);
      data.assign((unsigned char*)buf + pos,
                  (unsigned char*)buf + pos + data_size);

      pos += data_size;
    }
  }

  //! \brief delete ALL data
  void reset() {
    meta_.clear();
  } // reset

  /*!
   * \brief delete ALL data associated with this runtime namespace
   * \param [in] runtime_namespace the namespace to search
   */
  void reset( uintptr_t runtime_namespace ) {

    // check each namespace
    for ( auto & sub_map : meta_ ) {

      // the namespace data
      auto & namespace_key = sub_map.first;
      auto & meta_data = sub_map.second;

      // loop over each element in the namespace
      auto itr = meta_data.begin();
      while ( itr != meta_data.end() ) {
        // get the meta data key and label
        auto & meta_data_key = itr->first;
        auto & label = itr->second.label;
        // now build the hash for this label
        auto key_hash = 
          utils::hash<utils::const_string_t::hash_type_t>( label, label.size() );
        auto hash = key_hash ^ runtime_namespace;
        // test if it should be deleted
        if ( meta_data_key == hash )
          itr = meta_data.erase(itr);
        else
          ++itr;
      } // while
    } // for

  } // reset

  /*!
   * \brief delete specific data associated with this runtime namespace
   * \param [in] key the key to delete
   * \param [in] runtime_namespace the namespace to search
   */
  void release( 
    const utils::const_string_t & key,
    uintptr_t runtime_namespace 
  ) {

    auto hash = key.hash() ^ runtime_namespace;

    // check each namespace
    for ( auto & sub_map : meta_ ) {

      // the namespace data
      auto & namespace_key = sub_map.first;
      auto & meta_data = sub_map.second;

      // erase the data
      auto it = meta_data.erase( hash );

    } // for

  } // reset



  /*!
   * \brief move ALL data associated with this runtime namespace
   * \param [in] runtime_namespace the namespace to search
   */
  void move( uintptr_t from, uintptr_t to ) {

    // check each namespace
    for ( auto & sub_map : meta_ ) {

      // the namespace data
      auto & namespace_key = sub_map.first;
      auto & meta_data = sub_map.second;

      // create a temporary map
      using map_type = typename std::decay< decltype( meta_data ) >::type;
      map_type tmp_map;

      // loop over each element in the namespace and move
      // matching ones into the temp map
      auto itr = meta_data.begin();
      while ( itr != meta_data.end() ) {
        // get the meta data key and label
        auto & meta_data_key = itr->first;
        auto & label = itr->second.label;
        // now build the hash for this label
        auto key_hash = 
          utils::hash<utils::const_string_t::hash_type_t>( label, label.size() );
        auto from_hash = key_hash ^ from;
        // test if it should be moved, and move it
        if ( meta_data_key == from_hash ) {
          auto to_hash = key_hash ^ to;                // new hash
          tmp_map[to_hash] = std::move( itr->second ); // move data
          itr = meta_data.erase(itr);                  // delete old instance
        }
        // otherwise just go to next instance
        else {
          ++itr;
        }
      } // while


      // get move iterators
      using iterator = typename map_type::iterator;
      using move_iterator = std::move_iterator<iterator>;

      // move the data back into the meta data map with the new key
      meta_data.insert(
        move_iterator( tmp_map.begin() ),
        move_iterator( tmp_map.end() )
      );

    } // for

  } // reset

  /*--------------------------------------------------------------------------*
   * class global_accessor_t
   *--------------------------------------------------------------------------*/

  /*!
    \brief global_accessor_t provides type-based access to data
      variables that have been registered in the data model.

    \tparam T The type of the data variable.  If this type is not
      consistent with the type used to register the data, bad things
      can happen.  However, it can be useful to reinterpret the type, e.g.,
      when writing raw bytes.  This class is part of the low-level \e flecsi
      interface, so it is assumed that you know what you are doing...
   */
  template <typename T>
  class global_accessor_t
  {
   public:
    using iterator_t = utils::index_space_t::iterator_t;

    //! default constructor
    global_accessor_t() = default;

    /*!
      Constructor.

      \param label The c_str() version of the const_string used for
        this data variable's hash.
      \param data A pointer to the raw data.
      \param meta A reference to the user-defined meta data.
     */
    global_accessor_t(const std::string & label, const size_t size, T * data,
        const user_meta_data_t & meta)
        : label_(label), size_(1), data_(data), meta_(&meta), is_(1)
    {
    }

    /*!
      Copy Constructor.
     */
    global_accessor_t(const global_accessor_t & a)
        : label_(a.label_),
          size_(a.size_),
          data_(a.data_),
          meta_(a.meta_),
          is_(a.is_)
    {
    }

    /*!
      \brief Return a std::string containing the label of the data variable
        reference by this accessor.
     */
    const std::string & label() { return label_; }
    /*!
      \brief Return the size of the data variable referenced by this
        accessor.

      The size of the assocated index space.
     */
    size_t size() const { return size_; }
    /*!
      \brief Provide access to the data for this
        data variable.  This is the const operator version.
     */
    const T & operator*() const
    {
      return *data_;
    } // operator *

    /*!
      \brief Provide access to the data for this
        data variable.
     */
    T & operator*()
    {
      return *data_;
    } // operator *
    /*!
      \brief Provide access to the data for this
        data variable.  This is the const operator version.
     */
    const T * operator->() const
    {
      return data_;
    } // operator ->

    /*!
      \brief Provide access to the data for this
        data variable.
     */
    T * operator->()
    {
      return data_;
    } // operator ->
    /*!
      \brief Assignment Operator.

      \param a The value to assign to the data
     */
    global_accessor_t & operator=(const T & a)
    {
      *data_ = a;
      return *this;
    } // operator =

    /*!
      \brief Assignment Operator.

      \param a The accessor instance from which to assign values.
     */
    global_accessor_t & operator=(const global_accessor_t & a)
    {
      label_ = a.label_;
      data_ = a.data_;
      meta_ = a.meta_;
      return *this;
    } // operator =
    /*!
      \brief Implicit conversion operator.

      Using explicit keyword forces users to use static_cast<T>().  But
      if you dont use this, then it is ambiguous

        accessor<int> a, b;
        int c = 2;
        a = c; // ok, uses assignment
        b = 3; // ok, uses assignement
        c = a; // ok for non-explicit cases, uses conversion operator
        c = static_cast<int>(a); // works for both explicit and implicit cases

        // which one do you want? accessor/accessor assignement operator or
        // do you want to convert b to int, then assign int to a?
        a = b;

      Making this explicit forces you to have to static cast for all cases,
      which I think is less ambiguous

        a = static_cast<int>(b);
        a = static_cast<int>(c);

     */
    explicit operator T() const
    {
      return *data_;
    }

    /*!
      \brief Test to see if this accessor is empty

      \return true if registered.
     */
    operator bool() const
    {
      return (data_ != nullptr);
    }

    /*!
      \brief Return the user meta data for this data variable.

      \return The user meta data.
     */
    const user_meta_data_t & meta() const { return *meta_; }

   private:

    std::string label_ = "";
    size_t size_ = 0;
    T * data_ = nullptr;
    const user_meta_data_t * meta_ = nullptr;
    utils::index_space_t is_;

  }; // struct global_accessor_t


  /*--------------------------------------------------------------------------*
   * class dense_accessor_t
   *--------------------------------------------------------------------------*/

  /*!
    \brief dense_accessor_t provides logically array-based access to data
      variables that have been registered in the data model.

    \tparam T The type of the data variable.  If this type is not
      consistent with the type used to register the data, bad things
      can happen.  However, it can be useful to reinterpret the type, e.g.,
      when writing raw bytes.  This class is part of the low-level \e flecsi
      interface, so it is assumed that you know what you are doing...
   */
  template <typename T>
  class dense_accessor_t
  {
   public:
    using iterator_t = utils::index_space_t::iterator_t;

    //! default constructor
    dense_accessor_t() = default;

    /*!
      Constructor.

      \param label The c_str() version of the const_string used for
        this data variable's hash.
      \param size The size of the associated index space.
      \param data A pointer to the raw data.
      \param meta A reference to the user-defined meta data.
     */
    dense_accessor_t(const std::string & label, const size_t size, T * data,
        const user_meta_data_t & meta)
        : label_(label), size_(size), data_(data), meta_(&meta), is_(size_)
    {
    }

    /*!
      Copy Constructor.
     */
    dense_accessor_t(const dense_accessor_t & a)
        : label_(a.label_),
          size_(a.size_),
          data_(a.data_),
          meta_(a.meta_),
          is_(a.is_)
    {
    }

    /*!
      \brief Return a std::string containing the label of the data variable
        reference by this accessor.
     */
    const std::string & label() { return label_; }

    /*!
      \brief Return the size of the data variable referenced by this
        accessor.

      The size of the assocated index space.
     */
    size_t size() const { return size_; }

    /*!
      \brief Provide logical array-based access to the data for this
        data variable.  This is the const operator version.

      \tparam E A complex index type.

      This version of the operator is provided to support use with
      \e flecsi mesh entity types \ref mesh_entity_base_t.
     */
    template <typename E>
    const T & operator[](E * e) const
    {
      return this->operator[](e->id());
    } // operator

    /*!
      \brief Provide logical array-based access to the data for this
        data variable.

      \tparam E A complex index type.

      This version of the operator is provided to support use with
      \e flecsi mesh entity types \ref mesh_entity_base_t.
     */
    template <typename E>
    T & operator[](E * e)
    {
      return this->operator[](e->template id<0>());
    } // operator []

    /*!
      \brief Provide logical array-based access to the data for this
        data variable.  This is the const operator version.

      \param index The index of the data variable to return.
     */
    const T & operator[](size_t index) const
    {
      assert(index < size_ && "index out of range");
      return data_[index];
    } // operator []

    /*!
      \brief Provide logical array-based access to the data for this
        data variable.

      \param index The index of the data variable to return.
     */
    T & operator[](size_t index)
    {
      assert(index < size_ && "index out of range");
      return data_[index];
    } // operator []

    /*!
      \brief Assignment Operator.

      \param a The accessor instance from which to assign values.
     */
    dense_accessor_t & operator=(const dense_accessor_t & a)
    {
      label_ = a.label_;
      size_ = a.size_;
      data_ = a.data_;
      meta_ = a.meta_;
      is_ = a.is_;
    } // operator =

    /*!
      \brief Return the user meta data for this data variable.

      \return The user meta data.
     */
    const user_meta_data_t & meta() const { return *meta_; }

    /*!
      \brief Return an iterator to the beginning of this data data.
     */
    iterator_t begin() { return {is_, 0}; }

    /*!
      \brief Return an iterator to the end of this data data.
     */
    iterator_t end() { return {is_, size_}; }

    /*!
      \brief Test to see if this accessor is empty

      \return true if registered.
     */
    operator bool() const
    {
      return (data_ != nullptr);
    }

   private:

    std::string label_ = "";
    size_t size_ = 0;
    T * data_ = nullptr;
    const user_meta_data_t * meta_ = nullptr;
    utils::index_space_t is_;

  }; // struct dense_accessor_t

  /*--------------------------------------------------------------------------*
   * class sparse_accessor_t
   *--------------------------------------------------------------------------*/

  /*!
    \brief sparse_accessor_t provides logically array-based access to data
      variables that have been registered in the data model.

    \tparam T The type of the data variable.  If this type is not
      consistent with the type used to register the data, bad things
      can happen.  However, it can be useful to reinterpret the type, e.g.,
      when writing raw bytes.  This class is part of the low-level \e flecsi
      interface, so it is assumed that you know what you are doing...
   */
  template <typename T>
  class sparse_accessor_t
  {
   public:
    using iterator_t = utils::index_space_t::iterator_t;

    //! default constructor
    sparse_accessor_t() = default;

    /*!
      Constructor.

      \param label The c_str() version of the const_string used for
        this data variable's hash.
      \param size The size of the associated index space.
      \param data A pointer to the raw data.
      \param meta A reference to the user-defined meta data.
     */
    sparse_accessor_t(const std::string & label, const size_t size, T * data,
        const user_meta_data_t & meta)
        : label_(label), size_(size), data_(data), meta_(meta), is_(size_)
    {
    }

    /*!
      Copy Constructor.
     */
    sparse_accessor_t(const sparse_accessor_t & a)
        : label_(a.label_),
          size_(a.size_),
          data_(a.data_),
          meta_(a.meta_),
          is_(a.is_)
    {
    }

    /*!
      \brief Return a std::string containing the label of the data variable
        reference by this accessor.
     */
    const std::string & label() { return label_; }
    /*!
      \brief Return the size of the data variable referenced by this
        accessor.

      The size is in elements of type T.
     */
    size_t size() const { return size_; }
    /*!
      \brief Provide logical array-based access to the data for this
        data variable.  This is the const operator version.

      \tparam E A complex index type.

      This version of the operator is provided to support use with
      \e flecsi mesh entity types \ref mesh_entity_base_t.
     */
    template <typename E>
    const T & operator[](E * e) const
    {
      return this->operator[](e->id());
    } // operator

    /*!
      \brief Provide logical array-based access to the data for this
        data variable.

      \tparam E A complex index type.

      This version of the operator is provided to support use with
      \e flecsi mesh entity types \ref mesh_entity_base_t.
     */
    template <typename E>
    T & operator[](E * e)
    {
      return this->operator[](e->template id<0>());
    } // operator []

    /*!
      \brief Provide logical array-based access to the data for this
        data variable.  This is the const operator version.

      \param index The index of the data variable to return.
     */
    const T & operator[](size_t index) const
    {
      assert(index < size_ && "index out of range");
      return data_[index];
    } // operator []

    /*!
      \brief Provide logical array-based access to the data for this
        data variable.

      \param index The index of the data variable to return.
     */
    T & operator[](size_t index)
    {
      assert(index < size_ && "index out of range");
      return data_[index];
    } // operator []

    /*!
      \brief Assignment Operator.

      \param a The accessor instance from which to assign values.
     */
    sparse_accessor_t & operator=(const sparse_accessor_t & a)
    {
      label_ = a.label_;
      size_ = a.size_;
      data_ = a.data_;
      meta_ = a.meta_;
      is_ = a.is_;
    } // operator =

    /*!
      \brief Return the user meta data for this data variable.

      \return The user meta data.
     */
    const user_meta_data_t & meta() const { return meta_; }

    /*!
      \brief Return an iterator to the beginning of this data data.
     */
    iterator_t begin() { return {is_, 0}; }

    /*!
      \brief Return an iterator to the end of this data data.
     */
    iterator_t end() { return {is_, size_}; }

    /*!
      \brief Test to see if this accessor is empty

      \return true if registered.
     */
    operator bool() const
    {
      return (data_ != nullptr);
    }

   private:

    std::string label_ = "";
    size_t size_ = 0;
    T * data_ = nullptr;
    const user_meta_data_t & meta_ = user_meta_data_t();
    utils::index_space_t is_;

  }; // struct sparse_accessor_t

  /*--------------------------------------------------------------------------*
   * struct meta_data_t
   *--------------------------------------------------------------------------*/

  /*!
    \brief meta_data_t provides storage for extra information that is
      used to interpret data variable information at different points
      in the low-level runtime.
   */
  struct meta_data_t {
    std::string label;
    user_meta_data_t user_data;
    size_t size;
    size_t type_size;

    /*!
      \brief type_info_t allows creation of reference information
        to the user-specified type of the data data.

      The std::type_info type requires dynamic initialization.  The
        type_info_t type is designed to allow construction without
        needing a non-trivial default constructor for the
        meta_data_t type.
     */
    struct type_info_t {
      type_info_t(const std::type_info & type_info_) : type_info(type_info_) {}
      const std::type_info & type_info;
    }; // struct type_info_t

    std::shared_ptr<type_info_t> rtti;

    std::vector<uint8_t> data;
  }; // struct meta_data_t

  /*--------------------------------------------------------------------------*
   * global_accessor_t accessors
   *--------------------------------------------------------------------------*/

  /*!
    Return an accessor to the data for a given variable.

    \param key The name of the data to return.
   */
  template <typename T, size_t NS>
  global_accessor_t<T> 
  global_accessor(
    const utils::const_string_t & key,
    uintptr_t runtime_namespace
  ) {
    size_t h = key.hash() ^ runtime_namespace;
    auto search = meta_[NS].find(h);
    if ( search == meta_[NS].end() )
      return global_accessor_t<T>();
    else {
      auto & meta_data = search->second;
      return {meta_data.label, meta_data.size,
          reinterpret_cast<T *>(&meta_data.data[0]),
          meta_data.user_data};
    }
  } // accessor

  /*!
    Return an accessor to the data for a given variable.

    \param hash A hash key for the data to return.
   */
  template <typename T, size_t NS>
  global_accessor_t<T> 
  global_accessor(
    utils::const_string_t::hash_type_t hash,
    uintptr_t runtime_namespace
  ) {
    size_t h = hash ^ runtime_namespace;
    auto search = meta_[NS].find(h);
    if ( search == meta_[NS].end() )
      return global_accessor_t<T>();
    else {
      auto & meta_data = search->second;
      return {meta_data.label, meta_data.size,
          reinterpret_cast<T *>(&meta_data.data[0]),
          meta_data.user_data};
    }
  } // accessor

  /*!
    Return an accessor to the data for a given variable.

    \param hash A hash key for the data to return.
   */
  template <typename T, size_t NS>
  global_accessor_t<T>
  global_accessor(
    utils::const_string_t::hash_type_t hash
  ) {
    auto search = meta_[NS].find(hash);
    if ( search == meta_[NS].end() )
      return global_accessor_t<T>();
    else {
      auto & meta_data = search->second;
      return {meta_data.label, meta_data.size,
          reinterpret_cast<T *>(&meta_data.data[0]),
          /*  */
          meta_data.user_data};
    }
  } // accessor

  /*!
    Register a new state variable.

    \param key The key of the variable to add.
    \param indices The number of indices that parameterize the state.
   */
  template <typename T, size_t NS, typename... Args>
  decltype(auto) 
  register_global_state(
    const utils::const_string_t & key, 
    uintptr_t runtime_namespace, 
    Args &&... args
  ) {
    size_t h = key.hash() ^ runtime_namespace;

    // keys must be unique within a given namespace
    assert(meta_[NS].find(h) == meta_[NS].end() &&
      "key already exists");

    // user meta data
    meta_[NS][h].user_data.initialize(std::forward<Args>(args)...);

    // store the type size and allocate storage
    meta_[NS][h].label = key.c_str();
    meta_[NS][h].size = 1;
    meta_[NS][h].type_size = sizeof(T);
    meta_[NS][h].rtti.reset(
        new typename meta_data_t::type_info_t(typeid(T)));
    meta_[NS][h].data.resize(sizeof(T));

    return global_accessor<T, NS>(key.hash(), runtime_namespace);
  } // register

  /*!
    Return an accessor to all data for a given type.
   */
  template <typename T, size_t NS>
  std::vector<global_accessor_t<T>> global_accessors(
    uintptr_t runtime_namespace)
  {
    std::vector<global_accessor_t<T>> v;

    for (auto entry_pair : meta_[NS]) {
      auto a = global_accessor<T, NS>(entry_pair.first);
      if ( a )
        if (entry_pair.second.rtti->type_info == typeid(T))
          v.emplace_back( std::move(a) );
    } // for

    return v;
  } // global_accessors

  /*!
    Return an accessor to all data for a given type and predicate.
   */
  template <typename T, size_t NS, typename P>
  std::vector<global_accessor_t<T>> global_accessors(P && predicate,
    uintptr_t runtime_namespace)
  {
    std::vector<global_accessor_t<T>> v;

    for (auto entry_pair : meta_[NS]) {
      // create an accessor
      auto a = global_accessor<T, NS>(entry_pair.first);
      if ( a )
        if (entry_pair.second.rtti->type_info == typeid(T) && predicate(a))
          v.emplace_back( std::move(a) );
    } // for

    return v;
  } // accessors_predicate

  /*!
    Return accessors to all data.
   */
  template <size_t NS>
  std::vector<global_accessor_t<uint8_t>> global_accessors(
    uintptr_t runtime_namespace)
  {
    std::vector<global_accessor_t<uint8_t>> v;

    for (auto entry_pair : meta_[NS]) {
      auto a = global_accessor<uint8_t, NS>(entry_pair.first);
      if ( a ) v.emplace_back( std::move(a) );
    } // for

    return v;
  } // global_accessors

  /*!
    Return an accessor to all data for a given type and predicate.
   */
  template <size_t NS, typename P>
  std::vector<global_accessor_t<uint8_t>> global_accessors(P && predicate,
    uintptr_t runtime_namespace)
  {
    std::vector<global_accessor_t<uint8_t>> v;

    for (auto entry_pair : meta_[NS]) {
      // create an accessor
      auto a = global_accessor<uint8_t, NS>(entry_pair.first);
      if ( a )
        if ( predicate(a) ) v.emplace_back( std::move(a) );
    } // for

    return v;
  } // global_accessors_predicate



  /*--------------------------------------------------------------------------*
   * dense_accessor_t accessors
   *--------------------------------------------------------------------------*/


  /*!
    Return an accessor to the data for a given variable.

    \param key The name of the data to return.
   */
  template <typename T, size_t NS>
  dense_accessor_t<T> 
  dense_accessor(
    const utils::const_string_t & key,
    uintptr_t runtime_namespace
  ) {
    size_t h = key.hash() ^ runtime_namespace;
    auto search = meta_[NS].find(h);
    if ( search == meta_[NS].end() )
      return dense_accessor_t<T>();
    else {
      auto & meta_data = search->second;
      return {meta_data.label, meta_data.size,
          reinterpret_cast<T *>(&meta_data.data[0]),
          meta_data.user_data};
    }
  } // accessor

  /*!
    Return an accessor to the data for a given variable.

    \param hash A hash key for the data to return.
   */
  template <typename T, size_t NS>
  dense_accessor_t<T>
  dense_accessor(
    utils::const_string_t::hash_type_t hash,
    uintptr_t runtime_namespace
  ) {
    size_t h = hash ^ runtime_namespace;
    auto search = meta_[NS].find(h);
    if ( search == meta_[NS].end() )
      return dense_accessor_t<T>();
    else {
      auto & meta_data = search->second;
      return {meta_data.label, meta_data.size,
          reinterpret_cast<T *>(&meta_data.data[0]),
          meta_data.user_data};
    }
  } // accessor

  /*!
    Return an accessor to the data for a given variable.

    \param hash A hash key for the data to return.
   */
  template <typename T, size_t NS>
  dense_accessor_t<T> 
  dense_accessor(
    utils::const_string_t::hash_type_t hash
  ) {
    auto search = meta_[NS].find(hash);
    if ( search == meta_[NS].end() )
      return dense_accessor_t<T>();
    else {
      auto & meta_data = search->second;
      return {meta_data.label, meta_data.size,
          reinterpret_cast<T *>(&meta_data.data[0]),
          meta_data.user_data};
    }
  } // accessor

  /*!
    Register a new state variable.

    \param key The key of the variable to add.
    \param indices The number of indices that parameterize the state.
   */
  template <typename T, size_t NS, typename... Args>
  decltype(auto)
  register_state(
    const utils::const_string_t & key, 
    size_t indices, 
    uintptr_t runtime_namespace,
    Args &&... args
  ) {
    size_t h = key.hash() ^ runtime_namespace;

    // keys must be unique within a given namespace
    assert(
        meta_[NS].find(h) == meta_[NS].end() && "key already exists");

    // user meta data
    meta_[NS][h].user_data.initialize(std::forward<Args>(args)...);

    // store the type size and allocate storage
    meta_[NS][h].label = key.c_str();
    meta_[NS][h].size = indices;
    meta_[NS][h].type_size = sizeof(T);
    meta_[NS][h].rtti.reset(
        new typename meta_data_t::type_info_t(typeid(T)));
    meta_[NS][h].data.resize(indices * sizeof(T));

    return dense_accessor<T, NS>(key.hash(), runtime_namespace);
  } // register

  /*!
    Return an accessor to all data for a given type.
   */
  template <typename T, size_t NS>
  std::vector<dense_accessor_t<T>> dense_accessors(uintptr_t runtime_namespace)
  {
    std::vector<dense_accessor_t<T>> v;

    for (auto entry_pair : meta_[NS]) {
      auto a = dense_accessor<T, NS>(entry_pair.first);
      if ( a )
        if (entry_pair.second.rtti->type_info == typeid(T))
          v.emplace_back( std::move(a) );
    } // for

    return v;
  } // dense_accessors

  /*!
    Return an accessor to all data for a given type and predicate.
   */
  template <typename T, size_t NS, typename P>
  std::vector<dense_accessor_t<T>> dense_accessors(P && predicate,
    uintptr_t runtime_namespace)
  {
    std::vector<dense_accessor_t<T>> v;

    for (auto entry_pair : meta_[NS]) {
      // create an accessor
      auto a = dense_accessor<T, NS>(entry_pair.first);
      if ( a )
        if (entry_pair.second.rtti->type_info == typeid(T) && predicate(a))
          v.emplace_back( std::move(a) );
    } // for

    return v;
  } // accessors_predicate

  /*!
    Return accessors to all data.
   */
  template <size_t NS>
  std::vector<dense_accessor_t<uint8_t>> dense_accessors(
    uintptr_t runtime_namespace)
  {
    std::vector<dense_accessor_t<uint8_t>> v;

    for (auto entry_pair : meta_[NS]) {
      auto a = dense_accessor<uint8_t, NS>(entry_pair.first);
      if ( a ) v.emplace_back( std::move(a) );
    } // for

    return v;
  } // dense_accessors

  /*!
    Return an accessor to all data for a given type and predicate.
   */
  template <size_t NS, typename P>
  std::vector<dense_accessor_t<uint8_t>> dense_accessors(P && predicate,
    uintptr_t runtime_namespace)
  {
    std::vector<dense_accessor_t<uint8_t>> v;

    for (auto entry_pair : meta_[NS]) {
      // create an accessor
      auto a = dense_accessor<uint8_t, NS>(entry_pair.first);
      if ( a )
        if ( predicate(a) ) v.emplace_back( std::move(a) );
    } // for

    return v;
  } // dense_accessors_predicate

  /*--------------------------------------------------------------------------*
   * meta_data accessors
   *--------------------------------------------------------------------------*/

  /*!
    Return user meta data for a given variable.

    \param key The name of the data to return.
   */
  template <size_t NS>
  user_meta_data_t & 
  meta_data(
    const utils::const_string_t & key,
    uintptr_t runtime_namespace
  ) {
    size_t h = key.hash() ^ runtime_namespace;
    return meta_[NS].at(h).user_data;
  } // meta_data

  /*!
    Return pointer to raw data.

    \param name The name of the data to return.
   */
  template <typename T, size_t NS>
  std::shared_ptr<T> & 
  data(const 
    utils::const_string_t & key
  ) {
    std::shared_ptr<T> sp{new T[meta_[NS][key.hash()].size]};
    memcpy(
        *sp, &meta_[NS][key.hash()].data[0], meta_[NS][key.hash()].data.size());
    return sp;
  } // data

private:

  std::unordered_map<size_t,
    std::unordered_map<
      utils::const_string_t::hash_type_t, meta_data_t
    >
  > meta_;

}; // class default_data_storage_policy_t

} // namespace data
} // namespace flecsi

#endif // flecsi_default_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
