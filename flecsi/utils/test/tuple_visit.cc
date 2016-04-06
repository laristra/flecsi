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
#include "flecsi/utils/tuple_visit.h"


// using declarations
using std::cout;
using std::endl;
using flecsi::utils::tuple_visit;

//=============================================================================
//! \brief Test the tuple_visit function.
//=============================================================================
TEST(tuple_visit, simple) {

  struct NonPrintable { };

  auto x = std::make_tuple(2, "test", 3, 8.4);
  auto y = std::make_tuple("this", 8.4, 3, 2);

  tuple_visit( 
              [](const auto & a, const auto & b) { 
                cout << a << " " << b << endl;
              },
              x, y );


}
