cmake_minimum_required(VERSION 2.6)
project(pie)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories("piel", "pie")
include_directories("libzip/include")
link_directories("libzip/lib")

add_definitions(-DBOOST_LOG_DYN_LINK)
add_subdirectory(piel)
add_subdirectory(pie)

enable_testing()
add_subdirectory(tests)
