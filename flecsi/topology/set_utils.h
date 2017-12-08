/*~--------------------------------------------------------------------------~*
 *~--------------------------------------------------------------------------~*/

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Oct 03, 2017
//----------------------------------------------------------------------------//

#ifndef flecsi_topology_set_utils_h
#define flecsi_topology_set_utils_h

namespace flecsi{
namespace topology{

template<
  size_t INDEX,
  class TUPLE,
  class ENTITY
>
struct find_set_index_space__
{
  static
  constexpr
  size_t
  find()
  {
    using TUPLE_ELEMENT = typename std::tuple_element<INDEX - 1, TUPLE>::type;
    using INDEX_SPACE = typename std::tuple_element<0, TUPLE_ELEMENT>::type;
    using ELEMENT_ENTITY = typename std::tuple_element<1, TUPLE_ELEMENT>::type;

    return std::is_same<ELEMENT_ENTITY, ENTITY>::value ? INDEX_SPACE::value : 
      find_set_index_space__<INDEX - 1, TUPLE, ENTITY>::find();
  }

};

template<
  class TUPLE,
  class ENTITY
>
struct find_set_index_space__<0, TUPLE, ENTITY>
{

  static
  constexpr
  size_t
  find()
  {
    assert(false && "failed to find index space");
    return 1; 
  }

};

template<
  size_t INDEX,
  class TUPLE,
  class MAP_TYPE
>
struct map_set_index_spaces__
{
  static
  constexpr
  size_t
  map(
    MAP_TYPE& m
  )
  {
    using TUPLE_ELEMENT = typename std::tuple_element<INDEX - 1, TUPLE>::type;
    using INDEX_SPACE = typename std::tuple_element<0, TUPLE_ELEMENT>::type;

    m[INDEX_SPACE::value] = INDEX - 1;

    return map_set_index_spaces__<INDEX - 1, TUPLE, MAP_TYPE>::map(m);
  }
};

template<
  class TUPLE,
  class MAP_TYPE
>
struct map_set_index_spaces__<0, TUPLE, MAP_TYPE>
{
  static
  constexpr
  size_t
  map(
    MAP_TYPE& m
  )
  {
    return 0; 
  }
};

} // namespace topology
} // namespace flecsi

#endif // flecsi_topology_set_utils_h

/*~-------------------------------------------------------------------------~-*
 *~-------------------------------------------------------------------------~-*/
