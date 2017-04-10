/*~--------------------------------------------------------------------------~*
 *  @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
 * /@@/////  /@@          @@////@@ @@////// /@@
 * /@@       /@@  @@@@@  @@    // /@@       /@@
 * /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
 * /@@////   /@@/@@@@@@@/@@       ////////@@/@@
 * /@@       /@@/@@//// //@@    @@       /@@/@@
 * /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
 * //       ///  //////   //////  ////////  //
 *
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_utils_factory_h
#define flecsi_utils_factory_h

#include <map>
#include <iostream>

namespace flecsi {
namespace utils {

///
/// \class Factory_ Factory.h
/// \brief Factory_ provides a generic object factory class.
///
template <typename T, typename K, typename... Args>
class Factory_
{
public:

  /// Function pointer type for creation method.
  using createHandler = T * (*)(Args... args);

  /// Map key type.
  using key_t = K;

  /// Map type to associate ids and creation methods.
  using map_t = std::map<key_t, createHandler>;

  /// Constructor (hidden)
  Factory_(const Factory_ & f) = delete;

  /// Assignment operator (hidden)
  Factory_ & operator=(const Factory_ &) = delete;

  ///
  /// Return an instance of the Factory_ class. This uses a
  /// Meyer's singleton.
  ///
  static Factory_ & instance()
  {
    static Factory_ f;
    return f;
  } // instance

  ///
  /// Register a type with the factory.
  ///
  /// \param id The integer id to associate with this type.  This id
  /// will be used to lookup the creation method.
  /// \param ch The handler to call to create a new type associated
  /// with \emph id.
  ///
  bool registerType(key_t key, createHandler ch)
  {
    return map_.insert(typename map_t::value_type(key, ch)).second;
  } // registerType

  ///
  /// Create a new instance of the type associated with \emph id.

  /// \param id The integer id of the type to create.
  /// \param args The arguments to pass to the creation method.

  /// Note: The argument list uses a variadic template parameter to
  /// forward the arguments to the creation method.  This does not
  /// mean that the creation method needs to have support for
  /// variadic argument lists.  The std::forward method will
  /// expand the arguments appropriately.
  ///
  /// Note: Also note the use of the rvalue reference, "Args &&".
  /// This allows support of any create signature with perfect
  /// forwarding of the arguments.  Cool!!!  The improved type
  /// inference in C++0x means that the signature can be automatically
  /// detected from where this method is called.
  ///
  /// If you try to use this passing lvalues for the args, the compiler
  /// will complain that it can't convert from an lvalue to an rvalue
  /// reference.  If this happens, just wrap the argument in std::move(),
  /// like: factory_t::instance().create(id, std::move(my_lvalue_arg))
  ///
  /// This will statically cast the lvalue to an rvalue reference and
  /// everything will be good.
  ///
  T * create(key_t key, Args && ... args)
  {
    // lookup the create class
    typename map_t::const_iterator ita = map_.find(key);

    // make sure that the id exists in the map
    if (ita == map_.end()) {
      std::cerr << "Error Unknown Type ID" << std::endl;
      std::exit(1);
    } // if

    // call the constructor method
    return (ita->second(std::forward<Args>(args)...));
  } // create

private:

  map_t map_;

  /// Constructor
  Factory_() {}
  /// Destructor
  ~Factory_() {}

}; // class Factory_

} // namespace utils
} // namespace flecsi

#endif // flecsi_utils_factory_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
