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

#ifndef flexi_state_h
#define flexi_state_h

#include <unordered_map>
#include <memory>

#include "../utils/index_space.h"
#include "../utils/const_string.h"
#include "../utils/bitfield.h"

/*!
 * \file state.h
 * \authors bergen
 * \date Initial file creation: Oct 09, 2015
 */

namespace flexi {

enum class state_attribute : bitfield_t::field_type_t {
  persistent = 0
}; // state_attribute

const bitfield_t::field_type_t persistent =
  static_cast<bitfield_t::field_type_t>(flexi::state_attribute::persistent);

/*----------------------------------------------------------------------------*
 * class default_state_storage_policy_t
 *----------------------------------------------------------------------------*/

/*!
  \class default_state_storage_policy_t state.h
  \brief default_state_storage_policy_t provides...
 */

class default_state_storage_policy_t
{
protected:

  //! Constructor
  default_state_storage_policy_t() {}

  //! Desctructor
  virtual ~default_state_storage_policy_t() {}

  struct meta_data_t {
    size_t type_size;
    bitfield_t attributes;
    std::vector<uint8_t> data;
  }; // struct meta_data_

  /*!
    Register a new state variable.

    \param key The key of the variable to add.
    \param indices The number of indices that parameterize the state.
   */
  template<typename T, size_t D>
  void register_state(const_string_t key, size_t indices,
    bitfield_t attributes) {
    
    // add space as we need it
    if(meta_.size() < D+1) {
      meta_.resize(D+1);
    } // if

    // keys must be unique within a given dimension
    assert(meta_[D].find(key.hash()) == meta_[D].end() &&
      "key already exists");
    
    // store the type size and allocate storage
    meta_[D][key.hash()].type_size = sizeof(T);
    meta_[D][key.hash()].attributes = attributes;
    meta_[D][key.hash()].data.resize(indices*sizeof(T));
  } // register

  template<size_t D>
  bitfield_t & attributes(const_string_t key) {
    return meta_[D][key.hash()].attributes;
  } // attributes

  /*--------------------------------------------------------------------------*
   * class accessor_t
   *--------------------------------------------------------------------------*/

  template<typename T>
  class accessor_t
  {
  public:

    using iterator_t = index_space_t::iterator_t;

    accessor_t(size_t size, T * data)
      : size_(size), data_(data), is_(size_) {}

    const T & operator [] (size_t index) const {
      assert(index < size_ && "index out of range");
      return data_[index];
    } // operator

    T & operator [] (size_t index) {
      assert(index < size_ && "index out of range");
      return data_[index];
    } // operator

    iterator_t begin() { return { is_, 0 }; }
    iterator_t end() { return { is_, size_ }; }

  private:
    
    size_t size_;
    T * data_;
    index_space_t is_;

  }; // struct accessor_t

  /*!
    Return an accessor to the data for a given variable.

    \param name The name of the data to return.
   */
  template<typename T, size_t D>
  accessor_t<T> accessor(const_string_t key) {
    return { meta_[D][key.hash()].data.size()/sizeof(T),
      reinterpret_cast<T *>(&meta_[D][key.hash()].data[0]) };
  } // accessor

  /*!
    Return pointer to raw data.

    \param name The name of the data to return.
   */
  template<typename T, size_t D>
  T * data(const const_string_t& key) {
    return static_cast<T *>(&meta_[D][key.hash()].data[0]);
  } // data

private:

  std::vector<std::unordered_map<const_string_t::hash_type_t,
    meta_data_t>> meta_;
  
}; // class default_state_storage_policy_t

/*----------------------------------------------------------------------------*
 * class state_t
 *----------------------------------------------------------------------------*/

/*!
  \class state state.h
  \brief state provides...
 */

template<typename storage_policy_t = default_state_storage_policy_t>
class state_t : public storage_policy_t
{
public:

  template<typename T>
  using accessor_t = typename storage_policy_t::template accessor_t<T>;

  enum class attribute {
    persistent = 0x01
  }; // enum class attribute

  //! Default constructor
  state_t() : storage_policy_t() {}

  //! Destructor
   ~state_t() {}

  template<typename T, size_t D>
  void register_state(const const_string_t & key, size_t indices,
    bitfield_t attributes) {
    storage_policy_t::template register_state<T,D>(key, indices, attributes);
  } // register_state

  template<typename T, size_t D>
  accessor_t<T> accessor(const_string_t key) {
    return storage_policy_t::template accessor<T,D>(key);
  } // accessor

  template<size_t D>
  bitfield_t & attributes(const_string_t key) {
    return storage_policy_t::template attributes<D>(key);
  } // attributes

  template<typename T, size_t D>
  std::shared_ptr<T> & data(const_string_t key) {
    return storage_policy_t::template data<T,D>(key);
  } // data

  //! Copy constructor (disabled)
  state_t(const state_t &) = delete;

  //! Assignment operator (disabled)
  state_t & operator = (const state_t &) = delete;

}; // class state_t

} // namespace flexi

#endif // flexi_state_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
