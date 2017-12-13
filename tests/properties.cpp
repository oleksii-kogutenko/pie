#define BOOST_TEST_MODULE Properties
#include <boost/test/unit_test.hpp>

#include <sstream>
#include <properties.h>

std::string test_properties =   "test prop1=test val1\n"
                                "test_prop1=test_val1   \n"
                                "test_prop2    =    test_val2\n"
                                "#test_prop3  =test_val3\n"
                                "\n"
                                "\t\n"
                                "\n\n\n"
                                "  # comment\n"
                                "                 test_prop4=test_val4#Comment here\n"
                                "test_prop5=test_val5\n";

BOOST_AUTO_TEST_CASE(Properties)
{
    std::istringstream iss(test_properties);
    piel::lib::Properties p = piel::lib::Properties::load(iss);

    BOOST_CHECK_EQUAL("test val1", p["test prop1"]);
    BOOST_CHECK_EQUAL("test_val2", p["test_prop2"]);
    BOOST_CHECK_EQUAL("test_val4", p["test_prop4"]);
    BOOST_CHECK_EQUAL("test_val5", p["test_prop5"]);

    BOOST_CHECK_EQUAL("", p["test_prop3"]);
    BOOST_CHECK_EQUAL("", p["    test_prop5"]);

    BOOST_CHECK_NE("123", p["    test_prop5  "]);

}

