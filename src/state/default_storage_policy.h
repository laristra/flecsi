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

#include "../utils/index_space.h"
#include "../utils/const_string.h"

/*!
 * \file default_storage_policy.h
 * \authors bergen
 * \date Initial file creation: Oct 27, 2015
 */

namespace flecsi
{
/*----------------------------------------------------------------------------*
 * class default_state_storage_policy_t
 *----------------------------------------------------------------------------*/

/*!
  \class default_state_storage_policy_t default_storage_policy.h
  \brief default_state_storage_policy_t provides a serial/local storage
    policy for the \e flecsi state model.

  This storage policy is probably adequate for serial or MPI-based
  runtimes.  This implementation should not be used as a template
  for creating new storage policies.  If you are developing a new
  policy, look at the interface in \ref state.h.  This is the interface
  that needs to be implemented by new storage policies.
 */
template <typename user_meta_data_t>
class default_state_storage_policy_t
{
 protected:
  //! Constructor
  default_state_storage_policy_t() {}
  //! Desctructor
  virtual ~default_state_storage_policy_t() {}
  /*--------------------------------------------------------------------------*
   * class dense_accessor_t
   *--------------------------------------------------------------------------*/

  /*!
    \brief dense_accessor_t provides logically array-based access to state
      variables that have been registered in the state model.

    \tparam T The type of the state variable.  If this type is not
      consistent with the type used to register the state, bad things
      can happen.  However, it can be useful to reinterpret the type, e.g.,
      when writing raw bytes.  This class is part of the low-level \e flecsi
      interface, so it is assumed that you know what you are doing...
   */
  template <typename T>
  class dense_accessor_t
  {
   public:
    using iterator_t = index_space_t::iterator_t;

    /*!
      Constructor.

      \param label The c_str() version of the const_string used for
        this state variable's hash.
      \param size The size of the associated index space.
      \param data A pointer to the raw data.
      \param meta A reference to the user-defined meta data.
     */
    dense_accessor_t(const std::string & label, const size_t size, T * data,
        const user_meta_data_t & meta)
        : label_(label), size_(size), data_(data), meta_(meta), is_(size_)
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
      \brief Return a std::string containing the label of the state variable
        reference by this accessor.
     */
    const std::string & label() { return label_; }
    /*!
      \brief Return the size of the state variable referenced by this
        accessor.

      The size of the assocated index space.
     */
    size_t size() const { return size_; }
    /*!
      \brief Provide logical array-based access to the data for this
        state variable.  This is the const operator version.

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
        state variable.

      \tparam E A complex index type.

      This version of the operator is provided to support use with
      \e flecsi mesh entity types \ref mesh_entity_base_t.
     */
    template <typename E>
    T & operator[](E * e)
    {
      return this->operator[](e->template local_id<0>());
    } // operator []

    /*!
      \brief Provide logical array-based access to the data for this
        state variable.  This is the const operator version.

      \param index The index of the state variable to return.
     */
    const T & operator[](size_t index) const
    {
      assert(index < size_ && "index out of range");
      return data_[index];
    } // operator []

    /*!
      \brief Provide logical array-based access to the data for this
        state variable.

      \param index The index of the state variable to return.
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
      \brief Return the user meta data for this state variable.

      \return The user meta data.
     */
    const user_meta_data_t & meta() const { return meta_; }
    /*!
      \brief Return an iterator to the beginning of this state data.
     */
    iterator_t begin() { return {is_, 0}; }
    /*!
      \brief Return an iterator to the end of this state data.
     */
    iterator_t end() { return {is_, size_}; }
   private:
    std::string label_;
    size_t size_;
    T * data_;
    const user_meta_data_t & meta_;
    index_space_t is_;

  }; // struct dense_accessor_t

  /*--------------------------------------------------------------------------*
   * class sparse_accessor_t
   *--------------------------------------------------------------------------*/

  /*!
    \brief sparse_accessor_t provides logically array-based access to state
      variables that have been registered in the state model.

    \tparam T The type of the state variable.  If this type is not
      consistent with the type used to register the state, bad things
      can happen.  However, it can be useful to reinterpret the type, e.g.,
      when writing raw bytes.  This class is part of the low-level \e flecsi
      interface, so it is assumed that you know what you are doing...
   */
  template <typename T>
  class sparse_accessor_t
  {
   public:
    using iterator_t = index_space_t::iterator_t;

    /*!
      Constructor.

      \param label The c_str() version of the const_string used for
        this state variable's hash.
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
      \brief Return a std::string containing the label of the state variable
        reference by this accessor.
     */
    const std::string & label() { return label_; }
    /*!
      \brief Return the size of the state variable referenced by this
        accessor.

      The size is in elements of type T.
     */
    size_t size() const { return size_; }
    /*!
      \brief Provide logical array-based access to the data for this
        state variable.  This is the const operator version.

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
        state variable.

      \tparam E A complex index type.

      This version of the operator is provided to support use with
      \e flecsi mesh entity types \ref mesh_entity_base_t.
     */
    template <typename E>
    T & operator[](E * e)
    {
      return this->operator[](e->template local_id<0>());
    } // operator []

    /*!
      \brief Provide logical array-based access to the data for this
        state variable.  This is the const operator version.

      \param index The index of the state variable to return.
     */
    const T & operator[](size_t index) const
    {
      assert(index < size_ && "index out of range");
      return data_[index];
    } // operator []

    /*!
      \brief Provide logical array-based access to the data for this
        state variable.

      \param index The index of the state variable to return.
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
      \brief Return the user meta data for this state variable.

      \return The user meta data.
     */
    const user_meta_data_t & meta() const { return meta_; }
    /*!
      \brief Return an iterator to the beginning of this state data.
     */
    iterator_t begin() { return {is_, 0}; }
    /*!
      \brief Return an iterator to the end of this state data.
     */
    iterator_t end() { return {is_, size_}; }
   private:
    std::string label_;
    size_t size_;
    T * data_;
    const user_meta_data_t & meta_;
    index_space_t is_;

  }; // struct sparse_accessor_t

  /*!
    \brief meta_data_t provides storage for extra information that is
      used to interpret state variable information at different points
      in the low-level runtime.
   */
  struct meta_data_t {
    std::string label;
    user_meta_data_t user_data;
    size_t size;
    size_t type_size;

    /*!
      \brief type_info_t allows creation of reference information
        to the user-specified type of the state data.

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

  /*!
    Return an accessor to the data for a given variable.

    \param key The name of the data to return.
   */
  template <typename T, size_t NS>
  dense_accessor_t<T> dense_accessor(const const_string_t & key)
  {
    return {meta_[NS][key.hash()].label, meta_[NS][key.hash()].size,
        reinterpret_cast<T *>(&meta_[NS][key.hash()].data[0]),
        meta_[NS][key.hash()].user_data};
  } // accessor

  /*!
    Return an accessor to the data for a given variable.

    \param hash A hash key for the data to return.
   */
  template <typename T, size_t NS>
  dense_accessor_t<T> dense_accessor(const_string_t::hash_type_t hash)
  {
    return {meta_[NS][hash].label, meta_[NS][hash].size,
        reinterpret_cast<T *>(&meta_[NS][hash].data[0]),
        meta_[NS][hash].user_data};
  } // accessor

  /*!
    Register a new state variable.

    \param key The key of the variable to add.
    \param indices The number of indices that parameterize the state.
   */
  template <typename T, size_t NS, typename... Args>
  decltype(auto) register_state(
      const const_string_t & key, size_t indices, Args &&... args)
  {
    // add space as we need it
    if (meta_.size() < NS + 1) {
      meta_.resize(NS + 1);
    } // if

    // keys must be unique within a given namespace
    assert(
        meta_[NS].find(key.hash()) == meta_[NS].end() && "key already exists");

    // user meta data
    meta_[NS][key.hash()].user_data.initialize(std::forward<Args>(args)...);

    // store the type size and allocate storage
    meta_[NS][key.hash()].label = key.c_str();
    meta_[NS][key.hash()].size = indices;
    meta_[NS][key.hash()].type_size = sizeof(T);
    meta_[NS][key.hash()].rtti.reset(
        new typename meta_data_t::type_info_t(typeid(T)));
    meta_[NS][key.hash()].data.resize(indices * sizeof(T));

    return dense_accessor<T, NS>(key.hash());
  } // register

  /*!
    Return an accessor to all data for a given type.
   */
  template <typename T, size_t NS>
  std::vector<dense_accessor_t<T>> dense_accessors()
  {
    std::vector<dense_accessor_t<T>> v;

    for (auto entry_pair : meta_[NS]) {
      if (entry_pair.second.rtti->type_info == typeid(T)) {
        v.push_back(dense_accessor<T, NS>(entry_pair.first));
      } // if
    } // for

    return v;
  } // dense_accessors

  /*!
    Return an accessor to all data for a given type and predicate.
   */
  template <typename T, size_t NS, typename P>
  std::vector<dense_accessor_t<T>> dense_accessors(P && predicate)
  {
    std::vector<dense_accessor_t<T>> v;

    for (auto entry_pair : meta_[NS]) {
      // create an accessor
      auto a = dense_accessor<T, NS>(entry_pair.first);

      if (entry_pair.second.rtti->type_info == typeid(T) && predicate(a)) {
        v.push_back(a);
      } // if
    } // for

    return v;
  } // accessors_predicate

  /*!
    Return accessors to all data.
   */
  template <size_t NS>
  std::vector<dense_accessor_t<uint8_t>> dense_accessors()
  {
    std::vector<dense_accessor_t<uint8_t>> v;

    for (auto entry_pair : meta_[NS]) {
      v.push_back(dense_accessor<uint8_t, NS>(entry_pair.first));
    } // for

    return v;
  } // dense_accessors

  /*!
    Return an accessor to all data for a given type and predicate.
   */
  template <size_t NS, typename P>
  std::vector<dense_accessor_t<uint8_t>> dense_accessors(P && predicate)
  {
    std::vector<dense_accessor_t<uint8_t>> v;

    for (auto entry_pair : meta_[NS]) {
      // create an accessor
      auto a = dense_accessor<uint8_t, NS>(entry_pair.first);

      if (predicate(a)) {
        v.push_back(a);
      } // if
    } // for

    return v;
  } // dense_accessors_predicate

  /*!
    Return user meta data for a given variable.

    \param key The name of the data to return.
   */
  template <size_t NS>
  user_meta_data_t & meta_data(const const_string_t & key)
  {
    return meta_[NS][key.hash()].user_data;
  } // meta_data

  /*!
    Return pointer to raw data.

    \param name The name of the data to return.
   */
  template <typename T, size_t NS>
  std::shared_ptr<T> & data(const const_string_t & key)
  {
    std::shared_ptr<T> sp{new T[meta_[NS][key.hash()].size]};
    memcpy(
        *sp, &meta_[NS][key.hash()].data[0], meta_[NS][key.hash()].data.size());
    return sp;
  } // data

 private:
  std::vector<std::unordered_map<const_string_t::hash_type_t, meta_data_t>>
      meta_;

}; // class default_state_storage_policy_t

} // namespace flecsi

#endif // flecsi_default_storage_policy_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
