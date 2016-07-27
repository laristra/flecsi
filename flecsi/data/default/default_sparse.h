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

#ifndef flecsi_default_sparse_h
#define flecsi_default_sparse_h

#include <map>

#include "flecsi/utils/const_string.h"
#include "flecsi/data/default/default_storage_type.h"

#define np(X)                                                             \
  std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
             << ": " << #X << " = " << (X) << std::endl

/*!
 * \file default_sparse.h
 * \authors bergen
 * \date Initial file creation: Apr 17, 2016
 */

namespace flecsi
{
namespace data_model
{
namespace default_storage_policy
{

/*+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=*
 * Helper type definitions.
 *+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=*/

/*----------------------------------------------------------------------------*
 * Sparse accessor.
 *----------------------------------------------------------------------------*/

using index_pair_ = std::pair<size_t, size_t>;

template<typename T>
struct entry_value_{
  entry_value_(size_t entry)
  : entry(entry){}

  entry_value_(size_t entry, T value)
  : entry(entry),
  value(value){}

  entry_value_(){}

  size_t entry;
  T value;
};

template<typename T, typename MD>
struct sparse_accessor_t {

  /*--------------------------------------------------------------------------*
   * Type definitions.
   *--------------------------------------------------------------------------*/
  
  using iterator_t = index_space_t::iterator_t;
  using meta_data_t = MD;
  using user_meta_data_t = typename meta_data_t::user_meta_data_t;

  /*--------------------------------------------------------------------------*
   * Constructors.
   *--------------------------------------------------------------------------*/

  sparse_accessor_t() {}

  sparse_accessor_t(const std::string & label,
                    size_t version,
                    meta_data_t& meta_data_store,
                    const user_meta_data_t & meta_data)
    : label_(label), version_(version), meta_data_store_(meta_data_store),
      meta_data_(meta_data) {}

  T& operator()(size_t index, size_t material = 0){

  }

  void dump(){

  }

  T* data(){
    return nullptr;
  }

private:

  std::string label_ = "";
  size_t version_;
  const meta_data_t & meta_data_store_ = {}; 
  const user_meta_data_t & meta_data_ = {};
  
}; // struct sparse_accessor_t

template<typename T, typename MD>
struct sparse_mutator_t {

  /*--------------------------------------------------------------------------*
   * Type definitions.
   *--------------------------------------------------------------------------*/
  
  using iterator_t = index_space_t::iterator_t;
  using meta_data_t = MD;
  using user_meta_data_t = typename meta_data_t::user_meta_data_t;

  using entry_value_t = entry_value_<T>;

  /*--------------------------------------------------------------------------*
   * Constructors.
   *--------------------------------------------------------------------------*/

  sparse_mutator_t() {}

  sparse_mutator_t(size_t num_slots,
                   const std::string & label,
                   size_t version,
                   meta_data_t& meta_data_store,
                   const user_meta_data_t & meta_data)
    : num_slots_(num_slots), label_(label), version_(version), 
    meta_data_store_(meta_data_store), meta_data_(meta_data),
    num_indices_(meta_data_store_.size),
    num_materials_(meta_data_store_.num_materials),
    commited_(false){


  }

  ~sparse_mutator_t(){

  }

  T& operator()(size_t index, size_t material = 0){
    assert(index < num_indices_ && material < num_materials_);

    index_pair_& ip = indices_[index];
    
    size_t n = ip.second - ip.first;
    
    if(n >= num_slots_){
      return spare_map_.emplace(index, entry_value_t(material))->second.value;
    }

    entry_value_t* start = entries_ + index * num_slots_;     
    entry_value_t* end = start + n;     

    entry_value_t k(material);

    entry_value_t* itr = 
      lower_bound(start, end, k,
                  [](const auto& k1, const auto& k2) -> bool{
                    return k1.entry < k2.entry;
                  });

    while(end != itr){
      *(end) = *(end - 1);
      --end;
    }

    itr->entry = material;

    ip.second++;

    return itr->value;
  }

  void dump(){

  }

  T* data(){
    return nullptr;
  }

  void commit(){
    if(commited_){
      return;
    }

    constexpr size_t INDICES_KEY = 0;
    constexpr size_t ENTRIES_KEY = 1;

    size_t* indices = 
      reinterpret_cast<size_t*>(&meta_data_[data][INDICES_KEY][0]);

    std::vector<uint8_t>& raw_entries = meta_data_[data][ENTRIES_KEY];

    constexpr size_t ev_bytes = sizeof(entry_value_t);

    size_t s = raw_entries.size();
    size_t c = raw_entries.capacity();
    size_t d = (num_indices_ * num_slots_ + spare_map_.size()) * ev_bytes;

    if(c - s < d){
      raw_entries.resize(c * ev_bytes + d);
    }

    entry_value_t* entries = 
      reinterpret_cast<index_pair_*>(&meta_data_[data][ENTRIES_KEY][0]);

    entry_value_t* entries_end = entries + s / ev_bytes;

    for(size_t i = 0; i < num_indices_; ++i){
      index_pair_& ip = indices_[i];
      
      size_t n = ip.second - ip.first;

      size_t pos = indices[i];

      if(n == 0){
        indices[i + 1] = pos;
        continue;
      }

      auto p = spare_map_.equal_range(i);
      
      size_t m = distance(p.first, p.second);

      entry_value_t* start = entries + i * num_slots_;     
      entry_value_t* end = start + n; 

      auto iitr = entries + pos;

      std::copy(iitr, entries_end, iitr + n + m);
      std::copy(start, end, iitr);
      entries_end += n + m;

      auto cmp = [](const auto& k1, const auto& k2) -> bool{
                   return k1.entry < k2.entry;
                  };

      auto nitr = iitr + indices[i + 1];

      inplace_merge(iitr, nitr, nitr + n, cmp);

      if(m == 0){
        indices[i + 1] = pos + n;
        continue;
      }

      indices[i + 1] = pos + n + m;

      auto vitr = nitr + n;

      for(auto itr = p.first; itr != p.second; ++itr){
        vitr->column = itr->second.column;
        vitr->value = itr->second.value;
        ++vitr;
      }

      inplace_merge(iitr, nitr + n, nitr + n + m, cmp);
    }

    commited_ = true;
  }

private:
  using spare_map_t = std::multimap<size_t, entry_value_<T>>;

  size_t num_slots_;
  std::string label_ = "";
  size_t version_;
  const meta_data_t & meta_data_store_ = {}; 
  const user_meta_data_t & meta_data_ = {};

  size_t num_indices_;
  size_t num_materials_;
  index_pair_* indices_;
  entry_value_<T>* entries_;
  spare_map_t spare_map_;
  bool commited_;
}; // struct sparse_accessor_t

/*----------------------------------------------------------------------------*
 * Sparse handle.
 *----------------------------------------------------------------------------*/

template<typename T>
struct sparse_handle_t {
}; // struct sparse_handle_t

/*+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=*
 * Main type definition.
 *+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=*/

/*!
  FIXME: Sparse storage type.
 */
template<typename DS, typename MD>
struct storage_type_t<sparse, DS, MD> {

  /*--------------------------------------------------------------------------*
   * Type definitions.
   *--------------------------------------------------------------------------*/

	using data_store_t = DS;
	using meta_data_t = MD;

	template<typename T>
	using accessor_t = sparse_accessor_t<T, MD>;

  template<typename T>
  using mutator_t = sparse_mutator_t<T, MD>;

	template<typename T>
	using handle_t = sparse_handle_t<T>;

  /*--------------------------------------------------------------------------*
   * Data registration.
   *--------------------------------------------------------------------------*/

  template<typename T, size_t NS, typename ... Args>
  static handle_t<T> register_data(data_client_t & data_client,
    data_store_t & data_store, const const_string_t & key,
    size_t versions, size_t indices, size_t num_materials, Args && ... args) {

    np(num_materials);

		size_t h = key.hash() ^ data_client.runtime_id();

		// Runtime assertion that this key is unique
		assert(data_store[NS].find(h) == data_store[NS].end() &&
			"key already exists");

		data_store[NS][h].user_data.initialize(std::forward<Args>(args) ...);

		data_store[NS][h].label = key.c_str();
		data_store[NS][h].size = indices;
		data_store[NS][h].type_size = sizeof(T);
		data_store[NS][h].versions = versions;
		data_store[NS][h].rtti.reset(
			new typename meta_data_t::type_info_t(typeid(T)));

    data_store[NS][h].num_materials = num_materials;

		return {};
  } // register_data

  /*!
   */
  template<typename T, size_t NS>
  static accessor_t<T> get_accessor(data_client_t & data_client,
    data_store_t & data_store, const const_string_t & key,
    size_t version) {
    const size_t h = key.hash() ^ data_client.runtime_id();
    auto search = data_store[NS].find(h);

    if(search == data_store[NS].end()) {
      return {};
    }
    else {
      auto & meta_data = search->second;

      // check that the requested version exists
      assert(meta_data.versions > version && "version out-of-range");

      return { meta_data.label, version, meta_data, meta_data.user_data };
    } // if
  } // get_accessor

  /*!
   */
  template<typename T, size_t NS>
  static mutator_t<T> get_mutator(data_client_t & data_client,
    data_store_t & data_store, const const_string_t & key,
    size_t slots, size_t version) {
    const size_t h = key.hash() ^ data_client.runtime_id();
    auto search = data_store[NS].find(h);

    if(search == data_store[NS].end()) {
      return {};
    }
    else {
      auto & meta_data = search->second;

      // check that the requested version exists
      assert(meta_data.versions > version && "version out-of-range");

      return { slots, meta_data.label, version, meta_data, meta_data.user_data };
    } // if
  } // get_accessor

  /*!
   */
  template<typename T, size_t NS>
  static handle_t<T> get_handle(data_client_t & data_client,
    data_store_t & data_store, const const_string_t & key, size_t version) {
    return {};
  } // get_handle

}; // struct storage_type_t

} // namespace default_storage_policy
} // namespace data_model
} // namespace flecsi

#endif // flecsi_default_sparse_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
