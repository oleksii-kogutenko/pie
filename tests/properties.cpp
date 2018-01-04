/*
 * Copyright (c) 2017, Dmytro Iakovliev daemondzk@gmail.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Dmytro Iakovliev daemondzk@gmail.com ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Dmytro Iakovliev daemondzk@gmail.com BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#define BOOST_TEST_MODULE Properties
#include <boost/test/unit_test.hpp>

#include <sstream>
#include <properties.h>

std::string test_properties = "test prop1=test val1\n"
        "test_prop1=test_val1   \n"
        "test_prop2    =    test_val2\n"
        "#test_prop3  =test_val3\n"
        "\n"
        "\t\n"
        "\n\n\n"
        "  # comment\n"
        "                 test_prop4=test_val4#Comment here\n"
        "test_prop5=test_val5\n";

std::string stored_content = "test prop1=test val1\n"
        "test_prop1=test_val1\n"
        "test_prop2=test_val2\n"
        "test_prop4=test_val4\n"
        "test_prop5=test_val5\n";

BOOST_AUTO_TEST_CASE(Properties)
{
    std::istringstream iss( test_properties );
    piel::lib::Properties p = piel::lib::Properties::load( iss );

    BOOST_CHECK_EQUAL( "test val1", p["test prop1"] );
    BOOST_CHECK_EQUAL( "test_val2", p["test_prop2"] );
    BOOST_CHECK_EQUAL( "test_val4", p["test_prop4"] );
    BOOST_CHECK_EQUAL( "test_val5", p["test_prop5"] );

    std::ostringstream oss;
    p.store( oss );
    BOOST_CHECK_EQUAL( stored_content, oss.str() );

    BOOST_CHECK_EQUAL( "", p["test_prop3"] );
    BOOST_CHECK_EQUAL( "", p["    test_prop5"] );

    BOOST_CHECK_NE( "123", p["    test_prop5  "] );
}

BOOST_AUTO_TEST_CASE(Properties_from_env)
{
    piel::lib::Properties p;

    BOOST_CHECK_EQUAL("def_val",                    p.get(piel::lib::Properties::Property("test_prop", "def_val")).value());
    BOOST_CHECK_EQUAL("test_env_var_default_value", p.get(piel::lib::Properties::Property("test_prop", "def_val").default_from_env("test_env_var")).value());

    p.set("test_prop", "test_value");

    BOOST_CHECK_EQUAL("test_value",                 p.get(piel::lib::Properties::Property("test_prop", "def_val")).value());
    BOOST_CHECK_EQUAL("test_value",                 p.get(piel::lib::Properties::Property("test_prop", "def_val").default_from_env("test_env_var")).value());
}
