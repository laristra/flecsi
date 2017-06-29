/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Tests related to compressed storage.
////////////////////////////////////////////////////////////////////////////////

// user includes
#include "flecsi/utils/compressed_storage.h"


// system includes
#include<cinchtest.h>
#include<iostream>

// explicitly use some stuff
using compressed_row = flecsi::utils::compressed_row_storage<int>;

///////////////////////////////////////////////////////////////////////////////
//! \brief Test the construction of compressed_row_storage's.
///////////////////////////////////////////////////////////////////////////////
TEST(compressed_row_storage, construction) 
{

  // default constructor
  compressed_row mat1;
  ASSERT_EQ( mat1.rows(), 0 );
  ASSERT_EQ( mat1.size(), 0 );
  ASSERT_TRUE( mat1.empty() );

  // size constructor
  compressed_row mat2(5);
  ASSERT_EQ( 5, mat2.rows() );
  ASSERT_EQ( mat2.size(), 0 );
  ASSERT_TRUE( mat2.empty() );

  mat2.clear();
  ASSERT_EQ( 0, mat2.rows() );

#if 0
  // copy constructor
  fixed_vector vec6( vec5 );
  ASSERT_EQ( vec5.size(), vec6.size() );
  ASSERT_EQ( vec5, vec6 );

  // move constructor
  fixed_vector vec7( std::move(vec5) );
  ASSERT_EQ( vec6.size(), vec7.size() );
  ASSERT_EQ( vec6, vec7 );
#endif

} // TEST

///////////////////////////////////////////////////////////////////////////////
//! \brief Test the pushing back of compressed_row_storage's.
///////////////////////////////////////////////////////////////////////////////
TEST(compressed_row_storage, push_back) 
{
  int i = 0;

  compressed_row mat1;
  mat1.push_back( i++ );
  mat1.push_back( i++ );
  ASSERT_EQ( mat1.rows(), 2 );
  ASSERT_EQ( mat1.size(), 2 );
  ASSERT_EQ( mat1[0], 0 );
  ASSERT_EQ( mat1[1], 1 );

  compressed_row mat2;
  mat2.push_back( int(0) );
  mat2.push_back( int(1) );
  ASSERT_EQ( mat2.rows(), 2 );
  ASSERT_EQ( mat2.size(), 2 );
  ASSERT_EQ( mat2[0], 0 );
  ASSERT_EQ( mat2[1], 1 );

  std::vector<int> row1 = {0, 1};
  std::vector<int> row2 = {1, 0};
  compressed_row mat3;
  mat3.push_back( row1.begin(), row1.end() );
  mat3.push_back( row2.begin(), row2.end() );
  ASSERT_EQ( mat3.rows(), 2 );
  ASSERT_EQ( mat3.size(), 4 );
  ASSERT_EQ( mat3.row(0), row1 );
  ASSERT_EQ( mat3.row(1), row2 );

} // TEST
