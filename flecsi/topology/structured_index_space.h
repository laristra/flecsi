/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_structured_index_space_h
#define flecsi_topology_structured_index_space_h

#include <vector>
#include <cassert>
#include <algorithm>
#include <type_traits>

#include "flecsi/topology/structured_querytable.h"

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: 
//----------------------------------------------------------------------------//

namespace flecsi {
namespace topology {

//----------------------------------------------------------------------------//
//! The structured_index_space type...
//!
//! @ingroup
//! @param E  The entity type of the index space
//! @param DM num_dimensions 
//----------------------------------------------------------------------------//
template<class E, size_t DM>
class structured_index_space__{
public:
  using sm_id_t            = typename std::remove_pointer<E>::type::sm_id_t;
  using sm_id_array_t      = std::array<sm_id_t, DM>; 
  using sm_id_array_2d_t   = std::vector<std::array<sm_id_t, DM>>; 
  using sm_id_vector_t     = std::vector<sm_id_t>;
  using sm_id_vector_2d_t  = std::vector<std::vector<sm_id_t>>;
  using qtable_t           = typename query::QueryTable<DM,DM+1,DM,DM+1>;

 /******************************************************************************
 *               Constructors/Destructors/Initializations                      *    
 * ****************************************************************************/ 
 //--------------------------------------------------------------------------//
 //! Interface to initialize the index-space for entities of a given dimension
 //! in a cartesian block.  
 //!
 //! Block: Lower/Upper bounds of the cartesian block from integer space (Z^d). 
 //! In the current design, the lower bound is always the zero vector. 
 //!
 //! Index-space: An index-space is primarily an enumeration of entities of a
 //! particular dimension in a structurerd  mesh. Given a block representing the 
 //! the domain,, an index-space for a particular dimension can be represented 
 //! using either the input bounds or by creating sub-bounds from the input which
 //!  we call the dependent index-sapces.  
 //!
 //! @param primary Boolean representing if the index-space is primary. 
 //!                Currently, the primary index-space is not allowed 
 //!                multiple dependent index spaces.
 //! @param lbnds   The lower bound of the cartesian block
 //! @param ubnds   The upper bound of the cartesian block
 //! @param mubnds  The lower/upper bounds of the dependent index-space 
 //--------------------------------------------------------------------------//

  void init(bool primary,
            const sm_id_t primary_dim,  
            const sm_id_array_t &global_lbnds, 
            const sm_id_array_t &global_ubnds, 
            const sm_id_array_t &global_strides, 
            sm_id_vector_t &bnds)
  {
    // Check the array sizes for lower and upper bound are equal 
    assert(global_lbnds.size() == global_ubnds.size());

    // Check that the primary IS doesn't have multiple boxes
    if (primary) 
      assert (bnds.size()==DM);

    offset_ = 0;
    primary_ = primary;
    primary_dim_ = primary_dim; 
    num_boxes_ = bnds.size()/DM;
    sm_id_array_t global_low, global_up, global_str;
    sm_id_array_t local_low, local_up;

    for (size_t i = 0; i < num_boxes_; i++)
    {
      size_t cnt = 1;
      global_low.clear();
      global_up.clear();
      local_low.clear();
      local_up.clear();

      for (size_t j = 0; j < DM; j++)
      {
         cnt *= global_ubnds[j] + bnds[num_boxes_*i+j] - global_lbnds[j]+1;

         global_low[j] = global_lbnds[j];
         global_up[j]  = global_ubnds[j]+bnds[num_boxes_*i+j];
         global_str[j] = global_strides[j]+bnds[num_boxes_*i+j];
      
         local_low[j] = 0;
         local_up[j]  = global_ubnds[j]+bnds[num_boxes_*i+j]-global_lbnds[j];
      }

      lbox_offset_.push_back(0);
      lbox_size_.push_back(cnt);
      lbox_lowbnds_.push_back(local_low);
      lbox_upbnds_.push_back(local_up);
      gbox_size_.push_back(cnt);
      gbox_lowbnds_.push_back(global_low);
      gbox_upbnds_.push_back(global_up);
      gbox_strides_.push_back(global_str);
    }

    size_ = 0;
    for (size_t i = 0; i < num_boxes_; i++)
     size_ += lbox_size_[i];

    //debug print
    for (size_t i = 0; i < num_boxes_; i++)
    {
      std::cout<<"lBox-id = "<<i<<std::endl;
      std::cout<<" -- lBox-offset = "<<lbox_offset_[i]<<std::endl;
      std::cout<<" -- lBox-size   = "<<lbox_size_[i]<<std::endl;

      std::cout<<" ----lBox-lower-bnds = { ";
      for (size_t j = 0 ; j < DM; j++)
        std::cout<<lbox_lowbnds_[i][j]<<", ";
      std::cout<<"}"<<std::endl;
    
      std::cout<<" ----lBox-upper-bnds = { ";
      for (size_t j = 0 ; j < DM; j++)
        std::cout<<lbox_upbnds_[i][j]<<", ";
      std::cout<<"}"<<std::endl;
    }

    for (size_t i = 0; i < num_boxes_; i++)
    {
      std::cout<<"gBox-id = "<<i<<std::endl;
      std::cout<<" -- gBox-size   = "<<gbox_size_[i]<<std::endl;

      std::cout<<" ----gBox-lower-bnds = { ";
      for (size_t j = 0 ; j < DM; j++)
        std::cout<<gbox_lowbnds_[i][j]<<", ";
      std::cout<<"}"<<std::endl;
    
      std::cout<<" ----gBox-upper-bnds = { ";
      for (size_t j = 0 ; j < DM; j++)
        std::cout<<gbox_upbnds_[i][j]<<", ";
      std::cout<<"}"<<std::endl;

      std::cout<<" ----gBox-strides = { ";
      for (size_t j = 0 ; j < DM; j++)
        std::cout<<gbox_strides_[i][j]<<", ";
      std::cout<<"}"<<std::endl;
    }
   std::cout<<"size == "<<size_<<std::endl;
   std::cout<<"Primary == "<<primary<<std::endl;
   
  }
   
  //default constructor
  structured_index_space__(){};

  //default destructor
  ~structured_index_space__(){};

 /******************************************************************************
 *                              Basic Iterators                                *    
 * ****************************************************************************/ 
 //--------------------------------------------------------------------------//
 //! Abstract interface to get the entities of dimension \em to that define
 //! the entity of dimension \em from with the given identifier \em id.
 //!
 //! @param from_dimension The dimension of the entity for which the
 //!                       definition is being requested.
 //! @param to_dimension   The dimension of the entities of the definition.
 //! @param id             The id of the entity for which the definition is
 //!                       being requested.
 //--------------------------------------------------------------------------//

 template<typename S=E>
 auto iterate()
 { 
   return iterator_whole_t<S>(offset_, size_);
 }

 template< typename S = E>
 class iterator_whole_t
 {
    public:
     iterator_whole_t(sm_id_t start, sm_id_t sz):start_{start}, sz_{sz}{};
     ~iterator_whole_t(){};

      class iterator_t{
        public:
          iterator_t (sm_id_t offset):current{offset}
          {
            current_ent.set_id(current,0); //set to global id corresponding 
                                           //to current and similarly
                                           //set the right id during increment
                                           //operation. 

          };
    
          ~iterator_t(){};

          iterator_t& operator++()
          {
            ++current;
            current_ent.set_id(current,0);
            return *this;
          }

          bool operator!=(const iterator_t& rhs)
          {
           return (this->current != rhs.current);
          } 
          
          bool operator==(const iterator_t& rhs)
          {
           return (this->current == rhs.current);
          } 

          S& operator*()
          {
           return current_ent;
          }

       private:
        sm_id_t current;
        S current_ent;
     };
    
    auto begin()
    {
      return iterator_t(start_);
    };

    auto end()
    {
      return iterator_t(start_+sz_);
    }; 
 
   private:
    sm_id_t start_;
    sm_id_t sz_; 
 };
  

 /******************************************************************************
 *                         Query-Specific Iterators                            *    
 * ****************************************************************************/ 
 //--------------------------------------------------------------------------//
 //! Interface to get the entities of dimension \em FD to the entities of
 //!  dimension \em TD.
 //!
 //! @param from_dimension The dimension of the entity for which the
 //!                       definition is being requested.
 //! @param to_dimension   The dimension of the entities of the definition.
 //! @param id             The id of the entity for which the definition is
 //!                       being requested.
 //--------------------------------------------------------------------------//

  template <size_t TD, class S>
  auto traverse(size_t FD, size_t ID, sm_id_array_t &indices, qtable_t *qt)
  {
    return traversal<TD, S>(this, DM, FD, ID, indices, qt);
  }

  template<size_t TD1, class S1, class E1=E, size_t DM1 = DM>
  class traversal{
    public:
    
    //Constructor//Destructor
    traversal(
      structured_index_space__<E1, DM1> *is,
      sm_id_t MD1, 
      sm_id_t FD1, 
      sm_id_t ID1, 
      sm_id_array_t &indices,
      qtable_t *qt1):
      is_{is}, 
      MD1_{MD1}, 
      FD1_{FD1}, 
      ID1_{ID1}, 
      indices_{indices},
      qt1_{qt1}
    {
      TD1_ = TD1;
      sm_id_t nq = qt1_->entry[FD1_][ID1_][TD1_].size();
      start_ = 0;
      finish_ = nq;
    };

    ~traversal(){};
 
    //Iterator
    template<class S2 = S1, class E2 = E1, size_t DM2 = DM1>
    class iterator_t{
     public:
      iterator_t(
        structured_index_space__<E2,DM2> *is,
        qtable_t *qt2, 
        sm_id_t MD2, 
        sm_id_t FD2, 
        sm_id_t ID2, 
        sm_id_t TD2,
        sm_id_array_t &indices, 
        sm_id_t index,
        sm_id_t end_idx, 
        bool forward):
        is_{is}, 
        qt2_{qt2},
        MD2_{MD2}, 
        FD2_{FD2}, 
        ID2_{ID2},
        TD2_{TD2}, 
        indices_{indices}, 
        valid_idx_{index}, 
        end_idx_{end_idx},
        forward_{forward}
      {
        bool valid = isvalid(valid_idx_); 
        while (!valid)
        {
          if (forward) 
             valid_idx_ += 1; 
          else
             valid_idx_ -= 1;
          valid = isvalid(valid_idx_);
        }

        bool valid_end = isvalid(end_idx_ - 1);
        while (!valid_end)
        {
           end_idx_ -= 1;
           valid_end = isvalid(end_idx_ - 1);
        }
          
        sm_id_t entid = compute_id(valid_idx_);
        valid_ent_.set_id(entid,0);
      };

      ~iterator_t() { };

      iterator_t& operator++()
      {
        valid_idx_ += 1;
        if(valid_idx_ != end_idx_){
          bool valid = isvalid(valid_idx_); 
          while ((!valid) && (valid_idx_ != end_idx_))
          {
            valid_idx_ += 1;
            valid = isvalid(valid_idx_);
          }
         
          if (valid_idx_ != end_idx_)
          {
            id_t entid = compute_id(valid_idx_);
            valid_ent_.set_id(entid,0);
          }
        }
        return *this;
      };

      bool operator!=(const iterator_t& rhs)
      {
         return (this->valid_idx_ != rhs.end_idx_);
      };

      bool operator==(const iterator_t& rhs)
      {
         return (this->valid_idx_ == rhs.valid_idx_);
      };


      S2& operator*()
      {
        return valid_ent_;
      };

      bool isvalid(id_t vindex)
      {
        bool valid = true; 
        
        // Get box id 
        sm_id_t bid = qt2_->entry[FD2_][ID2_][TD2_].adjacencies[vindex].box_id;
 
        // Get number of directions to check
        sm_id_t nchk    = qt2_->entry[FD2_][ID2_][TD2_].adjacencies[vindex].numchk;
        auto dir     = qt2_->entry[FD2_][ID2_][TD2_].adjacencies[vindex].dir; 
        auto chk_bnd = qt2_->entry[FD2_][ID2_][TD2_].adjacencies[vindex].bnd_chk;
 
        for (sm_id_t i = 0; i < nchk; i++)
        {
          if (chk_bnd[i])
            valid = valid && (indices_[dir[i]] <= (is_->max(bid, dir[i])));
          else 
            valid = valid && (indices_[dir[i]] >= (is_->min(bid, dir[i]))+1);  
        }
   
        return valid;
      }

      auto compute_id(sm_id_t vindex)
      { 
        sm_id_array_t adj;
        sm_id_t bid = qt2_->entry[FD2_][ID2_][TD2_].adjacencies[vindex].box_id;
        auto offset = qt2_->entry[FD2_][ID2_][TD2_].adjacencies[vindex].offset;
        for (sm_id_t i = 0; i < MD2_; i++)
          adj[i] = indices_[i]+offset[i];     

        return is_->get_global_offset_from_indices(bid,adj);
      }

     private:
       structured_index_space__<E2,DM2> *is_;
       qtable_t *qt2_; 
       sm_id_t MD2_, FD2_, ID2_, TD2_;
       sm_id_array_t indices_;
       sm_id_t valid_idx_;
       sm_id_t end_idx_;
       S2  valid_ent_;
       bool forward_;
    };

    auto begin()
    {
      return iterator_t<S1, E1, DM1>(is_, qt1_, MD1_, FD1_, ID1_, TD1_, 
                                     indices_, start_, finish_, true); 
    };

    auto end()
    {
      return iterator_t<S1, E1, DM1>(is_, qt1_, MD1_, FD1_, ID1_, TD1_, 
                                     indices_, finish_-1, finish_, false);
    };

    private: 
       structured_index_space__<E1, DM1> *is_; 
       sm_id_t MD1_, FD1_, ID1_, TD1_;
       sm_id_array_t indices_;
       sm_id_t start_, finish_;
       qtable_t *qt1_; 
  };


/******************************************************************************
 * Methods to query various local/global, local to global and global to local *
 * details of the mesh representation.
 *
 * Local View:
 * local_offset: Unique offset in the local index space
 * local_box_offset: Offset w.r.t the local box to which the entity belongs
 * local_box_indices: Indices w.r.t the local box to which the entity belongs
 *
 * Global View:
 * global_offset: Unique offset in the global index space
 * global_box_offset: Offset w.r.t the global box to which the entity belongs
 * global_box_indices: Indices w.r.t the global box to which the entity belongs
 * *****************************************************************************/

 //--------------------------------------------------------------------------//
 //! Global to global 
 //--------------------------------------------------------------------------//

 //--------------------------------------------------------------------------//
 //! Return the global box offset given a global offset.
 //! @param global_offset Global offset of an entity
 //--------------------------------------------------------------------------//
  auto global_box_offset_from_global_offset(const sm_id_t& global_offset)
  {
    sm_id_t box_id = 0, box_offset = 0;
    global_box_offset_from_global_offset(global_offset, box_id, box_offset);
    
    return box_offset;
    
  }//global_box_offset_from_global_offset

   void global_box_offset_from_global_offset(const sm_id_t& global_offset, 
        sm_id_t &box_id, sm_id_t &box_offset)
  {
    if (num_boxes_ > 1)
      box_id = find_box_id_from_global_offset(global_offset);
    else 
      box_id = 0;  
    box_offset = global_offset;
    for (size_t i=0; i<box_id; ++i)  
       box_offset -= gbox_size_[i];
  }//global_box_offset_from_global_offset

  //--------------------------------------------------------------------------//
 //! Return the global offset given a global box id and offset w.r.t to
 //! the box.
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  auto global_offset_from_global_box_offset(const sm_id_t& global_box_id, 
       const sm_id_t& global_box_offset)
  {
    size_t value = global_box_offset;
    for (size_t i=0; i<global_box_id; ++i)
      value += gbox_size_[i];
    }

    return value;
  }//global_offset_from_global_box_offset

 
 //--------------------------------------------------------------------------//
 //! Return the global box indices given a global box id and offset w.r.t to
 //! the box.
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  auto global_box_indices_from_global_box_offset(const sm_id_t& global_box_id, 
       const sm_id_t& global_box_offset)
  {
    sm_id_array_t id;
    size_t factor, value;
    sm_id_t rem = global_box_offset;
    for (size_t i=0; i< DM; ++i)
    {
      factor = 1; 
      for (size_t j=0; j< DM-i-1; ++j)
       factor *= gbox_strides_[global_box_id][j] + 1; 
      value = rem/factor;
      id[DM-i-1] = value;
      rem -= value*factor;
    }
 
   return id;
  }//global_box_indices_from_global_box_offset

//--------------------------------------------------------------------------//
 //! Return the global box offset given a global box id and indices w.r.t to
 //! the box.
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  auto global_box_offset_from_global_box_indices(const sm_id_t& global_box_id, 
       const sm_id_array_t& global_box_indices)
  {
    size_t value =0;
    size_t factor;

    for (size_t i = 0; i < DM; ++i)
    {
      factor = 1;
      for (size_t j=0; j< DM-i-1; ++j)
        factor *= gbox_strides_[global_box_id][j]+1;
      value += global_box_indices[DM-i-1]*factor;
    }

    return value;

  }//global_box_offset_from_global_box_indices
 
//--------------------------------------------------------------------------//
 //! Return the global box indices given a global offset.
 //! @param global_offset Global offset of an entity
 //--------------------------------------------------------------------------//
   auto global_box_indices_from_global_offset(const sm_id_t& global_offset)
  {
    sm_id_t box_id = 0, offset; 
    sm_id_array_t gid;
    global_box_offset_from_global_offset(global_offset, box_id, offset);
    return global_box_indices_from_global_box_offset(box_id, offset);
  }//global_box_indices_from_global_offset

  }//global_box_indices_from_global_offset

   //--------------------------------------------------------------------------//
 //! Return the global offset given a global box id and indices w.r.t to
 //! the box.
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  auto global_offset_from_global_box_indices(const sm_id_t& global_box_id, 
       const sm_id_array_t& global_box_indices)
  {
     sm_id_t offset = global_box_offset_from_global_box_indices(global_box_id, 
      global_box_indices);
     return global_offset_from_global_box_offset(global_box_id, offset);
  }//global_offset_from_global_box_indices


 //--------------------------------------------------------------------------//
 //! Local to local
 //--------------------------------------------------------------------------//
  //--------------------------------------------------------------------------//
 //! Return the local box offset given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
 auto local_box_offset_from_local_offset(const sm_id_t& local_offset)
  {
    sm_id_t box_id = 0, box_offset = 0;
    local_box_offset_from_local_offset(local_offset, box_id, box_offset);
    
    return box_offset;
    
  }//local_box_offset_from_local_offset

  void local_box_offset_from_local_offset(const sm_id_t& local_offset, 
        sm_id_t &box_id, sm_id_t &box_offset)
  {
    if (num_boxes_ > 1)
      box_id = find_box_id_from_local_offset(local_offset);
    else 
      box_id = 0;  
    box_offset = local_offset;
    for (size_t i=0; i<box_id; ++i)  
       box_offset -= lbox_size_[i];
  }//local_box_offset_from_local_offset

  //--------------------------------------------------------------------------//
 //! Return the local offset given a local box id and offset w.r.t to
 //! the box.
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  auto local_offset_from_local_box_offset(const sm_id_t& local_box_id, 
       const sm_id_t& local_box_offset)
  {
    sm_id_t value = local_box_offset;
    for (size_t i=0; i<local_box_id; ++i)
      value += lbox_size_[i];

    return value;
  }//local_offset_from_local_box_offset

  //--------------------------------------------------------------------------//
 //! Return the local box offset given a local box id and offset w.r.t to
 //! the box.
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  auto local_box_indices_from_local_box_offset(const sm_id_t local_box_id, 
       const sm_id_t& local_box_offset)
  {
    sm_id_t rem = local_box_offset;
    sm_id_array_t id;
    size_t factor, value;

    for (size_t i=0; i< DM; ++i)
    {
      factor = 1; 
      for (size_t j=0; j< DM-i-1; ++j)
       factor *= lbox_upbnds_[local_box_id][j]-lbox_lowbnds_[local_box_id][j] + 1; 
      value = rem/factor;
      id[DM-i-1] = value;
      rem -= value*factor;
    }
 
   return id;
  }//local_box_indices_from_local_box_offset

   //--------------------------------------------------------------------------//
 //! Return the local box offset given a local box id and indices w.r.t to
 //! the box.
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  auto local_box_offset_from_local_box_indices(const sm_id_t& local_box_id, 
       const sm_id_array_t& local_box_indices)
  {
    size_t value =0;
    size_t factor;

    for (size_t i = 0; i < DM; ++i)
    {
      factor = 1;
      for (size_t j=0; j< DM-i-1; ++j)
        factor *= lbox_upbnds_[local_box_id][j]-lbox_lowbnds_[local_box_id][j]+1;
      value += local_box_indices[DM-i-1]*factor;
    }

    return value;
  }//local_box_offset_from_local_box_indices

//--------------------------------------------------------------------------//
 //! Return the local box indices given a local offset.
 //! @param local_offset local offset of an entity
 //--------------------------------------------------------------------------//
   auto local_box_indices_from_local_offset(const sm_id_t& local_offset)
  {
    sm_id_t box_id = 0, offset; 
    local_box_offset_from_local_offset(local_offset, box_id, offset);
    return local_box_indices_from_local_box_offset(box_id, offset);
  }//local_box_indices_from_global_offset

   //--------------------------------------------------------------------------//
 //! Return the local offset given a local box id and indices w.r.t to
 //! the box.
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  auto local_offset_from_local_box_indices(const sm_id_t& local_box_id, 
       const sm_id_array_t& local_box_indices)
  {
     sm_id_t offset = local_box_offset_from_local_box_indices(local_box_id, 
      local_box_indices);
     return local_offset_from_local_box_offset(local_box_id, offset);
  }//local_offset_from_local_box_indices

 //--------------------------------------------------------------------------//
 //! Global to local and local to global 
 //--------------------------------------------------------------------------//
//--------------------------------------------------------------------------//
 //! Return the global box indices given a local box id and indices w.r.t to
 //! the box.
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  auto global_box_indices_from_local_box_indices(const sm_id_t& local_box_id, 
       const sm_id_array_t& local_box_indices)
  {
    sm_id_array_t id; 
    for (size_t i=0; i <DM; ++i)
      id[i] = local_box_indices[i] + gbox_lowbnds_[local_box_id][i];
   return id; 

  } //global_box_indices_from_local_box_indices

//--------------------------------------------------------------------------//
 //! Return the local box indices given a global box id and indices w.r.t to
 //! the box.
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  auto local_box_indices_from_global_box_indices(const sm_id_t& global_box_id, 
       const sm_id_array_t& global_box_indices)
  {
    sm_id_array_t id; 
    for (size_t i=0; i <DM; ++i)
      id[i] = global_box_indices[i] - gbox_lowbnds_[global_box_id][i];
   return id; 
  }//local_box_indices_from_global_box_indices


//--------------------------------------------------------------------------//
 //! Return the global box offset given a local box id and offset w.r.t to
 //! the box.
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  auto global_box_offset_from_local_box_offset(const sm_id_t& local_box_id, 
       const sm_id_t& local_box_offset)
  {
    sm_id_array_t id;
    id = global_box_indices_from_local_box_offset(local_box_id, local_box_offset);
    return global_box_offset_from_global_box_indices(local_box_id, id);
  }//global_box_offset_from_local_box_offset


//--------------------------------------------------------------------------//
 //! Return the local box offset given a global box id and offset w.r.t to
 //! the box.
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  auto local_box_offset_from_global_box_offset(const sm_id_t& global_box_id, 
       const sm_id_t& global_box_offset)
  {
    sm_id_array_t id; 
    id = local_box_indices_from_global_box_offset(global_box_id, global_box_offset);
    return local_box_offset_from_local_box_indices(global_box_id, id); 
  }//local_box_offset_from_global_box_offset


//--------------------------------------------------------------------------//
 //! Return the global offset given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  auto global_offset_from_local_offset(const sm_id_t& local_offset)
  {
    sm_id_t box_id; 
    sm_id_array_t id;
    local_box_indices_from_local_offset(local_offset, box_id, id);  
    return global_offset_from_local_box_indices(box_id, id); 
     
  }//global_offset_from_local_offset


 //--------------------------------------------------------------------------//
 //! From global_offset to local_offset, local_box_offset, local_box_indices,
 //! global_box_offset, global_box_indices
 //--------------------------------------------------------------------------//

 //--------------------------------------------------------------------------//
 //! Return the local offset given a global offset.
 //! @param global_offset Global offset of an entity
 //--------------------------------------------------------------------------//
  auto local_offset_from_global_offset(const sm_id_t& global_offset)
  {
    sm_id_t box_id;
    sm_id_array_t id;  
    global_box_indices_from_global_offset(global_offset, box_id, id); 
    return local_offset_from_global_box_indices(box_id, id);
  }//local_offset_from_global_offset

//--------------------------------------------------------------------------//
 //! Return the global box indices given a local box id and offset w.r.t to
 //! the box.
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  auto global_box_indices_from_local_box_offset(const sm_id_t& local_box_id, 
       const sm_id_t& local_box_offset)
  {
    sm_id_array_t id; 
    id = local_box_indices_from_local_box_offset(local_box_id, local_box_offset);
    return global_box_indices_from_local_box_indices(local_box_id, id); 
  }//global_box_indices_from_local_box_offset

 //--------------------------------------------------------------------------//
 //! Return the local box offset given a global box id and indices w.r.t to
 //! the box.
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  auto local_box_offset_from_global_box_indices(const sm_id_t& global_box_id, 
       const sm_id_array_t& global_box_indices)
  {
    sm_id_array_t id;
    id = local_box_indices_from_global_box_indices(global_box_id, global_box_indices);
    return local_box_offset_from_local_box_indices(global_box_id, id); 
  }//local_box_offset_from_global_box_indices


//--------------------------------------------------------------------------//
 //! Return the global box offset given a local box id and indices w.r.t to
 //! the box.
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  auto global_box_offset_from_local_box_indices(const sm_id_t& local_box_id, 
       const sm_id_array_t& local_box_indices)
  {
    sm_id_array_t id; 
    id = global_box_indices_from_local_box_indices(local_box_id, local_box_indices);
    return global_box_offset_from_global_box_indices(local_box_id, id); 
  }//global_box_offset_from_local_box_indices

//--------------------------------------------------------------------------//
 //! Return the local box indices given a global box id and offset w.r.t to
 //! the box.
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  auto local_box_indices_from_global_box_offset(const sm_id_t& global_box_id, 
       const sm_id_t& global_box_offset)
  {
   sm_id_array_t id; 
   id = global_box_indices_from_global_box_offset(global_box_id, global_box_offset); 
   return local_box_indices_from_global_box_indices(global_box_id, id); 
  }//local_box_indices_from_global_box_offset


 //--------------------------------------------------------------------------//
 //! Return the global box offset given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  auto global_box_offset_from_local_offset(const sm_id_t& local_offset)
  {
     sm_id_t box_id; 
     sm_id_array_t id;
     local_box_indices_from_local_offset(local_offset, box_id, id);  
     return global_box_offset_from_local_box_indices(box_id, id); 

  }//global_box_offset_from_local_offset  

//--------------------------------------------------------------------------//
 //! Return the local offset given a global box id and offset w.r.t to
 //! the box.
 //! @param global_box_id     id of the global box
 //! @param global_box_offset offset w.r.t the global box
 //--------------------------------------------------------------------------//
  auto local_offset_from_global_box_offset(const sm_id_t& global_box_id, 
       const sm_id_t& global_box_offset)
  {
    sm_id_array_t id; 
    id = local_box_indices_from_global_box_offset(global_box_id, global_box_offset);
    return local_offset_from_local_box_indices(global_box_id, id); 
    
  }//local_offset_from_global_box_offset


  //--------------------------------------------------------------------------//
 //! Return the global offset given a local box id and offset w.r.t to
 //! the box.
 //! @param local_box_id      id of the local box
 //! @param local_box_offset offset w.r.t the local box
 //--------------------------------------------------------------------------//
  auto global_offset_from_local_box_offset(const sm_id_t& local_box_id, 
       const sm_id_t& local_box_offset)
  {
    sm_id_array_t id;
    id = global_box_indices_from_local_box_offset(local_box_id, local_box_offset);
    return global_offset_from_global_box_indices(local_box_id, id);

  }//global_offset_from_local_box_offset

 //--------------------------------------------------------------------------//
 //! Return the local box offset given a global offset.
 //! @param global_offset Global offset of an entity
 //--------------------------------------------------------------------------//
  auto local_box_offset_from_global_offset(const sm_id_t& global_offset)
  {
    sm_id_t box_id; 
    sm_id_array_t id;
    global_box_indices_from_global_offset(global_offset, box_id, id); 
    return local_box_offset_from_global_box_indices(box_id, id); 
  }//local_box_offset_from_global_offset


//--------------------------------------------------------------------------//
 //! Return the global box indices given a local offset
 //! @param local_offset  local offset of the entity
 //--------------------------------------------------------------------------//
  auto global_box_indices_from_local_offset(const sm_id_t& local_offset)
  {
     sm_id_t box_id; 
     sm_id_array_t id;
     local_box_indices_from_local_offset(local_offset, box_id, id);  
     return global_box_indices_from_local_box_indices(box_id, id); 
  }//global_box_indices_from_local_offset

  //! From local_box_offset to local_offset, local_box_indices, global_offset,
  //! global_box_offset, global_box_indices
  //

 //--------------------------------------------------------------------------//
 //! Return the local offset given a global box id and indices w.r.t to
 //! the box.
 //! @param global_box_id      id of the global box
 //! @param global_box_indices indices w.r.t the global box
 //--------------------------------------------------------------------------//
  auto local_offset_from_global_box_indices(const sm_id_t& global_box_id, 
       const sm_id_array_t& global_box_indices)
  {
    sm_id_array_t id;
    id = local_box_indices_from_global_box_indices(global_box_id, global_box_indices);
    return local_offset_from_local_box_indices(global_box_id, id); 
  }//local_offset_from_global_box_indices

 //--------------------------------------------------------------------------//
 //! Return the global offset given a local box id and indices w.r.t to
 //! the box.
 //! @param local_box_id      id of the local box
 //! @param local_box_indices indices w.r.t the local box
 //--------------------------------------------------------------------------//
  auto global_offset_from_local_box_indices(const sm_id_t& local_box_id, 
       const sm_id_array_t& local_box_indices)
  {
    sm_id_array_t id; 
    id = global_box_indices_from_local_box_indices(local_box_id, local_box_indices);
    return global_offset_from_global_box_indices(local_box_id, id); 
  }//global_offset_from_local_box_indices

//--------------------------------------------------------------------------//
 //! Return the local box indices given a global offset.
 //! @param global_offset Global offset of an entity
 //--------------------------------------------------------------------------//
  auto local_box_indices_from_global_offset(const sm_id_t& global_offset)
  {
    sm_id_t box_id; 
    sm_id_array_t id;
    global_box_indices_from_global_offset(global_offset, box_id, id); 
    return local_box_indices_from_global_box_indices(box_id, id); 
  }//local_box_indices_from_global_offset


 //--------------------------------------------------------------------------//
 //! Return the id of the global box which the query entity, of a specified
 //! topological dimension and domain, is part of.
 //!
 //! @param global_offset   global offset of the entity
 //--------------------------------------------------------------------------//
  auto find_box_id_from_global_offset(sm_id_t global_offset)
  {
    size_t bid = 0, low = 0, up = gbox_size_[0];
    for (size_t i = 0; i < num_boxes_; i++)
    {
      if (offset >= low && offset < up)
      {
        bid = i;
        break;
      }

      low = up;
      up  = up + gbox_size_[i+1];
    }
    return bid;

  } //find_box_id_from_global_offset
 
 //--------------------------------------------------------------------------//
 //! Return the id of the local box which the query entity, of a specified
 //! topological dimension and domain, is part of.
 //!
 //! @param local_offset   local offset of the entity
 //--------------------------------------------------------------------------//
  template<
    size_t D,
    size_t M = 0
  >
  auto find_box_id_from_local_offset(sm_id_t local_offset)
  {
    size_t bid = 0, low = 0, up = lbox_size_[0];
    for (size_t i = 0; i < num_boxes_; i++)
    {
      if (offset >= low && offset < up)
      {
        bid = i;
        break;
      }

      low = up;
      up  = up + lbox_size_[i+1];
    }
    return bid;
  
  } //get_box_id_from_local_offset

//--------------------------------------------------------------------------//
 //! Return the size along a direction of box B
 //--------------------------------------------------------------------------//
  template<size_t B, size_t D>
  auto get_size_in_direction()
  {
    assert(D>=0 && D < DM);
    return (lbox_upbnds_[B][D] - lbox_lowbnds_[B][D]+1);
  }

  template<size_t D>
  auto get_size_in_direction(size_t B)
  {
    assert(D>=0 && D < DM);
    return (lbox_upbnds_[B][D] - lbox_lowbnds_[B][D]+1);
  }

//--------------------------------------------------------------------------//
 //! Check if input index is between bounds along direction D of box B
 //!
 //--------------------------------------------------------------------------// 
  template<size_t B, size_t D>
  bool check_index_limits(size_t index)
  {
    return (index >= lbox_lowbnds_[B][D] && index <= lbox_upbnds_[B][D]);
  }

  template<size_t D>
  bool check_index_limits(size_t B, size_t index)
  {
    return (index >= lbox_lowbnds_[B][D] && index <= lbox_upbnds_[B][D]);
  }

//--------------------------------------------------------------------------//
 //! Return upper bound along direction D of box B
 //!
 //--------------------------------------------------------------------------// 
  template<size_t B, size_t D>
  auto lmax()
  {
    return lbox_upbnds_[B][D];
  }
  
  auto lmax(size_t B, size_t D)
  {
    return lbox_upbnds_[B][D];
  }
  
  template<size_t B, size_t D>
  auto gmax()
  {
    return gbox_upbnds_[B][D];
  }
  
  auto gmax(size_t B, size_t D)
  {
    return gbox_upbnds_[B][D];
  }

 //--------------------------------------------------------------------------//
 //! Return lower bound along direction D of box B
 //!
 //--------------------------------------------------------------------------// 
  template<size_t B, size_t D>
  auto lmin()
  {
    return lbox_lowbnds_[B][D];
  }

  auto lmin(size_t B, size_t D)
  {
    return lbox_lowbnds_[B][D];
  }

  template<size_t B, size_t D>
  auto gmin()
  {
    return gbox_lowbnds_[B][D];
  }

  auto gmin(size_t B, size_t D)
  {
    return gbox_lowbnds_[B][D];
  }


//--------------------------------------------------------------------------//
 //! Return total number of entities in the IS
 //!
 //--------------------------------------------------------------------------// 
  size_t size() const 
  {
    return size_;
  };
 
 private:
   bool primary_;                      // primary_ is set to true only for the IS of 
                                       // primary_dim_ dimensional entity. The primary IS
                                       // can contain only one box.

   bool primary_dim_;                  
 
  //! Local representation                                      
   sm_id_t offset_;                    // starting offset for the entire IS
   sm_id_t size_;                      // size of the entire IS

   sm_id_t num_boxes_;                 // number of boxes in this IS
   sm_id_vector_t lbox_size_;          // total number of entities in each local box
   sm_id_vector_t lbox_offset_;        // starting offset of each local box
   sm_id_array_2d_t lbox_lowbnds_;     // lower bounds of each local box
   sm_id_array_2d_t lbox_upbnds_;      // upper bounds of each local box

  //! Global representation
   sm_id_vector_t gbox_size_;          // total number of entities in each global box
   sm_id_array_2d_t gbox_lowbnds_;     // lower bounds of each global box 
   sm_id_array_2d_t gbox_upbnds_;      // upper bounds of each global box
   sm_id_array_2d_t gbox_strides_;     // strides along each direction of the global box

};
} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_structured_index_space_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
