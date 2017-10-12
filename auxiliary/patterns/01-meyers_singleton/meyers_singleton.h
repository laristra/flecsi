/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef singleton_h
#define singleton_h

#include <iostream>

class singleton_t
{
public:

  // Function static instance method with local static variable.
  static
  singleton_t &
  instance()
  {
    static singleton_t ms;
    return ms;
  } // instance

  // Trivial method to print "Hello World"
  void
  print()
  {
    std::cout << "Hello World" << std::endl;
  } // print

  // Copy constructor (disabled)
  singleton_t(const singleton_t &) = delete;

  // Assignment operator (disabled)
  singleton_t & operator = (const singleton_t &) = delete;

private:

  // Default constructor
  singleton_t() {}

  // Destructor
  ~singleton_t() {}

}; // class singleton_t

#endif // singleton_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
