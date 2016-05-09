/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_TaskWrapper_h
#define flecsi_TaskWrapper_h
//#define __cplusplus 201103L

//#include "flecsi/execution/context.h"
//#include  <legion/legion.h>
#include <tuple>
#include <utility>
#include <stddef.h>
#include <typeinfo>
#include <functional>
#include "flecsi/execution/context.h"

namespace flecsi
{

template<typename T>
class state_handle_t
{
public:
	typedef T stored_t;

	stored_t data;

};



template<typename T>
class state_accessor_t
{
public:
	typedef T stored_t;
	stored_t data;

};

class element_t
{

};




/*
 * @class state_handle_sep
 * @brief Class to ensure that all arguments are state_handle_t types
 */
template<class... A>
class state_handle_sep
{
public:
	typedef typename std::tuple<A...> handles;
};

template<class T0,template<class>class old_sh>
class state_handle_sep<std::tuple<old_sh<T0>>>
{
public:
	typedef typename state_handle_sep<state_handle_t<T0>>::handles handles;

};

template<class T0, class... TN, template<class>class old_sh>
class state_handle_sep<std::tuple<old_sh<T0>,TN...>>
{
public:
	typedef typename state_handle_sep<std::tuple<TN...>,state_handle_t<T0>>::handles handles;

};

template<class T0, class ...TN, template<class>class old_sh, class... SH>
class state_handle_sep<std::tuple<old_sh<T0>,TN...>,SH...>
{
public:
	typedef typename state_handle_sep<std::tuple<TN...>,SH...,state_handle_t<T0>>::handles handles;

};

template<class T0, template<class>class old_sh, class... SH>
class state_handle_sep<std::tuple<old_sh<T0>>,SH...>
{
public:
	typedef typename  state_handle_sep<SH...,state_handle_t<T0>>::handles handles;

};

template<class... A>
class ArgSplitter
{

};

template<class sArg0,class... rest>
class ArgSplitter<sArg0,rest...>
{
public:
	typedef ArgSplitter<std::tuple<sArg0>,rest...> NEXT;
	typedef typename NEXT::sArgT sArgT;
	typedef typename NEXT::aArgT aArgT;
	typedef typename NEXT::Tail Tail;
};

template<class sArg0>
class ArgSplitter<sArg0>
{
public:
	typedef ArgSplitter<std::tuple<sArg0>> NEXT;
	typedef typename NEXT::sArgT sArgT;
	typedef typename NEXT::aArgT aArgT;
	typedef typename NEXT::Tail Tail;
};

template<class sArg0, class... sArgs,class... rest>
class ArgSplitter<std::tuple<sArgs...>,sArg0,rest...>
{
public:
	typedef ArgSplitter<std::tuple<sArgs...,sArg0>,rest...> NEXT;
	typedef typename NEXT::sArgT sArgT;
	typedef typename NEXT::aArgT aArgT;
	typedef typename NEXT::Tail Tail;

};

template<bool qElement,class... sArgs>
class ArgSplitter_end
{

};

template<bool qElement,class... sArgs,class... aArgs>
class ArgSplitter_end<qElement,std::tuple<sArgs...>,std::tuple<aArgs...>>
{
public:
	typedef std::tuple<sArgs...> sArgT;
	typedef std::tuple<aArgs...> aArgT;

	template<std::size_t N>
	using type_s = typename std::tuple_element<N,std::tuple<sArgs...>>::type;

	template<std::size_t N>
	using type_a = typename std::tuple_element<N,std::tuple<aArgs...>>::type;

	static const bool needs_element = qElement;

	sArgT sargs;
	aArgT aargs;
};

template<class... sArgs>
class ArgSplitter_end<false,std::tuple<sArgs...>>
{
public:
	typedef std::tuple<sArgs...> sArgT;
	typedef void aArgT;

	template<std::size_t N>
	using type_s = typename std::tuple_element<N,std::tuple<sArgs...>>::type;

	template<std::size_t N>
	using type_a = std::false_type;

	static const bool needs_element = false;

	sArgT sargs;
};


template<class ... sArgs,class ... aArgs>
class ArgSplitter<std::tuple<sArgs...>,element_t,aArgs...>
{
public:
	typedef std::tuple<sArgs...> sArgT;
	typedef std::tuple<aArgs...> aArgT;
	typedef ArgSplitter_end<true,std::tuple<sArgs...>,std::tuple<aArgs...>> Tail;

};

template<class ... sArgs,class sType0,class ... aArgs>
class ArgSplitter<std::tuple<sArgs...>,state_accessor_t<sType0>,aArgs...>
{
public:
	typedef std::tuple<sArgs...> sArgT;
	typedef std::tuple<state_accessor_t<sType0>,aArgs...> aArgT;
	typedef ArgSplitter_end<false,std::tuple<sArgs...>,std::tuple<state_accessor_t<sType0>,aArgs...>> Tail;
};


template<class ... sArgs>
class ArgSplitter<std::tuple<sArgs...>>
{
public:
	typedef std::tuple<sArgs...> sArgT;
	typedef void aArgT;

	typedef ArgSplitter_end<false,std::tuple<sArgs...>> Tail;

};

template<class T>
class FunctionWrapper
{

};

template<class rVal,class... args>
class FunctionWrapper<rVal(args...)>
{
public:
	typedef ArgSplitter<args...> split_t;

};

template<bool isSingle,bool isIndex,int mapperID,bool isLeaf,class execution_policy_t,class callable>
class TaskWrapper
{

};

template<bool isSingle,bool isIndex,int mapperID,bool isLeaf,
		class execution_policy_t,
		class R,class... Args,template<class,class...>class callable>
class TaskWrapper<isSingle,isIndex,mapperID,isLeaf,execution_policy_t,callable<R(Args...)>>
{
public: // Required Flecsi members

	typename execution_policy_t::context_ep context;



	using wrapper_t = FunctionWrapper<R(Args...)>;
	using argsT = typename wrapper_t::split_t::Tail;
	using sArgT = typename argsT::sArgT;
	using aArgT = typename argsT::aArgT;

//	using hArgT = typename std::enable_if<!std::is_void<aArgT>::value,typename state_handle_sep<aArgT>::handles>::type;

	template<typename T,bool Enable>
	class hArgT_temp{};

	template<typename T>
	class hArgT_temp<T,true>
	{
		typedef typename state_handle_sep<T>::handles type;
		type handles;

	};

	template<typename T>
	class hArgT_temp<T,false>
	{
		typedef void* type;
	};

	using hArgT = hArgT_temp<aArgT,!std::is_void<aArgT>::value>;

	hArgT handles;

	sArgT const_args;
	template<typename U = std::is_void<aArgT>,typename = typename std::enable_if<!U::value>>
	TaskWrapper(hArgT _handles,sArgT _const_args,context_t<execution_policy_t> _context) :
		handles(_handles),const_args(_const_args),context(_context){}

	template<typename U = std::is_void<aArgT>,typename = typename std::enable_if<U::value>>
	TaskWrapper(sArgT _const_args,context_t<execution_policy_t> _context) :
		const_args(_const_args),context(_context){}



public: // Required static members
	static const int SINGLE = isSingle;
	static const int INDEX = isIndex;
	static const int MAPPER_ID = mapperID;
	static const bool IS_LEAF = isLeaf;
	static inline const char* TASK_NAME()
	{return typeid(TaskWrapper<isSingle,isIndex,mapperID,isLeaf,execution_policy_t,callable<R(Args...)>>).name();};
	static const size_t TASK_ID()
	{return typeid(TaskWrapper<isSingle,isIndex,mapperID,isLeaf,execution_policy_t,callable<R(Args...)>>).hash_code();}

public: // Task Simple Arguments


  void evaluate()
  {

  }


private:

}; // class TaskWrapper

} // namespace flecsi

#endif // flecsi_TaskWrapper_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
