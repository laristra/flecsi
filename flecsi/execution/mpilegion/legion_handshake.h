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

#ifndef flecsi_execution_mpilegion_legion_handshake_h
#define flecsi_execution_mpilegion_legion_handshake_h

#include <iostream>
#include <string>
#include <cstdio>
#include <mutex>
#include <condition_variable>

#include <mpi.h>
#include <legion.h>
#include <realm.h>

///
// \file mpilegion/legion_handshake.h
// \authors demeshko
// \date Initial file creation: Jul 2016
///

///
// the main idea of the handshake is change from MPI to Legion and vice
//    versa  through locking/unlocking threads's mutex
//    the order should be like next
// 
//   handshake->legion_init();
//   .. call legion tasks 
//   handshake->mpi_wait_on_legion();
//   handshake->mpi_init();
//   handshake->legion_handoff_to_mpi();
//   .. do some MPI staff
//   handshake.legion_wait_on_mpi();
//   handshake->mpi_handoff_to_legion();
//   .. do some legion execution
//   handshake->mpi_wait_on_legion();
//   handshake->legion_handoff_to_mpi();
///

#define CHECK_PTHREAD(cmd) do { \
  int ret = (cmd); \
  if(ret != 0) { \
    fprintf(stderr, "PTHREAD: %s = %d (%s)\n", #cmd, ret, strerror(ret)); \
    exit(1); \
  } \
} while(0)

namespace flecsi{
namespace execution{

class ext_legion_handshake_t
{
private:
  ///
  // constructor
  // it is private since ext_legion_handshake_t is a singleton
  ///
  ext_legion_handshake_t() {}

  ///
  // destructor
  ///
  ~ext_legion_handshake_t()
    {
    } // ~ext_legion_handshake_t

  ///
  // copy constructor
  ///
  ext_legion_handshake_t(ext_legion_handshake_t const &);     

  ///
  // assign operator as well as copy constructor should be private
  // to restric their usage
  ///
  ext_legion_handshake_t & operator=(ext_legion_handshake_t const &);

public:


  ///
  // getting unique instance of the singleton
  ///
  static
  ext_legion_handshake_t &
  instance()
  {
    static ext_legion_handshake_t hs;
    return hs;
  } // instance

	///
	// this method initializes all ext_legion_handshake_t with input and default 
	//   values
	//   state - is where ext_legion_handshake_t object is originally created:
	//      true - in MPI
	//      with 1 MPi and 1 Legion participant
  ///
  void
  initialize(
  )
  {
    handshake_ = Legion::Runtime::create_handshake(true/*MPI initial 
                                                          control*/,
                                        1/*MPI participants*/,
                                        1/*Legion participants*/);
	} // initialize

	/// 
  //  This method switches form MPI to Legion runtime
	///
  void
	mpi_handoff_to_legion()
	{
    handshake_.mpi_handoff_to_legion();
	} //ext_handoff_to_legion

	/// 
	//	waiting on all Legion tasks to complete and all legion threads 
	// switch mutex to EXT
	///
  void
	mpi_wait_on_legion()
  {
    handshake_.mpi_wait_on_legion();
  }

  /// 
  //This method switches form Legion to MPI runtime
  ///
  void
	legion_handoff_to_mpi()
	{
    handshake_.legion_handoff_to_mpi();
	}//legion_handoff_to_ext
 
	///
	//	waiting on all mutex to be switch to Legion
	/// 
  void 
	legion_wait_on_mpi()
	{
     handshake_.legion_wait_on_mpi();
	} // legion_wait_on_ext

public:

   std::function<void()> shared_func_;
   bool call_mpi_=false;
   int rank_;
 
protected:

  Legion::MPILegionHandshake handshake_;

}; // ext_legion_handshake_t


/*--------------------------------------------------------------------------*/

} // namespace execution
} // namespace flecsi

#endif // flecsi_execution_mpilegion_legion_handshake_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/

