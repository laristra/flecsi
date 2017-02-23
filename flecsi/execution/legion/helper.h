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

#ifndef flecsi_execution_legion_helper_h
#define flecsi_execution_legion_helper_h

#include "legion.h"
#include "arrays.h"

///
/// \file legion/helper.h
/// \authors nickm
/// \date Initial file creation: Nov 29, 2016
///

namespace flecsi {
namespace execution {

  ///
  /// FIXME documentation
  ///
  class legion_helper{
  public:
    legion_helper(Legion::Runtime* runtime, Legion::Context context)
    : runtime_(runtime),
    context_(context){}

    // structured
    Legion::IndexSpace create_index_space(unsigned start, unsigned end){
      assert(end >= start);
      LegionRuntime::Arrays::Rect<1> 
        rect(LegionRuntime::Arrays::Point<1>(start),
             LegionRuntime::Arrays::Point<1>(end - 0));
      return runtime_->create_index_space(context_, Legion::Domain::from_rect<1>(rect));  
    }

    Legion::DomainPoint domain_point(size_t p){
      return Legion::DomainPoint::from_point<1>(
        LegionRuntime::Arrays::make_point(p));
    }

    Legion::Domain domain_from_point(size_t p){
      LegionRuntime::Arrays::Rect<1> 
        rect(LegionRuntime::Arrays::Point<1>(p),
             LegionRuntime::Arrays::Point<1>(p - 0));
      return Legion::Domain::from_rect<1>(rect);
    }

    Legion::Domain domain_from_rect(size_t start, size_t end){
      LegionRuntime::Arrays::Rect<1> 
        rect(LegionRuntime::Arrays::Point<1>(start), LegionRuntime::Arrays::Point<1>(end - 0));
      return Legion::Domain::from_rect<1>(rect);
    }

    // unstructured
    Legion::IndexSpace create_index_space(size_t n) const{
      assert(n > 0);
      return runtime_->create_index_space(context_, n);  
    }

    Legion::FieldSpace create_field_space() const{
      return runtime_->create_field_space(context_);
    }

    Legion::FieldAllocator
    create_field_allocator(Legion::FieldSpace fs) const{
      return runtime_->create_field_allocator(context_, fs);
    }

    Legion::LogicalRegion
    create_logical_region(Legion::IndexSpace is,
                          Legion::FieldSpace fs) const{
      return runtime_->create_logical_region(context_, is, fs);
    }

/*
    Legion::Future execute_task(Legion::TaskLauncher l) const{
      return runtime_->execute_task(context_, l);
    }
*/

    Legion::Domain get_index_space_domain(Legion::IndexSpace is) const{
      return runtime_->get_index_space_domain(context_, is);
    }

    Legion::DomainPoint domain_point(size_t i) const{
      return Legion::DomainPoint::from_point<1>(
        LegionRuntime::Arrays::Point<1>(i)); 
    }

    Legion::FutureMap execute_index_space(Legion::IndexLauncher l) const{
      return runtime_->execute_index_space(context_, l);
    }

    Legion::IndexAllocator create_index_allocator(
      Legion::IndexSpace is) const{
      return runtime_->create_index_allocator(context_, is);
    }

    Legion::Domain get_domain(Legion::PhysicalRegion pr) const{
      Legion::LogicalRegion lr = pr.get_logical_region();
      Legion::IndexSpace is = lr.get_index_space();
      return runtime_->get_index_space_domain(context_, is);     
    }
    
     ///
     /// FIXME documentation
     ///
    template<class T>
    void get_buffer(Legion::PhysicalRegion pr,
                    T*& buf,
                    size_t field = 0) const{
      auto ac = pr.get_field_accessor(field).typeify<T>();
      Legion::Domain domain = get_domain(pr); 
      LegionRuntime::Arrays::Rect<1> r = domain.get_rect<1>();
      LegionRuntime::Arrays::Rect<1> sr;
      LegionRuntime::Accessor::ByteOffset bo[1];
      buf = ac.template raw_rect_ptr<1>(r, sr, bo);
    }

     ///
     /// FIXME documentation
     //
    char* get_raw_buffer(Legion::PhysicalRegion pr, size_t field = 0) const{
      auto ac = pr.get_field_accessor(field).typeify<char>();
      Legion::Domain domain = get_domain(pr); 
      LegionRuntime::Arrays::Rect<1> r = domain.get_rect<1>();
      LegionRuntime::Arrays::Rect<1> sr;
      LegionRuntime::Accessor::ByteOffset bo[1];
      return ac.template raw_rect_ptr<1>(r, sr, bo);
    }

     ///
     /// FIXME documentation
     //
    void unmap_region(Legion::PhysicalRegion pr) const{
      runtime_->unmap_region(context_, pr);
    }

  private:
    mutable Legion::Runtime* runtime_;
    mutable Legion::Context context_;
  };

} // namespace execution 
} // namespace flecsi

#endif // flecsi_execution_legion_helper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
