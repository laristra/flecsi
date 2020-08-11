/*~--------------------------------------------------------------------------~*
 *  * Copyright (c) 2017 Los Alamos National Security, LLC
 *   * All rights reserved.
 *    *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_unit_box_h
#define flecsi_topology_unit_box_h

#include <array>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <type_traits>
#include <vector>

#include "flecsi/topo/structured/box_utils.hh"

namespace flecsi { 
namespace topo {
namespace structured_impl {

//----------------------------------------------------------------------------//
//! The box type is the (much) simplified version of unit_box type. 
//! 
//! @ingroup structured
//! 
//----------------------------------------------------------------------------//

template<std::size_t MESH_DIMENSION>
class box 
{
public: 
 using id = std::size_t; 
 using id_array = id[MESH_DIMENSION]; 
 
 /************************************************************************
 * Constructors/initializers/destructors                        
 *
 *************************************************************************/                             
  box(){
   for (std::size_t i =0; i < MESH_DIMENSION; ++i) 
     upperbnds_[i] = 0; 
  }
 
  box(std::vector<std::size_t>& upperbnds){
   assert(upperbnds.size() == MESH_DIMENSION); 
   for (std::size_t i = 0; i < MESH_DIMENSION; ++i) 
     upperbnds_[i] = upperbnds[i]; 

    size_ = 1; 
    for (std::size_t i = 0; i < MESH_DIMENSION; ++i)
     size_ *= upperbnds_[i] + 1;
  }
 
  box(id_array& upperbnds){
   for (std::size_t i = 0; i < MESH_DIMENSION; ++i) 
     upperbnds_[i] = upperbnds[i]; 

    size_ = 1; 
    for (std::size_t i = 0; i < MESH_DIMENSION; ++i)
     size_ *= upperbnds_[i] + 1;
  } 


 ~box(){} 


 // This initialize function takes as input the bounds of each box representing 
 // this entity as well as the its boundary tags. f
 void initialize(std::vector<std::size_t> &upperbnds) 
 {
   assert(upperbnds.size() == MESH_DIMENSION); 
   for (std::size_t i = 0; i < MESH_DIMENSION; ++i) 
     upperbnds_[i] = upperbnds[i]; 

    size_ = 1; 
    for (std::size_t i = 0; i < MESH_DIMENSION; ++i)
     size_ *= upperbnds_[i] + 1;
 }//initialize
  
 /*****************************************************************************
 * Basic query methods: lower/upper bounds, strides, sizes, bounds-checking   *   
 *****************************************************************************/                             

 /* Upper bounds */
 auto upper_bounds()
 {
   return upperbnds_; 
 } //upper_bounds

 template<std::size_t DIM>
 void upper_bounds()
 {
   return upperbnds_[DIM]; 
 } //upper_bounds

 /* Strides */
 template<std::size_t DIM>
 auto stride()
 {
   return upperbnds_[DIM]+1; 
 } //stride

 auto stride(std::size_t dim)
 {
   return upperbnds_[dim]+1; 
 } //stride
 
/* Bounds checking */
 template<std::size_t DIM>
 bool check_bounds_index(id index)
 {
   return (index >= 0 && index <= upperbnds_[DIM]); 
 } //check_bounds_index
 
 bool check_bounds_index(std::size_t dim, id index)
 {
   return (index >= 0 && index <= upperbnds_[dim]); 
 } //check_bounds_index

 bool check_bounds_indices(id_array indices)
 {
   bool within_bnds = true; 
   for (std::size_t i = 0; i< MESH_DIMENSION; i++) { 
    within_bnds = within_bnds && 
		  ((indices[i] >= 0) &&
                   (indices[i] <= upperbnds_[i]));
   }
  return within_bnds; 
 } //check_bounds_indices

 /* Sizes */
 auto size()
 {
   return size_; 
 } //size

 /*****************************************************************************
 * Basic query methods: indices <-> offset              		      *   
 *****************************************************************************/                             
 auto offset_from_indices(const id_array &indices) 
 {
   id value; 
   switch (MESH_DIMENSION) { 
    case 1: 
	return value = indices[0]; 
    case 2:
       return value = indices[0] + stride<0>()*indices[1]; 
    case 3:
       return value = indices[0] + stride<0>()*indices[1] + stride<0>()*stride<1>()*indices[2]; 
   default:           
       value = indices[MESH_DIMENSION-2] + stride<MESH_DIMENSION-2>()*indices[MESH_DIMENSION-1]; 
       for (std::size_t i = MESH_DIMENSION-2; i > 0 ; --i) 
         value +=  indices[i-1] + stride(i-1)*value;
       return value;  
   }
   //std::size_t factor;
   /*for (std::size_t i = 0; i < MESH_DIMENSION; ++i)
    {
      factor = 1;
      for (std::size_t j=0; j< MESH_DIMENSION-i-1; ++j)
        factor *= stride(j);
      value += indices[MESH_DIMENSION-i-1]*factor;
    }
  return value;*/ 
 } //offset_from_indices

 void indices_from_offset( const id &offset, id_array &indices)
 {
    id rem = offset; 

    for (std::size_t i=0; i< MESH_DIMENSION; ++i) {
     indices[i] = rem % stride(i); 
     rem = (rem - indices[i])/stride(i); 
    }
    /*
    //std::size_t factor, value;
    for (std::size_t i=0; i< MESH_DIMENSION; ++i)
    {
      factor = 1;
      for (std::size_t j=0; j< MESH_DIMENSION-i-1; ++j)
       factor *= stride(j);
      value = rem/factor;
      indices[MESH_DIMENSION-i-1] = value;
      rem -= value*factor;
    }*/
 }   //indices_from_offset


 /************************************************************************
 * Iterators  
 *************************************************************************/                             
 auto begin() 
 {
   return iterator(0); 
 } 

 auto end() 
 {
   return iterator(size_); 
 } 

 class iterator 
 {
  public:
   iterator(id current) : current_{current} {} 
 
   ~iterator(){}
  
   iterator& operator++()
   {
     ++current_;
     return *this;  
   }
 
   iterator& operator--()
   {
     --current_; 
     return *this;  
   } 

   bool operator!=(const iterator& rhs)
   {
     return (this->current_ != rhs.current());  
   }; 

   bool operator==(const iterator& rhs)
   {
     return (this->current_ == rhs.current());  
   }

   bool operator<(const iterator& rhs)
   {
     return (this->current_ < rhs.current());  
   };
 
   bool operator>(const iterator& rhs)
   {
     return (this->current_ > rhs.current());  
   };

   bool operator<=(const iterator& rhs)
   {
     return (this->current_ <= rhs.current());  
   }; 
 
   bool operator>=(const iterator& rhs)
   {
     return (this->current_ >= rhs.current());  
   };
 
   id& operator*()
   {
     return current_; 
   }
 
   const id& current() const 
   {
     return current_; 
   }

  private: 
   id current_; 
 };  
 
private:
 id_array upperbnds_; 
 id size_ = 0; 
}; //box

} //namespace structured_impl
} //namespace topology
} //namespaceflecsi
#endif 

