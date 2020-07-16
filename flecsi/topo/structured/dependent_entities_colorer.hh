/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef dependent_entities_colorer_h
#define dependent_entities_colorer_h

//----------------------------------------------------------------------------//
//! @file
//! @date Initial file creation: Dec 05, 2017
//----------------------------------------------------------------------------//

#include <algorithm>
#include <cassert>
#include <cmath>
#include <type_traits>

#include "flecsi/topo/structured/box_colorer.hh"
#include "flecsi/topo/structured/box_utils.hh"
#include "flecsi/topo/structured/coloring_types.hh"

namespace flecsi {
namespace topology {
namespace structured_impl {

struct dependent_entities_colorer {
 
  //! Default constructor
  dependent_entities_colorer() {}

  //! Copy constructor (disabled)
  dependent_entities_colorer(const dependent_entities_colorer &) = delete;

  //! Assignment operator (disabled)
  dependent_entities_colorer & operator=(const dependent_entities_colorer &) = delete;

  //! Destructor
  ~dependent_entities_colorer() {} 

 // Coloring algorithm for coloring the intermediate entities
 // from an input partition of the cells in the mesh 
 auto color(box_coloring_t &colored_cells)
 {
    // Tag the boundaries of the colored cells to decipher which
    // intermediate entities are exclusive, shared, ghost and domain_halo. 
    tag_box_boundary_entities(colored_cells); 

    //Now create the boxes for the dependent entities and pass the
    //tag info from the cell boundaries to the box of a particular 
    //intermediate entity
  
    std::vector<box_coloring_t> colored_depents; 
    color_dependent_entities_(colored_cells, colored_depents); 
  
     return colored_depents; 
  } //color_dependent_entities


private: 
  void tag_box_boundary_entities(box_coloring_t& colored_cells)
  {
    std::vector<int> box_ids, box_bids; 
    auto& ebox = colored_cells.exclusive[0]; 
    auto& shboxes = colored_cells.shared[0]; 
    auto& ghboxes = colored_cells.ghost[0]; 
    auto& dhboxes = colored_cells.domain_halo[0]; 

    int dim = ebox.domain.dim;
    int nbids = pow(3, dim); 
    int owner_rank = ebox.colors[0]; 

    //exclusive and shared
    for (int b = 0; b < nbids; ++b) {
      find_incident_boxes(b, ebox, shboxes, box_ids); 
      if (box_ids.size() != 0) {
         ebox.tag[b] = false;
      }    
    }

    //shared 
    for (size_t s = 0; s < shboxes.size(); ++s) {
     for (int b = 0; b < nbids; ++b) {
      std::set<int> clrs; 
      clrs.insert(owner_rank);     
      // collect all the ghost ranks incident on the bid of this shared
      // box and set the tag to false if the owner rank is not the lowest
      find_incident_boxes(b, shboxes[s], ghboxes, box_ids); 
      for (size_t i = 0; i < box_ids.size(); ++i) {
         clrs.insert(ghboxes[box_ids[i]].colors[0]);
      }
    
      if (*clrs.begin() != owner_rank)
         shboxes[s].tag[b] = false;     
    }
   }

    //ghost 
    for (size_t g = 0; g < ghboxes.size(); ++g) {
      int nsh = shboxes.size();
      int ghost_rank = ghboxes[g].colors[0]; 
 
     //search over all shared and ghost boxes
     std::vector<box_color_t> search_boxes = shboxes; 
     for (size_t k = 0; k < ghboxes.size(); ++k) {
       if (k != g)
        search_boxes.push_back(ghboxes[k]); 
     }

     for (int b = 0; b < nbids; ++b) {
      std::set<int> clrs; 
      clrs.insert(ghost_rank);
    
      // collect all the ranks incident on the bid of this ghost
      // box and set the tag is the owner rank is not the lowest
      find_incident_boxes(b, ghboxes[g], search_boxes, box_ids); 
      for (size_t i = 0; i < box_ids.size(); ++i) {
        auto gclrs = search_boxes[box_ids[i]].colors;
        if (box_ids[i] < nsh) {
          clrs.insert(owner_rank); 
        }
        else {
          clrs.insert(gclrs[0]);
        } 
      }
    
      if (*clrs.begin() != ghost_rank)
         ghboxes[g].tag[b] = false;     
    }
   }
    //domain halo 
    for (size_t d = 0; d < dhboxes.size(); ++d) {
     //search over all exclusive, shared and ghost boxes
     std::vector<box_color_t> search_boxes{ebox};
     search_boxes.insert(search_boxes.end(), shboxes.begin(), shboxes.end());   
     search_boxes.insert(search_boxes.end(), ghboxes.begin(), ghboxes.end());   
     for (int b = 0; b < nbids; ++b) {
      find_incident_boxes(b, dhboxes[d], search_boxes, box_ids); 
      if (box_ids.size() != 0) {
         dhboxes[d].tag[b] = false;
      }    
     }
    }  
  }//tag_box_boundary_entities

  void find_incident_boxes(size_t bid, box_color_t& mbox, 
                           std::vector<box_color_t>& in_boxes, 
                           std::vector<int>& inbox_ids)
  {
    inbox_ids.clear(); 
    size_t dim = mbox.domain.dim;   
    
    auto bid2dim_map = bid2dim(dim); 
    //auto d = bid2dim_map[bid];

    //auto sites = dim2bid(dim, d);
    size_t nbids = pow(3,dim); 

    for (size_t b = 0; b < in_boxes.size(); b++) {
     for (size_t s = 0; s < nbids;  s++) {
      bool incident = true;
      //check incidence
      for (size_t d = 0; d <dim; d++) { 

       auto bid2dir_map = bid2dir(dim, d);
       auto mval = bid2dir_map[bid]; 
       auto inval = bid2dir_map[s];
 
       if ((mval == 2) && (inval == 2)) {
         incident = incident && 
                    (((mbox.domain.lowerbnd[d] >= in_boxes[b].domain.lowerbnd[d]) &&
                     (mbox.domain.upperbnd[d] <= in_boxes[b].domain.upperbnd[d])) || 
                    ((mbox.domain.lowerbnd[d] <= in_boxes[b].domain.lowerbnd[d]) &&
                     (mbox.domain.upperbnd[d] >= in_boxes[b].domain.upperbnd[d]))) ; 
       }
       else if ((mval == 0) && (inval == 2)) {
         incident = incident && 
                    (mbox.domain.lowerbnd[d] >= in_boxes[b].domain.lowerbnd[d]) &&
                    (mbox.domain.lowerbnd[d] <= in_boxes[b].domain.upperbnd[d]) ; 
       }
       else if ((mval == 1) && (inval == 2)) {
         incident = incident && 
                    (mbox.domain.upperbnd[d] >= in_boxes[b].domain.lowerbnd[d]) &&
                    (mbox.domain.upperbnd[d] <= in_boxes[b].domain.upperbnd[d]) ; 
       }
       else if ((mval == 0) && (inval == 0)) {
         incident = incident && 
                    (mbox.domain.lowerbnd[d] == in_boxes[b].domain.lowerbnd[d]); 
       }
       else if ((mval == 0) && (inval == 1)) {
         incident = incident && 
                    (mbox.domain.lowerbnd[d] - in_boxes[b].domain.upperbnd[d] == 1); 
       }
       else if ((mval == 1) && (inval == 0)) {
         incident = incident && 
                    (in_boxes[b].domain.lowerbnd[d] - mbox.domain.upperbnd[d] == 1); 
       }
       else if ((mval == 1) && (inval == 1)) {
         incident = incident && 
                    (mbox.domain.upperbnd[d] == in_boxes[b].domain.upperbnd[d]); 
       }
       else { 
        incident = false; 
       }
      } //dim
  
      if (incident) {
       inbox_ids.emplace_back(b);
       break; //this box is already incident through one site
      }
     } //sites
    } //boxes 
  } //find_incident_boxes 

  void color_dependent_entities_(
       box_coloring_t& colored_cells, 
       std::vector<box_coloring_t>& col_depents)
  {
    int dim = colored_cells.mesh_dim;
    col_depents.resize(dim); 

    auto& ebox = colored_cells.exclusive[0]; 
    auto& shboxes = colored_cells.shared[0]; 
    auto& ghboxes = colored_cells.ghost[0]; 
    auto& dhboxes = colored_cells.domain_halo[0]; 
    auto& obox = colored_cells.overlay[0]; 
    auto& strides = colored_cells.strides[0]; 
   

    for (int edim = 0; edim < dim; edim++) {
      auto map = dim2bounds(dim, edim);
      int nboxes = map.size(); 
      col_depents[edim].mesh_dim = dim; 
      col_depents[edim].entity_dim = edim; 
      col_depents[edim].primary_dim = dim; 
      col_depents[edim].num_boxes = nboxes; 
      col_depents[edim].resize(); 

      //exclusive 
      for (int b = 0; b < nboxes; ++b) { 
       //set bounds
       box_color_t box(dim); 
       for (int d = 0; d < dim; d++) {
        box.domain.lowerbnd[d] = ebox.domain.lowerbnd[d];
        box.domain.upperbnd[d] = ebox.domain.upperbnd[d]+map[b][d];
       }
       //set tags
       for (size_t i = dim; i <map[b].size(); ++i){
          auto bid = map[b][i];  
          box.tag[bid] = ebox.tag[bid];  
       }
       //set colors
       box.colors = ebox.colors; 
       col_depents[edim].exclusive[b] = box;  
      }
      
      //shared
      for (size_t s = 0; s < shboxes.size(); ++s) {
       for (int b = 0; b < nboxes; ++b) { 
        //set bounds
        box_color_t box(dim); 
        for (int d = 0; d < dim; d++) {
         box.domain.lowerbnd[d] = shboxes[s].domain.lowerbnd[d];
         box.domain.upperbnd[d] = shboxes[s].domain.upperbnd[d]+map[b][d];
        }
       //set tags
       for (size_t i = dim; i < map[b].size(); ++i){
          auto bid = map[b][i];  
          box.tag[bid] = shboxes[s].tag[bid];  
       }
       //set colors
       box.colors = shboxes[s].colors; 
       col_depents[edim].shared[b].push_back(box);  
       }
      }

      //ghost
      for (size_t g = 0; g < ghboxes.size(); ++g) {
       for (int b = 0; b < nboxes; ++b) { 
        //set bounds
        box_color_t box(dim); 
        for (int d = 0; d < dim; d++) {
         box.domain.lowerbnd[d] = ghboxes[g].domain.lowerbnd[d];
         box.domain.upperbnd[d] = ghboxes[g].domain.upperbnd[d]+map[b][d];
        }
       //set tags
       for (size_t i = dim; i < map[b].size(); ++i){
          auto bid = map[b][i];  
          box.tag[bid] = ghboxes[g].tag[bid];  
       }
       //set colors
       box.colors = ghboxes[g].colors; 
       col_depents[edim].ghost[b].push_back(box);  
       }
      }

      //domain halo
      for (size_t dh = 0; dh < dhboxes.size(); ++dh) {
       for (int b = 0; b < nboxes; ++b) { 
        //set bounds
        box_color_t box(dim); 
        for (int d = 0; d < dim; d++) {
         box.domain.lowerbnd[d] = dhboxes[dh].domain.lowerbnd[d];
         box.domain.upperbnd[d] = dhboxes[dh].domain.upperbnd[d]+map[b][d];
        }
       //set tags
       for (size_t i = dim; i < map[b].size(); ++i){
          auto bid = map[b][i];  
          box.tag[bid] = dhboxes[dh].tag[bid];  
       }
       //set colors
       box.colors = dhboxes[dh].colors; 
       col_depents[edim].domain_halo[b].push_back(box);  
       }
      }
         
      //overlay and strides
      for (int b = 0; b < nboxes; ++b) { 
       //set bounds
       box_t box(dim); 
       for (int d = 0; d < dim; d++) {
        box.lowerbnd[d] = obox.lowerbnd[d];
        box.upperbnd[d] = obox.upperbnd[d]+map[b][d];
       }
      
       std::vector<size_t> str(dim); 
       for (int d = 0; d < dim; d++) {
        str[d] = strides[d]+map[b][d];
       } 

       col_depents[edim].overlay[b] = box; 
       col_depents[edim].strides[b] = str; 
      }
     
    } //edim        
 
  } //color_dependent_entities_
}; // class depent_colorer
} // namespace structured_impl
} // namespace topology
} // namespace flecsi
#endif // simple_box_colorer_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
