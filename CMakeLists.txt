cmake_minimum_required ( VERSION 3.5 )
project ( BoostMPISandbox )

set ( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

include ( BoostMPI )

set (CMAKE_CXX_STANDARD 14)

add_subdirectory ( src )
