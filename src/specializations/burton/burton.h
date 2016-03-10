/*~--------------------------------------------------------------------------~*
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
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_burton_h
#define flecsi_burton_h

#include <string>

#include "burton_mesh.h"

/*!
 * \file burton.h
 * \authors bergen
 * \date Initial file creation: Nov 03, 2015
 */

/*----------------------------------------------------------------------------*
 * Execution Interface
 *----------------------------------------------------------------------------*/

/*!
  \brief Execute a task using the flecsi task runtime abstraction.

  This macro function will call into the flecsi::execution_t interface
  to execute the given task.  Task arguments will be analyzed and
  forwarded to the low-level runtime.

  \param[in] task The task to execute.  Note that the task must conform
    to the signature required by FleCSI.  FleCSI uses static type
    checking to insure that tasks are compliant.
  \param[in] ... The task input arguments (variadic list).  All arguments
   will be passed to the task when it is invoked.
 */
#define execute(task, ...) \
  flecsi::burton_mesh_t::mesh_execution_t::execute_task(task, ##__VA_ARGS__)

/*----------------------------------------------------------------------------*
 * State Interface
 *----------------------------------------------------------------------------*/

/*!
  \brief Register state with the mesh.

  \param[in] mesh The flecsi mesh instance with which to register the state.
  \param[in] key The string name of the state variable to register,
    e.g., "density".
  \param[in] site The data attachment site on the mesh where the state
    should reside.  Valid attachement sites are documented in
    \ref flecsi::burton_mesh_traits_t.
  \param[in] type A valid C++ type for the registered state.
  \param[in] ... A variadic list of arguments to pass to the initialization
    function of the user-defined meta data type.
 */
#define register_state(mesh, key, site, type, ...) \
  (mesh).register_state_<type>((key),              \
      flecsi::burton_mesh_traits_t::attachment_site_t::site, ##__VA_ARGS__)

/*!
  \brief Access state from a given \e mesh and \e key.

  \param[in] mesh The flecsi mesh instance from which to access the state.
  \param[in] key The string name of the state variable to retrieve.
  \param[in] type The C++ type of the requested state.

  \return An accessor to the state data.
 */
#define access_state(mesh, key, type) \
  (mesh).access_state_<type>(key, mesh.id())

/*!
  \brief Access all state of a given type from a given \e mesh and \e key.

  \param[in] mesh The flecsi mesh instance from which to access the state.
  \param[in] type The C++ type of the requested state.

  \return A std::vector<accessor_t<type>> holding accessors to
    the matching state data.
 */
#define access_type(mesh, type) (mesh).access_type_<type>()

/*!
  \brief Access all state of a given type from a given \e mesh and \e key that
    satisfy the given predicate.

  \param[in] mesh The flecsi mesh instance from which to access the state.
  \param[in] type The C++ type of the requested state.
  \param[in] type A predicate function that returns true or false
    given a state accessor as input.

  \return A std::vector<accessor_t<type>> holding accessors to
    the matching state data.
 */
#define access_type_if(mesh, type, predicate) \
  (mesh).access_type_if_<type>(predicate)

/*!
  \brief Select state variables at a given attachment site.

  Predicate function to select state variables that are defined at
  a specific attachment site.

  \param[in] attachment_site State data must be registered at this site
    to meet this predicate criterium.  Valid attachement sites are
    documented in \ref flecsi::burton_mesh_traits_t.

  \return True if the state is registered at the specified
    attachment site, false, otherwise.
 */
#define is_at(attachment_site)                                            \
  [](const auto & a) -> bool {                                            \
    return a.meta().site ==                                               \
        flecsi::burton_mesh_traits_t::attachment_site_t::attachment_site; \
  }

/*!
  \brief Select persistent state variables at an attachment site.

  Predicate function to select state variables that have been tagged as
  being persistent AND are defined at a specific attachment site.

  \param[in] attachment_site State data must be registered at this site
    to meet this predicate criterium.  Valid attachement sites are
    documented in \ref flecsi::burton_mesh_traits_t.

  \return True if the state is persistent and is registered at
    the specified attachment site, false, otherwise.
 */
#define is_persistent_at(attachment_site)                                   \
  [](const auto & a) -> bool {                                              \
    bitfield_t bf(a.meta().attributes);                                     \
    return a.meta().site ==                                                 \
        flecsi::burton_mesh_traits_t::attachment_site_t::attachment_site && \
        bf.bitsset(persistent);                                             \
  }
/*----------------------------------------------------------------------------*
 * Global State Interface
 *----------------------------------------------------------------------------*/

/*!
  \brief Register state with the mesh.

  \param[in] mesh The flecsi mesh instance with which to register the state.
  \param[in] key The string name of the state variable to register,
    e.g., "density".
  \param[in] type A valid C++ type for the registered state.
  \param[in] ... A variadic list of arguments to pass to the initialization
    function of the user-defined meta data type.
 */
#define register_global_state(mesh_, key, type, ...) \
  (mesh_).register_global_state_<type>((key), ##__VA_ARGS__)


/*!
  \brief Access state from a given \e mesh and \e key.

  \param[in] mesh The flecsi mesh instance from which to access the state.
  \param[in] key The string name of the state variable to retrieve.
  \param[in] type The C++ type of the requested state.

  \return An accessor to the state data.
 */
#define access_global_state(mesh, key, type) \
  (mesh).template access_global_state_<type>((key))

/*!
  \brief Access all state of a given type from a given \e mesh and \e key.

  \param[in] mesh The flecsi mesh instance from which to access the state.
  \param[in] type The C++ type of the requested state.

  \return A std::vector<accessor_t<type>> holding accessors to
    the matching state data.
 */
#define access_global_type(mesh, type) \
  (mesh).access_global_type_<type>()

/*!
  \brief Access all state of a given type from a given \e mesh and \e key that
    satisfy the given predicate.

  \param[in] mesh The flecsi mesh instance from which to access the state.
  \param[in] type The C++ type of the requested state.
  \param[in] type A predicate function that returns true or false
    given a state accessor as input.

  \return A std::vector<accessor_t<type>> holding accessors to
    the matching state data.
 */
#define access_global_type_if(mesh, type, predicate) \
  (mesh).access_global_type_if_<type>(predicate)


/*----------------------------------------------------------------------------*
 * General Interface
 *----------------------------------------------------------------------------*/

/*!
  \brief Return the attributes of a state quantity.

  \param[in] mesh The mesh the state is defined on.
  \param[in] key The state \e key to get the attributes for.
 */
#define state_attributes(mesh, key) (mesh).state_attributes_((key))

#endif // flecsi_burton_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
