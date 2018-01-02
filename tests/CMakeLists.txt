find_package (Boost COMPONENTS system filesystem random log unit_test_framework REQUIRED)
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

#############################################################
add_executable(maven_metadata_test maven_metadata_test.cpp)
target_link_libraries(maven_metadata_test piel ${Boost_LIBRARIES} crypto zip curl)
add_test (NAME MavenMetadataTest COMMAND maven_metadata_test)

#############################################################
add_executable(gavc_versions_filter gavc_versions_filter.cpp)
target_link_libraries(gavc_versions_filter piel ${Boost_LIBRARIES} crypto zip curl)
add_test (NAME GavcVersionsFilterTest COMMAND gavc_versions_filter)

#############################################################
add_executable(gavc_versions_matcher gavc_versions_matcher.cpp)
target_link_libraries(gavc_versions_matcher piel ${Boost_LIBRARIES} crypto zip curl)
add_test (NAME GavcVersionsMatcherTest COMMAND gavc_versions_matcher)

#############################################################
add_executable(gavc_versions_comparator gavc_versions_comparator.cpp)
target_link_libraries(gavc_versions_comparator piel ${Boost_LIBRARIES} crypto zip curl)
add_test (NAME GavcVersionsComparatorTest COMMAND gavc_versions_comparator)

#############################################################
add_executable(assets_tests assets_tests.cpp)
target_link_libraries(assets_tests piel ${Boost_LIBRARIES} crypto zip curl)
add_test (NAME AssetsTests COMMAND assets_tests)

#############################################################
add_executable(indexers_tests indexers_tests.cpp)
target_link_libraries(indexers_tests piel ${Boost_LIBRARIES} crypto zip curl)
add_test (NAME IndexersTests COMMAND indexers_tests)
