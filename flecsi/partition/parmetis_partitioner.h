/*~--------------------------------------------------------------------------~*
 * Copyright (c) 2015 Los Alamos National Security, LLC
 * All rights reserved.
 *~--------------------------------------------------------------------------~*/

#ifndef flecsi_dmp_parmetis_partitioner_h
#define flecsi_dmp_parmetis_partitioner_h

#include "flecsi/partition/partitioner.h"

#include <mpi.h>
#include <parmetis.h>

///
// \file parmetis_partitioner.h
// \authors bergen
// \date Initial file creation: Nov 24, 2016
///

namespace flecsi {
namespace dmp {

///
// \class parmetis_partitioner_t parmetis_partitioner.h
// \brief parmetis_partitioner_t provides...
///
class parmetis_partitioner_t
  : public partitioner_t
{
public:

  /// Default constructor
  parmetis_partitioner_t() {}

  /// Copy constructor (disabled)
  parmetis_partitioner_t(const parmetis_partitioner_t &) = delete;

  /// Assignment operator (disabled)
  parmetis_partitioner_t & operator = (const parmetis_partitioner_t &) = delete;

  /// Destructor
   ~parmetis_partitioner_t() {}

	///
	// Generate a primary partition using the ParMETIS library.
	///
  std::set<size_t>
  partition(
    dcrs_t & mesh
  )
  {
    int size;
    int rank;

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    idx_t wgtflag = 0;
    idx_t numflag = 0;
    idx_t ncon = 1;
    std::vector<real_t> tpwgts(size);

    real_t sum = 0.0;
		for(size_t i(0); i<tpwgts.size(); ++i) {
			if(i == (tpwgts.size()-1)) {
				tpwgts[i] = 1.0 - sum;
			}
			else {
				tpwgts[i] = 1.0/size;
				sum += tpwgts[i];
			} // if

			#if 0
			if(rank == 0) {
				std::cout << tpwgts[i] << std::endl;
			} // if
			#endif
		} // for

		real_t ubvec = 1.05;
		idx_t options = 0;
		idx_t edgecut;
		MPI_Comm comm = MPI_COMM_WORLD;
		std::vector<idx_t> part(mesh.indices());

		int result = ParMETIS_V3_PartKway(mesh.vtxdist(), mesh.xadj(),
			mesh.adjncy(), nullptr, nullptr, &wgtflag, &numflag, &ncon, &size,
			&tpwgts[0], &ubvec, &option, &edgecut, &part[0], &comm);
  } // partition

private:

}; // class parmetis_partitioner_t

} // namespace dmp
} // namespace flecsi

#endif // flecsi_dmp_parmetis_partitioner_h

/*~-------------------------------------------------------------------------~-*
 * Formatting options for vim.
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
