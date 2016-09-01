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

#include <iostream>
#include <typeinfo>

/*!
 * \file any.h
 * \authors demeshko
 * \date Initial file creation: Aug 29, 2016
 */

namespace flecsi
{
namespace utils
{

 class any_t
 {
  public:
	 any_t()
   {
    holder = nullptr;
   }

	 template<class T>
	 any_t(const T& value)
   {
     holder=new holder_t<T>(value);
   }

   ~any_t()
    {
      delete holder;
      holder = nullptr;
    }

	 any_t(const any_t& rhs)
   {
     holder=rhs.holder ? rhs.holder->copy() : nullptr;
   }

	 any_t& operator=(const any_t& rhs)
   {
     delete holder;
     holder = rhs.holder ? rhs.holder->copy() : nullptr;
      return *this;
   }

	 template<class T>
	 operator T() const
   {
    return dynamic_cast<holder_t<T>&>(*holder).value;
   }

  	template<class T>
  	friend const T& any_cast(any_t& rhs);

	  const std::type_info& get_type() const
    {
      return holder ? holder->get_type() : typeid(void);
    }

 private:
  	class i_holder_t
	 {
	  public:
      virtual ~i_holder_t(){}
      virtual i_holder_t* copy() const = 0;
      virtual const std::type_info& get_type() const = 0;
	  };//class i_holder_t

    template<class T>
    class holder_t : public i_holder_t
    {
	   public:
		   holder_t(const T& value):value(value){}
       holder_t* copy() const{return new holder_t(value);}
       const std::type_info& get_type() const{return typeid(T);}
     public:
        T value;
     };//end class Holder

     i_holder_t* holder; 
     //std::shared_ptr<i_holder_t> holder;
};//class Any




template<class T>
inline
const 
T& any_cast(any_t& rhs)
{
	return dynamic_cast<any_t::holder_t<T>&>(*(rhs.holder)).value;
}





}// namespace utils

} // namespace flecsi

#endif // flecsi_any_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
