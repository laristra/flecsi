# Meyer's Singleton

A singleton is a type instance for which there is one, and only one,
object in the system.

```
    struct singleton_t
    {

      static
      singleton_t &
      instance()
      {
        static singleton_t s;
        return s;
      } // instance

      singleton_t(const singleton_t &) = delete;
      singleton_t & operator = (const singleton_t &) = delete;

    private:

      singleton_t() {}
      ~singleton_t() {}

    }; // struct singleton_t
```

Attributed to Scott Meyers, this singleton pattern exploits three
important properties:

* Static function objects are initialized when control flow hits the
  function for the first time.
* The lifetime of function static variables begins the first time the
  program flow encounters the declaration and ends at program
  termination.
* If control enters the declaration concurrently while the variable is
  being initialized, the concurrent execution shall wait for completion
  of the initialization.

**Note:** It is important to disable both the copy constructor and
assignment operator. The constructor and destructor are not exposed
through the public interface.

Using this pattern guarantees that the single type instance is available
at any point during execution, and that it will be properly destroyed.
This pattern also insures thread-safe initialization. However, there are
pontential thread safety issues with data members of this singleton type.
These are discussed at length in *Modern C++ Design: Generic Programming
and Design Patterns Applied* by Andrei Alexandrescu.

More explanations about singletons in C++ are
[here](http://stackoverflow.com/questions/1008019/c-singleton-design-pattern).

Here is a more verbose description of static storage duration from the
C++ standard:

The zero-initialization of all block-scope variables with static storage
duration or thread storage duration is performed before any other
initialization takes place. Constant initialization of a block-scope
entity with static storage duration, if applicable, is performed before
its block is first entered. An implementation is permitted to perform
early initialization of other block-scope variables with static or
thread storage duration under the same conditions that an implementation
is permitted to statically initialize a variable with static or thread
storage duration in namespace scope. Otherwise such a variable is
initialized the first time control passes through its declaration; such
a variable is considered initialized upon the completion of its
initialization. If the initialization exits by throwing an exception,
the initialization is not complete, so it will be tried again the next
time control enters the declaration. If control enters the declaration
concurrently while the variable is being initialized, the concurrent
execution shall wait for completion of the initialization. If control
re-enters the declaration recursively while the variable is being
initialized, the behavior is undefined.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
