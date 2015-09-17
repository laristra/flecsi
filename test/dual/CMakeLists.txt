include_directories(${CMAKE_SOURCE_DIR}/src/base)

link_directories(${CMAKE_BINARY_DIR}/lib)

add_executable(dual dual.cc)

target_link_libraries(dual jali)
