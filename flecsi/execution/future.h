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

#ifndef FUTURE_H
#define FUTURE_H

#include <legion.h>

namespace flecsi
{
namespace execution
{

/*!
* \file mpilegion/future.h
* \authors demeshko
* \date Initial file creation: Jul 2016
*/
#if 0
class future_t
{
  public:
  future_t(void){};
  ~future_t(void){};
  virtual void wait(void) = 0;
};


template <typename T>
class future__ : public future_t
{
  future__(void){};
  ~future__(void){};
  void wait(void);
  T& operator=(const T &b);
  private:
  T future;
};

template<>
inline
void
future__<LegionRuntime::HighLevel::Future>::
wait(void)
{
  future.get_void_result();
}

template<>
inline
void
future__<LegionRuntime::HighLevel::FutureMap>::
wait(void)
{
  future.wait_all_results();
}

template<>
inline
LegionRuntime::HighLevel::Future&
future__<LegionRuntime::HighLevel::Future>::
operator=(const LegionRuntime::HighLevel::Future &B)
{
  future=B;
}

template<>
inline
LegionRuntime::HighLevel::FutureMap&
future__<LegionRuntime::HighLevel::FutureMap>::
operator=(const LegionRuntime::HighLevel::FutureMap &B)
{
  future=B;
}

#endif

class future_t
{
   class future_concept {
     public:
       virtual ~future_concept() {}
       virtual void wait(void)=0;
      // virtual future_concept& operator=(const future_concept &rhs) =0;
   };

   template< typename T > struct future_model : future_concept {
     public:
        future_model(const T &value):future(value){}
        void wait(void);
  //      future_model& operator=(const T &rhs) {future=rhs;}
     public:
       T future;
   };

   std::shared_ptr<future_concept> future;

  public:
   //future_t(){};
   //~future_t(){}; 
 
   future_t(){};
   
   template< typename T >
   future_t(const T &value):future(new future_model<T>(value))
   {
   } 
 
//   template< typename T > future_t( void ) :
//      future( new future_model<T>( ) ) {}

   void  wait( void ){ future->wait();}

   template< typename T >
   future_t& operator=(const T& rhs)
   { 
    future=rhs;
    return *this;
   }

   future_t& operator=(future_t & rhs)
   {
    future=rhs.future;
    return *this;
   }

};

   template<>
   inline
   void
   future_t::future_model<LegionRuntime::HighLevel::Future>::
   wait(void)
   {
     future.get_void_result();
   }

   template<>
   inline
   void
   future_t::future_model<LegionRuntime::HighLevel::FutureMap>::
   wait(void)
   {
     future.wait_all_results();
   }

}//namespace execute
}//namespace flecsi

#endif
/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
