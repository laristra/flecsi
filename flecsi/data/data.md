<!-- CINCHDOC DOCUMENT(user-guide) SECTION(data-model) -->

# Data Model

From the user's point of view, the FleCSI data model is extremely easy
to use.  Users can register data of any normal C++ type.  This includes
P.O.D. (plain-old-data) types and user-defined types.

## Data Clients

## Field Data

## Handles

## Permissions

In order for the FleCSI runtime to infer distributed-memory data
dependencies that must be updated during task execution, the user must
specify what permissions are required for each client or field handle
type for each task. Currently, permissions are specified as
template-style arguments to the parameters in the task signature:

```
  void task(mesh<ro> m, field<rw, rw, ro> f) { // ...
```

The permissions specifiers have the following meanings:

* **ro [read-only]**<br>  
  Read-only access implies that ghost values will be updated if
  necessary before the user task is executed.

* **wo [write-only]**<br>  
  Write-only access implies that shared values will be updated if
  necessary after the user task is executed.

* **rw [read-write]**<br>  
  Read-write access implies that ghost values will be updated if
  necessary before the user task is executed, and that the shared values
  will be updated if necessary after the user tash is executed.

## Storage Classes

The FleCSI data model provides an intuitive high-level user interface
that can be specialized using different low-level storage types. Each
storage type provides data registration, data handles, and data
mutators (when appropriate) that allow the user to modify data values
and structure. The currently supported storage types are:

* **dense**<br>  
  This storage type provides a one-dimensional dense array suitable for
  storing structured or unstructured data.

* **sparse**<br>  
  This storage type provides compressed storage for a logically dense
  index space.

* **global**<br>  
  This storage type is suitable for storing data that are
  non-enumerable, i.e., data that are not logically stored as an array.

* **color**<br>  
  This storage type is suitable for storing non-enumerable data on a
  *per-color* basis.

* **tuple**<br>  
  This storage type provides c-struct-like support so that task
  arguments can be associated without forcing a particular data layout.

## Example

```
flecsi_register_data_client(mesh_t, hydro, m);
flecsi_register_field(mesh_t, hydro, pressure, double, dense,
  1, cells);

void update_pressure(mesh<ro> m, field<rw, rw, ro> p) {
  
  for(auto c: m.cells(owned)) {
    p(c) = 2.0*p(c);
  } // for

} // update_pressure

```

--------------------------------------------------------------------------------

<!-- CINCHDOC DOCUMENT(developer-guide) SECTION(data-model) -->

# Data Model

The FleCSI data model provides an intuitive high-level user interface
that can be specialized using different low-level storage types. Each
storage type provides data registration, data handles, and data
mutators (when appropriate) that allow the user to modify data values
and structure. The currently supported storage types are:

* **dense**<br>  
  This storage type provides a one-dimensional dense array suitable for
  storing structured or unstructured data.

* **sparse**<br>  
  This storage type provides compressed storage for a logically dense
  index space.

* **global**<br>  
  This storage type is suitable for storing data that are
  non-enumerable, i.e., data that are not logically stored as an array.

* **color**<br>  
  This storage type is suitable for storing non-enumerable data on a
  *per-color* basis.

* **tuple**<br>  
  This storage type provides c-struct-like support so that task
  arguments can be associated without forcing a particular data layout.

--------------------------------------------------------------------------------

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
