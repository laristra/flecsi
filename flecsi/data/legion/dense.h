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

#ifndef flecsi_legion_dense_h
#define flecsi_legion_dense_h

//----------------------------------------------------------------------------//
// POLICY_NAMESPACE must be defined before including storage_type.h!!!
// Using this approach allows us to have only one storage_type_t
// definintion that can be used by all data policies -> code reuse...
#define POLICY_NAMESPACE legion
#include "flecsi/data/storage_type.h"
#undef POLICY_NAMESPACE
//----------------------------------------------------------------------------//

#include "flecsi/data/data_client.h"
#include "flecsi/data/accessor.h"
#include "flecsi/data/data_handle.h"
#include "flecsi/data/common/data_types.h"
#include "flecsi/utils/const_string.h"
#include "flecsi/utils/index_space.h"
#include "flecsi/execution/context.h"
#include "flecsi/data/legion/data_policy.h"
#include "flecsi/execution/legion/helper.h"
#include "flecsi/execution/task_ids.h"

#define np(X)                                                            \
 std::cout << __FILE__ << ":" << __LINE__ << ": " << __PRETTY_FUNCTION__ \
           << ": " << #X << " = " << (X) << std::endl

///
// \file legion/dense.h
// \authors bergen, nickm
// \date Initial file creation: Apr 7, 2016
///

namespace flecsi {
namespace data {
namespace legion {

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Helper type definitions.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Dense handle.
//----------------------------------------------------------------------------//

template<typename T, size_t EP, size_t SP, size_t GP>
struct dense_handle_t :
  public data_handle_t<T, EP, SP, GP>
{
  using type = T;
}; // struct dense_handle_t

//----------------------------------------------------------------------------//
// Dense accessor.
//----------------------------------------------------------------------------//

///
// \brief dense_accessor_t provides logically array-based access to data
//        variables that have been registered in the data model.
//
// \tparam T The type of the data variable. If this type is not
//           consistent with the type used to register the data, bad things
//           can happen. However, it can be useful to reinterpret the type,
//           e.g., when writing raw bytes. This class is part of the
//           low-level \e flecsi interface, so it is assumed that you
//           know what you are doing...
// \tparam MD The meta data type.
///
template<typename T, typename MD>
struct dense_accessor_t : public accessor__<T>
{
  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using iterator_t = utils::index_space_t::iterator_t;
  using meta_data_t = MD;
  using user_meta_data_t = typename meta_data_t::user_meta_data_t;

  //--------------------------------------------------------------------------//
  // Constructors.
  //--------------------------------------------------------------------------//

  dense_accessor_t() = default;
  
  ///
  // Constructor.
  //
  // \param label The c_str() version of the utils::const_string_t used for
  //              this data variable's hash.
  // \param size The size of the associated index space.
  // \param data A pointer to the raw data.
  // \param meta_data A reference to the user-defined meta data.
  ///
  dense_accessor_t(
    const std::string & label,
    const size_t size,
    std::vector<T>* data,
    Legion::PhysicalRegion pr,
    const user_meta_data_t & meta_data,
    bitset_t & user_attributes,
    size_t index_space)
    : label_(label),
    size_(size),
    data_(data),
    pr_(pr), 
    meta_data_(&meta_data),
    user_attributes_(&user_attributes),
    index_space_(index_space),
    is_(size) {}

	///
  // Copy constructor.
	///
  //dense_accessor_t(const dense_accessor_t & a) = delete;

  dense_accessor_t(const dense_accessor_t & a)
  : label_(a.label_),
    size_(a.size_),
    data_(a.data_),
    meta_data_(a.meta_data_),
    user_attributes_(a.user_attributes_),
    index_space_(a.index_space_),
    is_(a.is_){}

  dense_accessor_t(const data_handle_t<void, 0, 0, 0>& h)
  : data_(static_cast<std::vector<T>*>(h.exclusive_data)){}

  ~dense_accessor_t(){
    if(data_){
      flecsi::execution::context_t & context =
        flecsi::execution::context_t::instance();

      size_t task_key = 
        utils::const_string_t{"specialization_driver"}.hash();
      auto runtime = context.runtime(task_key);
      auto ctx = context.context(task_key);
      runtime->unmap_region(ctx, pr_);
      delete data_;
    }
  }

  //--------------------------------------------------------------------------//
  // Member data interface.
  //--------------------------------------------------------------------------//

	///
  // \brief Return a std::string containing the label of the data variable
  //        reference by this accessor.
	///
  const std::string &
  label() const
  {
    return label_;
  } // label

	///
  // \brief Return the index space size of the data variable
  //        referenced by this accessor.
	///
  size_t
  size() const
  {
    return size_;
  } // size

  ///
  ///
  ///
  size_t
  index_space() const
  {
    return index_space_;
  } // index_space

	///
  // \brief Return the user meta data for this data variable.
	///
  const user_meta_data_t &
  meta_data() const
  {
    return meta_data_;
  } // meta_data

  ///
  ///
  ///
  bitset_t &
  attributes()
  {
    return *user_attributes_;
  } // attributes

  const bitset_t &
  attributes() const
  {
    return *user_attributes_;
  } // attributes

  //--------------------------------------------------------------------------//
  // Iterator interface.
  //--------------------------------------------------------------------------//

  ///
  //
  ///
  iterator_t
  begin()
  {
    return {is_, 0};
  } // begin

  ///
  //
  ///
  iterator_t
  end()
  {
    return {is_, size_};
  } // end

  //--------------------------------------------------------------------------//
  // Operators.
  //--------------------------------------------------------------------------//

  /// \brief copy operator.
  /// \param [in] a  The accessor to copy.
  /// \return A reference to the new copy.
  dense_accessor_t & operator=(const dense_accessor_t & a)
  {
    label_ = a.label_;
    size_ = a.size_;
    data_ = a.data_;
    pr_ = a.pr_;
    meta_data_ = a.meta_data_;
    user_attributes_ = a.user_attributes_;
    index_space_ = a.index_space_;
    is_ = a.is_;
    return *this;
  } // operator =

	///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \tparam E A complex index type.
  //
  // This version of the operator is provided to support use with
  // \e flecsi mesh entity types \ref mesh_entity_base_t.
	///
  template<typename E>
  const T &
  operator [] (
    E * e
  ) const
  {
    return this->operator[](e->template id<0>());
  } // operator []

	///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \tparam E A complex index type.
  //
  // This version of the operator is provided to support use with
  // \e flecsi mesh entity types \ref mesh_entity_base_t.
	///
  template<typename E>
  T &
  operator [] (
    E * e
  )
  {
    return this->operator[](e->template id<0>());
  } // operator []

	///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
	///
  const T &
  operator [] (
    size_t index
  ) const
  {
    assert(index < size_ && "index out of range");
    assert(data_ && "data has not been mapped");
    return (*data_)[index];
  } // operator []

	///
  // \brief Provide logical array-based access to the data for this
  //        data variable.  This is the const operator version.
  //
  // \param index The index of the data variable to return.
	///
  T &
  operator [] (
    size_t index
  )
  {
    assert(index < size_ && "index out of range");
    assert(data_ && "data has not been mapped");
    return (*data_)[index];
  } // operator []

  ///
  /// \brief Provide logical array-based access to the data for this
  ///        data variable.  This is the const operator version.
  ///
  /// \param index The index of the data variable to return.
  ///
  T &
  operator () (
    size_t index
  )
  {
    assert(index < size_ && "index out of range");
    assert(data_ && "data has not been mapped");

    //return data_[index];
  } // operator []

	///
  // \brief Test to see if this accessor is empty
  //
  // \return true if registered.
	///
  operator bool() const
  {
    return data_ != nullptr;
  } // operator bool

private:

  std::string label_ = "";
  size_t size_ = 0;
  const user_meta_data_t * meta_data_ = nullptr;
  bitset_t * user_attributes_ = nullptr;
  utils::index_space_t is_;
  size_t index_space_ = 0;
  std::vector<T>* data_ = nullptr;
  Legion::PhysicalRegion pr_;
}; // struct dense_accessor_t

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//
// Main type definition.
//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=//

//----------------------------------------------------------------------------//
// Dense storage type.
//----------------------------------------------------------------------------//

///
// FIXME: Dense storage type.
///
template<typename DS, typename MD>
struct storage_type_t<dense, DS, MD>
{
  //--------------------------------------------------------------------------//
  // Type definitions.
  //--------------------------------------------------------------------------//

  using data_store_t = DS;
  using meta_data_t = MD;

  template<typename T>
  using accessor_t = dense_accessor_t<T, MD>;

  template<typename T,  size_t EP, size_t SP, size_t GP>
  using handle_t = dense_handle_t<T, EP, SP, GP>;

  using st_t = storage_type_t<dense, DS, MD>;

  //--------------------------------------------------------------------------//
  // Data registration.
  //--------------------------------------------------------------------------//

  ///
  // \tparam T Data type to register.
  // \tparam NS Namespace
  // \tparam Args Variadic arguments that are passed to
  //              metadata initialization.
  //
  // \param data_client Base class reference to client.
  // \param data_store A reference for accessing the low-level data.
  // \param key A const string instance containing the variable name.
  // \param versions The number of variable versions for this datum.
  // \param indices The number of indices in the index space.
  ///
  template<
    typename T,
    size_t NS,
    typename ... Args
  >
  static
  handle_t<T, 0, 0, 0>
  register_data(
    const data_client_t & data_client,
    data_store_t & data_store,
    const utils::const_string_t & key,
    size_t versions,
    size_t index_space,
    Args && ... args
  )
  {

    size_t h = key.hash()/* ^ data_client.runtime_id()*/;
    
    // Runtime assertion that this key is unique
    assert(data_store[NS].find(h) == data_store[NS].end() &&
      "key already exists");

    //------------------------------------------------------------------------//
    // Call the user meta data initialization method passing variadic
    // user arguments
    //------------------------------------------------------------------------//

    data_store[NS][h].user_data.initialize(std::forward<Args>(args) ...);

    //------------------------------------------------------------------------//
    // Set the data label
    //------------------------------------------------------------------------//
    
    data_store[NS][h].label = key.c_str();

    //------------------------------------------------------------------------//
    // Set the data size by calling the data clients indeces method.
    // This allows the user to interpret the index space argument
    // in whatever way they want.
    //------------------------------------------------------------------------//
    
    size_t size = data_client.indices(index_space);

    data_store[NS][h].size = size;

    //------------------------------------------------------------------------//
    // Store the index space.
    //------------------------------------------------------------------------//

    data_store[NS][h].index_space = index_space;

    //------------------------------------------------------------------------//
    // Store the data type size information.
    //------------------------------------------------------------------------//

    size_t type_size = sizeof(T);

    data_store[NS][h].type_size = type_size;

    //------------------------------------------------------------------------//
    // This allows us to set the runtime-type-information, which requires
    // a const reference.
    //------------------------------------------------------------------------//

    data_store[NS][h].rtti.reset(
      new typename meta_data_t::type_info_t(typeid(T)));

    //------------------------------------------------------------------------//
    // Store the number of versions.
    //------------------------------------------------------------------------//

    data_store[NS][h].versions = versions;

    //------------------------------------------------------------------------//
    // Allocate data for each version.
    //------------------------------------------------------------------------//

    flecsi::execution::context_t & context =
      flecsi::execution::context_t::instance();

    size_t task_key = utils::const_string_t{"specialization_driver"}.hash();
    auto runtime = context.runtime(task_key);
    auto ctx = context.context(task_key);

    execution::legion_helper helper(runtime, ctx);

    execution::field_ids_t & fid_t = execution::field_ids_t::instance(); 

    for(size_t i=0; i<versions; ++i) {
      data_store[NS][h].attributes[i].reset();

      legion_data_policy_t::partitioned_index_space& isp = 
        data_client.get_index_space(index_space);

      auto data = data_store[NS][h].create_legion_data();

      data.is = isp.entities_lr.get_index_space();
      
      data.fs = helper.create_field_space();
      LegionRuntime::HighLevel::FieldAllocator a =
          helper.create_field_allocator(data.fs);
      a.allocate_field(type_size, fid_t.fid_value);
      data.lr = helper.create_logical_region(data.is, data.fs);

      data.exclusive_ip = isp.exclusive_ip;
      data.shared_ip = isp.shared_ip;
      data.ghost_ip = isp.ghost_ip;

      data_store[NS][h].put_legion_data(i, data);

    } // for

    //------------------------------------------------------------------------//
    // num_entries is unused for this storage type.
    //------------------------------------------------------------------------//

    data_store[NS][h].num_entries = 0;

    return {};
  } // register_data

  //--------------------------------------------------------------------------//
  // Data accessors.
  //--------------------------------------------------------------------------//

  /// \brief Return a dense_accessor_t.
  ///
  /// \param [in] meta_data  The meta data to use to build the accessor.
  /// \param [in] version   The version to select.
  ///
  /// \remark In this version, the search for meta data was already done.
  template< typename T >
  static
  accessor_t<T>
  get_accessor(
    meta_data_t & meta_data,
    size_t version
  )
  {

    // check that the requested version exists
    if ( version >= meta_data.versions ) {
      std::cerr << "version out of range" << std::endl;
      std::abort();
    }

    flecsi::execution::context_t & context =
      flecsi::execution::context_t::instance();

    size_t task_key = utils::const_string_t{"specialization_driver"}.hash();
    auto runtime = context.runtime(task_key);
    auto ctx = context.context(task_key);

    execution::legion_helper helper(runtime, ctx);

    auto& data = meta_data.get_legion_data(version);

    execution::field_ids_t & fid_t = execution::field_ids_t::instance(); 

    LegionRuntime::HighLevel::RegionRequirement rr(data.lr, READ_WRITE, EXCLUSIVE, data.lr);
    rr.add_field(fid_t.fid_value);
    LegionRuntime::HighLevel::InlineLauncher il(rr);

    auto pr = runtime->map_region(ctx, il);
    pr.wait_until_valid();

    auto ac = 
      pr.get_field_accessor(fid_t.fid_value).typeify<T>();
    
    IndexIterator itr(runtime, ctx, data.lr.get_index_space());
    
    auto values = new std::vector<T>; 

    while(itr.has_next()){
      values->push_back(ac.read(itr.next()));
    }

    // construct an accessor from the meta data
    return { meta_data.label, meta_data.size,
      values, pr,
      meta_data.user_data, meta_data.attributes[version],
      meta_data.index_space };
  }

  /// \brief Return a dense_accessor_t.
  ///
  /// \param [in] data_store   The data store to search.
  /// \param [in] hash   The hash to search for.
  /// \param [in] version   The version to select.
  ///
  /// \remark In this version, the search for meta data has not been done,
  ///   but the key has already been hashed.
  template<
    typename T,
    size_t NS
  >
  static
  accessor_t<T>
  get_accessor(
    data_store_t & data_store,
    const utils::const_string_t::hash_type_t & hash,
    size_t version
  )
  {
    auto search = data_store[NS].find(hash);

    assert(search != data_store[NS].end() && "invalid key");
    
    return get_accessor<T>( search->second, version );
  } // get_accessor

  ///
  //
  ///
  template<
    typename T,
    size_t NS
  >
  static
  accessor_t<T>
  get_accessor(
    const data_client_t & data_client,
    data_store_t & data_store,
    const utils::const_string_t & key,
    size_t version
  )
  {
    auto hash = key.hash()/* ^ data_client.runtime_id()*/;
    return get_accessor<T,NS>(data_store, hash, version);
  } // get_accessor

  ///
  //
  ///
  template<
    typename T,
    size_t NS,
    typename Predicate
  >
  static
  decltype(auto)
  get_accessors(
    const data_client_t & data_client,
    data_store_t & data_store,
    size_t version,
    Predicate && predicate,
    bool sorted
  )
  {

    std::vector< accessor_t<T> > as;

    // the runtime id
    auto runtime_id = data_client.runtime_id();

    // loop over each key pair
    for (auto & entry_pair : data_store[NS]) {
      // get the meta data key and label
      const auto & meta_data_key = entry_pair.first;
      auto & meta_data = entry_pair.second;
      // now build the hash for this label
      const auto & label = meta_data.label;
      auto key_hash = 
        utils::hash<utils::const_string_t::hash_type_t>(label, label.size());
      auto hash = key_hash ^ runtime_id;
      // filter out the accessors for different data_clients
      if ( meta_data_key != hash ) continue;
      // if the reconstructed hash matches the meta data key,
      // then we may want this one
      auto a = get_accessor<T>( meta_data, version );
      if ( a )
        if (meta_data.rtti->type_info == typeid(T) && predicate(a))
          as.emplace_back( std::move(a) );
    } // for

    // if sorting is requested
    if (sorted) 
      std::sort( 
        as.begin(), as.end(), 
        [](const auto & a, const auto &b) { return a.label()<b.label(); } 
      );

    return as;
  }

  template<
    typename T,
    typename Predicate
  >
  static
  decltype(auto)
  get_accessors(
    const data_client_t & data_client,
    data_store_t & data_store,
    size_t version,
    Predicate && predicate,
    bool sorted
  )
  {
    std::vector< accessor_t<T> > as;

    // the runtime id
    auto runtime_id = data_client.runtime_id();

    // check each namespace
    for (auto & namespace_map : data_store) {

      // the namespace data
      auto & namespace_key = namespace_map.first;
      auto & namespace_data = namespace_map.second;
      
      // loop over each key pair
      for (auto & entry_pair : namespace_data) {
        // get the meta data key and label
        const auto & meta_data_key = entry_pair.first;
        auto & meta_data = entry_pair.second;
        // now build the hash for this label
        const auto & label = meta_data.label;
        auto key_hash = 
          utils::hash<utils::const_string_t::hash_type_t>(label, label.size());
        auto hash = key_hash ^ runtime_id;
        // filter out the accessors for different data_clients
        if ( meta_data_key != hash ) continue;
        // if the reconstructed hash matches the meta data key,
        // then we may want this one
        auto a = get_accessor<T>( meta_data, version );
        if ( a )
          if (meta_data.rtti->type_info == typeid(T) && predicate(a))
            as.emplace_back( std::move(a) );
      } // for each key pair
    } // for each namespace

    // if sorting is requested
    if (sorted) 
      std::sort( 
        as.begin(), as.end(), 
        [](const auto & a, const auto &b) { return a.label()<b.label(); } 
      );

    return as;
  }

  template<
    typename T,
    size_t NS
  >
  static
  decltype(auto)
  get_accessors(
    const data_client_t & data_client,
    data_store_t & data_store,
    size_t version,
    bool sorted
  )
  {
    std::vector< accessor_t<T> > as;

    // the runtime id
    auto runtime_id = data_client.runtime_id();

    // loop over each key pair
    for (auto & entry_pair : data_store[NS]) {
      // get the meta data key and label
      const auto & meta_data_key = entry_pair.first;
      auto & meta_data = entry_pair.second;
      // now build the hash for this label
      const auto & label = meta_data.label;
      auto key_hash = 
        utils::hash<utils::const_string_t::hash_type_t>(label, label.size());
      auto hash = key_hash ^ runtime_id;
      // filter out the accessors for different data_clients
      if ( meta_data_key != hash ) continue;
      // if the reconstructed hash matches the meta data key,
      // then we may want this one
      auto a = get_accessor<T>( meta_data, version );
      if ( a )
        if (meta_data.rtti->type_info == typeid(T))
          as.emplace_back( std::move(a) );
    } // for

    // if sorting is requested
    if (sorted) 
      std::sort( 
        as.begin(), as.end(), 
        [](const auto & a, const auto &b) { return a.label()<b.label(); } 
      );

    return as;
  }

  template<
    typename T
  >
  static
  decltype(auto)
  get_accessors(
    const data_client_t & data_client,
    data_store_t & data_store,
    size_t version,
    bool sorted
  )
  {
    std::vector< accessor_t<T> > as;

    // the runtime id
    auto runtime_id = data_client.runtime_id();

    // check each namespace
    for (auto & namespace_map : data_store) {

      // the namespace data
      auto & namespace_key = namespace_map.first;
      auto & namespace_data = namespace_map.second;
      
      // loop over each key pair
      for (auto & entry_pair : namespace_data) {
        // get the meta data key and label
        const auto & meta_data_key = entry_pair.first;
        auto & meta_data = entry_pair.second;
        // now build the hash for this label
        const auto & label = meta_data.label;
        auto key_hash = 
          utils::hash<utils::const_string_t::hash_type_t>(label, label.size());
        auto hash = key_hash ^ runtime_id;
        // filter out the accessors for different data_clients
        if ( meta_data_key != hash ) continue;
        // if the reconstructed hash matches the meta data key,
        // then we may want this one
        auto a = get_accessor<T>( meta_data, version );
        if ( a )
          if (meta_data.rtti->type_info == typeid(T))
            as.emplace_back( std::move(a) );
      } // for each key pair
    } // for each namespace

    // if sorting is requested
    if (sorted) 
      std::sort( 
        as.begin(), as.end(), 
        [](const auto & a, const auto &b) { return a.label()<b.label(); } 
      );

    return as;
  }

  //--------------------------------------------------------------------------//
  // Data handles.
  //--------------------------------------------------------------------------//

  ///
  //
  ///
  template<
    typename T,
    size_t NS,
    size_t EP,
    size_t SP,
    size_t GP
  >
  static
  handle_t<T, EP, SP, GP>
  get_handle(
    const data_client_t & data_client,
    data_store_t & data_store,
    const utils::const_string_t & key,
    size_t version
  )
  {
    using namespace execution;

    auto hash = key.hash()/* ^ data_client.runtime_id()*/;
    auto itr = data_store[NS].find(hash);
    assert(itr != data_store[NS].end() && "invalid key");
    auto& md = itr->second;

    auto& data = md.get_legion_data(version);

    handle_t<T, EP, SP, GP> h;
    h.lr = data.lr;
    h.exclusive_ip = data.exclusive_ip;
    h.shared_ip = data.shared_ip;
    h.ghost_ip = data.ghost_ip;
    h.exclusive_lr = data.exclusive_lr;
    h.shared_lr = data.shared_lr;
    h.ghost_lr = data.ghost_lr;

    return h;
  } // get_handle

  static void get_all_handles(const data_client_t & data_client,
                       data_store_t & data_store,
                       std::vector<data_handle_t<void, 0, 0, 0>>& handles,
                       std::vector<size_t>& hashes,
                       std::vector<size_t>& namespaces,
                       std::vector<size_t>& versions){
    
    for(auto& itr : data_store){
      for(auto& itr2 : itr.second){

        auto& md = itr2.second;

        for(auto& itr3 : md.data){
          hashes.emplace_back(itr.first);
          namespaces.emplace_back(itr2.first);
          versions.emplace_back(itr3.first);

          auto& ld = itr3.second;
          data_handle_t<void, 0, 0, 0> h;
          h.lr = ld.lr;
          h.exclusive_ip = ld.exclusive_ip;
          h.shared_ip = ld.shared_ip;
          h.ghost_ip = ld.ghost_ip;
          handles.emplace_back(std::move(h));
        }
      }
    }
  }

  static
  void
  put_all_handles(
     const data_client_t & data_client,
     data_store_t & data_store,
     size_t num_handles,
     data_handle_t<void, 0, 0, 0>* handles,
     size_t* hashes,
     size_t* namespaces,
     size_t* versions
    )
  {
    for(size_t i = 0; i < num_handles; ++i){
      auto& ld = data_store[hashes[i]][namespaces[i]].data[versions[i]];
      auto& hi = handles[i];
      ld.exclusive_ip = hi.exclusive_ip;
      ld.shared_ip = hi.shared_ip;
      ld.ghost_ip = hi.ghost_ip;
      ld.exclusive_lr = hi.exclusive_lr;
      ld.shared_lr = hi.shared_lr;
      ld.ghost_lr = hi.ghost_lr;
    }
  }


}; // struct storage_type_t

} // namespace legion
} // namespace data
} // namespace flecsi

#endif // flecsi_legion_dense_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
