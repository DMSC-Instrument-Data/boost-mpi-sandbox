add_executable ( send send.cc )

include_directories(${MPI_INCLUDE_PATH})

target_link_libraries( send ${MPI_LIBRARIES})

if(MPI_COMPILE_FLAGS)
  set_target_properties(send PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(send PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

target_link_libraries ( send Boost::mpi )
