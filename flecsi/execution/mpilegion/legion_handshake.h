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

#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <string>

#include <legion.h>
#include <mpi.h>
#include <realm.h>

///
/// \file
/// \date Initial file creation: Jul 2016
///

namespace flecsi{
namespace execution{

///
/// \class ext_legion_handshake_t
/// \brief a class that is used to switch between MPI and Legion runtimes
/// the main idea of the ext_legion_handshake is to switch from MPI to 
/// Legion and vice  versa  through using legion's phase barriers
/// 
class ext_legion_handshake_t
{
private:
  ///
  /// constructor
  /// it is private since ext_legion_handshake_t is a singleton
  ///
  ext_legion_handshake_t() {}

  ///
  /// destructor
  ///
  ~ext_legion_handshake_t()
    {
    } // ~ext_legion_handshake_t

  ///
  /// copy constructor
  ///
  ext_legion_handshake_t(ext_legion_handshake_t const &);     

  ///
  /// assign operator as well as copy constructor should be private
  /// to restric their usage
  ///
  ext_legion_handshake_t & operator=(ext_legion_handshake_t const &);

public:

  ///
  /// return unique instance of the singleton
  ///
  static
  ext_legion_handshake_t &
  instance()
  {
    static ext_legion_handshake_t hs;
    return hs;
  } // instance

	///
	/// initialize all ext_legion_handshake_t with input and default 
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
  ///  switch form MPI to Legion runtime
	///
  void
	mpi_handoff_to_legion()
	{
    handshake_.mpi_handoff_to_legion();
	} //ext_handoff_to_legion

	/// 
	///	wait on all Legion tasks to complete and all legion threads 
	/// switch mutex to EXT
	///
  void
	mpi_wait_on_legion()
  {
    handshake_.mpi_wait_on_legion();
  }

  /// 
  ///switch form Legion to MPI runtime
  ///
  void
	legion_handoff_to_mpi()
	{
    handshake_.legion_handoff_to_mpi();
	}//legion_handoff_to_ext
 
	///
	///	wait on all mutex to be switch to Legion
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

