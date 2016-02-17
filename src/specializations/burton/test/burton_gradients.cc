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

using vertex_t = burton_mesh_t::vertex_t;
using edge_t = burton_mesh_t::edge_t;
//using cell_t = burton_mesh_t::cell_t;
using point_t = burton_mesh_t::point_t;
using vector_t = burton_mesh_t::vector_t;
using real_t = burton_mesh_t::real_t;

const int W = 20, H = 20;
const real_t xmin = -1.0, xmax = 1.0, ymin = -1.0, ymax = 1.0;
const real_t dx = (xmax-xmin)/real_t(W), dy = (ymax-ymin)/real_t(H);
#if 1
real_t x(size_t i, size_t j) { return xmin + pow(j+1,0.2)*i*dx; }
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

// this may not be a portable way to point to lapacke.h. works on darwin.
#include <ccomplex>
#include <lapacke/lapacke.h>
// compute cell centered average of scalar s. used in test below.
real_t interpolate(mesh_t & m, auto & s, auto & c) {

  // from:
  // http://math.stackexchange.com/questions/828392/spatial-interpolation-for-irregular-grid

  // s(x,y) = a_xx x^2 + a_xy x y  + a_yy y^2 + b_x x + b_y y + c

  // minimize e = axx^2 + axy^2 + ayy^2 by solving the following. Then evaluate
  // the above.

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

  const int N = 10;

  // rhs in b
  real_t b[N]; for (int i=0; i<N; i++) b[i] = 0.0;
  // x,y coords in x, y.
  real_t x[4], y[4];
  int i = 0;
  for (auto v: m.vertices(c)) {
    x[i] = v->coordinates()[0];
    y[i] = v->coordinates()[1];
    b[i+6] = s[v]; // the scalar at the vertices
    i++;
  }

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

  auto cc = c->centroid();

  //s(x,y) = a_xx x^2 + a_xy x y  + a_yy y^2 + b_x x + b_y y + c
  return b[0]*cc[0]*cc[0] + b[1]*cc[0]*cc[1] + b[2]*cc[1]*cc[1]
    + b[3]*cc[0] + b[4]*cc[1] + b[5];
}

TEST_F(Burton, gradients) {

  /*--------------------------------------------------------------------------*
   * Example 1.
   * Vertex centered gradient computed in two loops. The first loop stores
   * the cell centered average in a temporary variable. The second loop
   * computes the gradient.
   *--------------------------------------------------------------------------*/

  {
    // register state
    // vertex based scalar (sv = scalar vertex)
    register_state(b, "sv", vertices, real_t, persistent);
    // cell based scalar (sc = scalar cell)
    register_state(b, "sc", cells, real_t, persistent);
    // vertex based vector (gsv = gradient scalar vector)
    register_state(b, "gsv", vertices, vector_t, persistent);
    
    auto sv = access_state(b, "sv", real_t);
    auto sc = access_state(b, "sc", real_t);
    auto gsv = access_state(b, "gsv", vector_t);

    // go over vertices and initialize vertex data
    const real_t pi = 3.1415;
    for(auto v: b.vertices()) {
      auto xv = v->coordinates();
      sv[v] = sin(pi*xv[0]) * cos(pi*xv[1]);
    } // for
    
    // go over cells to get zone centered average stored in sc
    for(auto c: b.cells()) {
      sc[c] = interpolate(b, sv, c);
    } // for

  // go over vertices and compute gradient using cell centered average
  for(auto v: b.vertices()) {

    // volume of dual mesh surrounding vertex

    /*!
      Area of a quadrilateral. No sides parallel.

                    D
             /-------------\
            /                \
        B  /                   \  C
          /                      \
         /-------------------------\
                    A

        area = 1/2 mag(A X B) + 1/2 mag(C X D)
     */

    // The first vertex is hosed
    std::cerr << "==================\n";
    std::cerr << "vertex: " << v.id() << std::endl;
    real_t area = 0.0;
    for(auto cnr: b.corners(v)) {
      std::cerr << "corner: " << cnr.id() << std::endl;

      // the cell center for the cell containing this corner
      auto xc = b.cells(cnr).to_vec()[0]->centroid();
      auto xv = v->coordinates();
      std::vector<point_t> mp;
      for(auto e: b.edges(cnr)) { // the midpoints of the edges for this corner
        mp.push_back(e->midpoint());
      }

      vector_t A; A[0] = mp[0][0]-xv[0]; A[1] = mp[0][1]-xv[1];
      vector_t B; B[0] = mp[1][0]-xv[0]; B[1] = mp[1][1]-xv[1];
      vector_t C; C[0] = mp[0][0]-xc[0]; C[1] = mp[0][1]-xc[1];
      vector_t D; D[0] = mp[1][0]-xc[0]; D[1] = mp[1][1]-xc[1];
      area += 0.5*(cross_magnitude(A,B) + cross_magnitude(C,D));
    }

    std::cerr << "vertex area: " << area << std::endl;

//    for(auto w: wedges(v)) {
//      auto Si = normal(w, SIDE_FACET);
//      // normal(w, CELL_FACET) is also defined

//      gsv(v) += Si * sc(cell(w))/volume;
//    } // for
  } // for

    // write the mesh
    std::string name("test/ex1.exo");
    ASSERT_FALSE(write_mesh(name, b));
  } // scope



  /*--------------------------------------------------------------------------*
   * Example 4.
   * Vertex centered gradient computed in one loop. The edge centered
   * average is computed in the loop. The se field is not needed. There
   * is an se scalar inside the loop.
   *--------------------------------------------------------------------------*/

  {
//  // register state
//  register_scalar("sv", VERTEX);
//  register_vector("gsv", VERTEX);
//
//  // Do other stuff...
//
//  auto sv = get_scalar("sv");
//  auto gsv = get_vector("gsv");
//
//  for(auto v: vertices()) {
//    auto volume = volume(v);
//
//    for(auto w: wedges(v)) {
//      auto se = 0.0;
//
//      // do we need to iterate over edges of a wedge???
//      for(auto e: edges(w)) {
//        for(auto c: cells(e)) {
//          for(auto vv: vertices(c)) {
//            auto xv = coordinates(vv);
//            se += f(sv(vv), xv, xe);
//          } // for
//        } // for
//      } // for
//
//      auto Si = normal(w, SIDE_FACET);
//      gsv(v) += Si * se/volume;
//    } // for
//  } // for
//
//    // write the mesh
//    std::string name("test/ex4.exo");
//    ASSERT_FALSE(write_mesh(name, b));
  } // scope
}

/*~------------------------------------------------------------------------~--*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
