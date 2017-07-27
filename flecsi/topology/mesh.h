/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_topology_mesh_h
#define flecsi_topology_mesh_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Jul 18, 2017
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//! @def flecsi_register_number_dimensions
//!
//! This can be used to set the number of dimensions of the mesh topology.
//!
//! @ingroup topology
//----------------------------------------------------------------------------//

#define flecsi_register_number_dimensions(dimensions)                          \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  static constexpr size_t num_dimensions = dimensions

//----------------------------------------------------------------------------//
//! @def flecsi_register_number_domains
//!
//! This can be used to set the number of topological domains of the mesh.
//!
//! @ingroup topology
//----------------------------------------------------------------------------//

#define flecsi_register_number_domains(domains)                                \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  static constexpr size_t num_domains = domains

//----------------------------------------------------------------------------//
//! @def flecsi_register_entity_types
//!
//! This macro is a convenience interface to std::tuple. It is provided for
//! consistency.
//!
//! @ingroup topology
//----------------------------------------------------------------------------//

#define flecsi_register_entity_types(...)                                      \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  using entity_types = std::tuple<__VA_ARGS__>

//----------------------------------------------------------------------------//
//! @def flecsi_entity_type
//!
//! This macro defines an entity type suitable for populating the
//! \em entity_types parameter for a FleCSI specialization.
//!
//! @ingroup topology
//----------------------------------------------------------------------------//

#define flecsi_entity_type(index, domain, type)                                \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  std::tuple<                                                                  \
    flecsi::topology::index_space_<index>,                                     \
    flecsi::topology::domain_<domain>,                                         \
    type                                                                       \
    >

//----------------------------------------------------------------------------//
//! @def flecsi_register_entity_types
//!
//! This macro is a convenience interface to std::tuple. It is provided for
//! consistency.
//!
//! @ingroup topology
//----------------------------------------------------------------------------//

#define flecsi_register_connectivities(...)                                    \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  using connectivities = std::tuple<__VA_ARGS__>

//----------------------------------------------------------------------------//
//! @def flecsi_connectivity
//!
//! This macro defines a connectivity type suitable for populating the
//! \em connectivities parameter for a FleCSI specialization.
//!
//! @ingroup topology
//----------------------------------------------------------------------------//

#define flecsi_connectivity(index, domain, from_type, to_type)                 \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  std::tuple<                                                                  \
    flecsi::topology::index_space_<index>,                                     \
    flecsi::topology::domain_<domain>,                                         \
    from_type,                                                                 \
    to_type                                                                    \
  >

//----------------------------------------------------------------------------//
//! @def flecsi_register_bindings
//!
//! This macro is a convenience interface to std::tuple. It is provided for
//! consistency.
//!
//! @ingroup topology
//----------------------------------------------------------------------------//

#define flecsi_register_bindings(...)                                          \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  using bindings = std::tuple<__VA_ARGS__>

//----------------------------------------------------------------------------//
//! @def flecsi_binding
//!
//! This macro defines a connectivity type suitable for populating the
//! \em connectivities parameter for a FleCSI specialization.
//!
//! @ingroup topology
//----------------------------------------------------------------------------//

#define flecsi_binding(index, from_domain, to_domain, from_type, to_type)      \
/* MACRO IMPLEMENTATION */                                                     \
                                                                               \
  std::tuple<                                                                  \
    flecsi::topology::index_space_<index>,                                     \
    flecsi::topology::domain_<from_domain>,                                    \
    flecsi::topology::domain_<to_domain>,                                      \
    from_type,                                                                 \
    to_type                                                                    \
  >

#endif // flecsi_topology_mesh_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
