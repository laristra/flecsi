#ifndef structured_querytable_h
#define structured_querytable_h

#include <array>
#include <vector>
#include<iostream>
#include <type_traits>

namespace flecsi {
namespace topology{
namespace query{

template<size_t MD>
struct QueryUnit
{
  size_t                        box_id;
  size_t                        numchk;
  std::array<size_t, MD>           dir;
  std::array<size_t, MD>       bnd_chk;
  std::array<std::intmax_t, MD> offset;
};

template<size_t MD>
struct QuerySequence
{
  std::vector<QueryUnit<MD>> adjacencies;
  size_t size(){return adjacencies.size();};
};

template<size_t MD, size_t MAXFD, size_t MAXIN, size_t MAXTD>
struct QueryTable
{
  QuerySequence<MD> entry[MAXFD][MAXIN][MAXTD];
}; 

void qtable(QueryTable<1,2,1,2> *qt)
{
  //QueryTable<1,2,1,2> *qt = new QueryTable<1,2,1,2>();

  size_t FD = 2, IN = 1, TD = 2;

  for (size_t i=0; i<FD; i++)
  for (size_t j=0; i<IN; i++)
  for (size_t k=0; i<TD; i++)
    qt->entry[i][j][k] = QuerySequence<1>();

  //V-->E
  qt->entry[0][0][1].adjacencies.push_back({0,1,{0},{1},{0}});
  qt->entry[0][0][1].adjacencies.push_back({0,1,{0},{0},{-1}});

  //E-->V
  qt->entry[1][0][0].adjacencies.push_back({0,0,{},{},{0}});
  qt->entry[1][0][0].adjacencies.push_back({0,0,{},{},{1}});

  //return qt;
};

void qtable(QueryTable<2,3,2,3> *qt)
{
  //QueryTable<2,3,2,3> *qt = new QueryTable<2,3,2,3>();

  size_t FD = 3, IN = 2, TD = 3;

  for (size_t i=0; i<FD; i++)
  for (size_t j=0; i<IN; i++)
  for (size_t k=0; i<TD; i++)
    qt->entry[i][j][k] = QuerySequence<2>();

  //V-->E
  qt->entry[0][0][1].adjacencies.push_back({1,1,{0,0},{1,0},{0,0}});
  qt->entry[0][0][1].adjacencies.push_back({0,1,{1,0},{1,0},{0,0}});
  qt->entry[0][0][1].adjacencies.push_back({1,1,{0,0},{0,0},{-1,0}});
  qt->entry[0][0][1].adjacencies.push_back({0,1,{1,0},{0,0},{0,-1}});

  //V-->F
  qt->entry[0][0][2].adjacencies.push_back({0,2,{0,1},{1,1},{0,0}});
  qt->entry[0][0][2].adjacencies.push_back({0,2,{0,1},{0,1},{-1,0}});
  qt->entry[0][0][2].adjacencies.push_back({0,2,{0,1},{0,0},{-1,-1}});
  qt->entry[0][0][2].adjacencies.push_back({0,2,{0,1},{1,0},{0,-1}});

  //E-->V
  qt->entry[1][0][0].adjacencies.push_back({0,0,{},{},{0,0}});
  qt->entry[1][0][0].adjacencies.push_back({0,0,{},{},{0,1}});
      
  qt->entry[1][1][0].adjacencies.push_back({0,0,{},{},{0,0}});
  qt->entry[1][1][0].adjacencies.push_back({0,0,{},{},{1,0}});

  //E-->F
  qt->entry[1][0][2].adjacencies.push_back({0,1,{0,0},{1,0},{0,0}});
  qt->entry[1][0][2].adjacencies.push_back({0,1,{0,0},{0,0},{-1,0}});
      
  qt->entry[1][1][2].adjacencies.push_back({0,1,{1,0},{1,0},{0,0}});
  qt->entry[1][1][2].adjacencies.push_back({0,1,{1,0},{0,0},{0,-1}});

  //F-->V
  qt->entry[2][0][0].adjacencies.push_back({0,0,{},{},{0,0}});
  qt->entry[2][0][0].adjacencies.push_back({0,0,{},{},{1,0}});
  qt->entry[2][0][0].adjacencies.push_back({0,0,{},{},{1,1}});
  qt->entry[2][0][0].adjacencies.push_back({0,0,{},{},{0,1}});

  //F-->E
  qt->entry[2][0][1].adjacencies.push_back({1,0,{},{},{0,0}});
  qt->entry[2][0][1].adjacencies.push_back({0,0,{},{},{1,0}});
  qt->entry[2][0][1].adjacencies.push_back({1,0,{},{},{0,1}});
  qt->entry[2][0][1].adjacencies.push_back({0,0,{},{},{0,0}});

  //return qt;
};

void qtable(QueryTable<3,4,3,4> *qt)
{
  //QueryTable<3,4,3,4> *qt = new QueryTable<3,4,3,4>();

  size_t FD = 4, IN = 3, TD = 4;

  for (size_t i=0; i<FD; i++)
  for (size_t j=0; i<IN; i++)
  for (size_t k=0; i<TD; i++)
    qt->entry[i][j][k] = QuerySequence<3>();

  //V-->E
  qt->entry[0][0][1].adjacencies.push_back({0,1,{1,0,0},{1,0,0},{0,0,0}});
  qt->entry[0][0][1].adjacencies.push_back({0,1,{1,0,0},{0,0,0},{0,-1,0}});
  qt->entry[0][0][1].adjacencies.push_back({1,1,{0,0,0},{1,0,0},{0,0,0}});
  qt->entry[0][0][1].adjacencies.push_back({1,1,{0,0,0},{0,0,0},{-1,0,0}});
  qt->entry[0][0][1].adjacencies.push_back({2,1,{2,0,0},{1,0,0},{0,0,0}});
  qt->entry[0][0][1].adjacencies.push_back({2,1,{2,0,0},{0,0,0},{0,0,-1}});

  //V-->F
  qt->entry[0][0][2].adjacencies.push_back({0,2,{1,2,0},{1,1,0},{0,0,0}});
  qt->entry[0][0][2].adjacencies.push_back({0,2,{1,2,0},{0,1,0},{0,-1,0}});
  qt->entry[0][0][2].adjacencies.push_back({0,2,{1,2,0},{0,0,0},{0,-1,-1}});
  qt->entry[0][0][2].adjacencies.push_back({0,2,{1,2,0},{1,0,0},{0,0,-1}});
  qt->entry[0][0][2].adjacencies.push_back({1,2,{0,2,0},{1,1,0},{0,0,0}});
  qt->entry[0][0][2].adjacencies.push_back({1,2,{0,2,0},{0,1,0},{-1,0,0}});
  qt->entry[0][0][2].adjacencies.push_back({1,2,{0,2,0},{0,0,0},{-1,0,-1}});
  qt->entry[0][0][2].adjacencies.push_back({1,2,{0,2,0},{1,0,0},{0,0,-1}});
  qt->entry[0][0][2].adjacencies.push_back({2,2,{0,1,0},{1,1,0},{0,0,0}});
  qt->entry[0][0][2].adjacencies.push_back({2,2,{0,1,0},{0,1,0},{-1,0,0}});
  qt->entry[0][0][2].adjacencies.push_back({2,2,{0,1,0},{0,0,0},{-1,-1,0}});
  qt->entry[0][0][2].adjacencies.push_back({2,2,{0,1,0},{1,0,0},{0,-1,0}});

  //V-->C
  qt->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
  qt->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{0,1,1},{-1,0,0}});
  qt->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{0,0,1},{-1,-1,0}});
  qt->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{1,0,1},{0,-1,0}});
  qt->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{1,1,0},{0,0,-1}});
  qt->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{0,1,0},{-1,0,-1}});
  qt->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{0,0,0},{-1,-1,-1}});
  qt->entry[0][0][3].adjacencies.push_back({0,3,{0,1,2},{1,0,0},{0,-1,-1}});


  //E-->V
  qt->entry[1][0][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
  qt->entry[1][0][0].adjacencies.push_back({0,0,{},{},{0,1,0}});

  qt->entry[1][1][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
  qt->entry[1][1][0].adjacencies.push_back({0,0,{},{},{1,0,0}});

  qt->entry[1][2][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
  qt->entry[1][2][0].adjacencies.push_back({0,0,{},{},{0,0,1}});   

  //E-->F
  qt->entry[1][0][2].adjacencies.push_back({2,2,{0,2,0},{1,1,0},{0,0,0}});
  qt->entry[1][0][2].adjacencies.push_back({0,2,{0,2,0},{1,1,0},{0,0,0}});
  qt->entry[1][0][2].adjacencies.push_back({2,2,{0,2,0},{0,1,0},{-1,0,0}});
  qt->entry[1][0][2].adjacencies.push_back({0,2,{0,2,0},{1,0,0},{0,0,-1}});

  qt->entry[1][1][2].adjacencies.push_back({2,2,{1,2,0},{1,1,0},{0,0,0}});
  qt->entry[1][1][2].adjacencies.push_back({1,2,{1,2,0},{1,1,0},{0,0,0}});
  qt->entry[1][1][2].adjacencies.push_back({2,2,{1,2,0},{0,1,0},{0,-1,0}});
  qt->entry[1][1][2].adjacencies.push_back({1,2,{1,2,0},{1,0,0},{0,0,-1}});

  qt->entry[1][2][2].adjacencies.push_back({1,2,{0,1,0},{1,1,0},{0,0,0}});
  qt->entry[1][2][2].adjacencies.push_back({0,2,{0,1,0},{1,1,0},{0,0,0}});
  qt->entry[1][2][2].adjacencies.push_back({1,2,{0,1,0},{0,1,0},{-1,0,0}});
  qt->entry[1][2][2].adjacencies.push_back({0,2,{0,1,0},{1,0,0},{0,-1,0}});

  //E-->C
  qt->entry[1][0][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
  qt->entry[1][0][3].adjacencies.push_back({0,3,{0,1,2},{0,1,1},{-1,0,0}});
  qt->entry[1][0][3].adjacencies.push_back({0,3,{0,1,2},{0,1,0},{-1,0,-1}});
  qt->entry[1][0][3].adjacencies.push_back({0,3,{0,1,2},{1,1,0},{0,0,-1}});

  qt->entry[1][1][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
  qt->entry[1][1][3].adjacencies.push_back({0,3,{0,1,2},{1,0,1},{0,-1,0}});
  qt->entry[1][1][3].adjacencies.push_back({0,3,{0,1,2},{1,0,0},{0,-1,-1}});
  qt->entry[1][1][3].adjacencies.push_back({0,3,{0,1,2},{1,1,0},{0,0,-1}});

  qt->entry[1][2][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
  qt->entry[1][2][3].adjacencies.push_back({0,3,{0,1,2},{0,1,1},{-1,0,0}});
  qt->entry[1][2][3].adjacencies.push_back({0,3,{0,1,2},{0,0,1},{-1,-1,0}});
  qt->entry[1][2][3].adjacencies.push_back({0,3,{0,1,2},{1,0,1},{0,-1,0}});

  //F-->V
  qt->entry[2][0][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
  qt->entry[2][0][0].adjacencies.push_back({0,0,{},{},{0,1,0}});
  qt->entry[2][0][0].adjacencies.push_back({0,0,{},{},{0,1,1}});
  qt->entry[2][0][0].adjacencies.push_back({0,0,{},{},{0,0,1}});

  qt->entry[2][1][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
  qt->entry[2][1][0].adjacencies.push_back({0,0,{},{},{1,0,0}});
  qt->entry[2][1][0].adjacencies.push_back({0,0,{},{},{1,0,1}});
  qt->entry[2][1][0].adjacencies.push_back({0,0,{},{},{0,0,1}});

  qt->entry[2][2][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
  qt->entry[2][2][0].adjacencies.push_back({0,0,{},{},{1,0,0}});
  qt->entry[2][2][0].adjacencies.push_back({0,0,{},{},{1,1,0}});
  qt->entry[2][2][0].adjacencies.push_back({0,0,{},{},{0,1,0}});

  //F-->E
  qt->entry[2][0][1].adjacencies.push_back({0,0,{},{},{0,0,0}});
  qt->entry[2][0][1].adjacencies.push_back({2,0,{},{},{0,1,0}});
  qt->entry[2][0][1].adjacencies.push_back({0,0,{},{},{0,0,1}});
  qt->entry[2][0][1].adjacencies.push_back({2,0,{},{},{0,0,0}});

  qt->entry[2][1][1].adjacencies.push_back({1,0,{},{},{0,0,0}});
  qt->entry[2][1][1].adjacencies.push_back({2,0,{},{},{1,0,0}});
  qt->entry[2][1][1].adjacencies.push_back({1,0,{},{},{0,0,1}});
  qt->entry[2][1][1].adjacencies.push_back({2,0,{},{},{0,0,0}});

  qt->entry[2][2][1].adjacencies.push_back({1,0,{},{},{0,0,0}});
  qt->entry[2][2][1].adjacencies.push_back({0,0,{},{},{1,0,0}});
  qt->entry[2][2][1].adjacencies.push_back({1,0,{},{},{0,1,0}});
  qt->entry[2][2][1].adjacencies.push_back({0,0,{},{},{0,0,0}});

  //F-->C
  qt->entry[2][0][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
  qt->entry[2][0][3].adjacencies.push_back({0,3,{0,1,2},{0,1,1},{-1,0,0}});

  qt->entry[2][1][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
  qt->entry[2][1][3].adjacencies.push_back({0,3,{0,1,2},{1,0,1},{0,-1,0}});

  qt->entry[2][2][3].adjacencies.push_back({0,3,{0,1,2},{1,1,1},{0,0,0}});
  qt->entry[2][2][3].adjacencies.push_back({0,3,{0,1,2},{1,1,0},{0,0,-1}});


  //C-->V
  qt->entry[3][0][0].adjacencies.push_back({0,0,{},{},{0,0,0}});
  qt->entry[3][0][0].adjacencies.push_back({0,0,{},{},{1,0,0}});
  qt->entry[3][0][0].adjacencies.push_back({0,0,{},{},{1,1,0}});
  qt->entry[3][0][0].adjacencies.push_back({0,0,{},{},{0,1,0}});
  qt->entry[3][0][0].adjacencies.push_back({0,0,{},{},{0,0,1}});
  qt->entry[3][0][0].adjacencies.push_back({0,0,{},{},{1,0,1}});
  qt->entry[3][0][0].adjacencies.push_back({0,0,{},{},{1,1,1}});
  qt->entry[3][0][0].adjacencies.push_back({0,0,{},{},{0,1,1}});

  //C-->E
  qt->entry[3][0][1].adjacencies.push_back({1,0,{},{},{0,0,0}});
  qt->entry[3][0][1].adjacencies.push_back({0,0,{},{},{1,0,0}});
  qt->entry[3][0][1].adjacencies.push_back({1,0,{},{},{0,1,0}});
  qt->entry[3][0][1].adjacencies.push_back({0,0,{},{},{0,0,0}});
  qt->entry[3][0][1].adjacencies.push_back({2,0,{},{},{0,0,0}});
  qt->entry[3][0][1].adjacencies.push_back({2,0,{},{},{1,0,0}});
  qt->entry[3][0][1].adjacencies.push_back({2,0,{},{},{1,1,0}});
  qt->entry[3][0][1].adjacencies.push_back({2,0,{},{},{0,1,0}});
  qt->entry[3][0][1].adjacencies.push_back({1,0,{},{},{0,0,1}});
  qt->entry[3][0][1].adjacencies.push_back({0,0,{},{},{1,0,1}});
  qt->entry[3][0][1].adjacencies.push_back({1,0,{},{},{0,1,1}});
  qt->entry[3][0][1].adjacencies.push_back({0,0,{},{},{0,0,1}});

  //C-->F
  qt->entry[3][0][2].adjacencies.push_back({1,0,{},{},{0,0,0}});
  qt->entry[3][0][2].adjacencies.push_back({0,0,{},{},{1,0,0}});
  qt->entry[3][0][2].adjacencies.push_back({1,0,{},{},{0,1,0}});
  qt->entry[3][0][2].adjacencies.push_back({0,0,{},{},{0,0,0}});
  qt->entry[3][0][2].adjacencies.push_back({2,0,{},{},{0,0,0}});
  qt->entry[3][0][2].adjacencies.push_back({2,0,{},{},{0,0,1}});

 // return qt;
};

} //query
} //topology
} //flecsi
#endif
