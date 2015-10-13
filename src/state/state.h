/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flexi_state_h
#define flexi_state_h

#include "../utils/index_space.h"

/*!
 * \file state.h
 * \authors bergen
 * \date Initial file creation: Oct 09, 2015
 */

namespace flexi {

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
    std::vector<uint8_t> data;
  }; // struct meta_data_

  /*!
    Register a new state variable.

    \param key The has of the variable to add.
    \param indices The number of indices that parameterize the state.
   */
  template<typename T, size_t D>
  void register_state(const char * key, size_t indices) {
    
    // add space as we need it
    if(meta_.size() < D+1) {
      meta_.resize(D+1);
    } // if

    // keys must be unique within a given dimension
    assert(meta_[D].find(key) == meta_[D].end() && "key already exists");
    
    // store the type size and allocate storage
    meta_[D][key].type_size = sizeof(T);
    meta_[D][key].data.resize(indices*sizeof(T));
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
    Return an accessor to the data for a given key.

    \param key The hash of the data to return.
   */
  template<typename T, size_t D>
  accessor_t<T> accessor(const char * key) {
    return { meta_[D][key].data.size()/sizeof(T),
      reinterpret_cast<T *>(&meta_[D][key].data[0]) };
  } // accessor

  /*!
    Return pointer to raw data.

    \param key The hash of the data to return.
   */
  template<typename T, size_t D>
  T * data(const char * key) {
    return static_cast<T *>(&meta_[D][key].data()[0]);
  } // data

private:

  std::vector<std::map<const char *, meta_data_t>> meta_;
  
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

  //! Default constructor
  state_t() : storage_policy_t() {}

  //! Destructor
   ~state_t() {}

  template<typename T, size_t D>
  void register_state(const char * key, size_t indices) {
    storage_policy_t::template register_state<T,D>(key, indices);
  } // register_state

  template<typename T, size_t D>
  accessor_t<T> accessor(const char * key) {
    return storage_policy_t::template accessor<T,D>(key);
  } // accessor

  template<typename T, size_t D>
  T * data(const char * key) {
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
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
