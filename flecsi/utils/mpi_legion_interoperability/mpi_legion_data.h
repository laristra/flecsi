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
#ifndef MPI_LEGION_DATA_HPP
#define MPI_LEGION_DATA_HPP

#include <iostream>
#include <string>
#include <cstdio>
#include <condition_variable>

#include "legion.h"
#include "realm.h"

#include "flecsi/utils/mpi_legion_interoperability/legion_arrays.h"

template <typename Type>
class MPILegionArray{

 public:
  MPILegionArray();
  ~MPILegionArray(){ delete[] mpi_object;}

 public:

 LogicalArray<Type> legion_object;
 Type *mpi_object;

 void  allocate_legion(int64_t nElems,
        LegionRuntime::HighLevel::Context &ctx,
        LegionRuntime::HighLevel::HighLevelRuntime *lrt)
       {
           legion_object.allocate(nElems, ctx, lrt);
       }
 void  partition_legion(int64_t nElems,
        LegionRuntime::HighLevel::Context &ctx,
        LegionRuntime::HighLevel::HighLevelRuntime *lrt)
       {
         legion_object.partition(nElems, ctx, lrt);
       }

 void allocate_mpi(int64_t nElems)
      {
        Type *mpi_object=new Type[nElems];
      }

 Type legion_accessor(const PhysicalRegion &physicalRegion,
                      Context ctx,
                      HighLevelRuntime *runtime)
      {
        PhysicalArray<Type> PS (physicalRegion, ctx, runtime);
        return PS.data();
 //       Type *data = PS.data();
  //      assert (data);
  //      return data;
      }

 Type mpi_accessor(void)
       {
         return mpi_object;
       }

  Type mpi_ptr (void)
        {
          return *mpi_object;
        }

  Type legion_ptr (void)
        {
          return *legion_object.legion_accessor;
        }

  void copy_legion_to_mpi (LegionRuntime::HighLevel::Context &ctx,
          HighLevelRuntime *runtime)
  {
   auto *acc=legion_object.get_accessor(READ_ONLY, EXCLUSIVE, ctx, runtime);
   for(GenericPointInRectIterator<1> pir(legion_object.bounds); pir; pir++)
    *mpi_object++ = acc.read(DomainPoint::from_point<1>(pir.p));
  }

  void copy_mpi_to_legion (LegionRuntime::HighLevel::Context &ctx,
          HighLevelRuntime *runtime)
  {
     auto *acc=legion_object.get_accessor(WRITE_DISCARD, EXCLUSIVE, ctx, runtime);
     for(GenericPointInRectIterator<1> pir(legion_object.bounds); pir; pir++)
          acc.write(DomainPoint::from_point<1>(pir.p), *mpi_object++);    

  }

};




#endif
/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
