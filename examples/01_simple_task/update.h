/*~--------------------------------------------------------------------------~*
 * placeholder
 *~--------------------------------------------------------------------------~*/

#ifndef update_h
#define update_h

#include "types.h"

int32_t update(burton_mesh_t & mesh, const accessor_t & p,
  const accessor_t & d, accessor_t & r) {

  for(auto z: mesh.cells()) {
    r[z] = p[z] + d[z];
  } // for

  return 0;
} // driver

#endif // update_h

/*~-------------------------------------------------------------------------~-*
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
