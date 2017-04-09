# Object Factory

An object factory is a design pattern that allows a program to have
multiple registered handlers for different types. As a motivating
example, consider a program that must be able to read in a variety of
image formats:

```
int main(int argc, char ** argv) {

  std::string suffix = get_suffix(argv[1]);

  base_io_handler_t * handler{nullptr};

  if(suffix == "gif") {
    handler = new gif_io_handler_t(argv[1]);
  }
  else if(suffix == "jpeg") {
    handler = new jpeg_io_handler_t(argv[1]);
  }
  else if(suffix == "png") {
    handler = new png_io_handler_t(argv[1]);
  } // if

  handler->render();

  return 0;
} // main
```

This code is tedious to maintain for at least two reasons:

* It explicitly collects all of the types that can be handled by the
  program into a single logic block. This means that every time a new
  format is added, we must go back to this code and add a case for it.
* The composing object must include all of the type information for the
  various handlers. This may not be desirable or possible in all cases,
  or may simply violate the project's design.

A much better mechanism is to use an object factory:

```
int main(int argc, char ** argv) {

  base_io_handler_t * handler = factory_t::instance().create_handler(argv[0]);
      
  handler->render();

} // main
```

The code section below shows the factory type that makes this possible.
There are three primary properties of the factory type that are
important to this design pattern:

1. The factory is a singleton, and can be called from external scope to
  register the various handler types. In this example, we are using the
  Meyer's singleton discussed in the last section.
2. The factory exposes an interface to register a callback function for
  each suffix type. We will see how this can be called from each handler
  type's implementation to avoid having to collect all of the handlers
  together in one place.
3. The factory exposes a creation method that uses the filename's suffix
  to lookup the correct callback function to create a handler of the
  appropriate type.

These are enumerated in the code comments in the following section:

```
struct factory_t
{

  using callback_t =
    std::function<base_io_handler_t *(const std::string &)>;

  // 1. Meyer's singleton instance method.
  static
  factory_t &
  instance()
  {
    static factory_t s;
    return s;
  } // instance

  // 2. Callback registration method.
  bool
  register_handler(
    const std::string suffix,
    callback_t & callback
  )
  {
    if(registry_.find(suffix) == registry_.end()) {
      registry_[suffix] = callback;
    } // if
  } // register_handler

  // 3. Handler creation method.
  base_io_handler_t *
  create_handler(
    const std::string & filename
  )
  {
    const std::string suffix = get_suffix(filename);

    assert(registry_.find(suffix) != registry_.end());

    return registry_[suffix](filename);
  } // create_handler

  factory_t(const factory_t &) = delete;
  factory_t & operator = (const factory_t &) = delete;

private:

  std::unordered_map<std::string, callback_t> registry_;

  factory_t() {}
  ~factory_t() {}

}; // struct factory_t
```

Using the *factory_t* interface, we can register a particular handler
like this:

```
// 1. Define derived handler class (in this case for GIF images).
struct gif_io_handler_t : public base_io_handler_t
{
  gif_io_handler_t(
    const std::string & filename
  )
  {
    // Initialize handler and read GIF image.
  } // gif_io_handler_t

}; // struct gif_io_handler_t

// 2. Define a function to create a new instance of gif_io_handler_t.
base_io_handler_t *
create_gif_io_handler(
  const std::string & filename
)
{
  return new gif_io_handler_t(filename);
} // create_gif_io_handler

// 3. Register the handler callback function with the object factory.
bool gif_io_handler_registered =
  factory_t::instance().register_handler("gif", create_gif_io_handler);
```

Notice that the logic used to register a new handler can be called from
within the file that defines it. This is extremely useful in maintaining
code because there is no single place where all of the handlers must be
known. The factory keeps track of this for us with its *registry_* map.
Now, if the developer wants to add a new type, they must only define the
type, deriving from the base *base_io_handler_t* type, and register it.

<!-- vim: set tabstop=2 shiftwidth=2 expandtab fo=cqt tw=72 : -->
