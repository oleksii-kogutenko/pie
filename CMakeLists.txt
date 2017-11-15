cmake_minimum_required(VERSION 2.6)
project(pie)

add_definitions(-DBOOST_LOG_DYN_LINK)

include_directories("piel", "pie")

add_subdirectory(piel)
add_subdirectory(pie)
