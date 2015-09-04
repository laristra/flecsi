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

#ifndef __UNIFORM_MESH_H__
#define __UNIFORM_MESH_H__

  class UniformMesh1dType{
  public:
    using Id = uint64_t;
    using Float = double;

    static constexpr size_t topologicalDimension(){
      return 1;
    }

    static constexpr size_t geometricDimension(){
      return 1;
    }

    static size_t numEntitiesPerCell(size_t dim){
      switch(dim){
      case 0:
        return 2;
      case 1:
        return 1;
      default:
        assert(false && "invalid dimension");
      }
    }

    static constexpr size_t verticesPerCell(){
      return 2;
    }
  
    static size_t numVerticesPerEntity(size_t dim){
      switch(dim){
      case 0:
        return 1;
      case 1:
        return 2;
      default:
        assert(false && "invalid dimension");
      }
    }
  
    static void createEntities(size_t dim, std::vector<Id>& e, Id* v){
      assert(false && "nothing to build");
    }
  
  };

  class UniformMesh2dType{
  public:
    using Id = uint64_t;
    using Float = double;

    static constexpr size_t topologicalDimension(){
      return 2;
    }

    static constexpr size_t geometricDimension(){
      return 2;
    }

    static size_t numEntitiesPerCell(size_t dim){
      switch(dim){
      case 0:
        return 4;
      case 1:
        return 4;
      case 2:
        return 1;
      default:
        assert(false && "invalid dimension");
      }
    }

    static constexpr size_t verticesPerCell(){
      return 4;
    }
  
    static size_t numVerticesPerEntity(size_t dim){
      switch(dim){
      case 0:
        return 1;
      case 1:
        return 2;
      case 2:
        return 4;
      default:
        assert(false && "invalid dimension");
      }
    }
  
    static void createEntities(size_t dim, std::vector<Id>& e, Id* v){
      assert(dim = 1);
      assert(e.size() == 8);
    
      e[0] = v[0];
      e[1] = v[2];

      e[2] = v[1];
      e[3] = v[3];
    
      e[4] = v[0];
      e[5] = v[1];
    
      e[6] = v[2];
      e[7] = v[3];
    }
  
  };

  class UniformMesh3dType{
  public:
    using Id = uint64_t;
    using Float = double;

    static constexpr size_t topologicalDimension(){
      return 3;
    }

    static constexpr size_t geometricDimension(){
      return 3;
    }

    static size_t numEntitiesPerCell(size_t dim){
      switch(dim){
      case 0:
        return 8;
      case 1:
        return 12;
      case 2:
        return 6;
      default:
        assert(false && "invalid dimension");
      }
    }

    static constexpr size_t verticesPerCell(){
      return 4;
    }
  
    static size_t numVerticesPerEntity(size_t dim){
      switch(dim){
      case 0:
        return 1;
      case 1:
        return 2;
      case 2:
        return 4;
      case 3:
        return 8;
      default:
        assert(false && "invalid dimension");
      }
    }
  
    static void createEntities(size_t dim, std::vector<Id>& e, Id* v){
      if(dim == 1){
        assert(e.size() == 24);
    
        e[0] = v[0];
        e[1] = v[2];

        e[2] = v[1];
        e[3] = v[3];
    
        e[4] = v[0];
        e[5] = v[1];
    
        e[6] = v[2];
        e[7] = v[3];

        e[8] = v[4];
        e[9] = v[6];

        e[10] = v[5];
        e[11] = v[7];

        e[12] = v[4];
        e[13] = v[5];
    
        e[14] = v[6];
        e[15] = v[7];

        e[16] = v[1];
        e[17] = v[5];

        e[18] = v[3];
        e[19] = v[7];

        e[20] = v[0];
        e[21] = v[4];

        e[22] = v[2];
        e[23] = v[6];
      }
      else if(dim == 2){
        assert(e.size() == 24);

        e[0] = v[0];
        e[1] = v[1];
        e[2] = v[3];
        e[3] = v[2];

        e[4] = v[4];
        e[5] = v[5];
        e[6] = v[7];
        e[7] = v[6];

        e[8] = v[1];
        e[9] = v[5];
        e[10] = v[7];
        e[11] = v[3];

        e[12] = v[0];
        e[13] = v[4];
        e[14] = v[6];
        e[15] = v[2];

        e[16] = v[0];
        e[17] = v[1];
        e[18] = v[5];
        e[19] = v[4];

        e[20] = v[2];
        e[21] = v[3];
        e[22] = v[7];
        e[23] = v[6];
      }
      else{
        assert(false && "invalid dim");
      }
    }
  
  };

#endif // __UNIFORM_MESH_H__
