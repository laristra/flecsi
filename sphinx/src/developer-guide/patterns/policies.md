# Policy-Based Design

FleCSI makes heavy use of policy types to specialize the behavior of
higher-level interfaces. Simply put, a *policy* type is a template
parameter that is used to customize the behavior of a *wrapper* or
*host* type (these terms are interchangeable.

Consider this simple definition of a *host* type that takes a *policy*
that specializes the *execute* method:

```cpp
template<typename POLICY>
struct HOST : public POLICY
{
  decltype(auto) execute() {
    return POLICY::execute();
  } // execute
}; // struct HOST
```

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
