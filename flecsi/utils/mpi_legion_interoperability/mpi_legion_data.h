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
#include "flecsi/execution/mpilegion_execution_policy.h"
namespace flecsi
{
namespace mpilegion
{

class MPILegionArrayStorage_t
{
  public:
  MPILegionArrayStorage_t(){};
  ~MPILegionArrayStorage_t(){};
  virtual void mpi_init(void) = 0;
  virtual void legion_init(context_t<mpilegion_execution_policy_t>  &ctx) = 0;
  virtual void  allocate_legion(context_t<mpilegion_execution_policy_t>  &ctx) = 0;
  virtual void  deallocate_legion(context_t<mpilegion_execution_policy_t>  &ctx) = 0;
  virtual void  partition_legion( context_t<mpilegion_execution_policy_t> &ctx) = 0;
  virtual uint64_t size(void) = 0;
  virtual void copy_legion_to_mpi (context_t<mpilegion_execution_policy_t>  &ctx) = 0;
   virtual void copy_mpi_to_legion (context_t<mpilegion_execution_policy_t>  &ctx) = 0;
  virtual void dump_legion(const std::string &prefix,
                  int64_t nle,
                  context_t<mpilegion_execution_policy_t>  &ctx) =0;
  virtual void dump_mpi(const std::string &prefix) = 0;
};

template <typename Type,  uint64_t N>
class MPILegionArray : public MPILegionArrayStorage_t{

 public:
  MPILegionArray(){};

 private: 
  bool mpi_allocated=true;
  
 public:

 LogicalArray<Type> legion_object;
 std::array<Type,N> mpi_object;

 void  allocate_legion(context_t<mpilegion_execution_policy_t>  &ctx)
       {
           legion_object.allocate(N, ctx.legion_ctx(),
                                     ctx.runtime());
      }

 void  deallocate_legion(context_t<mpilegion_execution_policy_t>  &ctx)
       {
           legion_object.deallocate( ctx.legion_ctx(),
                                     ctx.runtime());
       }

 void  partition_legion( context_t<mpilegion_execution_policy_t> &ctx)
       {
         legion_object.partition(N, ctx.legion_ctx(), ctx.runtime());
       }

// void allocate_mpi(void)
//      {
//        mpi_allocated=true;
//        //Type *mpi_object=new Type[numberOfElements];
//      }


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

 Type *  mpi_accessor(void)
       {
        assert (mpi_allocated);
         return mpi_object.data();
       }

  Type mpi_ptr (void)
        {
          return *mpi_object;
        }

  Type legion_ptr (void)
        {
          return *legion_object.legion_accessor;
        }

  uint64_t size(void){ return N;}

  void copy_legion_to_mpi (context_t<mpilegion_execution_policy_t>  &ctx)
  {
    RegionRequirement req(
            legion_object.logicalRegion, READ_ONLY, EXCLUSIVE, legion_object.logicalRegion);
   req.add_field(legion_object.fid);
   InlineLauncher accessorl(req);
   PhysicalRegion preg= ctx.runtime()->map_region(ctx.legion_ctx(),accessorl);
   preg.wait_until_valid();

   LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, Type, Type> acc=
     preg.get_field_accessor(legion_object.fid).template typeify<Type>();

    //TOFIX: should we use pointers for an mpi_object here?
   int count =0;
   for(GenericPointInRectIterator<1> pir(legion_object.bounds); pir; pir++){
    mpi_object[count] = acc.read(DomainPoint::from_point<1>(pir.p));
   count++;
   }
   ctx.runtime()->unmap_region(ctx.legion_ctx(),preg);
  }

  void copy_mpi_to_legion (context_t<mpilegion_execution_policy_t>  &ctx)
  {

   using namespace LegionRuntime::HighLevel;
   using namespace LegionRuntime::Accessor;
   using LegionRuntime::Arrays::Rect;
   RegionRequirement req(
            legion_object.logicalRegion, WRITE_DISCARD, EXCLUSIVE, legion_object.logicalRegion);
   req.add_field(legion_object.fid);
   InlineLauncher accessorl(req);
   PhysicalRegion preg= ctx.runtime()->map_region(ctx.legion_ctx(),accessorl);
   preg.wait_until_valid();
   
   LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, Type, Type> acc=
     preg.get_field_accessor(legion_object.fid).template typeify<Type>();


   //TOFIX: should we use pointers for an mpi_object here?
     int count=0;
     for(GenericPointInRectIterator<1> pir(legion_object.bounds); pir; pir++){
          acc.write(DomainPoint::from_point<1>(pir.p), mpi_object[count]);
          count++;
     }
    ctx.runtime()->unmap_region(ctx.legion_ctx(),preg);
  }

 void dump_legion(const std::string &prefix,
                  int64_t nle,
                  context_t<mpilegion_execution_policy_t>  &ctx)
 {
  
   legion_object.dump(prefix, nle, ctx.legion_ctx(), ctx.runtime());
 }

 void dump_mpi (const std::string &prefix)
  {
    Type *temp=mpi_object.data();
    std::cout <<prefix<< "  " <<std::endl;
      for (int i=0; i<N;i++)
         std::cout << temp[i] <<std::endl;
  }

 void mpi_init(Type &init_value)
 {
    Type *temp=mpi_object.data();
    for (int i=0; i<N ;i++)
      temp[i]=init_value;
  }

 void mpi_init( void )
 {
    Type *temp=mpi_object.data();
    for (int i=0; i<N ;i++)
      temp[i]=Type(0);
  }

 void legion_init(Type &init_value, 
         context_t<mpilegion_execution_policy_t>  &ctx)
 {
   using namespace LegionRuntime::HighLevel;
   using namespace LegionRuntime::Accessor;
   using LegionRuntime::Arrays::Rect;
   RegionRequirement req(
            legion_object.logicalRegion, WRITE_DISCARD, EXCLUSIVE, legion_object.logicalRegion);
   req.add_field(legion_object.fid);
   InlineLauncher accessorl(req);
   PhysicalRegion preg= ctx.runtime()->map_region(ctx.legion_ctx(),accessorl);
   preg.wait_until_valid();

   LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, Type, Type> acc=
     preg.get_field_accessor(legion_object.fid).template typeify<Type>();
   for(GenericPointInRectIterator<1> pir(legion_object.bounds); pir; pir++){
      acc.write(DomainPoint::from_point<1>(pir.p), init_value);
    }   
    ctx.runtime()->unmap_region(ctx.legion_ctx(),preg);
  }

  void legion_init( context_t<mpilegion_execution_policy_t>  &ctx)
 {
   using namespace LegionRuntime::HighLevel;
   using namespace LegionRuntime::Accessor;
   using LegionRuntime::Arrays::Rect;
   RegionRequirement req(
            legion_object.logicalRegion, WRITE_DISCARD, EXCLUSIVE, legion_object.logicalRegion);
   req.add_field(legion_object.fid);
   InlineLauncher accessorl(req);
   PhysicalRegion preg= ctx.runtime()->map_region(ctx.legion_ctx(),accessorl);
   preg.wait_until_valid();

   LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, Type, Type> acc=
     preg.get_field_accessor(legion_object.fid).template typeify<Type>();
   for(GenericPointInRectIterator<1> pir(legion_object.bounds); pir; pir++){
      acc.write(DomainPoint::from_point<1>(pir.p), Type(0));
    }
    ctx.runtime()->unmap_region(ctx.legion_ctx(),preg);
  }


};

}//end namespace mpilegion

}//end namespace flecsi


#endif
/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
