<!-- CINCHDOC DOCUMENT(User Guide) SECTION(Data Model) -->

# Data Model

From the user's point of view, the FleCSI data model is extremely
easy to use.  Users can register data of any normal C++ type.  This includes
P.O.D. (plain-old-data) types and user-defined types.

<!-- CINCHDOC DOCUMENT(Developer Guide) SECTION(Data Model) -->

# Data Model

The FleCSI data model provides an intuitive high-level user interface
that can be specialized using different low-level storage types. Each
storage type provides data registration, data accessors, and data
mutators (when appropriate) that allow the user to modify data values
and structure. The currently supported storage types are:

* **dense**  
  This storage type provides a one-dimensional dense array suitable for
  storing structured or unstructured data.

* **sparse**  
  This storage type provides compressed storage for a logically dense
  index space.

* **global**  
  This storage type is suitable for storing data that are
  non-enumerable, i.e., data that are not logically stored as an array.

* **local**  
  This storage type is designed to provide scratch space that does not
  need to be managed by the runtime.

* **tuple**  
  This storage type provides c-struct-like support so that task
  arguments can be associated without forcing a particular data layout.

--------------------------------------------------------------------------------

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
