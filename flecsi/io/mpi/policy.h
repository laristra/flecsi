/*
    @@@@@@@@  @@           @@@@@@   @@@@@@@@ @@
   /@@/////  /@@          @@////@@ @@////// /@@
   /@@       /@@  @@@@@  @@    // /@@       /@@
   /@@@@@@@  /@@ @@///@@/@@       /@@@@@@@@@/@@
   /@@////   /@@/@@@@@@@/@@       ////////@@/@@
   /@@       /@@/@@//// //@@    @@       /@@/@@
   /@@       @@@//@@@@@@ //@@@@@@  @@@@@@@@ /@@
   //       ///  //////   //////  ////////  //

   Copyright (c) 2016, Triad National Security, LLC
   All rights reserved.
                                                                              */
#pragma once

/*!  @file */

#include <iostream>
#include <string>

#include <hdf5.h>
#include <mpi.h>

#include "flecsi/data/common/row_vector.h"
#include "flecsi/data/common/serdez.h"
#include "flecsi/data/data_constants.h"
#include "flecsi/execution/context.h"

namespace flecsi {
namespace io {

struct mpi_policy_t {

  bool create_hdf5_file(hid_t & hdf5_file_id,
    const std::string & file_name,
    MPI_Comm mpi_hdf5_comm) {
    int rank;
    MPI_Comm_rank(mpi_hdf5_comm, &rank);

    hid_t file_creation_plist_id = H5P_DEFAULT; // File creation property list
    hid_t file_access_plist_id = H5P_DEFAULT; // File access property list
    MPI_Info mpi_info = MPI_INFO_NULL; // For MPI IO hints

    // Set up file access property list with parallel I/O access
    // H5Pcreate is a general property list create function
    // Here we are creating properties for file access
    file_access_plist_id = H5Pcreate(H5P_FILE_ACCESS);
    assert(file_access_plist_id);

    // Stores the MPI parameters -- comm, info -- in the property list
    int iret = H5Pset_fapl_mpio(file_access_plist_id, mpi_hdf5_comm, mpi_info);
    assert(iret != -1);

    // Open the file collectively
    assert(hdf5_file_id == -1);
    // H5F_ACC_TRUNC is for overwrite existing file if it exists. H5F_ACC_EXCL
    // is no overwrite 3rd argument is file creation property list. Using
    // default here 4th argument is the file access property list identifier
    hdf5_file_id = H5Fcreate(file_name.c_str(), H5F_ACC_TRUNC,
      file_creation_plist_id, file_access_plist_id);
    if(hdf5_file_id < 0 && rank == 0) {
      std::cout << " H5Fcreate failed: " << hdf5_file_id << std::endl;
      return false;
    }
    if(rank == 0) {
      std::cout << " create HDF5 file " << file_name << " file_id "
                << hdf5_file_id << std::endl;
    }

    // Terminates access to property list and frees all memory resources.
    iret = H5Pclose(file_access_plist_id);
    assert(iret != -1);

    return true;
  }

  bool open_hdf5_file(hid_t & hdf5_file_id,
    const std::string & file_name,
    MPI_Comm mpi_hdf5_comm) {
    int rank;
    MPI_Comm_rank(mpi_hdf5_comm, &rank);

    hid_t file_creation_plist_id = H5P_DEFAULT; // File creation property list
    hid_t file_access_plist_id = H5P_DEFAULT; // File access property list
    MPI_Info mpi_info = MPI_INFO_NULL; // For MPI IO hints

    // Set up file access property list with parallel I/O access
    // H5Pcreate is a general property list create function
    // Here we are creating properties for file access
    file_access_plist_id = H5Pcreate(H5P_FILE_ACCESS);
    assert(file_access_plist_id);

    // Stores the MPI parameters -- comm, info -- in the property list
    int iret = H5Pset_fapl_mpio(file_access_plist_id, mpi_hdf5_comm, mpi_info);
    assert(iret != -1);

    assert(hdf5_file_id == -1);
    hdf5_file_id =
      H5Fopen(file_name.c_str(), H5F_ACC_RDWR, file_access_plist_id);
    if(hdf5_file_id < 0 && rank == 0) {
      std::cout << " H5Fopen failed: " << hdf5_file_id << std::endl;
      return false;
    }
    if(rank == 0) {
      std::cout << " open HDF5 file " << file_name << " file_id "
                << hdf5_file_id << std::endl;
    }
    return true;
  }

  bool close_hdf5_file(hid_t & hdf5_file_id, MPI_Comm mpi_hdf5_comm) {
    int rank;
    MPI_Comm_rank(mpi_hdf5_comm, &rank);

    assert(hdf5_file_id >= 0);
    herr_t status;
    status = H5Fflush(hdf5_file_id, H5F_SCOPE_LOCAL);
    assert(status == 0);
    status = H5Fclose(hdf5_file_id);
    assert(status == 0);
    if(rank == 0) {
      std::cout << " close HDF5 file_id " << hdf5_file_id << std::endl;
    }
    hdf5_file_id = -1;
    return true;
  }

  bool create_hdf5_dataset(const hid_t hdf5_file_id,
    const std::string & dataset_name,
    int buffer_size,
    MPI_Comm mpi_hdf5_comm) {
    int rank;
    MPI_Comm_rank(mpi_hdf5_comm, &rank);

    const int ndims = 1;
    hsize_t dims[ndims];
    dims[0] = buffer_size;

    // 1st argument -- number of dimensions
    // 2nd argument -- array of current dimensions
    // 3rd argument -- maximum number of dimensions. NULL means that current is
    // maximum returns the dataspace id. Fortran interface has this inserted as
    // arg 3 and arg 4 as err(max moves to arg 5).
    hid_t file_dataspace_id = H5Screate_simple(ndims, dims, NULL);

    // Creates a new dataset and links to file
    hid_t link_creation_plist_id = H5P_DEFAULT; // Link creation property list
    hid_t dataset_creation_plist_id =
      H5P_DEFAULT; // Dataset creation property list
    hid_t dataset_access_plist_id = H5P_DEFAULT; // Dataset access property list
    hid_t dataset_id = H5Dcreate2(hdf5_file_id, // Arg 1: location identifier
      dataset_name.c_str(), // Arg 2: dataset name
      H5T_STD_U32LE, // Arg 3: datatype identifier
      file_dataspace_id, // Arg 4: dataspace identifier
      link_creation_plist_id, // Arg 5: link creation property list
      dataset_creation_plist_id, // Arg 6: dataset creation property list
      dataset_access_plist_id); // Arg 7: dataset access property list

    if(dataset_id < 0 && rank == 0) {
      std::cout << " H5Dcreate2 failed: " << dataset_id << std::endl;
      H5Sclose(file_dataspace_id);
      H5Fclose(hdf5_file_id);
      return false;
    }

    herr_t status;
    status = H5Dclose(dataset_id);
    assert(status == 0);
    status = H5Sclose(file_dataspace_id);
    assert(status == 0);
    status = H5Fflush(hdf5_file_id, H5F_SCOPE_LOCAL);
    assert(status == 0);
    return true;
  }

  bool write_data_to_hdf5(const hid_t & hdf5_file_id,
    const std::string & dataset_name,
    const void * buffer,
    int nsize,
    int displ,
    MPI_Comm mpi_hdf5_comm,
    const int * offset_buf = nullptr) {
    int rank;
    MPI_Comm_rank(mpi_hdf5_comm, &rank);

    hid_t data_access_plist_id = H5P_DEFAULT;
    hid_t dataset_id =
      H5Dopen2(hdf5_file_id, dataset_name.c_str(), data_access_plist_id);
    if(dataset_id < 0 && rank == 0) {
      std::cout << " H5Dopen2 failed: " << dataset_id << std::endl;
      H5Fclose(hdf5_file_id);
      return false;
    }

    const int ndims = 1;
    hsize_t count[1];
    hsize_t offset[1];
    count[0] = nsize;
    offset[0] = displ;
    hid_t mem_dataspace_id = H5Screate_simple(ndims, count, NULL);

    /*
     * Select hyperslab in the file.
     */
    hid_t file_dataspace_id = H5Dget_space(dataset_id);
    H5Sselect_hyperslab(
      file_dataspace_id, H5S_SELECT_SET, offset, NULL, count, NULL);

    // Create property list for collective dataset write.
    hid_t xfer_plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(xfer_plist_id, H5FD_MPIO_COLLECTIVE);

    // To write dataset independently use
    //    H5Pset_dxpl_mpio(xfer_plist_id, H5FD_MPIO_INDEPENDENT);

    if(offset_buf) {
      herr_t status;
      hsize_t dims[1] = {static_cast<hsize_t>(new_world_size + 1)};
      hid_t attribute_space_id = H5Screate_simple(1, dims, NULL);
      hid_t attribute_id = H5Acreate(dataset_id, "offsets", H5T_NATIVE_UINT,
        attribute_space_id, H5P_DEFAULT, H5P_DEFAULT);

      status = H5Awrite(attribute_id, H5T_NATIVE_UINT, offset_buf);
      assert(status == 0);
      status = H5Aclose(attribute_id);
      assert(status == 0);
    }

    herr_t status;
    status = H5Dwrite(dataset_id, H5T_STD_U32LE, mem_dataspace_id,
      file_dataspace_id, xfer_plist_id, buffer);
    assert(status == 0);
    status = H5Pclose(xfer_plist_id);
    assert(status == 0);
    status = H5Dclose(dataset_id);
    assert(status == 0);
    return true;
  }

  bool read_data_from_hdf5(const hid_t & hdf5_file_id,
    const std::string & dataset_name,
    void * buffer,
    int nsize,
    int displ,
    MPI_Comm mpi_hdf5_comm) {
    int rank;
    MPI_Comm_rank(mpi_hdf5_comm, &rank);

    hid_t data_access_plist_id = H5P_DEFAULT;
    hid_t dataset_id =
      H5Dopen2(hdf5_file_id, dataset_name.c_str(), data_access_plist_id);
    if(dataset_id < 0 && rank == 0) {
      std::cout << " H5Dopen2 failed: " << dataset_id << std::endl;
      H5Fclose(hdf5_file_id);
      return false;
    }

    const int ndims = 1;
    hsize_t count[1];
    hsize_t offset[1];
    count[0] = nsize;
    offset[0] = displ;
    hid_t mem_dataspace_id = H5Screate_simple(ndims, count, NULL);

    /*
     * Select hyperslab in the file.
     */
    hid_t file_dataspace_id = H5Dget_space(dataset_id);
    H5Sselect_hyperslab(
      file_dataspace_id, H5S_SELECT_SET, offset, NULL, count, NULL);

    // Create property list for collective dataset write.
    hid_t xfer_plist_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(xfer_plist_id, H5FD_MPIO_COLLECTIVE);

    herr_t status;
    status = H5Dread(dataset_id, H5T_STD_U32LE, mem_dataspace_id,
      file_dataspace_id, xfer_plist_id, buffer);
    assert(status == 0);
    status = H5Pclose(xfer_plist_id);
    assert(status == 0);
    H5Dclose(dataset_id);
    assert(status == 0);
    return true;
  }

  void create_hdf5_comm() {
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    nb_files = std::ceil(((double) world_size) / ranks_per_file);

    MPI_Comm new_comm;
    new_color = rank / ranks_per_file;
    nb_new_comms = world_size / ranks_per_file;
    MPI_Comm_split(MPI_COMM_WORLD, new_color, rank, &new_comm);

    mpi_hdf5_comm = new_comm;

    MPI_Comm_size(new_comm, &new_world_size);
    MPI_Comm_rank(new_comm, &new_rank);
  }

  void checkpoint_field(const hid_t hdf5_file_id,
    const std::string & field_name,
    const void * buffer,
    const int nsize) {
    // TODO:  do these calcs only once per index space
    int sum_nsize;
    MPI_Allreduce(&nsize, &sum_nsize, 1, MPI_INT, MPI_SUM, mpi_hdf5_comm);
    int displ;
    MPI_Exscan(&nsize, &displ, 1, MPI_INT, MPI_SUM, mpi_hdf5_comm);
    if(new_rank == 0)
      displ = 0;

    bool return_val = false;

    return_val = create_hdf5_dataset(
      hdf5_file_id, field_name.data(), sum_nsize, mpi_hdf5_comm);
    assert(return_val);

    return_val = write_data_to_hdf5(
      hdf5_file_id, field_name.data(), buffer, nsize, displ, mpi_hdf5_comm);
    assert(return_val);
  } // checkpoint_field

  void checkpoint_field_ragged(const hid_t hdf5_file_id,
    const std::string & field_name,
    const std::vector<uint8_t> & rows,
    const int nrows,
    const size_t fid) {
    auto & context = execution::context_t::instance();
    auto serdez = context.get_serdez(fid);
    int data_size = 0;
    int row_vector_size = sizeof(data::row_vector_u<uint8_t>);
    for(int i = 0; i < nrows; ++i) {
      data_size += serdez->serialized_size(&rows[i * row_vector_size]);
    }
    // TODO:  handle case where this doesn't divide evenly
    data_size /= sizeof(int);

    int sum_data_size;
    MPI_Allreduce(
      &data_size, &sum_data_size, 1, MPI_INT, MPI_SUM, mpi_hdf5_comm);
    int displ;
    MPI_Exscan(&data_size, &displ, 1, MPI_INT, MPI_SUM, mpi_hdf5_comm);
    if(new_rank == 0)
      displ = 0;
    std::vector<int> offset_buf(new_world_size + 1);
    MPI_Allgather(
      &displ, 1, MPI_INT, offset_buf.data(), 1, MPI_INT, mpi_hdf5_comm);
    offset_buf[new_world_size] = sum_data_size;

    bool return_val = false;
    return_val = create_hdf5_dataset(
      hdf5_file_id, field_name.data(), sum_data_size, mpi_hdf5_comm);
    assert(return_val);

    std::vector<int> buffer(data_size);
    const char * row_ptr = (char *)rows.data();
    char * buf_ptr = (char *)buffer.data();
    for(int i = 0; i < nrows; ++i) {
      int item_size = serdez->serialize(row_ptr, buf_ptr);
      row_ptr += row_vector_size;
      buf_ptr += item_size;
    }

    return_val = write_data_to_hdf5(hdf5_file_id, field_name, buffer.data(),
      data_size, displ, mpi_hdf5_comm, offset_buf.data());
    assert(return_val);
  } // checkpoint_field_ragged

  void recover_field(const hid_t hdf5_file_id,
    const std::string & field_name,
    void * buffer,
    const int nsize) {
    int displ;
    MPI_Exscan(&nsize, &displ, 1, MPI_INT, MPI_SUM, mpi_hdf5_comm);
    if(new_rank == 0)
      displ = 0;

    bool return_val = false;

    return_val = read_data_from_hdf5(
      hdf5_file_id, field_name.data(), buffer, nsize, displ, mpi_hdf5_comm);
    assert(return_val);
  } // recover_field

  void recover_field_ragged(const hid_t hdf5_file_id,
    const std::string & field_name,
    const std::vector<uint8_t> & rows,
    const int nrows,
    const size_t fid) {
    hid_t attribute_id = H5Aopen_by_name(
      hdf5_file_id, field_name.c_str(), "offsets", H5P_DEFAULT, H5P_DEFAULT);
    std::vector<int> offset_buf(new_world_size + 1);
    herr_t status;
    status = H5Aread(attribute_id, H5T_NATIVE_UINT, offset_buf.data());
    assert(status == 0);
    status = H5Aclose(attribute_id);
    assert(status == 0);

    int data_size = offset_buf[new_rank + 1] - offset_buf[new_rank];
    int displ = offset_buf[new_rank];
    std::vector<int> buffer(data_size);

    bool return_val = false;
    return_val = read_data_from_hdf5(
      hdf5_file_id, field_name, buffer.data(), data_size, displ, mpi_hdf5_comm);
    assert(return_val);

    auto & context = execution::context_t::instance();
    auto serdez = context.get_serdez(fid);
    char * row_ptr = (char *)rows.data();
    const char * buf_ptr = (char *)buffer.data();
    int row_vector_size = sizeof(data::row_vector_u<uint8_t>);
    for(int i = 0; i < nrows; ++i) {
      int item_size = serdez->deserialize(row_ptr, buf_ptr);
      row_ptr += row_vector_size;
      buf_ptr += item_size;
    }

  } // recover_field_ragged

  void checkpoint_all_fields(const std::string & file_name_in) {
    // TODO:  make this happen only once
    create_hdf5_comm();

    hid_t hdf5_file_id = -1;
    bool return_val = false;

    // initialize HDF5 library
    // this is only really required for the Fortran interface
    return_val = H5open();
    assert(return_val == 0);

    // create hdf5 file
    if(rank == 0)
      std::cout << "Creating HDF5 file " << std::endl << world_size;

    std::string file_name = file_name_in + std::to_string(new_color);
    return_val = create_hdf5_file(hdf5_file_id, file_name, mpi_hdf5_comm);
    assert(return_val);

    // checkpoint
    if(rank == 0)
      std::cout << "Writing checkpoint" << std::endl;
    auto & context = execution::context_t::instance();
    const auto & field_data = context.registered_field_data();
    const auto & sparse_field_data = context.registered_sparse_field_data();
    const auto & field_info = context.registered_fields();
    for(const auto & info : field_info) {
      switch(info.storage_class) {
        case data::dense: {
          size_t fid = info.fid;
          std::string field_name = "fid_" + std::to_string(fid);
          auto & data = field_data.at(fid);
          // TODO:  handle case where this doesn't divide evenly
          int size = data.size() / sizeof(int);
          const void * buffer = data.data();
          checkpoint_field(hdf5_file_id, field_name, buffer, size);
        } break;

        case data::ragged:
        case data::sparse: {
          size_t fid = info.fid;
          std::string field_name = "fid_" + std::to_string(fid);
          auto & data = sparse_field_data.at(fid);
          int nrows = data.num_total;
          const auto & rows = data.rows;
          checkpoint_field_ragged(hdf5_file_id, field_name, rows, nrows, fid);
        } break;

        default:
          // do nothing
          break;
      }
    }

    return_val = close_hdf5_file(hdf5_file_id, mpi_hdf5_comm);
    assert(return_val);
  } // checkpoint_all_fields

  void recover_all_fields(const std::string & file_name_in) {
    create_hdf5_comm();

    hid_t hdf5_file_id = -1;
    bool return_val = false;

    // recover
    if(rank == 0)
      std::cout << "Recovering checkpoint" << std::endl;
    std::string file_name = file_name_in + std::to_string(new_color);
    return_val = open_hdf5_file(hdf5_file_id, file_name, mpi_hdf5_comm);
    assert(return_val);

    auto & context = execution::context_t::instance();
    auto & field_data = context.registered_field_data();
    const auto & sparse_field_data = context.registered_sparse_field_data();
    const auto & field_info = context.registered_fields();
    auto & index_space_info = context.sparse_index_space_info_map();
    for(const auto & info : field_info) {
      switch(info.storage_class) {
        case data::dense: {
          size_t fid = info.fid;
          size_t is = info.index_space;
          std::string field_name = "fid_" + std::to_string(fid);
          auto it = field_data.find(fid);
          if(it == field_data.end()) {
            // instantiate field if not already there
            const auto & color_info =
              (context.coloring_info(is)).at(context.color());
            size_t size = info.size * (color_info.exclusive +
                                        color_info.shared + color_info.ghost);
            context.register_field_data(fid, size);
            it = field_data.find(fid);
          }
          assert(it != field_data.end() && "messed up");
          auto & data = field_data.at(fid);
          size_t size = data.size() / sizeof(int);
          void * buffer = data.data();
          recover_field(hdf5_file_id, field_name, buffer, size);
        } break;

        case data::ragged:
        case data::sparse: {
          size_t fid = info.fid;
          size_t is = info.index_space;
          std::string field_name = "fid_" + std::to_string(fid);
          auto it = sparse_field_data.find(fid);
          if(it == sparse_field_data.end()) {
            // instantiate field if not already there
            const auto & color_info =
              (context.coloring_info(is)).at(context.color());
            const auto & is_info = index_space_info.at(is);
            const auto max_entries_per_index = is_info.max_entries_per_index;
            context.register_sparse_field_data(
              fid, info.size, color_info, max_entries_per_index);
            it = sparse_field_data.find(fid);
          }
          assert(it != sparse_field_data.end() && "messed up");
          auto & data = sparse_field_data.at(fid);
          int nrows = data.num_total;
          int type_size = data.type_size;
          const auto & rows = data.rows;
          recover_field_ragged(hdf5_file_id, field_name, rows, nrows, fid);
        } break;

        default:
          // do nothing
          break;
      }
    }

    return_val = close_hdf5_file(hdf5_file_id, mpi_hdf5_comm);
    assert(return_val);
  } // recover_all_fields

  int ranks_per_file = 1;
  int nb_files;

  int world_size, rank, new_world_size, new_rank;

  int new_color;
  int nb_new_comms;

  MPI_Comm mpi_hdf5_comm;
}; // struct mpi_policy_t

} // namespace io
} // namespace flecsi
