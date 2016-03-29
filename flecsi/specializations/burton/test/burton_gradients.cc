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

#include <cinchtest.h>

#include "flecsi/specializations/burton/burton.h"
#include "flecsi/io/io.h"
#include "flecsi/specializations/burton/burton_io_exodus.h"

using namespace std;
using namespace flecsi;

using real_t = burton_mesh_t::real_t;

using point_t = burton_mesh_t::point_t;
using vector_t = burton_mesh_t::vector_t;

using vertex_t = burton_mesh_t::vertex_t;
using edge_t = burton_mesh_t::edge_t;
//using cell_t = burton_mesh_t::cell_t;
using corner_t = burton_mesh_t::corner_t;

const int W = 64, H = 64;
const real_t xmin = -1.0, xmax = 1.0, ymin = -1.0, ymax = 1.0;
const real_t dx = (xmax-xmin)/real_t(W), dy = (ymax-ymin)/real_t(H);
#if 1
real_t x(size_t i, size_t j) { return xmin + pow(j+1,0.15)*i*dx; }
real_t y(size_t i, size_t j) { return ymin + pow(i+1,0.2)*j*dy; }
#else
real_t x(size_t i, size_t j) { return xmin + i*dx; }
real_t y(size_t i, size_t j) { return ymin + j*dy; }
#endif
// test fixture for creating the mesh
class Burton : public ::testing::Test {
protected:
  virtual void SetUp() {
    vector<vertex_t*> vs;
  
    b.init_parameters((height+1)*(width+1));

    size_t max_rank = (height+1)*(width+1) - 1;

    for(size_t j = 0; j < height + 1; ++j){
      for(size_t i = 0; i < width + 1; ++i){
        auto v = b.create_vertex({x(i,j), y(i,j)});
        vs.push_back(v);
      } // for
    } // for

    size_t width1 = width + 1;
    for(size_t j = 0; j < height; ++j){
      for(size_t i = 0; i < width; ++i){
        // go over vertices counter clockwise to define cell
        auto c = b.create_cell({vs[i + j * width1],
            vs[i + 1 + j * width1],
            vs[i + 1 + (j + 1) * width1],
            vs[i + (j + 1) * width1]});
      } // for
    } // for

    b.init();
  } // SetUp

  virtual void TearDown() { }

  burton_mesh_t b;
  const size_t width = W;
  const size_t height = H;
};

#include <ccomplex>
#include <lapacke.h>

const int N = 10;

// core interpolation shared by vertex and cell centered gradients
void interpolate(const auto x[4], const auto y[4], auto b[N]) {
  /*
    from:

    http://math.stackexchange.com/questions/828392
    /spatial-interpolation-for-irregular-grid
   */

  // s(x,y) = a_xx x^2 + a_xy x y  + a_yy y^2 + b_x x + b_y y + c

  // Minimize e = axx^2 + axy^2 + ayy^2 by solving the following.
  // The calling function evaluates s(x,y) using the contents of b.

  // 4x6
  //     | x1^2 x1y1 y1^2 x1 y1 1 |
  // X = | x2^2 x2y2 y2^2 x2 y2 1 |
  //     | x3^2 x3y3 y3^2 x3 y3 1 |
  //     | x4^2 x4y4 y4^2 x4 y4 1 |

  // 6x1
  //     | axx |
  //     | axy |
  // a = | ayy |
  //     | bx  |
  //     | by  |
  //     | c   |

  // 4x1
  //     | z1 |
  // z = | z2 |
  //     | z3 |
  //     | z4 |
  
  // 6x6
  //     | 1 0 0 0 0 0 |
  //     | 0 1 0 0 0 0 |
  // E = | 0 0 1 0 0 0 |
  //     | 0 0 0 0 0 0 |
  //     | 0 0 0 0 0 0 |
  //     | 0 0 0 0 0 0 |

  // 4x1
  //     | l1 |
  // l = | l2 |
  //     | l3 |
  //     | l4 |
  
  // 10x10
  // | E  X^T | | a |   | 0 |
  // |        | |   | = |   |
  // | X   0  | | l |   | z |

  real_t A[N*N]; for (int i=0; i<N*N;i++) A[i] = 0.0;
  // row 1
  A[0] = 1.0;
  A[6] = x[0]*x[0]; A[7] = x[1]*x[1]; A[8] = x[2]*x[2]; A[9] = x[3]*x[3];
  // row 2
  A[11] = 1.0;
  A[16] = x[0]*y[0]; A[17] = x[1]*y[1]; A[18] = x[2]*y[2]; A[19] = x[3]*y[3];
  // row 3
  A[22] = 1.0;
  A[26] = y[0]*y[0]; A[27] = y[1]*y[1]; A[28] = y[2]*y[2]; A[29] = y[3]*y[3];
  // row 4
  A[36] = x[0]; A[37] = x[1]; A[38] = x[2]; A[39] = x[3];
  // row 5
  A[46] = y[0]; A[47] = y[1]; A[48] = y[2]; A[49] = y[3];
  // row 6
  A[56] = 1.0; A[57] = 1.0; A[58] = 1.0; A[59] = 1.0;
  // row 7
  A[60] = x[0]*x[0]; A[61] = x[0]*y[0]; A[62] = y[0]*y[0];
  A[63] = x[0]; A[64] = y[0]; A[65] = 1.0;
  // row 8
  A[70] = x[1]*x[1]; A[71] = x[1]*y[1]; A[72] = y[1]*y[1];
  A[73] = x[1]; A[74] = y[1]; A[75] = 1.0;
  // row 9
  A[80] = x[2]*x[2]; A[81] = x[2]*y[2]; A[82] = y[2]*y[2];
  A[83] = x[2]; A[84] = y[2]; A[85] = 1.0;
  // row 10
  A[90] = x[3]*x[3]; A[91] = x[3]*y[3]; A[92] = y[3]*y[3];
  A[93] = x[3]; A[94] = y[3]; A[95] = 1.0;
  
  lapack_int ipiv[N]; for (int i=0; i<N; i++) ipiv[i] = 0;

  int nrhs = 1; // NB: last argument, ldb, is equal to 1, not N!
  lapack_int err = LAPACKE_dgesv(LAPACK_ROW_MAJOR, N, nrhs, A,
    N, ipiv, b, nrhs);
  assert(err == 0);
}

// compute cell centered average of vertex centered scalar s at cell c.
real_t interpolate_to_cell(mesh_t & m, auto & s, auto & c) {
  // declare and initialize b, x, y for sending to interpolate.
  real_t b[N]; for (int i=0; i<N; i++) b[i] = 0.0;
  // x,y coords in x,y.
  real_t x[4] = {0.0,0.0,0.0,0.0}, y[4] = {0.0,0.0,0.0,0.0};
  int i = 0;
  for (auto v: m.vertices(c)) {
    x[i] = v->coordinates()[0];
    y[i] = v->coordinates()[1];
    b[i+6] = s[v]; // the scalar at the vertices
    i++;
  }

  interpolate(x,y,b);

  auto xc = c->centroid();

  // evaluate function at xc
  // s(x,y) = a_xx x^2 + a_xy x y  + a_yy y^2 + b_x x + b_y y + c
  return b[0]*xc[0]*xc[0] + b[1]*xc[0]*xc[1] + b[2]*xc[1]*xc[1]
    + b[3]*xc[0] + b[4]*xc[1] + b[5];
}

TEST_F(Burton, vertex_gradient) {

  /*--------------------------------------------------------------------------*
   * Vertex centered gradient. Store the cell centered average in a temporary
   * variable, and then compute the gradient. Punt on boundary vertices.
   *--------------------------------------------------------------------------*/

  // register state
  // vertex based scalar (sv = scalar vertex)
  register_state(b, "sv", vertices, real_t, persistent);
  // cell based scalar (sc = scalar cell). make it "persistent" so it appears
  // in graphics dumps.
  register_state(b, "sc", cells, real_t, persistent);
  // vertex based vector (gsv = gradient scalar vector)
  register_state(b, "gsv", vertices, vector_t, persistent);
  // analytic answer for vertex based vector
  register_state(b, "ans_gsv", vertices, vector_t, persistent);
    
  // access the state
  auto sv = access_state(b, "sv", real_t);
  auto sc = access_state(b, "sc", real_t);
  auto gsv = access_state(b, "gsv", vector_t);
  auto ans_gsv = access_state(b, "ans_gsv", vector_t);

  // go over vertices and initialize vertex data and answer
  const real_t pi = 3.1415;
  for(auto v: b.vertices()) {
    auto xv = v->coordinates();
    sv[v] = sin(pi*xv[0]) * cos(pi*xv[1]);
    ans_gsv[v][0] = pi * cos(pi*xv[0]) * cos(pi*xv[1]);
    ans_gsv[v][1] = -pi * sin(pi*xv[0]) * sin(pi*xv[1]);
    gsv[v] = 0.0;
  } // for

  // go over cells to initialize sc
  for(auto c: b.cells()) {
    sc[c] = 0.0;
  } // for
    
  // go over cells to get zone centered average stored in sc
  for(auto c: b.cells()) {
    sc[c] = interpolate_to_cell(b, sv, c);
  } // for

  // go over vertices and compute gradient using cell centered average
  for(auto v: b.vertices()) {

    // go over corners for vertex to get the area
    real_t area = 0.0;
    for(auto cnr: b.corners(v)) {
      area += cnr->area();
    } // for

    // This hack avoids computing gradients on the mesh boundary. Since
    // boundary conditions are not implemented, the contributions from Si are
    // not "offset" for boundary vertices and leads to large gradient
    // values on the boundary. This makes it a PIA in the viz tool where
    // one has to rescale plot attributes to compare results. Instead this
    // just skips over boundary vertices.
    if (b.wedges(v).size() < 8) continue;

    // go over wedges at vertex to finalize gradient
    for(auto w: b.wedges(v)) {
      auto Si = w->side_facet_normal();
      // need to clean up the access to a wedge's cell
      auto c = w->cell();
      gsv[v] += Si * sc[c]/area;
    } // for

  } // for

  // write the mesh
  std::string name("vertex_gradient.exo");
  ASSERT_FALSE(write_mesh(name, b));
}

// compute vertex centered average of cell centered scalar s at vertex v.
real_t interpolate_to_vertex(mesh_t & m, auto & s, auto & v) {
  // declare and initialize b, x, y for sending to interpolate.
  real_t b[N]; for (int i=0; i<N; i++) b[i] = 0.0;
  // x,y coords in x, y.
  real_t x[4] = {0.0,0.0,0.0,0.0}, y[4] = {0.0,0.0,0.0,0.0};
  int i = 0;
  for (auto c: m.cells(v)) {
    x[i] = c->centroid()[0];
    y[i] = c->centroid()[1];
    b[i+6] = s[c]; // the scalar at the centroids
    i++;
  } // for

  // punt on computing and interpolation for the boundary vertices.
  if (i < 4) return 0.0;

  interpolate(x,y,b);

  auto xv = v->coordinates();

  // evaluate function at xv
  // s(x,y) = a_xx x^2 + a_xy x y  + a_yy y^2 + b_x x + b_y y + c
  return b[0]*xv[0]*xv[0] + b[1]*xv[0]*xv[1] + b[2]*xv[1]*xv[1]
    + b[3]*xv[0] + b[4]*xv[1] + b[5];
}


TEST_F(Burton, cell_gradient) {

  /*--------------------------------------------------------------------------*
   * Cell centered gradient. Store the vertex centered average in a temporary
   * variable, and then compute the gradient. Punt on boundary cells.
   *--------------------------------------------------------------------------*/

  // register state
  // cell based scalar (sc = scalar cell)
  register_state(b, "sc", cells, real_t, persistent);
  // vertex based scalar (sv = scalar vertex). make it "persistent" so it
  // appears in graphics dumps.
  register_state(b, "sv", vertices, real_t, persistent);
  // cell based vector (gsc = gradient scalar cell)
  register_state(b, "gsc", cells, vector_t, persistent);
  // analytic answer for vertex based vector
  register_state(b, "ans_gsc", cells, vector_t, persistent);

  // access the state
  auto sc = access_state(b, "sc", real_t);
  auto sv = access_state(b, "sv", real_t);
  auto gsc = access_state(b, "gsc", vector_t);
  auto ans_gsc = access_state(b, "ans_gsc", vector_t);

  // go over cells and initialize cell data and answer
  const real_t pi = 3.1415;
  for(auto c: b.cells()) {
    auto xc = c->centroid();
    sc[c] = sin(pi*xc[0]) * cos(pi*xc[1]);
    ans_gsc[c][0] = pi * cos(pi*xc[0]) * cos(pi*xc[1]);
    ans_gsc[c][1] = -pi * sin(pi*xc[0]) * sin(pi*xc[1]);
    gsc[c] = 0.0;
  } // for

  // go over vertices to initialize sv
  for(auto v: b.vertices()) {
    sv[v] = 0.0;
  } // for

  // go over vertices to get vertex centered average stored in sv
  for(auto v: b.vertices()) {
    sv[v] = interpolate_to_vertex(b, sc, v);
  } // for

  // iterate over cells.
  for (auto c: b.cells()) {
    // get the area of the cell from the provided function
    real_t area = c->area();

    // punt on cells on the boundary
    bool bdry = false;
    for (auto v: b.vertices(c)) { if (b.wedges(v).size() < 8) bdry = true; }
    if (bdry) continue;

    // go over wedges of the cell
    for (auto w: b.wedges(c)) {
      auto Ni = w->cell_facet_normal();
      // get the wedge's vertex to look up the vertex value
      auto v = w->vertex();
      gsc[c] += Ni * sv[v]/area;
    } // for
  } // for

  // write the mesh
  std::string name("cell_gradient.exo");
  ASSERT_FALSE(write_mesh(name, b));

}

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
