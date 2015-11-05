<!-- CINCHDOC DOCUMENT(User Guide) SECTION(State Model) -->

# State Model

From the user's point of view, the \flexi{} state model is extremely
easy to use.  Users can register state of any normal C++ type.  This includes
P.O.D. (plain-old-data) types and user-defined types.

<!-- CINCHDOC DOCUMENT(Developer Guide) SECTION(State Model) -->

# State Model

The \flexi{} state model \texttt{state\_t} is an interface class that
provides a standard interface to policy-based C++ implementations called
storage policies.  Allowing for different storage policies allows \flexi{}
to target many different low-level runtime data managers.
