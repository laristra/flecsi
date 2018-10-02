/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/
#pragma once

#include <array>
#include <vector>
#include<iostream>
#include <type_traits>

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation:
//----------------------------------------------------------------------------//

namespace flecsi {
namespace topology{
namespace query{

//----------------------------------------------------------------------------//
//! The structured_index_space type...
//!
//! @ingroup
//----------------------------------------------------------------------------//

template<size_t MESH_DIMENSION>
struct QueryUnit
{
  size_t box_id;
  size_t numchk;
  std::array<size_t, MESH_DIMENSION> dir;
  std::array<size_t, MESH_DIMENSION> bnd_chk;
  std::array<std::intmax_t, MESH_DIMENSION> offset;
};

//----------------------------------------------------------------------------//
//! The structured_index_space type...
//!
//! @ingroup
//----------------------------------------------------------------------------//

template<size_t MESH_DIMENSION>
struct QuerySequence
{
  std::vector<QueryUnit<MESH_DIMENSION>> adjacencies;
  size_t size(){return adjacencies.size();};
};

//----------------------------------------------------------------------------//
//! The structured_index_space type...
//!
//! @ingroup
//----------------------------------------------------------------------------//

template<size_t MESH_DIMENSION, size_t MAXFD, size_t MAXIN, size_t MAXTD>
struct QueryTable
{
   // QueryTable(){};
}; 

template<>
struct QueryTable<1,2,1,2>
{
  void create_table(){
    size_t FD = 2, IN = 1, TD = 2;

    for (size_t i=0; i<FD; i++)
    for (size_t j=0; i<IN; i++)
    for (size_t k=0; i<TD; i++)
      this->entry[i][j][k] = QuerySequence<1>();

    //V-->E
    this->entry[0][0][1].adjacencies.push_back({0,1,{0},{1},{0}});
    this->entry[0][0][1].adjacencies.push_back({0,1,{0},{0},{-1}});

    //E-->V
    this->entry[1][0][0].adjacencies.push_back({0,0,{},{},{0}});
    this->entry[1][0][0].adjacencies.push_back({0,0,{},{},{1}});
  }

  QuerySequence<1> entry[2][1][2];
};

//----------------------------------------------------------------------------//
//! The structured_index_space type...
//!
//! @ingroup
//----------------------------------------------------------------------------//
//template<>
template<>
struct QueryTable<2,3,2,3> 
{
  void create_table(){
    size_t FD = 3, IN = 2, TD = 3;

    for (size_t i=0; i<FD; i++)
    for (size_t j=0; i<IN; i++)
    for (size_t k=0; i<TD; i++)
      this->entry[i][j][k] = QuerySequence<2>();

    //V-->E
    this->entry[0][0][1].adjacencies.push_back({1,1,{0,0},{1,0},{0,0}});
    this->entry[0][0][1].adjacencies.push_back({0,1,{1,0},{1,0},{0,0}});
    this->entry[0][0][1].adjacencies.push_back({1,1,{0,0},{0,0},{-1,0}});
    this->entry[0][0][1].adjacencies.push_back({0,1,{1,0},{0,0},{0,-1}});

    //V-->F
    this->entry[0][0][2].adjacencies.push_back({0,2,{0,1},{1,1},{0,0}});
    this->entry[0][0][2].adjacencies.push_back({0,2,{0,1},{0,1},{-1,0}});
    this->entry[0][0][2].adjacencies.push_back({0,2,{0,1},{0,0},{-1,-1}});
    this->entry[0][0][2].adjacencies.push_back({0,2,{0,1},{1,0},{0,-1}});

    //E-->V
    this->entry[1][0][0].adjacencies.push_back({0,0,{},{},{0,0}});
    this->entry[1][0][0].adjacencies.push_back({0,0,{},{},{0,1}});
        
    this->entry[1][1][0].adjacencies.push_back({0,0,{},{},{0,0}});
    this->entry[1][1][0].adjacencies.push_back({0,0,{},{},{1,0}});

    //E-->F
    this->entry[1][0][2].adjacencies.push_back({0,1,{0,0},{1,0},{0,0}});
    this->entry[1][0][2].adjacencies.push_back({0,1,{0,0},{0,0},{-1,0}});
        
    this->entry[1][1][2].adjacencies.push_back({0,1,{1,0},{1,0},{0,0}});
    this->entry[1][1][2].adjacencies.push_back({0,1,{1,0},{0,0},{0,-1}});

    //F-->V
    this->entry[2][0][0].adjacencies.push_back({0,0,{},{},{0,0}});
    this->entry[2][0][0].adjacencies.push_back({0,0,{},{},{1,0}});
    this->entry[2][0][0].adjacencies.push_back({0,0,{},{},{1,1}});
    this->entry[2][0][0].adjacencies.push_back({0,0,{},{},{0,1}});

    //F-->E
    this->entry[2][0][1].adjacencies.push_back({1,0,{},{},{0,0}});
    this->entry[2][0][1].adjacencies.push_back({0,0,{},{},{1,0}});
    this->entry[2][0][1].adjacencies.push_back({1,0,{},{},{0,1}});
    this->entry[2][0][1].adjacencies.push_back({0,0,{},{},{0,0}});
  }

  QuerySequence<2> entry[3][2][3];
};

//----------------------------------------------------------------------------//
//! The structured_index_space type...
//!
//! @ingroup
//----------------------------------------------------------------------------//
template<>
struct QueryTable<3,4,3,4>
{
  void create_table(){
    size_t FD = 4, IN = 3, TD = 4;

    for (size_t i=0; i<FD; i++)
    for (size_t j=0; i<IN; i++)
    for (size_t k=0; i<TD; i++)
      this->entry[i][j][k] = QuerySequence<3>();

    //V-->E
    this->entry[0][0][1].adjacencies.push_back({0,1,{1,0,0},{1,0,0},{0,0,0}});
    this->entry[0][0][1].adjacencies.push_back({0,1,{1,0,0},{0,0,0},{0,-1,0}});
    this->entry[0][0][1].adjacencies.push_back({1,1,{0,0,0},{1,0,0},{0,0,0}});
    this->entry[0][0][1].adjacencies.push_back({1,1,{0,0,0},{0,0,0},{-1,0,0}});
    this->entry[0][0][1].adjacencies.push_back({2,1,{2,0,0},{1,0,0},{0,0,0}});
    this->entry[0][0][1].adjacencies.push_back({2,1,{2,0,0},{0,0,0},{0,0,-1}});

    //V-->F
    this->entry[0][0][2].adjacencies.push_back({0,2,{1,2,0},{1,1,0},{0,0,0}});
    this->entry[0][0][2].adjacencies.push_back({0,2,{1,2,0},{0,1,0},{0,-1,0}});
    this->entry[0][0][2].adjacencies.push_back({0,2,{1,2,0},{0,0,0},{0,-1,-1}});
    this->entry[0][0][2].adjacencies.push_back({0,2,{1,2,0},{1,0,0},{0,0,-1}});
    this->entry[0][0][2].adjacencies.push_back({1,2,{0,2,0},{1,1,0},{0,0,0}});
    this->entry[0][0][2].adjacencies.push_back({1,2,{0,2,0},{0,1,0},{-1,0,0}});
    this->entry[0][0][2].adjacencies.push_back({1,2,{0,2,0},{0,0,0},{-1,0,-1}});
    this->entry[0][0][2].adjacencies.push_back({1,2,{0,2,0},{1,0,0},{0,0,-1}});
    this->entry[0][0][2].adjacencies.push_back({2,2,{0,1,0},{1,1,0},{0,0,0}});
    this->entry[0][0][2].adjacencies.push_back({2,2,{0,1,0},{0,1,0},{-1,0,0}});
    this->entry[0][0][2].adjacencies.push_back({2,2,{0,1,0},{0,0,0},{-1,-1,0}});
    this->entry[0][0][2].adjacencies.push_back({2,2,{0,1,0},{1,0,0},{0,-1,0}});

    //V-->C
    this->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
    this->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{0,1,1},{-1,0,0}});
    this->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{0,0,1},{-1,-1,0}});
    this->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{1,0,1},{0,-1,0}});
    this->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{1,1,0},{0,0,-1}});
    this->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{0,1,0},{-1,0,-1}});
    this->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{0,0,0},{-1,-1,-1}});
    this->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{1,0,0},{0,-1,-1}});


    //E-->V
    this->entry[1][0][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
    this->entry[1][0][0].adjacencies.push_back({0,0,{},{},{0,1,0}});

    this->entry[1][1][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
    this->entry[1][1][0].adjacencies.push_back({0,0,{},{},{1,0,0}});

    this->entry[1][2][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
    this->entry[1][2][0].adjacencies.push_back({0,0,{},{},{0,0,1}});   

    //E-->F
    this->entry[1][0][2].adjacencies.push_back({2,2,{0,2,0},{1,1,0},{0,0,0}});
    this->entry[1][0][2].adjacencies.push_back({0,2,{0,2,0},{1,1,0},{0,0,0}});
    this->entry[1][0][2].adjacencies.push_back({2,2,{0,2,0},{0,1,0},{-1,0,0}});
    this->entry[1][0][2].adjacencies.push_back({0,2,{0,2,0},{1,0,0},{0,0,-1}});

    this->entry[1][1][2].adjacencies.push_back({2,2,{1,2,0},{1,1,0},{0,0,0}});
    this->entry[1][1][2].adjacencies.push_back({1,2,{1,2,0},{1,1,0},{0,0,0}});
    this->entry[1][1][2].adjacencies.push_back({2,2,{1,2,0},{0,1,0},{0,-1,0}});
    this->entry[1][1][2].adjacencies.push_back({1,2,{1,2,0},{1,0,0},{0,0,-1}});

    this->entry[1][2][2].adjacencies.push_back({1,2,{0,1,0},{1,1,0},{0,0,0}});
    this->entry[1][2][2].adjacencies.push_back({0,2,{0,1,0},{1,1,0},{0,0,0}});
    this->entry[1][2][2].adjacencies.push_back({1,2,{0,1,0},{0,1,0},{-1,0,0}});
    this->entry[1][2][2].adjacencies.push_back({0,2,{0,1,0},{1,0,0},{0,-1,0}});

    //E-->C
    this->entry[1][0][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
    this->entry[1][0][3].adjacencies.push_back({0,3,{0,1,2},{0,1,1},{-1,0,0}});
    this->entry[1][0][3].adjacencies.push_back({0,3,{0,1,2},{0,1,0},{-1,0,-1}});
    this->entry[1][0][3].adjacencies.push_back({0,3,{0,1,2},{1,1,0},{0,0,-1}});

    this->entry[1][1][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
    this->entry[1][1][3].adjacencies.push_back({0,3,{0,1,2},{1,0,1},{0,-1,0}});
    this->entry[1][1][3].adjacencies.push_back({0,3,{0,1,2},{1,0,0},{0,-1,-1}});
    this->entry[1][1][3].adjacencies.push_back({0,3,{0,1,2},{1,1,0},{0,0,-1}});

    this->entry[1][2][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
    this->entry[1][2][3].adjacencies.push_back({0,3,{0,1,2},{0,1,1},{-1,0,0}});
    this->entry[1][2][3].adjacencies.push_back({0,3,{0,1,2},{0,0,1},{-1,-1,0}});
    this->entry[1][2][3].adjacencies.push_back({0,3,{0,1,2},{1,0,1},{0,-1,0}});

    //F-->V
    this->entry[2][0][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
    this->entry[2][0][0].adjacencies.push_back({0,0,{},{},{0,1,0}});
    this->entry[2][0][0].adjacencies.push_back({0,0,{},{},{0,1,1}});
    this->entry[2][0][0].adjacencies.push_back({0,0,{},{},{0,0,1}});

    this->entry[2][1][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
    this->entry[2][1][0].adjacencies.push_back({0,0,{},{},{1,0,0}});
    this->entry[2][1][0].adjacencies.push_back({0,0,{},{},{1,0,1}});
    this->entry[2][1][0].adjacencies.push_back({0,0,{},{},{0,0,1}});

    this->entry[2][2][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
    this->entry[2][2][0].adjacencies.push_back({0,0,{},{},{1,0,0}});
    this->entry[2][2][0].adjacencies.push_back({0,0,{},{},{1,1,0}});
    this->entry[2][2][0].adjacencies.push_back({0,0,{},{},{0,1,0}});

    //F-->E
    this->entry[2][0][1].adjacencies.push_back({0,0,{},{},{0,0,0}});
    this->entry[2][0][1].adjacencies.push_back({2,0,{},{},{0,1,0}});
    this->entry[2][0][1].adjacencies.push_back({0,0,{},{},{0,0,1}});
    this->entry[2][0][1].adjacencies.push_back({2,0,{},{},{0,0,0}});

    this->entry[2][1][1].adjacencies.push_back({1,0,{},{},{0,0,0}});
    this->entry[2][1][1].adjacencies.push_back({2,0,{},{},{1,0,0}});
    this->entry[2][1][1].adjacencies.push_back({1,0,{},{},{0,0,1}});
    this->entry[2][1][1].adjacencies.push_back({2,0,{},{},{0,0,0}});

    this->entry[2][2][1].adjacencies.push_back({1,0,{},{},{0,0,0}});
    this->entry[2][2][1].adjacencies.push_back({0,0,{},{},{1,0,0}});
    this->entry[2][2][1].adjacencies.push_back({1,0,{},{},{0,1,0}});
    this->entry[2][2][1].adjacencies.push_back({0,0,{},{},{0,0,0}});

    //F-->C
    this->entry[2][0][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
    this->entry[2][0][3].adjacencies.push_back({0,3,{0,1,2},{0,1,1},{-1,0,0}});

    this->entry[2][1][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
    this->entry[2][1][3].adjacencies.push_back({0,3,{0,1,2},{1,0,1},{0,-1,0}});

    this->entry[2][2][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
    this->entry[2][2][3].adjacencies.push_back({0,3,{0,1,2},{1,1,0},{0,0,-1}});


    //C-->V
    this->entry[3][0][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
    this->entry[3][0][0].adjacencies.push_back({0,0,{},{},{1,0,0}});
    this->entry[3][0][0].adjacencies.push_back({0,0,{},{},{1,1,0}});
    this->entry[3][0][0].adjacencies.push_back({0,0,{},{},{0,1,0}});
    this->entry[3][0][0].adjacencies.push_back({0,0,{},{},{0,0,1}});
    this->entry[3][0][0].adjacencies.push_back({0,0,{},{},{1,0,1}});
    this->entry[3][0][0].adjacencies.push_back({0,0,{},{},{1,1,1}});
    this->entry[3][0][0].adjacencies.push_back({0,0,{},{},{0,1,1}});

    //C-->E
    this->entry[3][0][1].adjacencies.push_back({1,0,{},{},{0,0,0}});
    this->entry[3][0][1].adjacencies.push_back({0,0,{},{},{1,0,0}});
    this->entry[3][0][1].adjacencies.push_back({1,0,{},{},{0,1,0}});
    this->entry[3][0][1].adjacencies.push_back({0,0,{},{},{0,0,0}});
    this->entry[3][0][1].adjacencies.push_back({2,0,{},{},{0,0,0}});
    this->entry[3][0][1].adjacencies.push_back({2,0,{},{},{1,0,0}});
    this->entry[3][0][1].adjacencies.push_back({2,0,{},{},{1,1,0}});
    this->entry[3][0][1].adjacencies.push_back({2,0,{},{},{0,1,0}});
    this->entry[3][0][1].adjacencies.push_back({1,0,{},{},{0,0,1}});
    this->entry[3][0][1].adjacencies.push_back({0,0,{},{},{1,0,1}});
    this->entry[3][0][1].adjacencies.push_back({1,0,{},{},{0,1,1}});
    this->entry[3][0][1].adjacencies.push_back({0,0,{},{},{0,0,1}});

    //C-->F
    this->entry[3][0][2].adjacencies.push_back({1,0,{},{},{0,0,0}});
    this->entry[3][0][2].adjacencies.push_back({0,0,{},{},{1,0,0}});
    this->entry[3][0][2].adjacencies.push_back({1,0,{},{},{0,1,0}});
    this->entry[3][0][2].adjacencies.push_back({0,0,{},{},{0,0,0}});
    this->entry[3][0][2].adjacencies.push_back({2,0,{},{},{0,0,0}});
    this->entry[3][0][2].adjacencies.push_back({2,0,{},{},{0,0,1}});
 }

  QuerySequence<3> entry[4][3][4];
};

} //query
} //topology
} //flecsi

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
