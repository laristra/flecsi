.. |br| raw:: html

   <br />

Data Model
==========

**'flecsi/data'**

* **data.h** |br|
  This file contains the high-level, macro interface for the FleCSI data
  model. This interface is documented in the Doxygen documentation.
  **FIXME: Need link**

* **client.h** |br|
  This file contains the C++ data model interface *client_interface_t*
  for FleCSI topology data structures, which are also referred to as
  *data clients* in this context. The *client_interface_t* type is
  a specialization of the *client_interface_u* type on the backend
  runtime policy that is selected at compile time.

* **field.h** |br|
  This file contains the C++ data model interface *field_interface_t*
  for FleCSI field data structures. The *field_interface_t* type is a
  specialization of the *field_interface_u* type on the backend runtimne
  policy that is selected at compile time.

**'flecsi/data/common'**

* **client_handle.h** |br|
  This file defines the type identifier type *client_handle_base_t*, and
  the basic client handle type *client_handle_u*, which is parameterized
  on the *data client* type. These types provide the basic structure
  for implementing handles to the various FleCSI data client types.

  A handle is a cross-address-space-safe reference type (like a pointer)
  that can be passed into a FleCSI task to allow access to a data client.
  Data clients are types that expose one or more index spaces that can
  have fields registered against them. Some examples are
  *mesh_topology_u*, and *tree_topology_u*.

* **client_handler.h** |br|
  This file defines the base \em client_handler_u type that is customized
  for each specific data client type using partial specialization.

* **client_registration_wrapper.h** |br|
  This file contains specializations of the
  *client_registration_wrapper_u* type for the various FleCSI
  topology types. In general, the registration wrapper provides a
  mechanism for data clients to register meta data with the runtime.

* **client_registration_wrapper.h** |br|
  This file contains the base \em data \em client type identifier
  *data_client_t*. All data client types should inherit from this type.

* **field_registration_wrapper.h** |br|
  This file contains the *field_registration_wrapper_u* type. This type
  is currently used for all field registrations regardless of runtime or
  client type.

* **field_registration_wrapper.h** |br|
  This file defines the base *storage_class_u* type that can be
  specialized by specific storage class, and by data client type.  This
  file also defines the storage classes for the internal *global* and
  *color* client types.

.. vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 :
