#include <iostream>
#include <vector>

#include <cinchtest.h>
#include "mpi.h"

#include <flecsi/coloring/box_types.h>
#include <flecsi/coloring/simple_box_colorer.h>

using namespace std;

void print_colored_info(flecsi::coloring::box_coloring_t &colbox)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  cout<<"\n\n"<<endl;
  //Print out partition info: need a proper check against a blessed file. 
  if (colbox.primary)
  {
   cout<<"Rank-"<<rank<<":: Partition = { "<<colbox.partition[0].box.lowerbnd[0]<<", "
   <<colbox.partition[0].box.lowerbnd[1]<<" } - { "<<colbox.partition[0].box.upperbnd[0]<<", "
   <<colbox.partition[0].box.upperbnd[1]<<" }, Strides = { "<<colbox.strides[0][0]<<", "
   <<colbox.strides[0][1]<<" }, #GHalo =  "<<colbox.partition[0].nhalo
   <<", #DHalo = " <<colbox.partition[0].nhalo_domain
   <<", thru_dim = " <<colbox.partition[0].thru_dim
   <<", onbnd = [ "<<colbox.partition[0].onbnd[0]<<" "<<colbox.partition[0].onbnd[1]
   <<" "<<colbox.partition[0].onbnd[2]<<" "<<colbox.partition[0].onbnd[3]<<" ]"<<std::endl;
  }

  for (size_t i = 0; i < colbox.num_boxes; i++)
  {
    flecsi::coloring::box_color_t e = colbox.exclusive[i]; 

    cout<<"Rank-"<<rank<<":: Exclusive = { "<<e.domain.box.lowerbnd[0]<<", "
    <<e.domain.box.lowerbnd[1]<<" } - { "<<e.domain.box.upperbnd[0]<<", "
    <<e.domain.box.upperbnd[1]<<"}, tags = [ "<<e.domain.tag[0]<<" "
    <<e.domain.tag[1]<<" "<<e.domain.tag[2]<<" "<<e.domain.tag[3]<<" "
    <<e.domain.tag[4]<<" "<<e.domain.tag[5]<<" "<<e.domain.tag[6]<<" "
    <<e.domain.tag[7]<<" ], colors = "<<e.colors[0]<<endl;

    for (auto s: colbox.shared[i])
    {
      if (!s.domain.box.isempty())
      {
       cout<<"Rank-"<<rank<<":: Shared = { "<<s.domain.box.lowerbnd[0]<<", "
       <<s.domain.box.lowerbnd[1]<<" } - { "<<s.domain.box.upperbnd[0]<<", "
       <<s.domain.box.upperbnd[1]<<"}, tags = [ "<<s.domain.tag[0]<<" "
       <<s.domain.tag[1]<<" "<<s.domain.tag[2]<<" "<<s.domain.tag[3]<<" "
       <<s.domain.tag[4]<<" "<<s.domain.tag[5]<<" "<<s.domain.tag[6]<<" "
       <<s.domain.tag[7]<<" ], colors = { ";
       for (auto c: s.colors)
	cout<<c<<" ";
       cout<<" }"<<endl;
      }
    }

    for (auto g: colbox.ghost[i])
    {
      if (!g.domain.box.isempty())
      {
       cout<<"Rank-"<<rank<<":: Ghost = { "<<g.domain.box.lowerbnd[0]<<", "
       <<g.domain.box.lowerbnd[1]<<" } - { "<<g.domain.box.upperbnd[0]<<", "
       <<g.domain.box.upperbnd[1]<<"}, tags = [ "<<g.domain.tag[0]<<" "
       <<g.domain.tag[1]<<" "<<g.domain.tag[2]<<" "<<g.domain.tag[3]<<" "
       <<g.domain.tag[4]<<" "<<g.domain.tag[5]<<" "<<g.domain.tag[6]<<" "
       <<g.domain.tag[7]<<" ], colors = { ";
       for (auto c: g.colors)
	cout<<c<<" ";
       cout<<" }"<<endl;
      }
    }

    /*
    for (auto d: colbox.domain_halo[i])
    {
       cout<<"Rank-"<<rank<<":: DomainHalo = { "<<d.box.lowerbnd[0]<<", "
       <<d.box.lowerbnd[1]<<" } - { "<<d.box.upperbnd[0]<<", "
       <<d.box.upperbnd[1]<<"}, tags = [ "<<d.tag[0]<<" "
       <<d.tag[1]<<" "<<d.tag[2]<<" "<<d.tag[3]<<" "
       <<d.tag[4]<<" "<<d.tag[5]<<" "<<d.tag[6]<<" "
       <<d.tag[7]<<" ]"<<endl;
    }*/

    cout<<"Rank-"<<rank<<":: Overlay = { "<<colbox.overlay[i].lowerbnd[0]<<", "
    <<colbox.overlay[i].lowerbnd[1]<<" } - { "<<colbox.overlay[i].upperbnd[0]<<", "
    <<colbox.overlay[i].upperbnd[1]<<" }, Strides = { "<<colbox.strides[i][0]<<", "
    <<colbox.strides[i][1]<<" }"<<endl;  

    cout<<"\n"<<endl;
   }
};


TEST(simple_colorer, simpletest2d)
{
  int size;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Input parameters
  size_t grid_size[2] = {10,10};
  size_t ncolors[4]={2,2};
  size_t nhalo = 1;
  size_t nhalo_domain = 1;
  size_t thru_dim = 0;
  flecsi::coloring::simple_box_colorer_t<2> sbc;

  // Color primary entities i.e., cells.
  auto col_cells = sbc.color(grid_size, nhalo, nhalo_domain, thru_dim, ncolors);

  //Print out partition info: need a proper check against a blessed file. 
  print_colored_info(col_cells); 

  // Color dependent entities i.e., vertices and edges
  auto col_depents = sbc.color_dependent_entities(col_cells);
  print_colored_info(col_depents[0]); 
  print_colored_info(col_depents[1]); 
}

