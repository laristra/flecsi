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

#ifndef flecsi_any_h
#define flecsi_any_h

/*!
 * \file any.h
 * \authors demeshko
 * \date Initial file creation: Aug 29, 2016
 */

namespace flecsi
{
namespace utils
{

 class any_t {
  
   struct any_concept_t {
       virtual ~any_concept_t() {}
       virtual const std::type_info& type() const = 0;
   };

   template< typename T > 
   struct any_model_t : any_concept_t {
       any_model_t( const T& t ) : any_object_( t ) {}
       virtual ~any_model_t() {}
       virtual const std::type_info& type() const 
             { 
               return typeid(T); 
             }
     private:
       T any_object_;
   };

   std::shared_ptr<any_concept_t> any_object_;

  public:
   template< typename T > any_t( const T& obj ) :
      any_object_( new any_model_t<T>( obj ) ) {}
  public:
        const std::type_info& type() const { return any_object_->type(); }
};

template <typename T>
inline
T* 
any_cast (any_t* A)
{
  assert (typeid(T)==A->type());
  if (A)
   return reinterpret_cast<T*>(A);
  return nullptr;
}

template <typename T>
inline
const 
T* 
any_cast (const any_t* A)
{
  assert (typeid(T)==A->type());
  if (A)
   return reinterpret_cast<T*>(A);
  return nullptr;
}


}// namespace utils

} // namespace flecsi

#endif // flecsi_any_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
