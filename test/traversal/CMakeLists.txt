include_directories(${CMAKE_SOURCE_DIR}/src/base)

link_directories(${CMAKE_BINARY_DIR}/lib)

add_executable(traversal traversal.cc)

target_link_libraries(traversal jali)
