# Design Pattern: Tuple Walker

A tuple is a fix-sized collection of hetergeneous values. C++ has had
support for tuples since C++11 through the
[std::tuple](http://en.cppreference.com/w/cpp/utility/tuple) type. In
addition to the tuple type itself, C++ provides utilities for creating
and accessing tuples:

```
    // Make a tuple of type <double, std::string, int>.
    auto t = std::make_tuple(1.0, "a string", 10);

    // Get the zero'th element of the tuple 't'.
    auto e = std::get<0>(t);

    // Get the type of the zero'th element.
    using type = typename std::tuple_element<0, decltype(t)>::type;
```

One common use for tuples is to capture type and runtime information
from a variadic argument list:

```
    template<
      size_t KEY,
      typename ARG_TUPLE,
      typename ... ARGS
    >
    static
    decltype(auto)
    execute_task(
      launch_type_t launch,
      size_t parent,
      ARGS && ... args
    )
    {
      // Create a tuple of the arguments to a user task.
      ARG_TUPLE task_args = std::make_tuple(args ...);
    } // execute_task
```

In the above example, we create a tuple instance from a variadic list of
arguments that are to be passed to a user-defined task (FleCSI infers
the tuple type when the task is registered.) As an example, consider the
following:

```
double task(int a, double x, double y) {
  if(a > 0) {
    return x*y;
  }
  else {
    return 0.0;
  } // if
} // task

flecsi_register_task(task, loc, single);

int main() {
  flecsi_execute_task(task, single, 1, 2.0, 3.0);
} // main
```

In this case, the variadic argument list is \<int, double, double\>.
Suppose that we need to perform additional checks or processing on a
particular argument type, e.g., the doubles must be non-negative. We can
do this by applying a handle function to the argument tuple as it passes
through the runtime:

```
  init_args_t init_handles;
  init_args.walk(args);
```

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
