#ifndef flecsi_topology_structured_topology_h
#define flecsi_topology_structured_topology_h

#include "flecsi/util/static_verify.hh"
#include "flecsi/topo/structured/mesh_utils.hh"
#include "flecsi/topo/structured/partition.hh"
#include "flecsi/topo/structured/structured_topology_storage.hh"

namespace flecsi{ 
namespace topology{
namespace structured_impl {

namespace verify_structmesh {
  FLECSI_MEMBER_CHECKER(num_dimensions);
  FLECSI_MEMBER_CHECKER(entity_types);
} // namespace verify_structmesh

template<typename POLICY>
class structured_topology_u : public structured_topology_base_t
{

 /*
  * Verify the existence of following fields in the mesh policy MT
  * num_dimensions
  * num_domains
  * entity_types
  */
  // static verification of mesh policy

  static_assert(verify_structmesh::has_member_num_dimensions<POLICY>::value,
                "mesh policy missing num_dimensions size_t");

  static_assert(std::is_convertible<decltype(POLICY::num_dimensions),
    size_t>::value, "mesh policy num_dimensions must be size_t");

  static_assert(verify_structmesh::has_member_entity_types<POLICY>::value,
                "mesh policy missing entity_types tuple");

  static_assert(util::is_tuple<typename POLICY::entity_types>::value,
                "mesh policy entity_types is not a tuple");

public:
 // used to find the entity type of topological dimension D 
  template<size_t D>
  using entity_type = typename find_entity_st_<POLICY, D>::type;

  using id_t        = int64_t;
  using id_array_t = id_t[POLICY::num_dimensions]; 

  // storage type
  using storage_t = structured_topology_storage_u<POLICY::num_dimensions>;

  // base type for mesh
  //using base_t = structured_topology_base__<storage_t>;

 //--------------------------------------------------------------------------//
  // This type definition is needed so that data client handles can be
  // specialized for particular data client types, e.g., mesh topologies vs.
  // tree topologies. It is also useful for detecting illegal usage, such as
  // when a user adds data members.
  //--------------------------------------------------------------------------//
  using type_identifier_t = structured_topology_u;

  // Don't allow the mesh to be copied
  structured_topology_u & operator=(const structured_topology_u &)
  = delete;

  // Allow move operations
  structured_topology_u(structured_topology_u && o) = default;
  structured_topology_u & operator=(structured_topology_u && o)
  = default;

  // Copy constructor
  structured_topology_u(const structured_topology_u & m) : storage_(m.storage()) {}

  //! Constructor
  structured_topology_u(storage_t * ms = nullptr) : storage_{ms} {}

  // mesh destructor
  virtual ~structured_topology_u() {}

  // set/get storage
  void set_storage(storage_t *ptr_to_str)
  {
    storage_ = ptr_to_str;
  }  // set_storage

  auto storage()
  {
    return storage_;
  }  //storage


 /************************************************************************
 * Access entities of particular partition type
 *************************************************************************/
  template<class ENTITY>
  auto overlay()
  {
    auto s = std::get<ENTITY::dimension>(storage_->topology); 
    return s.overlay();  
  } //overlay

  template<size_t ENTITY_DIMENSION>
  auto overlay()
  {
    auto s = std::get<ENTITY_DIMENSION>(storage_->topology); 
    return s.overlay();  
  } //overlay

  template<class ENTITY>
  auto exclusive()
  {
    auto s = std::get<ENTITY::dimension>(storage_->topology); 
    return s.exclusive();  
  } //exclusive 
 
  template<size_t ENTITY_DIMENSION>
  auto exclusive()
  {
    auto s = std::get<ENTITY_DIMENSION>(storage_->topology); 
    return s.exclusive();  
  } //exclusive 

  template<class ENTITY>
  auto shared()
  {
    auto s = std::get<ENTITY::dimension>(storage_->topology); 
    return s.shared();  
  } //shared 

  template<size_t ENTITY_DIMENSION>
  auto shared()
  {
    auto s = std::get<ENTITY_DIMENSION>(storage_->topology); 
    return s.shared();  
  } //shared 

  template<class ENTITY>
  auto ghost()
  {
    auto s = std::get<ENTITY::dimension>(storage_->topology); 
    return s.ghost();  
  } //ghost

  template<size_t ENTITY_DIMENSION>
  auto ghost()
  {
    auto s = std::get<ENTITY_DIMENSION>(storage_->topology); 
    return s.ghost();  
  } //ghost

  template<class ENTITY>
  auto domain_halo()
  {
    auto s = std::get<ENTITY::dimension>(storage_->topology); 
    return s.domain_halo();  
  } //domain_halo

  template<size_t ENTITY_DIMENSION>
  auto domain_halo()
  {
    auto s = std::get<ENTITY_DIMENSION>(storage_->topology); 
    return s.domain_halo();  
  } //domain_halo

  /*template<class ENTITY, partition_t pid>
  auto entities()
  {
     switch (pid) {
       case 1: 
	 return overlay<ENTITY>(); 
       case 2: 
	 return exclusive<ENTITY>(); 
       case 3: 
	 return shared<ENTITY>(); 
       case 4: 
	 return ghost<ENTITY>(); 
       case 5: 
	 return domain_halo<ENTITY>(); 
       default: 
         break; 
    }
  } //entities 
  
  template<size_t ENTITY_DIMENSION, partition_t pid>
  auto entities()
  {
     switch (pid) {
       case 1: 
	 return overlay<ENTITY_DIMENSION>(); 
       case 2: 
	 return exclusive<ENTITY_DIMENSION>(); 
       case 3: 
	 return shared<ENTITY_DIMENSION>(); 
       case 4: 
	 return ghost<ENTITY_DIMENSION>(); 
       case 5: 
	 return domain_halo<ENTITY_DIMENSION>(); 
       default: 
         break; 
    }
  } //entities 
  */
 /*
  template<typename P>
  auto entities(size_t entity_dim) {}

  template<>
  auto entities<partition_t::overlay>(size_t entity_dim) 
  {
    return overlay<entity_dim>(); 
  } 

  template<>
  auto entities<partition_t::exclusive>(size_t entity_dim) 
  {
    return exclusive<entity_dim>(); 
  } 

  template<>
  auto entities<partition_t::shared>(size_t entity_dim) 
  {
    return shared<entity_dim>(); 
  } 
 */

private: 
  
  storage_t *storage_; 

} ;//structured_topology_u

} //structured_impl
} //topology
} //flecsi
#endif 
