/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */
// user includes
#include <flecsi/utils/reorder.h>

#include <flecsi/utils/ftest.h>
#include <random>

// some using declarations
using std::cout;
using std::endl;
using std::vector;

using flecsi::utils::reorder;
using flecsi::utils::reorder_destructive;

// the test data
const vector<int> v = {0, 3, 2, 1};
const vector<int> order = {3, 0, 1, 2};
const vector<int> ans = {3, 2, 1, 0};


void reorder_inplace(int argc, char ** argv) {

	FTEST();

  auto vcpy = v;
  auto ocpy = order;
  reorder(ocpy.begin(), ocpy.end(), vcpy.begin());
  ASSERT_EQ(vcpy, ans);

}

void reorder_destroy(int argc, char ** argv) {

  FTEST();
 
  auto vcpy = v;
  auto ocpy = order;
  reorder_destructive(ocpy.begin(), ocpy.end(), vcpy.begin());
  ASSERT_EQ(vcpy, ans);

}

void reorder_both(int argc, char ** argv) {

  FTEST();

  // We'll run our test ntimes times, each time on a vector
  // with a randomly computed length in [0..maxlen]
  const std::size_t ntimes = 10000;
  const std::size_t maxlen = 100;

  // random constructs
  std::mt19937 random;
  random.seed(12345);
  using random_t = decltype(random());

  // ntimes times
  for (std::size_t t = ntimes; t--;) {

    // ------------------------
    // Initialize
    // ------------------------

    // Compute random size ( <= maxlen ) for vector
    std::size_t len = std::size_t(random()) % (maxlen + 1);
    while (len == 0)
        len = std::size_t(random()) % (maxlen + 1);

    // Make an initial vector of the above-computed random size,
    // and set up a random ordering specification for the vector
    std::vector<random_t> initial(len);
    std::vector<std::size_t> order(len);
    for (std::size_t i = 0; i < len; ++i) {
      initial[i] = random(); // random values
      order[i] = i;          // unique indices
    }
    std::random_shuffle(order.begin(), order.end()); // still unique indices

    // ------------------------
    // Inplace reorder:
    // order preserving
    // ------------------------

    // reorder a copy of the initial vector
    std::vector<random_t> copy = initial;
    flecsi::utils::reorder(order.begin(), order.end(), copy.begin());

    // direct check if copy equals order-reordered initial
    for (std::size_t i = 0; i < len; ++i)
      EXPECT_EQ(copy[order[i]], initial[i]);

    // ------------------------
    // Inplace reorder:
    // order destroying
    // ------------------------

    // reorder a copy of the initial vector
    copy = initial;
    std::vector<std::size_t> original_order = order; // because we'll wreck it
    flecsi::utils::reorder_destructive               // destructive!
        (order.begin(), order.end(), copy.begin());

    // direct check if copy equals order-reordered initial
    for (std::size_t i = 0; i < len; ++i)
      EXPECT_EQ(copy[original_order[i]], initial[i]);
  } // for

}

ftest_register_test(reorder_inplace);
ftest_register_test(reorder_destroy);
ftest_register_test(reorder_both);
