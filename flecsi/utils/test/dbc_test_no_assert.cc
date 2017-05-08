// dbc_test_no_assert.cc
// T. M. Kelley
// May 02, 2017
// (c) Copyright 2017 LANSLLC, all rights reserved


#include "cinchtest.h"
/* Test will take control of the test environment */
#ifdef FLECSI_DBC_THROW
#undef FLECSI_DBC_THROW
#endif

#ifdef FLECSI_REQUIRE_ON
#undef FLECSI_REQUIRE_ON
#endif

#ifdef FLECSI_DBC_NOTIFY
#undef FLECSI_DBC_NOTIFY
#endif

#include "flecsi/utils/dbc.h"

using namespace flecsi::dbc;

TEST(dbc_no_assert,compiles){
  ASSERT_TRUE(true);
}

/**\brief Is s1 a substring of s2? */
inline
bool is_substring_of(std::string const &s1, std::string const &s2){
  return s2.find(s1) != std::string::npos;
}

/**\brief Check that s1 is a substring of s2, print them if not*/
inline
bool check_messages(std::string const &s1, std::string const &s2){
  bool const msg_ok(is_substring_of(s1,s2));
  if(!msg_ok){
    printf(
      "%s:%i expected: %s\n", __FUNCTION__, __LINE__, s1.c_str());
    printf("%s:%i actual: %s\n",__FUNCTION__,__LINE__,s2.c_str());
  }
  return msg_ok;
} // check_messages

TEST(dbc_no_assert,Equal){
  // int
  {
    int i = 1;
    int j = 2;
    int k = 1;
    int fail_line = -1;
    try{
      bool ret_val(true);
      ret_val = Equal(i,k);
      EXPECT_TRUE(ret_val); // clang-format off
      fail_line = __LINE__;ret_val = Equal(i,j);
      // clang-format on
      /* Should not get here with exceptions*/
#if (REQUIRE_ON && FLECSI_DBC_THROW)
      EXPECT_TRUE(false);
#elif REQUIRE_ON
      EXPECT_EQ(false,ret_val);
#else
      EXPECT_FALSE(ret_val);
#endif
    }
    catch(std::exception & e){
      // exact message depends on build details. This part shd be invariant:
      std::stringstream exp_msg;
      exp_msg << "flecsi/utils/test/dbc_test_no_assert.cc:" << fail_line
        << ":TestBody Assertion 'i != j' failed";
      std::string excmsg(e.what());
      EXPECT_TRUE(check_messages(exp_msg.str(),excmsg));
    }
  }// int
} // TEST(dbc,Equal){

TEST(dbc_no_assert,InOpenRange){
  // int
  {
    int i = 1;
    int j = 2;
    int k = 4;
    int fail_line = -1;
    try{
      bool ret_val(true);
      ret_val = InOpenRange(j,i,k);
      EXPECT_TRUE(ret_val); // clang-format off
      fail_line = __LINE__;ret_val = InOpenRange(i,j,k);
      // clang-format on
      /* Should not get here with exceptions*/
#if (REQUIRE_ON && FLECSI_DBC_THROW)
      EXPECT_TRUE(false);
#elif REQUIRE_ON
      EXPECT_EQ(false,ret_val);
#else
      EXPECT_FALSE(ret_val);
#endif
    }
    catch(std::exception & e){
      // exact message depends on build details. This part shd be invariant:
      std::stringstream exp_msg;
      exp_msg << "flecsi/utils/test/dbc_test_no_assert.cc:" << fail_line
        << ":TestBody Assertion 'i (1) was not in range (2,4)' failed";
      std::string excmsg(e.what());
      EXPECT_TRUE(check_messages(exp_msg.str(),excmsg));
    }
  }// int
} // TEST(dbc,InOpenRange){

TEST(dbc_no_assert,Insist){
  // int
  {
    int i = 1;
    int j = 2;
    int k = 4;
    int fail_line = -1;
    try{
      fail_line = __LINE__; Insist(i == j, "i == j")
#if (REQUIRE_ON && FLECSI_DBC_THROW)
      EXPECT_TRUE(false);  // Should not get here with exceptions
#else
      EXPECT_TRUE(true);
#endif
    }
    catch(std::exception & e){
      // exact message depends on build details. This part shd be invariant:
      std::stringstream exp_msg;
      exp_msg << "flecsi/utils/test/dbc_test_no_assert.cc:" << fail_line
        << ":TestBody Assertion 'i == j' failed";
      std::string excmsg(e.what());
      EXPECT_TRUE(check_messages(exp_msg.str(),excmsg));
    }
  }// int
} // TEST(dbc,InOpenRange){

TEST(dbc_no_assert,LessThan){
  // int
  {
    int i = 1;
    int j = 2;
    int k = 4;
    int fail_line = -1;
    try{
      bool ret_val = LessThan(i,j);
      EXPECT_TRUE(ret_val);
      fail_line = __LINE__; ret_val = LessThan(j,i);
#if (REQUIRE_ON && FLECSI_DBC_THROW)
      EXPECT_TRUE(false);  // Should not get here with exceptions
#else
      EXPECT_FALSE(ret_val);
#endif
    }
    catch(std::exception & e){
      // exact message depends on build details. This part shd be invariant:
      std::stringstream exp_msg;
      exp_msg << "flecsi/utils/test/dbc_test_no_assert.cc:" << fail_line
        << ":TestBody Assertion 'j (2) >= 1' failed";
      std::string excmsg(e.what());
      EXPECT_TRUE(check_messages(exp_msg.str(),excmsg));
    }
  }// int
} // TEST(dbc,InOpenRange){


// End of file
