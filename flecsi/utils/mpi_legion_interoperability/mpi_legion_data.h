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

#include "flecsi/execution/context.h"
//#include "flecsi/execution/mpilegion/execution_policy.h"
#include "flecsi/utils/mpi_legion_interoperability/legion_arrays.h"

//TOFIX:: this shouldn't be here (should be defined at the context.h)
//namespace flecsi{
//using context_t = context__<mpilegion_context_policy_t>
//}

namespace flecsi
{
namespace mpilegion
{

class MPILegionArrayStorage_t
{
  public:
  MPILegionArrayStorage_t(){};
  ~MPILegionArrayStorage_t(){};
  virtual void mpi_init(void)=0;
  virtual void legion_init(void)=0;
  virtual void allocate_legion(void)=0;
  virtual void deallocate_legion(void)=0;
  virtual void partition_legion(uint nParts)=0;
  virtual uint64_t size(void)=0;
  virtual void copy_legion_to_mpi () = 0;
  virtual void copy_mpi_to_legion (void) = 0;
  virtual void dump_legion(const std::string &prefix,
                  int64_t nle) =0;
  virtual void dump_mpi(const std::string &prefix) = 0;
};

template <typename Type,  uint64_t N>
class MPILegionArray : public MPILegionArrayStorage_t{

 public:
  MPILegionArray(){};
   
 private: 
  bool mpi_allocated=true;
  LegionAccessor<Type> *acc=nullptr;
  
 public:
 LogicalArray<Type> legion_object;
 std::array<Type,N> mpi_object;
 typedef  LegionRuntime::Accessor::RegionAccessor
       < LegionRuntime::Accessor::AccessorType::Generic, Type> legion_acc_type;

 void  allocate_legion(void)
       {
           legion_object.allocate(N, flecsi::context_t::instance().context(),
                                     context_t::instance().runtime());
      }

 void  deallocate_legion(void)
       {
           legion_object.deallocate( context_t::instance().context(),
                                     context_t::instance().runtime());
       }

 void  partition_legion(uint nParts)
       {
         legion_object.partition(nParts, context_t::instance().context(),
                                         context_t::instance().runtime());
       }

// void allocate_mpi(void)
//      {
//        mpi_allocated=true;
//        //Type *mpi_object=new Type[numberOfElements];
//      }

      Type * get_legion_accessor
           (Legion::PrivilegeMode priviledge,
           Legion::CoherenceProperty coherence_property)
      {
        acc=legion_object.get_legion_accessor(priviledge, 
              coherence_property,context_t::instance().context(), 
              context_t::instance().runtime());
        return acc->mData;
      }
  void return_legion_accessor(void)
     {
        legion_object.return_legion_accessor(acc, 
                                             context_t::instance().context(),
                                             context_t::instance().runtime());
     }

 Type *legion_accessor(
      const LegionRuntime::HighLevel::PhysicalRegion &physicalRegion)
      {
        PhysicalArray<Type> PS (physicalRegion,
                                context_t::instance().context(), 
                                context_t::instance().runtime());
        return PS.data();
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

  void copy_legion_to_mpi (void)
  {
    LegionRuntime::HighLevel::RegionRequirement req(
            legion_object.logicalRegion, 
            READ_ONLY, EXCLUSIVE, 
            legion_object.logicalRegion);
   req.add_field(legion_object.fid);
   LegionRuntime::HighLevel::InlineLauncher accessorl(req);
   LegionRuntime::HighLevel::PhysicalRegion preg= 
        context_t::instance().runtime()->map_region(
                        context_t::instance().context(),accessorl);
   preg.wait_until_valid();

   LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, Type, Type> acc=
     preg.get_field_accessor(legion_object.fid).template typeify<Type>();

    //TOFIX: should we use pointers for an mpi_object here?
   int count =0;
   for(GenericPointInRectIterator<1> pir(legion_object.bounds); pir; pir++)
   {
      mpi_object[count] = acc.read(Legion::DomainPoint::from_point<1>(pir.p));
      count++;
   }
   context_t::instance().runtime()->unmap_region(context_t::instance().context(),preg);
  }

  void copy_mpi_to_legion (void)
  {

   LegionRuntime::HighLevel::RegionRequirement req(
            legion_object.logicalRegion,
            WRITE_DISCARD, EXCLUSIVE,
            legion_object.logicalRegion);
   req.add_field(legion_object.fid);
   LegionRuntime::HighLevel::InlineLauncher accessorl(req);
   LegionRuntime::HighLevel::PhysicalRegion preg=
       ctx.runtime()->map_region(ctx.legion_ctx(),accessorl);
   preg.wait_until_valid();
   
   LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, Type, Type> acc=
     preg.get_field_accessor(legion_object.fid).template typeify<Type>();


   //TOFIX: should we use pointers for an mpi_object here?
     int count=0;
     for(GenericPointInRectIterator<1> pir(legion_object.bounds); pir; pir++)
     {
        acc.write(Legion::DomainPoint::from_point<1>(pir.p), mpi_object[count]);
        count++;
     }
    context_t::instance().runtime()->unmap_region(
           context_t::instance().context(),preg);
  }

 void dump_legion(const std::string &prefix,
                  int64_t nle)
 {
  
   legion_object.dump(prefix, nle, context_t::instance().context(), 
                                   context_t::instance().runtime());
 }

 void dump_mpi (const std::string &prefix)
  {
    Type *temp=mpi_object.data();
    std::cout <<prefix<< "  " <<std::endl;
      for (int i=0; i<N;i++)
         std::cout << temp[i] <<std::endl;
  }

 void mpi_init(Type init_value)
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

 void legion_init(Type init_value)
 {
   LegionRuntime::HighLevel::RegionRequirement req(
            legion_object.logicalRegion,
            WRITE_DISCARD, EXCLUSIVE,
            legion_object.logicalRegion);
   req.add_field(legion_object.fid);
   LegionRuntime::HighLevel::InlineLauncher accessorl(req);
   LegionRuntime::HighLevel::PhysicalRegion preg=
      context_t::instance().runtime()->map_region(
                    context_t::instance().context(),accessorl);
   preg.wait_until_valid();

   LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, Type, Type> acc=
     preg.get_field_accessor(legion_object.fid).template typeify<Type>();
   for(GenericPointInRectIterator<1> pir(legion_object.bounds); pir; pir++)
   {
      acc.write( Legion::DomainPoint::from_point<1>(pir.p), init_value);
    }   
    context_t::instance().runtime()->unmap_region(
                     context_t::instance().context(),preg);
  }

  void legion_init(void)
 {
   LegionRuntime::HighLevel::RegionRequirement req(
            legion_object.logicalRegion,
            WRITE_DISCARD, EXCLUSIVE,
            legion_object.logicalRegion);
   req.add_field(legion_object.fid);
   LegionRuntime::HighLevel::InlineLauncher accessorl(req);
   LegionRuntime::HighLevel::PhysicalRegion preg=
      context_t::instance().runtime()->map_region(
                    context_t::instance().context(),accessorl);
   preg.wait_until_valid();

   LegionRuntime::Accessor::RegionAccessor<LegionRuntime::Accessor::AccessorType::Generic, Type, Type> acc=
     preg.get_field_accessor(legion_object.fid).template typeify<Type>();
   for(GenericPointInRectIterator<1> pir(legion_object.bounds); pir; pir++)
   {
      acc.write(Legion::DomainPoint::from_point<1>(pir.p), Type(0));
    }
    context_t::instance().runtime()->unmap_region(context_t::instance().context(),preg);
  }


};

}//end namespace mpilegion

}//end namespace flecsi


#endif
/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
