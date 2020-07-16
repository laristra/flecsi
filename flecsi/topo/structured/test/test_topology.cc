#include <iostream>
#include <memory>

#include "flecsi/util/unit.hh"
#include <flecsi/util/tuple_walker.hh>
#include <flecsi/util/demangle.hh>

#include "flecsi/topo/structured/simple_box_colorer.hh"
#include "flecsi/topo/structured/dependent_entities_colorer.hh"
#include "flecsi/topo/structured/structured_entity_types.hh"
#include "flecsi/topo/structured/structured_index_space.hh"
#include "flecsi/topo/structured/structured_topology.hh"
#include "flecsi/topo/structured/structured_topology_storage.hh"

using namespace flecsi::topology::structured_impl;

//Create entity types: 
//1d
class vertex_1d: public structured_mesh_entity_u<0> {};
class cell_1d: public structured_mesh_entity_u<1> {}; 

//Create a mesh policy
class TestMesh1dType{
public:
  static constexpr size_t num_dimensions = 1;

  using entity_types = std::tuple<
  std::tuple<index_space_<0>, vertex_1d>,
  std::tuple<index_space_<1>, cell_1d>>;
};

void print(box_coloring_t& colored_ents)
{
    int dim = colored_ents.mesh_dim;
    int edim = colored_ents.entity_dim;
    int nbids = pow(3, dim);

    //Print colored ents: overlay, exclusive, shared, ghost, domain-halos
    size_t de_nboxes = colored_ents.num_boxes; ;
    auto de_ebox     = colored_ents.exclusive;
    auto de_shboxes  = colored_ents.shared;
    auto de_ghboxes  = colored_ents.ghost;
    auto de_dhboxes  = colored_ents.domain_halo;
    auto de_obox     = colored_ents.overlay;
    auto de_strides  = colored_ents.strides; 

    UNIT_CAPTURE() <<"ENTITY OF DIM "<<edim<<" COLORING"<<std::endl;

    UNIT_CAPTURE() <<"   ----->Overlay:\n";
    for (size_t n = 0 ; n < de_nboxes; ++n) {
     UNIT_CAPTURE() <<"  ------->box_id "<<n<<" "<<std::endl; 
     for (int i = 0; i < dim ; ++i)
       UNIT_CAPTURE() <<"          dim "<<i<<" : "<<de_obox[n].lowerbnd[i]
                <<", "<<de_obox[n].upperbnd[i]<<std::endl;
    }

    UNIT_CAPTURE() <<"   ----->Strides:\n";
    for (size_t n = 0 ; n < de_nboxes; ++n) {
     UNIT_CAPTURE() <<"  ------->box_id "<<n<<" "<<std::endl; 
     for (int i = 0; i < dim ; ++i)
      UNIT_CAPTURE() <<"           dim "<<i<<" : "<<de_strides[n][i]<<std::endl;
    }

    UNIT_CAPTURE() <<"   ----->Exclusive:\n";
    for (size_t n = 0 ; n < de_nboxes; ++n) {
     UNIT_CAPTURE() <<"  ------->box_id "<<n<<" "<<std::endl; 
     for (int i = 0; i < dim ; ++i)
       UNIT_CAPTURE() <<"          dim "<<i<<" : "<<de_ebox[n].domain.lowerbnd[i]
                <<", "<<de_ebox[n].domain.upperbnd[i]<<std::endl;
    
   UNIT_CAPTURE() <<"          bid tags = [";
     for (int b = 0; b < nbids ; ++b)
       UNIT_CAPTURE() <<de_ebox[n].tag[b]<<" ";
     UNIT_CAPTURE() <<"]"<<std::endl; 

     UNIT_CAPTURE() <<"          colors = [";
     for (size_t c = 0; c < de_ebox[n].colors.size() ; ++c)
       UNIT_CAPTURE() <<de_ebox[n].colors[c]<<" ";
     UNIT_CAPTURE() <<"]"<<std::endl; 
   } //de_nboxes

    UNIT_CAPTURE() <<"   ----->Shared:\n";
    size_t nsh = de_shboxes[0].size(); 
    for (size_t s = 0; s < nsh; ++s) {
     UNIT_CAPTURE() <<"   ---------->Shared Box "<<s<<":\n";
     for (size_t n = 0 ; n < de_nboxes; ++n) {
       UNIT_CAPTURE() <<"  ------->box_id "<<n<<" "<<std::endl; 
       for (int i = 0; i < dim ; ++i)
        UNIT_CAPTURE() <<"           dim "<<i<<" : "<<de_shboxes[n][s].domain.lowerbnd[i]
                 <<", "<<de_shboxes[n][s].domain.upperbnd[i]<<std::endl;
      
      UNIT_CAPTURE() <<"           bid tags = [";
      for (int b = 0; b < nbids ; ++b)
        UNIT_CAPTURE() <<de_shboxes[n][s].tag[b]<<" ";
      UNIT_CAPTURE() <<"]"<<std::endl; 

      UNIT_CAPTURE() <<"           colors = [";
      for (size_t c = 0; c < de_shboxes[n][s].colors.size() ; ++c)
        UNIT_CAPTURE() <<de_shboxes[n][s].colors[c]<<" ";
      UNIT_CAPTURE() <<"]"<<std::endl; 
     } // shared 
    }//de_nboxes

    UNIT_CAPTURE() <<"   ----->Ghost:\n";
    size_t ngh = de_ghboxes[0].size(); 
    for (size_t g = 0; g < ngh; ++g) {
     UNIT_CAPTURE() <<"   ---------->Ghost Box "<<g<<":\n";
     for (size_t n = 0 ; n < de_nboxes; ++n) {
      UNIT_CAPTURE() <<"  ------->box_id "<<n<<" "<<std::endl; 
      for (int i = 0; i < dim ; ++i)
        UNIT_CAPTURE() <<"          dim "<<i<<" : "<<de_ghboxes[n][g].domain.lowerbnd[i]
                 <<", "<<de_ghboxes[n][g].domain.upperbnd[i]<<std::endl;
      
      UNIT_CAPTURE() <<"          bid tags = [";
      for (int b = 0; b < nbids ; ++b)
        UNIT_CAPTURE() <<de_ghboxes[n][g].tag[b]<<" ";
      UNIT_CAPTURE() <<"]"<<std::endl; 

      UNIT_CAPTURE() <<"          colors = [";
      for (size_t c = 0; c < de_ghboxes[n][g].colors.size() ; ++c)
        UNIT_CAPTURE() <<de_ghboxes[n][g].colors[c]<<" ";
      UNIT_CAPTURE() <<"]"<<std::endl; 
    } // ghost
   }//de_nboxes

    UNIT_CAPTURE() <<"   ----->Domain Halo:\n";
    size_t ndh = de_dhboxes[0].size(); 
    for (size_t h = 0; h < ndh; ++h) {
    UNIT_CAPTURE() <<"   ---------->Domain Halo Box "<<h<<":\n";
     for (size_t n = 0 ; n < de_nboxes; ++n) {
      UNIT_CAPTURE() <<"  ------->box_id "<<n<<" "<<std::endl; 
      for (int i = 0; i < dim ; ++i)
        UNIT_CAPTURE() <<"          dim "<<i<<" : "<<de_dhboxes[n][h].domain.lowerbnd[i]
                 <<", "<<de_dhboxes[n][h].domain.upperbnd[i]<<std::endl;
      
      UNIT_CAPTURE() <<"          bid tags = [";
      for (int b = 0; b < nbids ; ++b)
        UNIT_CAPTURE() <<de_dhboxes[n][h].tag[b]<<" ";
      UNIT_CAPTURE() <<"]"<<std::endl; 
    } // domain halo
   }//de_nboxes

} //print

int topology_driver_1d()
{
  UNIT { 
  // STEP 1: Mesh partitioning 
  //Define bounds of a structured mesh
  size_t grid_size[1] = {6};
  size_t ncolors[1]={2};
  size_t nhalo = 1;
  size_t nhalo_domain = 2;
  size_t thru_dim = 0;

  std::cout<<"in topology driver 1d --->"<<std::endl;
  // Create a simple partition of the mesh
  box_coloring_t colored_cells;
  box_aggregate_info_t colored_cells_aggregate;

  // Create a colorer instance to generate the coloring.
  auto colorer = std::make_shared<simple_box_colorer_t<1>>();

  //Create the coloring info for cells
  colored_cells = colorer->color(grid_size, nhalo, nhalo_domain, thru_dim, ncolors);

  //Create the coloring info for dependent ents : vertices
  auto dep_colorer = std::make_shared<dependent_entities_colorer>(); 
  auto depcols = dep_colorer->color(colored_cells); 
  //print(depcols[0]);

  // STEP 2: Create topology storage and initialize index spaces in the topology 
  // from coloring data 
  auto ms = new structured_topology_storage_u<TestMesh1dType::num_dimensions>();  

  auto& sv = std::get<vertex_1d::dimension>(ms->topology);
  sv.initialize(depcols[0]); 

  auto& sc = std::get<cell_1d::dimension>(ms->topology);
  sc.initialize(colored_cells);  
 
  // STEP 3: Mesh instance creation 
  auto mesh = std::make_shared<structured_topology_u<TestMesh1dType>>(ms);   

  // STEP 4: Traverse mesh:overlay, exclusive, shared, ghost and domain_halos
  UNIT_CAPTURE()<<"CELLS---->\n";
  for (auto c: mesh->overlay<cell_1d::dimension>())
    UNIT_CAPTURE()<<"overlay cell id = "<<c<<std::endl;

  for (auto c: mesh->exclusive<cell_1d::dimension>())
    UNIT_CAPTURE()<<"exclusive cell id = "<<c<<std::endl;

  for (auto sh: mesh->shared<cell_1d::dimension>())
  {
    for (auto c: sh)
     UNIT_CAPTURE()<<"shared cell id = "<<c<<std::endl;
  }  

  for (auto gh: mesh->ghost<cell_1d::dimension>())
  {
    for (auto c: gh)
     UNIT_CAPTURE()<<"ghost cell id = "<<c<<std::endl;
  }  

  for (auto dh: mesh->domain_halo<cell_1d::dimension>())
  {
    for (auto c: dh) 
     UNIT_CAPTURE()<<"domain_halo cell id = "<<c<<std::endl;
  }

  UNIT_CAPTURE()<<"VERTICES---->\n";
  for (auto c: mesh->overlay<vertex_1d::dimension>())
    UNIT_CAPTURE()<<"overlay vertex id = "<<c<<std::endl;

  for (auto c: mesh->exclusive<vertex_1d::dimension>())
    UNIT_CAPTURE()<<"exclusive vertex id = "<<c<<std::endl;

  for (auto sh: mesh->shared<vertex_1d::dimension>())
  {
    for (auto c: sh)
     UNIT_CAPTURE()<<"shared vertex id = "<<c<<std::endl;
  }  

  for (auto gh: mesh->ghost<vertex_1d::dimension>())
  {
    for (auto c: gh)
     UNIT_CAPTURE()<<"ghost vertex id = "<<c<<std::endl;
  }  

  for (auto dh: mesh->domain_halo<vertex_1d::dimension>())
  {
    for (auto c: dh) 
     UNIT_CAPTURE()<<"domain_halo vertex id = "<<c<<std::endl;
  }
 
  //dump in a file
  int dim = colored_cells.exclusive[0].domain.dim;
  int owner = colored_cells.exclusive[0].colors[0]; 
  std::string fname = "smesh_"+ std::to_string(dim) + "d_" + std::to_string(owner) + ".txt"; 
  UNIT_WRITE(fname); 

  delete ms; 
  //return 0;
  }; //UNIT
}

flecsi::unit::driver<topology_driver_1d> driver;
