<!-- CINCHDOC DOCUMENT(User Guide) SECTION(Data Model) -->

# Data Model

From the user's point of view, the \flecsi{} data model is extremely
easy to use.  Users can register data of any normal C++ type.  This includes
P.O.D. (plain-old-data) types and user-defined types.

<!-- CINCHDOC DOCUMENT(Developer Guide) SECTION(Data Model) -->

# Data Model

The \flecsi{} data model \texttt{data\_t} is an interface class that
provides a standard interface to policy-based C++ implementations called
storage policies.  Allowing for different storage policies allows \flecsi{}
to target many different low-level runtime data managers.
