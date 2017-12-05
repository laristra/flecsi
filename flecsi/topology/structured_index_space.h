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
//----------------------------------------------------------------------------//
template<class E, size_t DM=0>
class structured_index_space{
public:
  using sm_id_t        = typename std::remove_pointer<E>::type::sm_id_t;
  using sm_id_vector_t = typename std::conditional<DM==0, 
                               std::vector<size_t>, 
                               std::array<size_t,DM>>::type;
  using sm_id_array_t  = std::vector<std::vector<sm_id_t>>;
  using qtable_t    = typename query::QueryTable<DM,DM+1,DM,DM+1>;

 /******************************************************************************
 *               Constructors/Destructors/Initializations                      *    
 * ****************************************************************************/ 
 //--------------------------------------------------------------------------//
 //! Interface to initialize the index-space for given lower and upper bounds
 //! of the cartesian block.  
 //!
 //! @param primary Boolean representing if the index-space is primary. 
 //!                Currently, the primary index-space is not allowed 
 //!                multiple sub-blocks
 //! @param lbnds   The lower-bound 
 //! @param ubnds   The upper-bound
 //! @param mubnds  The 
 //--------------------------------------------------------------------------//

  void init(bool primary, const sm_id_vector_t &lbnds, const sm_id_vector_t &ubnds, 
            sm_id_array_t &mubnds)
  {
    assert(lbnds.size() == ubnds.size());
    // this check is to ensure that the primary IS doesn't have 
    // multiple boxes
    if (primary) 
      assert (mubnds.size()==1);

    offset_ = 0;
    primary_ = primary;
    num_boxes_ = mubnds.size();
    std::vector<size_t> ubnds_new, lbnds_new;

    for (size_t i = 0; i < num_boxes_; i++)
    {
      size_t cnt = 1;
      ubnds_new.clear();
      lbnds_new.clear();
      for (size_t j = 0; j < DM; j++)
      {
         cnt *= ubnds[j]+mubnds[i][j]-lbnds[j]+1;
         ubnds_new.push_back(ubnds[j]+mubnds[i][j]);
         lbnds_new.push_back(lbnds[j]);
      }

      box_offset_[i] = 0;
      box_size_[i] = cnt;
      box_lowbnds_.push_back(lbnds_new);
      box_upbnds_.push_back(ubnds_new);
    }

    size_ = 0;
    for (size_t i = 0; i < num_boxes_; i++)
     size_ += box_size_[i];

    //debug print
    /*for (size_t i = 0; i < num_boxes_; i++)
    {
      std::cout<<"Box-id = "<<i<<std::endl;
      std::cout<<" -- Box-offset = "<<box_offset_[i]<<std::endl;
      std::cout<<" -- Box-size   = "<<box_size_[i]<<std::endl;

      std::cout<<" ----Box-lower-bnds = { ";
      for (size_t j = 0 ; j < DM; j++)
        std::cout<<box_lowbnds_[i][j]<<", ";
      std::cout<<"}"<<std::endl;
    
      std::cout<<" ----Box-upper-bnds = { ";
      for (size_t j = 0 ; j < DM; j++)
        std::cout<<box_upbnds_[i][j]<<", ";
      std::cout<<"}"<<std::endl;
    }
   std::cout<<"Primary == "<<primary<<std::endl;
   */
  }
   
  //default constructor
  structured_index_space(){};

  //default destructor
  ~structured_index_space(){};

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
            current_ent = new S;
            current_ent->set_id(current,0);
          };
    
          ~iterator_t()
           {
             delete current_ent;
           };

          iterator_t& operator++()
          {
            ++current;
            current_ent->set_id(current,0);
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
           return *current_ent;
          }

       private:
        sm_id_t current;
        S* current_ent;
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
 //! Abstract interface to get the entities of dimension \em to that define
 //! the entity of dimension \em from with the given identifier \em id.
 //!
 //! @param from_dimension The dimension of the entity for which the
 //!                       definition is being requested.
 //! @param to_dimension   The dimension of the entities of the definition.
 //! @param id             The id of the entity for which the definition is
 //!                       being requested.
 //--------------------------------------------------------------------------//

  template <size_t TD, class S>
  auto traverse(size_t FD, size_t ID, sm_id_vector_t &indices, qtable_t *qt)
  {
    return traversal<TD, S>(this, DM, FD, ID, indices, qt);
  }

  template<size_t TD1, class S1, class E1=E, size_t DM1 = DM>
  class traversal{
    public:
    
    //Constructor//Destructor
    traversal(
      structured_index_space<E1, DM1> *is,
      sm_id_t MD1, 
      sm_id_t FD1, 
      sm_id_t ID1, 
      sm_id_vector_t &indices,
      qtable_t *qt1):
      is_{is}, 
      MD1_{MD1}, 
      FD1_{FD1}, 
      ID1_{ID1}, 
      indices_{indices},
      qt1_{qt1}
    {
      TD1_ = TD1;
      //auto qt = query::qtable(MD1_);
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
        structured_index_space<E2,DM2> *is,
        qtable_t *qt2, 
        sm_id_t MD2, 
        sm_id_t FD2, 
        sm_id_t ID2, 
        sm_id_t TD2,
        sm_id_vector_t &indices, 
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
        valid_ent_ = new S2; 
        valid_ent_->set_id(entid,0);
      };

      ~iterator_t()
       {
         delete valid_ent_;
       };

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
            valid_ent_->set_id(entid,0);
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
        return *valid_ent_;
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
        sm_id_vector_t adj;
        sm_id_t bid = qt2_->entry[FD2_][ID2_][TD2_].adjacencies[vindex].box_id;
        auto offset = qt2_->entry[FD2_][ID2_][TD2_].adjacencies[vindex].offset;
        for (sm_id_t i = 0; i < MD2_; i++)
          adj[i] = indices_[i]+offset[i];     

        return is_->get_global_offset_from_indices(bid,adj);
      }

     private:
       structured_index_space<E2,DM2> *is_;
       qtable_t *qt2_; 
       sm_id_t MD2_, FD2_, ID2_, TD2_;
       sm_id_vector_t indices_;
       sm_id_t valid_idx_;
       sm_id_t end_idx_;
       S2*  valid_ent_;
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
       structured_index_space<E1, DM1> *is_; 
       sm_id_t MD1_, FD1_, ID1_, TD1_;
       sm_id_vector_t indices_;
       sm_id_t start_, finish_;
       qtable_t *qt1_; 
  };

 /******************************************************************************
 *          Offset --> Indices & Indices --> Offset Routines                   *    
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
   // Return the global offset id w.r.t the box id B  
  template<size_t B>
  sm_id_t get_global_offset_from_indices(sm_id_vector_t &idv)
  {
    sm_id_t lval = get_local_offset_from_indices<B>(idv);
    
    for (sm_id_t i = 1; i < B; i++)
      lval += box_size_[i-1];
    return lval;
  }
  
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
  sm_id_t get_global_offset_from_indices(sm_id_t B, sm_id_vector_t &idv)
  {
    sm_id_t lval = get_local_offset_from_indices(B, idv);
    
    for (sm_id_t i = 1; i <= B; i++)
      lval += box_size_[i-1];
    return lval;
  }

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
  // Return the local offset id w.r.t the box id B
  template<size_t B>
  sm_id_t get_local_offset_from_indices(sm_id_vector_t &idv) 
  {
    //add range check for idv to make sure it lies within the bounds
    size_t value =0;
    size_t factor;

    for (size_t i = 0; i < DM; ++i)
    {
      factor = 1;
      for (size_t j=0; j< DM-i-1; ++j)
      {
        factor *= box_upbnds_[B][j]-box_lowbnds_[B][j]+1;
      }
      value += idv[DM-i-1]*factor;
    }

    return value;
  }
  
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
  sm_id_t get_local_offset_from_indices(size_t B, sm_id_vector_t &idv) 
  {
    //add range check for idv to make sure it lies within the bounds
    size_t value =0;
    size_t factor;

    for (size_t i = 0; i < DM; ++i)
    {
      factor = 1;
      for (size_t j=0; j < DM-i-1; ++j)
      {
        factor *= box_upbnds_[B][j]-box_lowbnds_[B][j]+1;
      }
      value += idv[DM-i-1]*factor;
    }

    return value;
  }

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
  auto get_indices_from_offset(sm_id_t offset)
  {
    //Find the box from which the indices have to be computed
    auto box_id = 0;
    if (num_boxes_ > 0)
       box_id = find_box_id(offset);
 
    size_t rem = offset;
    for (size_t i=0; i<box_id; ++i)
      rem -= box_size_[i];

    sm_id_vector_t id;
    size_t factor, value;

    for (size_t i=0; i< DM; ++i)
    {
      factor = 1; 
      for (size_t j=0; j< DM-i-1; ++j)
      {
       factor *= box_upbnds_[box_id][j]-box_lowbnds_[box_id][j] + 1; 
      }
      value = rem/factor;
      id[DM-i-1] = value;
      rem -= value*factor;
    }
 
   return id;
  }

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
  auto find_box_id(sm_id_t offset)
  {
    //assert(num_boxes>0);
    size_t bid = 0, low = 0, up = box_size_[0];
    for (size_t i = 0; i < num_boxes_; i++)
    {
      if (offset >= low && offset < up)
      {
        bid = i;
        break;
      }

      low = up;
      up  = up + box_size_[i+1];
    }
    return bid;
  }

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
  // Return size along direction D of box B in IS.
  template<size_t B, size_t D>
  auto get_size_in_direction()
  {
    assert(D>=0 && D < DM);
    return (box_upbnds_[B][D] - box_lowbnds_[B][D]+1);
  }

  template<size_t D>
  auto get_size_in_direction(size_t B)
  {
    assert(D>=0 && D < DM);
    return (box_upbnds_[B][D] - box_lowbnds_[B][D]+1);
  }

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
  // Check if input index is between bounds along direction
  // D of box B. 
  template<size_t B, size_t D>
  bool check_index_limits(size_t index)
  {
    return (index >= box_lowbnds_[B][D] && index <= box_upbnds_[B][D]);
  }

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
  template<size_t D>
  bool check_index_limits(size_t B, size_t index)
  {
    return (index >= box_lowbnds_[B][D] && index <= box_upbnds_[B][D]);
  }

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
  // Return upper bound along direction D of box B
  template<size_t B, size_t D>
  auto max()
  {
    auto val = box_upbnds_[B][D];
    return val;
  }
  
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
  auto max(size_t B, size_t D)
  {
    auto val = box_upbnds_[B][D];
    return val;
  }


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
  // Return lower bound along direction D of box B
  template<size_t B, size_t D>
  auto min()
  {
    auto val = box_lowbnds_[B][D];
    return val;
  }

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
  auto min(size_t B, size_t D)
  {
    auto val = box_lowbnds_[B][D];
    return val;
  }
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
  size_t size() const 
  {
    return size_;
  };
 
 private:
   bool primary_;                  // primary_ is set to true only for the IS of 
                                   // highest-dimensional entity 
   sm_id_t offset_;                // starting offset for the entire IS
   sm_id_t size_;                  // size of the entire IS

   sm_id_t        num_boxes_;      // number of boxes in this IS
   sm_id_vector_t box_size_;       // total number of entities in each box
   sm_id_vector_t box_offset_;     // starting offset of each box
   sm_id_array_t  box_lowbnds_;    // lower bounds of each box
   sm_id_array_t  box_upbnds_;     // upper bounds of each box

};
} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_structured_index_space_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
