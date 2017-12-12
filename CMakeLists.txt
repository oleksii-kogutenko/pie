cmake_minimum_required(VERSION 2.6)
project(pie)

include_directories("piel", "pie")

add_definitions(-DBOOST_LOG_DYN_LINK)
add_subdirectory(piel)
add_subdirectory(pie)

enable_testing()
add_subdirectory(tests)
