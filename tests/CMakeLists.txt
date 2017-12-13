find_package (Boost COMPONENTS system filesystem log unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_definitions(-DBOOST_TEST_DYN_LINK)

include_directories(${Boost_INCLUDE_DIR})

#############################################################
add_executable(properties properties.cpp)
target_link_libraries(properties piel ${Boost_LIBRARIES} crypto zip curl)
add_test (NAME Properties COMMAND properties)

#############################################################
add_executable(gavc_query_parsing gavc_query_parsing.cpp)
target_link_libraries(gavc_query_parsing piel ${Boost_LIBRARIES} crypto zip curl)
add_test (NAME GacvQueryParsing COMMAND gavc_query_parsing)
