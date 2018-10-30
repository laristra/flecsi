# Policy-Based Design

FleCSI makes heavy use of policy types to specialize the behavior of
higher-level interfaces. Simply put, a *policy* type is a template
parameter that is used to customize the behavior of a *wrapper* or
*host* type (these terms are interchangeable.

Consider this simple definition of a *host* type that takes a *policy*
that specializes the *execute* method:
```cpp
template<typename POLICY>
struct host__ : public POLICY
{
  decltype(auto) execute() {
    return POLICY::execute();
  } // execute
}; // struct host__
```
This pattern is something like a static version of dynamic polymorphism,
i.e., the *host* type defines an interface that is implemented by the
*policy*, much in the same way that a derived class implements a *pure*
virtual interface from a base class. In this analogy, the host class
plays the role of the base class, and the policy plays the role of the
derived class. By construction, if the policy type does not implement
the *execute* method in our example, a compiler error will be generated.
This is similar to the error that would arise if one attempts to
instantiated a derived class that does not implement a pure virtual
function.

*Aside: In the case of the policy type, there is no formal rule that
requires this, as the policy and host types are not formally associated
with each other. Future versions of C++ may have a language feature
called *concepts* that foramlizes the notion of static polymorphism.*

One thing to notice about this example is that the host type inherits
from the policy type. This is a very useful feature of the policy design
pattern! In particular, it allows the policy to define aggregate data
members that it might need for its implementation, *and they will
automatically be carried with the host type instance.* This allows
policies to add state that is transparent to the user of the host type.
FleCSI often uses this feature of policy design to carry state that is
required by a particular backend, e.g., the Legion context carries
handshake and process maps for MPI interoperability that are not
required by other runtime backends.
```cpp
struct policy_t
{
  double exucute() {
    // ...do something with data_ and return a copy
    return data_;
  } // execute

private:

  double data_;

}; // struct policy_t
```
Another feature of this design pattern is that policies can add to the
public interface of the host type. This is useful in cases where the
policy type is known in some part of the code, and the *hidden* policy
interface can be used to apply runtime-specific execution. An example of
this feature is the MPI interoperability interface methods in the Legion
context.
```cpp
struct policy_t
{
  double & data() {
    return data_;
  }

private:

  double data_;

}; // struct policy_t
```

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
