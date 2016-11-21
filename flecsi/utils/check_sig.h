/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_check_sig_h
#define flecsi_check_sig_h

/*!
 * \file check_sig.h
 * \authors payne
 * \date Initial file creation: Feb 22, 2016
 */
#include <typeinfo>
#include <type_traits>

namespace flecsi {

/*!
  \class check_sig check_sig.h
  \brief check_sig provides...
 */
template< typename T,class B>
struct check_sig
{

};

template< typename T,class rVal,class ...Args>
struct check_sig<T,rVal(*)(Args...)>
{
	/* SFINAE foo-has-correct-sig :) */
	template<typename A>
	static std::true_type test(rVal (A::*)(Args...)) {
		return std::true_type();
	}



	/* SFINAE foo-exists :) */
	template<typename A>
	static decltype(test(&A::operator()))
	test(decltype(&A::operator()),void*) {
		/* foo exists. What about sig? */
		typedef decltype(test(&A::operator())) return_type;
		return return_type();
	}

	/* SFINAE game over :( */
	template<typename A>
	static std::false_type test(...) {
		return std::false_type();
	}

	/* This will be either `std::true_type` or `std::false_type` */
	typedef decltype(test<T>(0,0)) type;

	static const bool value = bool(type::value); /* Which is it? */

	/*  `eval(T const &,std::true_type)`
		delegates to `T::foo()` when `type` == `std::true_type`
	*/
	static rVal eval(T  & t, std::true_type,Args... ar) {
		return t(ar...);
	}
	/* `eval(...)` is a no-op for otherwise unmatched arguments */
	template<class... Args2>
	static rVal eval(T  & t, std::false_type,Args2... ar){
		// This output for demo purposes. Delete
		std::cout << typeid(T).name() << "::operator() not called" << std::endl;
		return rVal();
	}

	/* `eval(T const & t)` delegates to :-
		- `eval(t,type()` when `type` == `std::true_type`
		- `eval(...)` otherwise
	*/
	static rVal eval(T& t,Args... ar) {
		return eval(t,type(),ar...);
	}
};

// Dummy function for generating specific function sigs.
template<
  class rT,
  class ... Args
>
rT
DummyFoo(
  Args ... args
)
{
  return rT();
} // DummyFoo

} // namespace flecsi

#endif // flecsi_check_sig_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
