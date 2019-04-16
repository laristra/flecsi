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

#include <flecsi/utils/ftest.h>

#define __FLECSI_PRIVATE__
#include <flecsi/execution/execution.h>

using namespace flecsi::execution;

flog_register_tag(color);

/*
  Test the raw context interface for gettting color information from
  the FleCSI runtime.
 */

int
color_raw(int argc, char ** argv) {

  FTEST();

  auto depth = context_t::instance().task_depth();
  auto shard = context_t::instance().shard();
  auto shards = context_t::instance().shards();
  auto cps = context_t::instance().colors_per_shard();

  {
    flog_tag_guard(color);
    flog(info) << "color(raw): " << shard << std::endl
               << "colors(raw): " << shards << std::endl
               << "colors_per_shard(raw): " << cps << std::endl;
  }

  ASSERT_EQ(shards, 4);
  ASSERT_LT(shard, shards);

  return 0;
}

ftest_register_test(color_raw);

#if 0
/*
  Test the macro interface for gettting color information from
  the FleCSI runtime.
 */

int
color_ui(int argc, char ** argv) {

  FTEST();

  auto color = flecsi_color();
  auto colors = flecsi_colors();

  {
    flog_tag_guard(color);
    flog(info) << "color(macro): " << color << std::endl
               << "colors(macro): " << colors << std::endl;
  }

  ASSERT_EQ(colors, 4);
  ASSERT_LT(color, colors);

  return 0;
}

ftest_register_test(color_ui);
#endif
