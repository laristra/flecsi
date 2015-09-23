/*
 * ###########################################################################
 * Copyright (c) 2015, Los Alamos National Security, LLC.
 * All rights reserved.
 *
 *  Copyright 2015. Los Alamos National Security, LLC. This software was
 *  produced under U.S. Government contract DE-AC52-06NA25396 for Los
 *  Alamos National Laboratory (LANL), which is operated by Los Alamos
 *  National Security, LLC for the U.S. Department of Energy. The
 *  U.S. Government has rights to use, reproduce, and distribute this
 *  software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY,
 *  LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY
 *  FOR THE USE OF THIS SOFTWARE.  If software is modified to produce
 *  derivative works, such modified software should be clearly marked,
 *  so as not to confuse it with the version available from LANL.
 *
 *  Additionally, redistribution and use in source and binary forms,
 *  with or without modification, are permitted provided that the
 *  following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *    * Neither the name of Los Alamos National Security, LLC, Los
 *      Alamos National Laboratory, LANL, the U.S. Government, nor the
 *      names of its contributors may be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
 *  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 * ###########################################################################
 *
 * Notes
 *
 * #####
 */

#include "mesh_topology.h"

using namespace std;
using namespace scout;

#include <unordered_map>

namespace{

  using UniformMesh1d = MeshTopology<UniformMesh1dType>;
  using UniformMesh2d = MeshTopology<UniformMesh2dType>;
  using UniformMesh3d = MeshTopology<UniformMesh3dType>;

} // namespace

extern "C"{

  void* __scrt_create_uniform_mesh1d(uint64_t width){
    auto mesh = new UniformMesh1d;

    size_t id = 0;
    for(size_t i = 0; i < width + 1; ++i){
      mesh->addVertex(id++, {UniformMesh2dType::Float(i)}); 
    }

    id = 0;
    for(size_t i = 0; i < width; ++i){
      mesh->addCell(id++, {i, i + 1});
    }

    return mesh;
  }

  void* __scrt_create_uniform_mesh2d(uint64_t width, uint64_t height){
    auto mesh = new UniformMesh2d;

    size_t id = 0;
    for(size_t j = 0; j < height + 1; ++j){
      for(size_t i = 0; i < width + 1; ++i){
        mesh->addVertex(id++, {UniformMesh2dType::Float(i),
                               UniformMesh2dType::Float(j)}); 
      }
    }

    id = 0;
    size_t width1 = width + 1;
    for(size_t j = 0; j < height; ++j){
      for(size_t i = 0; i < width; ++i){
        mesh->addCell(id++,
                      {i + j * width1,
                       i + (j + 1) * width1,
                       i + 1 + j * width1,
                       i + 1 + (j + 1) * width1}
                      );
      }
    }
    
    // although these can be computed in MeshTopology, for uniform
    // mesh, it is straight-forward, so we compute here for better
    // performance
    using CellToEdgesMap = unordered_multimap<size_t, size_t>;
    CellToEdgesMap m;

    size_t edgeId = 0;
    size_t cellId = 0;
    for(size_t j = 0; j < height; ++j){
      for(size_t i = 0; i < width; ++i){
        size_t v0 = i + j * width1;
        size_t v1 = i + 1 + j * width1;
        size_t v2 = i + (j + 1) * width1;
        size_t v3 = i + 1 + (j + 1) * width1;

        if(j == 0){
          m.insert({cellId, edgeId});
          mesh->addEdge(edgeId++, v0, v1);
        }
  
        if(j < height - 1){
          m.insert({cellId + width, edgeId});
        }
        m.insert({cellId, edgeId});
        mesh->addEdge(edgeId++, v2, v3); 

        if(i == 0){
          m.insert({cellId, edgeId});
          mesh->addEdge(edgeId++, v0, v2);
        }
 
        if(i < width - 1){
          m.insert({cellId + 1, edgeId});
        }
        m.insert({cellId, edgeId});
        mesh->addEdge(edgeId++, v1, v3);

        ++cellId;
      }
    }

    for(size_t i = 0; i < cellId; ++i){
      auto itr = m.equal_range(i).first;
      mesh->addCellEdges(i,
                         {(itr++)->second,
                          (itr++)->second,
                          (itr++)->second,
                          (itr++)->second});
    }

    /*
    mesh->compute(2, 1);
    mesh->compute(2, 0);
    mesh->compute(1, 0);
    mesh->compute(1, 2);
    mesh->compute(0, 1);
    mesh->compute(0, 2);
    mesh->dump();
    */

    return mesh;
  }

  void* __scrt_create_uniform_mesh3d(uint64_t width,
                                     uint64_t height,
                                     uint64_t depth){
    auto mesh = new UniformMesh3d;

    size_t id = 0;
    for(size_t k = 0; k < depth + 1; ++k){
      for(size_t j = 0; j < height + 1; ++j){
        for(size_t i = 0; i < width + 1; ++i){
          mesh->addVertex(id++, 
                          {UniformMesh2dType::Float(i),
                           UniformMesh2dType::Float(j),
                           UniformMesh2dType::Float(k)}); 
        }
      }
    }

    size_t width1 = width + 1;
    size_t height1 = height + 1;

    id = 0;
    for(size_t k = 0; k < depth; ++k){
      for(size_t j = 0; j < height; ++j){
        for(size_t i = 0; i < width; ++i){
          mesh->addCell(id++,
                        {i + width1 * (j + k * height1),
                         i + width1 * ((j + 1) + k * height1),
                         i + 1 + width1 * (j + k * height1),
                         i + 1 + width1 * ((j + 1) + k * height1),
                         i + width1 * (j + (k + 1) * height1),
                         i + width1 * ((j + 1) + (k + 1) * height1),
                         i + 1 + width1 * (j + (k + 1) * height1),
                         i + 1 + width1 * ((j + 1) + (k + 1) * height1)
                        }
                        );
        }
      }
    }

    return mesh;
  }

  void __scrt_mesh_topology_compute_all(void* topology){
    return static_cast<MeshTopologyBase*>(topology)->computeAll();
  }

  // return end index
  uint64_t __scrt_mesh_num_entities(void* topology, uint32_t dim){
    return static_cast<MeshTopologyBase*>(topology)->numEntities(dim);
  }

  uint64_t* __scrt_mesh_get_to_indices(void* topology,
                                       uint32_t fromDim,
                                       uint32_t toDim){
    return static_cast<MeshTopologyBase*>(topology)->getToIndices(
      fromDim, toDim);
  }

  uint64_t* __scrt_mesh_get_from_indices(void* topology,
                                         uint32_t fromDim,
                                         uint32_t toDim){
    return static_cast<MeshTopologyBase*>(topology)->getFromIndices(
      fromDim, toDim);
  }
  
} // extern "C"
