find_package (Boost COMPONENTS system filesystem log unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_definitions(-DBOOST_TEST_DYN_LINK)

include_directories(${Boost_INCLUDE_DIR})

#############################################################
add_executable(properties properties.cpp)
target_link_libraries(properties piel ${Boost_LIBRARIES} crypto zip curl)
add_test (NAME Properties COMMAND properties)
