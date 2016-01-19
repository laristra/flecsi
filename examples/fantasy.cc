/*~-------------------------------------------------------------------------~~*
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
 *~-------------------------------------------------------------------------~~*/

#include <flecsi/specializations/burton/burton.h>

#include <vector>

using namespace std;
using namespace flecsi;

int main(int argc, char ** argv) {

  /*--------------------------------------------------------------------------*
   * Example 1.
   * Vertex centered gradient computed in two loops. The first loop stores
   * the cell centered average in a temporary variable. The second loop
   * computes the gradient.
   *--------------------------------------------------------------------------*/

  {
  // register state
  register_scalar("sv", VERTEX);
  register_scalar("sc", CELL);
  register_vector("gsv", VERTEX);

  // Do other stuff...

  auto sv = get_scalar("sv");
  auto sc = get_scalar("sc");
  auto gsv = get_vector("gsv");

  // go over cells to get zone centered average sc
  for(auto c: cells()) {
    auto xc = coordinates(c);

    for(auto v: vertices(c)) {
      auto xv = coordinates(v);

      sc(c) += f(sv(v), xv, xc);
    } // for
  } // for

  // go over vertices and compute gradient using cell centered average
  for(auto v: vertices()) {

    // volume of dual mesh surrounding vertex
    auto volume = volume(v);

    for(auto w: wedges(v)) {
      auto Si = normal(w, SIDE_FACET);
      // normal(w, CELL_FACET) is also defined

      gsv(v) += Si * sc(cell(w))/volume;
    } // for
  } // for

  } // scope

  /*--------------------------------------------------------------------------*
   * Example 4.
   * Vertex centered gradient computed in one loop. The edge centered
   * average is computed in the loop. The se field is not needed. There
   * is an se scalar inside the loop.
   *--------------------------------------------------------------------------*/

  {
  // register state
  register_scalar("sv", VERTEX);
  register_vector("gsv", VERTEX);

  // Do other stuff...

  auto sv = get_scalar("sv");
  auto gsv = get_vector("gsv");

  for(auto v: vertices()) {
    auto volume = volume(v);

    for(auto w: wedges(v)) {
      auto se = 0.0;

      // do we need to iterate over edges of a wedge???
      for(auto e: edges(w)) {
        for(auto c: cells(e)) {
          for(auto vv: vertices(c)) {
            auto xv = coordinates(vv);
            se += f(sv(vv), xv, xe);
          } // for
        } // for
      } // for

      auto Si = normal(w, SIDE_FACET);
      gsv(v) += Si * se/volume;
    } // for
  } // for

  } // scope

  return 0;
} // main
