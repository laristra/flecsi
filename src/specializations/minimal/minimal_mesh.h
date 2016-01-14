/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_minimal_mesh_h
#define flecsi_minimal_mesh_h

#include "flecsi/mesh/mesh_topology.h"
#include "flecsi/specializations/minimal/minimal_mesh_types.h"

/*!
 * \file minimal_mesh.h
 * \authors bergen
 * \date Initial file creation: Dec 26, 2015
 */

namespace flecsi {

/*!
  \class minimal_mesh minimal_mesh.h
  \brief minimal_mesh provides...
 */
class minimal_mesh_t
{
private:

  using mesh_t = mesh_topology_t<minimal_mesh_types_t>;

public:

  /*!
   */
  void init() {
    mesh_.init();
  } // init

private:

  mesh_t mesh_;

}; // class minimal_mesh_t

} // namespace flecsi

#endif // flecsi_minimal_mesh_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
