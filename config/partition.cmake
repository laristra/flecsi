option(ENABLE_PARTITION "Enable partitioning with third party libraries." OFF)

if(ENABLE_PARTITION)

  set( PARTITION_LIBRARIES )

  find_library ( METIS_LIBRARY 
                 NAMES metis 
                 PATHS ${METIS_ROOT} 
                 PATH_SUFFIXES lib
                 NO_DEFAULT_PATH )

  find_path    ( METIS_INCLUDE_DIR 
                 NAMES metis.h 
                 PATHS ${METIS_ROOT} 
                 PATH_SUFFIXES include
                 NO_DEFAULT_PATH )

  find_library ( SCOTCH_LIBRARY 
                 NAMES scotch
                 PATHS ${SCOTCH_ROOT} 
                 PATH_SUFFIXES lib
                 NO_DEFAULT_PATH )

  find_library ( SCOTCH_ERR_LIBRARY 
                 NAMES scotcherr
                 PATHS ${SCOTCH_ROOT} 
                 PATH_SUFFIXES lib
                 NO_DEFAULT_PATH )

  find_path    ( SCOTCH_INCLUDE_DIR 
                 NAMES scotch.h
                 PATHS ${SCOTCH_ROOT} 
                 PATH_SUFFIXES include
                 NO_DEFAULT_PATH )

  if (METIS_LIBRARY AND METIS_INCLUDE_DIR) 
     message(STATUS "Found METIS")
     set( METIS_FOUND TRUE )
     include_directories(${METIS_INCLUDE_DIR})
     list( APPEND PARTITION_LIBRARIES ${METIS_LIBRARY} )
  endif()

  if (SCOTCH_LIBRARY AND SCOTCH_ERR_LIBRARY AND SCOTCH_INCLUDE_DIR) 
     message(STATUS "Found SCOTCH" )
     set( SCOTCH_FOUND TRUE )
     include_directories(${SCOTCH_INCLUDE_DIR})
     list( APPEND PARTITION_LIBRARIES ${SCOTCH_LIBRARY} ${SCOTCH_ERR_LIBRARY} )
  endif()

  if ( NOT PARTITION_LIBRARIES )
     MESSAGE( FATAL_ERROR "Need to specify either SCOTCH or METIS" )
  endif()

endif()
