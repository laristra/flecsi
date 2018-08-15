/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef factory_h
#define factory_h

#include <unordered_map>

class factory_t
{
public:

  // Function static instance method with local static variable.
  static
  factory_t &
  instance()
  {
    static factory_t ms;
    return ms;
  } // instance

  // Define a callback function type.
  using callback_t = std::function<void(size_t)>;

  // Register a callback to an id.
  bool
  register_callback(
    size_t id,
    callback_t & callback
  )
  {
    if(registry_.find(id) == registry_.end()) {
      registry_[id] = callback;
    } // if
  } // register_callback

  // Copy constructor (disabled)
  factory_t(const factory_t &) = delete;

  // Assignment operator (disabled)
  factory_t & operator = (const factory_t &) = delete;

private:

  // Map to store callback funtions.
  std::unordered_map<size_t, callback_t> registry_;

  // Default constructor
  factory_t() {}

  // Destructor
  ~factory_t() {}

}; // class factory_t

#endif // factory_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
