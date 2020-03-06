#include <iostream>
#include <vector>

#include "mpi.h"
#include <cinchtest.h>

#include <flecsi/coloring/simple_box_colorer.h>

using namespace std;

TEST(simple_colorer, simpletest2d) {
  int size;
  int rank;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  size_t grid_size[2] = {10, 10};
  size_t ncolors[2] = {2, 2};
  size_t nhalo = 1;
  size_t nhalo_domain = 1;
  size_t thru_dim = 0;

  flecsi::coloring::simple_box_colorer_t<2> sbc;
  auto col = sbc.color(grid_size, nhalo, nhalo_domain, thru_dim, ncolors);

  cout << "Rank-" << rank << "::Primary Box:LBND  = { "
       << col.primary.box.lowerbnd[0] << ", " << col.primary.box.lowerbnd[1]
       << " } " << endl;

  cout << "Rank-" << rank << "::Primary Box:UBND  = { "
       << col.primary.box.upperbnd[0] << ", " << col.primary.box.upperbnd[1]
       << " } " << endl;

  cout << "Rank-" << rank << "::Primary-Box:#Halo = " << col.primary.nhalo
       << endl;
  cout << "Rank-" << rank
       << "::Primary-Box:#DomainHalo = " << col.primary.nhalo_domain << endl;
  cout << "Rank-" << rank
       << "::Primary-Box:Through dim = " << col.primary.thru_dim << endl;
  cout << "Rank-" << rank << "::Primary-Box:On domain boundary = [ "
       << col.primary.onbnd << " ]" << endl;

  cout << "Rank-" << rank << "::Exclusive-Box:LBND  = { "
       << col.exclusive.box.lowerbnd[0] << ", " << col.exclusive.box.lowerbnd[1]
       << " } " << endl;

  cout << "Rank-" << rank << "::Exclusive-Box:UBND  = { "
       << col.exclusive.box.upperbnd[0] << ", " << col.exclusive.box.upperbnd[1]
       << " } " << endl;

  cout << "Rank-" << rank
       << "::Exclusive-Box:colors = " << col.exclusive.colors[0] << endl;

  for(auto s : col.shared) {
    cout << "Rank-" << rank << "::Shared-Boxes:LBND  = { " << s.box.lowerbnd[0]
         << ", " << s.box.lowerbnd[1] << " } " << endl;

    cout << "Rank-" << rank << "::Shared-Boxes:UBND  = { " << s.box.upperbnd[0]
         << ", " << s.box.upperbnd[1] << " } " << endl;

    cout << "Rank-" << rank << "::Shared-Boxes:#Colors = " << s.colors.size()
         << endl;
    for(auto c : s.colors)
      cout << "Rank-" << rank << "::Shared-Boxes:colors = " << c << endl;
    cout << endl;
  }

  for(auto g : col.ghost) {
    cout << "Rank-" << rank << "::Ghost-Boxes:LBND  = { " << g.box.lowerbnd[0]
         << ", " << g.box.lowerbnd[1] << " } " << endl;

    cout << "Rank-" << rank << "::Ghost-Boxes:UBND  = { " << g.box.upperbnd[0]
         << ", " << g.box.upperbnd[1] << " } " << endl;

    cout << "Rank-" << rank << "::Ghost-Boxes:#Colors = " << g.colors.size()
         << endl;
    ;
    for(auto c : g.colors)
      cout << "Rank-" << rank << "::Ghost-Boxes:colors = " << c << endl;
    cout << endl;
  }

  for(auto d : col.domain_halo) {
    cout << "Rank-" << rank << "::DomainHalo-Boxes:LBND  = { " << d.lowerbnd[0]
         << ", " << d.lowerbnd[1] << " } " << endl;

    cout << "Rank-" << rank << "::DomainHalo-Boxes:UBND  = { " << d.upperbnd[0]
         << ", " << d.upperbnd[1] << " } " << endl;
  }
}
