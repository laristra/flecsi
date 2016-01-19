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

//using vertex_t = burton::vertex_t;
//using edge_t = burton::edge_t;
//using cell_t = burton::cell_t;

int main(int argc, char ** argv) {
  size_t width = 2;
  size_t height = 2;

#if 0
  burton b;

  vector<vertex_t*> vs;
  
  for(size_t j = 0; j < height + 1; ++j){
    for(size_t i = 0; i < width + 1; ++i){
      auto v = b.create_vertex({double(i), double(j)});
      vs.push_back(v);
    }
  }

  size_t width1 = width + 1;
  for(size_t j = 0; j < height; ++j){
    for(size_t i = 0; i < width; ++i){
      auto c = 
        b.create_cell({vs[i + j * width1],
                       vs[i + (j + 1) * width1],
                       vs[i + 1 + j * width1],
                       vs[i + 1 + (j + 1) * width1]});
    }
  }
#endif

  /*
   * Case A: Wedge flux across control volume boundary facet
   */

  /*
   * Case B: Wedge flux across cell boundary facet
   */

  /*
   * Case C: Wedge quadrature, i.e., cell integral
   */

  /*
   * Case D: Iterator examples
   */

  // wedges@cell

  // wedges@vertex

  // wedges@face

  // wedges@corner

  mesh_t & m = mesh_t::instance();

  /*--------------------------------------------------------------------------*
   * Example 1.
   * Vertex centered gradient computed in two loops. The first loop stores
   * the cell centered average in a temporary variable. The second loop
   * computes the gradient.
   *--------------------------------------------------------------------------*/

  {
  // register state
  state.define_scalar_field("sv", VERTEX);
  state.define_scalar_field("sc", CELL);
  state.define_vector_field("gsv", VERTEX);

  // Do other stuff...

  auto sv = m.state.get_scalar_field("sv");
  auto sc = m.state.get_scalar_field("sc");
  auto gsv = m.state.get_vector_field("gsv");

  // go over cells to get zone centered average sc
  for(auto c: m.cells()) {
    auto xc = c.coordinates();

    for(auto v: m.vertices(c)) {
      auto xv = v.coordinates();

      sc(c) += f(sv(v), xv, xc);
    } // for
  } // for

  // go over vertices and compute gradient using cell centered average
  for(auto v: m.vertices()) {

    // volume of dual mesh surrounding vertex
    auto volume = v.volume();

    for(auto w: m.wedges(v)) {
      auto Si = w.side_facet_normal();
      // or
      auto Si = normal(w, SIDE_FACET);

      gsv(v) += Si * sc(w.cell())/volume;
    } // for
  } // for

  } // scope

  /*--------------------------------------------------------------------------*
   * Example 2.
   * Vertex centered gradient computed in one loop. The cell centered
   * average is computed in the loop. The sc field is not needed. There
   * is an sc scalar inside the loop.
   *--------------------------------------------------------------------------*/

  {
  // register state
  state.define_scalar_field("sv", VERTEX);
  state.define_vector_field("gsv", VERTEX);

  for(auto v: m.vertices()) {
    auto volume = v.volume();

    for(auto w: m.wedges(v)) {
      auto sc = 0.0;
      auto c = w->cell_id();

      auto xc = c.coordinates();

      for(auto vv: m.vertices(c)) {
        auto xv = vv->coordinates();
        sc += fn(sv(vv), xv, xc);
      } // for

      auto Si = w.side_facet_normal();
      gsv(v) += Si *sc/vol;
    } // for
  } // for

  } // scope

  /*--------------------------------------------------------------------------*
   * Example 3.
   * Vertex centered gradient computed in two loops. The first loop stores
   * the edge centered average in a temporary variable. The second loop
   * computes the gradient.
   *--------------------------------------------------------------------------*/

  {
  // register state
  state.define_scalar_field("sv", VERTEX);
  state.define_scalar_field("se", EDGE);
  state.define_vector_field("gsv", VERTEX);

  // Do other stuff...

  auto sv = m.state.get_scalar_field("sv");
  auto se = m.state.get_scalar_field("se");
  auto gsv = m.state.get_vector_field("gsv");

  // go over cells to get zone centered average sc
  for(auto e: m.edges()) {
    auto xe = e.coordinates();

    for(auto c: m.cells(e)) {
      for(auto v: m.vertices(c)) {
        auto xv = v.coordinates();

        se(e) += f(sv(v), xv, xe);
      } // for
    } // for
  } // for

  // go over vertices and compute gradient using cell centered average
  for(auto v: m.vertices()) {

    // volume of dual mesh surrounding vertex
    auto volume = v.volume();

    for(auto w: m.wedges(v)) {
      auto Si = w.side_facet_normal();
      // or
      auto Si = normal(w, SIDE_FACET);

      for(auto e: m.edges(w)) {
        gsv(v) += Si * se(e)/volume;
      } // for
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
  state.define_scalar_field("sv", VERTEX);
  state.define_vector_field("gsv", VERTEX);

  for(auto v: m.vertices()) {
    auto volume = v.volume();

    for(auto w: m.wedges(v)) {
      auto se = 0.0;

      // do we need to iterate over edges of a wedge???
      for(auto e: m.edges(w)) {
        for(auto c: m.cells(e)) {
          for(auto vv: m.vertices(c)) {
            auto xv = vv.coordinates();
            se += f(sv(vv), xv, xe);
          } // for
        } // for
      } // for

      auto Si = w.side_facet_normal();
      gsv(v) += Si * se/volume;

    } // for
  } // for

  } // scope

  return 0;
} // main

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
