
namespace flecsi {
namespace utils {
namespace detail {

// create index list
template<class T>
constexpr std::make_index_sequence<std::tuple_size<T>::value>
get_indexes( T const& )
{ return {}; }

// actuall call to functions
template<size_t... Is, class Tuple, class F>
void tuple_for_each( std::index_sequence<Is...>, Tuple&& tup, F&& f ) {
  using std::get;
  int unused[] = { 0, ( (void)f(get<Is>(std::forward<Tuple>(tup))), 0 )... };
}




// convenience define
template<typename TupleType>
using __flecsi_integral_constant_t = std::integral_constant<size_t,
   std::tuple_size<typename std::remove_reference<TupleType>::type>::value>;

// Empty sequence call to terminate
template<typename TupleType, typename FunctionType>
void tuple_for_each(TupleType &&, FunctionType,
   __flecsi_integral_constant_t<TupleType>) {}

// Expand each index
template<
   size_t I,
   typename TupleType,
   typename FunctionType,
   typename = typename std::enable_if<I!=std::tuple_size<
      typename std::remove_reference<TupleType>::type>::value>::type
>
void tuple_for_each(TupleType && t, FunctionType f,
   std::integral_constant<size_t, I>) {
      f(std::get<I>(t));
      tuple_for_each(std::forward<TupleType>(t), f,
         std::integral_constant<size_t, I + 1>());
}

} // namespace
} // namespace
} // namespace
