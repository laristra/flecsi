#include <iostream>
#include <vector>

#include <cinchtest.h>
#include "mpi.h"

#include <flecsi/coloring/box_types.h>
#include <flecsi/coloring/simple_box_colorer.h>

using namespace std;

TEST(simple_colorer, simpletest2d)
{
  int size;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Input parameters
  size_t grid_size[2] = {10,10};
  size_t ncolors[2]={2,1};
  size_t nhalo = 1;
  size_t nhalo_domain = 1;
  size_t thru_dim = 0;
  flecsi::coloring::simple_box_colorer_t<2> sbc;

  // Color primary entities i.e., cells.
  auto col_cells = sbc.color(grid_size, nhalo, nhalo_domain, thru_dim, ncolors);

  //Print out partition info: need a proper check against a blessed file. 
  cout<<"Rank-"<<rank<<"::Partition Box:LBND  = { "<<col_cells.partition[0].box.lowerbnd[0]<<", "
                                                   <<col_cells.partition[0].box.lowerbnd[1]<<" } "<<endl;

  cout<<"Rank-"<<rank<<"::Partition Box:UBND  = { "<<col_cells.partition[0].box.upperbnd[0]<<", "
                                                   <<col_cells.partition[0].box.upperbnd[1]<<" } "<<endl;

  cout<<"Rank-"<<rank<<"::Partition Box:Strides  = { "<<col_cells.strides[0][0]<<", "
                                                      <<col_cells.strides[0][1]<<" } "<<endl;
  cout<<"Rank-"<<rank<<"::Partition-Box:#Halo = " <<col_cells.partition[0].nhalo<<endl;
  cout<<"Rank-"<<rank<<"::Partition-Box:#DomainHalo = " <<col_cells.partition[0].nhalo_domain<<endl;
  cout<<"Rank-"<<rank<<"::Partition-Box:Through dim = " <<col_cells.partition[0].thru_dim<<endl;
  
  cout<<"Rank-"<<rank<<"::Partition-Box:On domain boundary = [ ";
  for (size_t i = 0 ; i < 4; i++)
     cout<<col_cells.partition[0].onbnd[i]<<" ";
  cout<<"]"<<std::endl;

  cout<<"Rank-"<<rank<<"::Exclusive-Box:LBND  = { "<<col_cells.exclusive[0].box.lowerbnd[0]<<", "
                                                   <<col_cells.exclusive[0].box.lowerbnd[1]<<" } "<<endl;

  cout<<"Rank-"<<rank<<"::Exclusive-Box:UBND  = { "<<col_cells.exclusive[0].box.upperbnd[0]<<", "
                                                   <<col_cells.exclusive[0].box.upperbnd[1]<<" } "<<endl;

  cout<<"Rank-"<<rank<<"::Exclusive-Box:colors = "<<col_cells.exclusive[0].colors[0]<<endl;

  for (auto s: col_cells.shared[0])
  {
    cout<<"Rank-"<<rank<<"::Shared-Boxes:LBND  = { "<<s.box.lowerbnd[0]<<", "
                                                    <<s.box.lowerbnd[1]<<" } "<<endl;

    cout<<"Rank-"<<rank<<"::Shared-Boxes:UBND  = { "<<s.box.upperbnd[0]<<", "
                                                    <<s.box.upperbnd[1]<<" } "<<endl;

    cout<<"Rank-"<<rank<<"::Shared-Boxes:#Colors = "<<s.colors.size()<<endl;
    for (auto c: s.colors)
     cout<<"Rank-"<<rank<<"::Shared-Boxes:colors = "<<c<<endl;
    cout<<endl;
  }

  for (auto g: col_cells.ghost[0])
  {
    cout<<"Rank-"<<rank<<"::Ghost-Boxes:LBND  = { "<<g.box.lowerbnd[0]<<", "
                                                   <<g.box.lowerbnd[1]<<" } "<<endl;

    cout<<"Rank-"<<rank<<"::Ghost-Boxes:UBND  = { "<<g.box.upperbnd[0]<<", "
                                                   <<g.box.upperbnd[1]<<" } "<<endl;

    cout<<"Rank-"<<rank<<"::Ghost-Boxes:#Colors = "<<g.colors.size()<<endl;;
    for (auto c: g.colors)
     cout<<"Rank-"<<rank<<"::Ghost-Boxes:colors = "<<c<<endl;
    cout<<endl;
  }

  for (auto d: col_cells.domain_halo[0])
  {
    cout<<"Rank-"<<rank<<"::DomainHalo-Boxes:LBND  = { "<<d.lowerbnd[0]<<", "
                                                        <<d.lowerbnd[1]<<" } "<<endl;

    cout<<"Rank-"<<rank<<"::DomainHalo-Boxes:UBND  = { "<<d.upperbnd[0]<<", "
                                                        <<d.upperbnd[1]<<" } "<<endl;
  }

  for (auto d: col_cells.overlay)
  {
    cout<<"Rank-"<<rank<<"::Overlay-Boxes:LBND  = { "<<d.lowerbnd[0]<<", "
                                                        <<d.lowerbnd[1]<<" } "<<endl;

    cout<<"Rank-"<<rank<<"::Overlay-Boxes:UBND  = { "<<d.upperbnd[0]<<", "
                                                        <<d.upperbnd[1]<<" } "<<endl;
  }

  for (auto d: col_cells.strides){
    cout<<"Rank-"<<rank<<"::Strides  = { "<<d[0]<<", "<<d[1]<<" } "<<endl;
  }
  
  // Color dependent entities i.e., vertices and edges
  auto col_depents = sbc.color_dependent_entities(col_cells);

  cout << "Rank="<<rank<<"::-----VERTICES-----"<<endl;
  flecsi::coloring::box_color_t exclusive = col_depents[0].exclusive[0]; 
  cout<<"Rank-"<<rank<<"::Exclusive-Box:LBND  = { "<<exclusive.box.lowerbnd[0]<<", "
                                                   <<exclusive.box.lowerbnd[1]<<" } "<<endl;

  cout<<"Rank-"<<rank<<"::Exclusive-Box:UBND  = { "<<exclusive.box.upperbnd[0]<<", "
                                                   <<exclusive.box.upperbnd[1]<<" } "<<endl;

  cout<<"Rank-"<<rank<<"::Exclusive-Box:colors = "<<exclusive.colors[0]<<endl;

  for (auto s: col_depents[0].shared[0])
  {
    cout<<"Rank-"<<rank<<"::Shared-Boxes:LBND  = { "<<s.box.lowerbnd[0]<<", "
                                                    <<s.box.lowerbnd[1]<<" } "<<endl;

    cout<<"Rank-"<<rank<<"::Shared-Boxes:UBND  = { "<<s.box.upperbnd[0]<<", "
                                                    <<s.box.upperbnd[1]<<" } "<<endl;

    cout<<"Rank-"<<rank<<"::Shared-Boxes:#Colors = "<<s.colors.size()<<endl;
    for (auto c: s.colors)
     cout<<"Rank-"<<rank<<"::Shared-Boxes:colors = "<<c<<endl;
    cout<<endl;
  }

  for (auto g: col_depents[0].ghost[0])
  {
    cout<<"Rank-"<<rank<<"::Ghost-Boxes:LBND  = { "<<g.box.lowerbnd[0]<<", "
                                                   <<g.box.lowerbnd[1]<<" } "<<endl;

    cout<<"Rank-"<<rank<<"::Ghost-Boxes:UBND  = { "<<g.box.upperbnd[0]<<", "
                                                   <<g.box.upperbnd[1]<<" } "<<endl;

    cout<<"Rank-"<<rank<<"::Ghost-Boxes:#Colors = "<<g.colors.size()<<endl;;
    for (auto c: g.colors)
     cout<<"Rank-"<<rank<<"::Ghost-Boxes:colors = "<<c<<endl;
    cout<<endl;
  }

  for (auto d: col_depents[0].domain_halo[0])
  {
    cout<<"Rank-"<<rank<<"::DomainHalo-Boxes:LBND  = { "<<d.lowerbnd[0]<<", "
                                                        <<d.lowerbnd[1]<<" } "<<endl;

    cout<<"Rank-"<<rank<<"::DomainHalo-Boxes:UBND  = { "<<d.upperbnd[0]<<", "
                                                        <<d.upperbnd[1]<<" } "<<endl;
  }

  for (auto d: col_depents[0].overlay)
  {
    cout<<"Rank-"<<rank<<"::Overlay-Boxes:LBND  = { "<<d.lowerbnd[0]<<", "
                                                        <<d.lowerbnd[1]<<" } "<<endl;

    cout<<"Rank-"<<rank<<"::Overlay-Boxes:UBND  = { "<<d.upperbnd[0]<<", "
                                                        <<d.upperbnd[1]<<" } "<<endl;
  }

  for (auto d: col_depents[0].strides){
    cout<<"Rank-"<<rank<<"::Strides  = { "<<d[0]<<", "<<d[1]<<" } "<<endl;
  }


  cout << "Rank="<<rank<<"::-----EDGES-----"<<endl;
  for (size_t i = 0; i < 2; i++){ 
      flecsi::coloring::box_color_t exclusive = col_depents[1].exclusive[i]; 
      cout<<"Rank-"<<rank<<"::Exclusive-Box:LBND  = { "<<exclusive.box.lowerbnd[0]<<", "
                                                   <<exclusive.box.lowerbnd[1]<<" } "<<endl;

      cout<<"Rank-"<<rank<<"::Exclusive-Box:UBND  = { "<<exclusive.box.upperbnd[0]<<", "
						       <<exclusive.box.upperbnd[1]<<" } "<<endl;

      cout<<"Rank-"<<rank<<"::Exclusive-Box:colors = "<<exclusive.colors[0]<<endl;

      for (auto s: col_depents[1].shared[i])
      {
	cout<<"Rank-"<<rank<<"::Shared-Boxes:LBND  = { "<<s.box.lowerbnd[0]<<", "
							<<s.box.lowerbnd[1]<<" } "<<endl;

	cout<<"Rank-"<<rank<<"::Shared-Boxes:UBND  = { "<<s.box.upperbnd[0]<<", "
							<<s.box.upperbnd[1]<<" } "<<endl;

	cout<<"Rank-"<<rank<<"::Shared-Boxes:#Colors = "<<s.colors.size()<<endl;
	for (auto c: s.colors)
	 cout<<"Rank-"<<rank<<"::Shared-Boxes:colors = "<<c<<endl;
	cout<<endl;
      }

      for (auto g: col_depents[1].ghost[i])
      {
	cout<<"Rank-"<<rank<<"::Ghost-Boxes:LBND  = { "<<g.box.lowerbnd[0]<<", "
						       <<g.box.lowerbnd[1]<<" } "<<endl;

	cout<<"Rank-"<<rank<<"::Ghost-Boxes:UBND  = { "<<g.box.upperbnd[0]<<", "
						       <<g.box.upperbnd[1]<<" } "<<endl;

	cout<<"Rank-"<<rank<<"::Ghost-Boxes:#Colors = "<<g.colors.size()<<endl;;
	for (auto c: g.colors)
	 cout<<"Rank-"<<rank<<"::Ghost-Boxes:colors = "<<c<<endl;
	cout<<endl;
      }

      for (auto d: col_depents[1].domain_halo[i])
      {
	cout<<"Rank-"<<rank<<"::DomainHalo-Boxes:LBND  = { "<<d.lowerbnd[0]<<", "
							    <<d.lowerbnd[1]<<" } "<<endl;

	cout<<"Rank-"<<rank<<"::DomainHalo-Boxes:UBND  = { "<<d.upperbnd[0]<<", "
							    <<d.upperbnd[1]<<" } "<<endl;
      }

      for (auto d: col_depents[1].overlay)
      {
	cout<<"Rank-"<<rank<<"::Overlay-Boxes:LBND  = { "<<d.lowerbnd[0]<<", "
							    <<d.lowerbnd[1]<<" } "<<endl;

	cout<<"Rank-"<<rank<<"::Overlay-Boxes:UBND  = { "<<d.upperbnd[0]<<", "
							    <<d.upperbnd[1]<<" } "<<endl;
      }

      for (auto d: col_depents[1].strides){
	cout<<"Rank-"<<rank<<"::Strides  = { "<<d[0]<<", "<<d[1]<<" } "<<endl;
      }
   }

}
