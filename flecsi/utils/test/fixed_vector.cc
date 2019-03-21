/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Tests related to a fixed vector.
////////////////////////////////////////////////////////////////////////////////

// user includes
#include <flecsi/utils/fixed_vector.h>

// system includes
#include <cinchtest.h>
#include <iostream>

// explicitly use some stuff
using fixed_vector = flecsi::utils::fixed_vector<int, 5>;

///////////////////////////////////////////////////////////////////////////////
//! \brief Test the construction of fixed_vector's.
///////////////////////////////////////////////////////////////////////////////
TEST(fixed_vector, construction) {

  // default constructor
  fixed_vector vec1;
  ASSERT_TRUE(vec1.empty());

  // size constructor
  fixed_vector vec2(5);
  ASSERT_EQ(5, vec2.size());

  // size and value constructor
  fixed_vector vec3(4, 1);
  ASSERT_EQ(4, vec3.size());
  for(int i = 0; i < 4; ++i)
    ASSERT_EQ(1, vec3[i]);

  // stl-like constructor
  fixed_vector vec4(vec3.begin(), vec3.end());
  ASSERT_EQ(vec3.size(), vec4.size());
  for(auto x : vec4)
    ASSERT_EQ(1, x);

  // initializer fixed_vector constructor
  fixed_vector vec5({1, 1});
  ASSERT_EQ(2, vec5.size());
  for(auto x : vec5)
    ASSERT_EQ(1, x);

  // copy constructor
  fixed_vector vec6(vec5);
  ASSERT_EQ(vec5.size(), vec6.size());
  ASSERT_EQ(vec5, vec6);

  // move constructor
  fixed_vector vec7(std::move(vec5));
  ASSERT_EQ(vec6.size(), vec7.size());
  ASSERT_EQ(vec6, vec7);

} // TEST

///////////////////////////////////////////////////////////////////////////////
//! \brief Test the modification of fixed_vector's.
///////////////////////////////////////////////////////////////////////////////
TEST(fixed_vector, insertion) {
  fixed_vector vec1(5);
  vec1.clear();
  ASSERT_EQ(0, vec1.size());
  ASSERT_TRUE(vec1.empty());

  // move insertion of a single value
  auto it = vec1.insert(vec1.begin(), 4);
  ASSERT_EQ(vec1.begin(), it);

  vec1.insert(vec1.begin(), 3);
  vec1.insert(vec1.begin(), 2);
  vec1.insert(vec1.begin(), 1);
  vec1.insert(vec1.begin(), 0);
  for(auto i = 0; i < vec1.size(); i++)
    ASSERT_EQ(i, vec1[i]);

  // copy insertion of a single value
  vec1.clear();
  for(auto i = 4; i <= 0; i--)
    it = vec1.insert(vec1.begin(), i);
  ASSERT_EQ(vec1.begin(), it);
  for(auto i = 0; i < vec1.size(); i++)
    ASSERT_EQ(i, vec1[i]);

  // copy insertion of a number of values
  vec1.clear();
  vec1.insert(vec1.begin(), 3);
  vec1.insert(vec1.begin(), 1);
  vec1.insert(vec1.begin(), 0);
  it = vec1.insert(std::next(vec1.begin(), vec1.size() - 1), 1, 2);
  for(auto i = 0; i < vec1.size(); i++)
    ASSERT_EQ(i, vec1[i]);
  ASSERT_EQ(2, *it);

  // copy insertion of a number of values using iterators
  fixed_vector vec2{2};
  vec1.clear();
  vec1.insert(vec1.begin(), 3);
  vec1.insert(vec1.begin(), 1);
  vec1.insert(vec1.begin(), 0);
  it = vec1.insert(
    std::next(vec1.begin(), vec1.size() - 1), vec2.begin(), vec2.end());
  for(auto i = 0; i < vec1.size(); i++)
    ASSERT_EQ(i, vec1[i]);
  ASSERT_EQ(2, *it);

  // copy insertion of a number of values using initializer fixed_vector
  vec1.clear();
  vec1.insert(vec1.begin(), 3);
  vec1.insert(vec1.begin(), 1);
  vec1.insert(vec1.begin(), 0);
  it = vec1.insert(std::next(vec1.begin(), vec1.size() - 1), {2});
  for(auto i = 0; i < vec1.size(); i++)
    ASSERT_EQ(i, vec1[i]);
  ASSERT_EQ(2, *it);
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Test the erasing of fixed_vector's.
///////////////////////////////////////////////////////////////////////////////
TEST(fixed_vector, erase) {

  // erase a specific value
  fixed_vector vec1 = {0, 1, 2, 3, 3};
  auto it = std::next(vec1.begin(), 3);
  ASSERT_EQ(3, *it);

  it = vec1.erase(it);
  ASSERT_EQ(4, vec1.size());
  for(auto i = 0; i < vec1.size(); i++)
    ASSERT_EQ(i, vec1[i]);
  ASSERT_EQ(3, *it);

  // erase a range
  fixed_vector vec2 = {0, 1, 2, 3, 3};
  it = std::next(vec2.begin(), 3);
  ASSERT_EQ(3, *it);

  it = vec2.erase(it, std::next(it));
  ASSERT_EQ(4, vec2.size());
  for(auto i = 0; i < vec2.size(); i++)
    ASSERT_EQ(i, vec2[i]);
  ASSERT_EQ(3, *it);
}

///////////////////////////////////////////////////////////////////////////////
//! \brief Test the expansion of fixed_vector's.
///////////////////////////////////////////////////////////////////////////////
TEST(fixed_vector, append) {

  // erase a specific value
  fixed_vector vec;
  vec.push_back(0);
  auto i = 1;
  vec.push_back(i);
  ASSERT_EQ(2, vec.size());
  for(i = 0; i < vec.size(); i++)
    ASSERT_EQ(i, vec[i]);
}
