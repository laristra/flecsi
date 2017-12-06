# CMake generated Testfile for 
# Source directory: /home/ollie/ClionProjects/flecsi/flecsi/data
# Build directory: /home/ollie/ClionProjects/flecsi/build_master_mpi/flecsi/data
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(client_registration "/home/ollie/opt/mpich-3.2/bin/mpirun" "-np" "2" "/home/ollie/ClionProjects/flecsi/build_master_mpi/test/data/client_registration" "--gtest_color=no")
set_tests_properties(client_registration PROPERTIES  WORKING_DIRECTORY "/home/ollie/ClionProjects/flecsi/build_master_mpi/test/data")
