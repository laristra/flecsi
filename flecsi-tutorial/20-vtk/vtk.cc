/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Los Alamos National Security, LLC
   All rights reserved.
                                                                              */

#include <iostream>

#include<flecsi-tutorial/specialization/mesh/mesh.h>
#include<flecsi/data/data.h>
#include<flecsi/execution/execution.h>
#include<flecsi/io/vtk/structuredGrid.h>

using namespace flecsi;
using namespace flecsi::tutorial;

flecsi_register_data_client(mesh_t, clients, mesh);
flecsi_register_field(mesh_t, example, field, double, dense, 1, cells);

namespace example {

void initialize_field(mesh<ro> mesh, field<rw> f) {
  for(auto c: mesh.cells(owned)) {
    f(c) = double(c->id());
  } // for
} // initialize_field

flecsi_register_task(initialize_field, example, loc, single);

void print_field(mesh<ro> mesh, field<ro> f) {
  for(auto c: mesh.cells(owned)) {
    std::cout << "cell id: " << c->id() << " has value " <<
      f(c) << std::endl;
  } // for
} // print_field

flecsi_register_task(print_field, example, loc, single);



void output_field(mesh<ro> mesh, field<ro> f) {

  vtkOutput::StructuredGrid temp;
  temp.setDims(256,0,0);
  double *cellData = new double[256];

  int count = 0;
  for(auto c: mesh.cells(owned)) {

    double pnt[3];
    pnt[0]=c->id(); pnt[1]=0; pnt[2]=0;
    temp.addPoint(pnt);

    cellData[count] = f(c);
    count++;
  } // for

  temp.addScalarCellData("cell-data-scalar", 256, cellData);
  temp.pushPointsToGrid();
  temp.write("testVTK");
} // print_field


flecsi_register_task(output_field, example, loc, single);

} // namespace example

namespace flecsi {
namespace execution {

void driver(int argc, char ** argv) {

  auto m = flecsi_get_client_handle(mesh_t, clients, mesh);
  auto f = flecsi_get_handle(m, example, field, double, dense, 0);

  flecsi_execute_task(initialize_field, example, single, m, f);
  flecsi_execute_task(print_field, example, single, m, f);

} // driver

} // namespace execution
} // namespace flecsi


/*
int testStructured1(std::string filename)
{
  vtkOutput::StructuredGrid temp;

  temp.setDims(2,3,1);

  double pnt[3];
  pnt[0]=0; pnt[1]=0; pnt[2]=0; temp.addPoint(pnt);
  pnt[0]=1; pnt[1]=0; pnt[2]=0; temp.addPoint(pnt);
  pnt[0]=0; pnt[1]=1; pnt[2]=0; temp.addPoint(pnt);
  pnt[0]=1; pnt[1]=1; pnt[2]=0; temp.addPoint(pnt);
  pnt[0]=0; pnt[1]=2; pnt[2]=0; temp.addPoint(pnt);
  pnt[0]=1; pnt[1]=2; pnt[2]=1; temp.addPoint(pnt);
  
  temp.pushPointsToGrid();
  temp.write(filename);

  return 0;
}
*/