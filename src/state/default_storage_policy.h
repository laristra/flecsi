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
    user_meta_data_t user_data;
    size_t type_size;
    std::vector<uint8_t> data;
  }; // struct meta_data_

  /*!
    Register a new state variable.

    \param key The key of the variable to add.
    \param indices The number of indices that parameterize the state.
   */
  template<typename T, size_t NS, typename ... Args>
  void register_state(const_string_t key, size_t indices, Args && ... args) {
    
    // add space as we need it
    if(meta_.size() < NS+1) {
      meta_.resize(NS+1);
    } // if

    // keys must be unique within a given dimension
    assert(meta_[NS].find(key.hash()) == meta_[NS].end() &&
      "key already exists");

    // user meta data
    meta_[NS][key.hash()].user_data.initialize(std::forward<Args>(args) ...);
    
    // store the type size and allocate storage
    meta_[NS][key.hash()].type_size = sizeof(T);
    meta_[NS][key.hash()].data.resize(indices*sizeof(T));
  } // register

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
  template<typename T, size_t NS>
  accessor_t<T> accessor(const_string_t key) {
    return { meta_[NS][key.hash()].data.size()/sizeof(T),
      reinterpret_cast<T *>(&meta_[NS][key.hash()].data[0]) };
  } // accessor

  /*!
    Return meta data for a given variable.

    \param name The name of the data to return.
   */
  template<size_t NS>
  user_meta_data_t & meta_data(const_string_t key) {
    return meta_[NS][key.hash()].user_data;
  } // meta_data

  /*!
    Return pointer to raw data.

    \param name The name of the data to return.
   */
  template<typename T, size_t NS>
  T * data(const const_string_t& key) {
    return static_cast<T *>(&meta_[NS][key.hash()].data[0]);
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
