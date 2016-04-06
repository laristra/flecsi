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

// system includes
#include <cinchtest.h>
#include <iostream>
#include <string>

// user includes
#include "flecsi/utils/tuple_for_each.h"


// using declarations
using std::cout;
using std::endl;
using std::get;
using flecsi::utils::tuple_for_each;

//=============================================================================
//! \brief Test the tuple_zip function.
//=============================================================================
TEST(tuple_for_each, simple) {

  auto x = std::make_tuple(2, "test", 3, 8.4);

  tuple_for_each( x, 
                  [](auto & tup) { 
                    cout << tup << endl; 
                  } );
}


/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/

