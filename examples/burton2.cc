/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

#include <flexi/specializations/burton.h>

#include <vector>

using namespace std;
using namespace flexi;

using vertex_t = burton_mesh_t::vertex_t;
using edge_t = burton_mesh_t::edge_t;
using cell_t = burton_mesh_t::cell_t;
using point_t = burton_mesh_t::point_t;

int main(int argc, char ** argv) {
  size_t width = 2;
  size_t height = 2;

  burton_mesh_t b;

  vector<vertex_t*> vs;
  
  for(size_t j = 0; j < height + 1; ++j){
    for(size_t i = 0; i < width + 1; ++i){
      auto v = b.create_vertex({double(i), double(j)});
      v->setRank(1);
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

  b.init();

  for(auto v : b.vertices()){
    cout << "----------- vertex: " << v->id() << endl;
  }

  for(auto e : b.edges()){
    cout << "----------- edge: " << e->id() << endl;
  }

  for(auto c : b.cells()){
    cout << "----------- cell: " << c->id() << endl;
    for(auto e : b.sortedEdges(c)){
      cout << "++++ edge of: " << e->id() << endl;
    }
    for(auto w : c->wedges()){
      cout << "++++ wedge of: " << w->id() << endl;
      cout << "### corner of: " << w->corner()->id() << endl;
    }

    for(auto c2 : c->corners()){
      cout << "++++ corner of: " << c2->id() << endl;
      for(auto w : c2->wedges()){
        cout << "++ wedge of: " << w->id() << endl;
        for(auto v : b.vertices(w)){
          cout << "- vertex of: " << v->id() << endl;
        }
      }
    }
  }

  return 0;
} // main

/*~------------------------------------------------------------------------~--*
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
