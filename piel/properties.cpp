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

#include <properties.h>

#include <logging.h>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/algorithm/string/trim.hpp>

typedef std::pair<std::string,std::string> StdPair;
BOOST_FUSION_ADAPT_STRUCT(
    StdPair,
    (std::string, first)
    (std::string, second)
)

namespace piel { namespace lib {

namespace java_properties {

    namespace qi = boost::spirit::qi;

    std::string trim_str(const std::string& input) {
        std::string result = input;
        boost::algorithm::trim(result);
        return result;
    }
    BOOST_PHOENIX_ADAPT_FUNCTION(std::string, trim_str_, trim_str, 1)

    template<typename Iterator>
    struct grammar: qi::grammar<Iterator, StdPair()> {
        grammar(): grammar::base_type(_data)
        {
            using qi::char_;
            using qi::space;
            using qi::skip;
            using qi::_1;
            using qi::_val;
            using qi::as_string;
            using boost::phoenix::ref;

            _syms       = (char_('=')|char_('#'));
            _comment    = skip['#' > -( +( char_ ) ) ];
            _first      = as_string[ +( char_ - _syms ) > skip[ '=' ] ] [ _val = trim_str_(_1) ];
            _second     = as_string[ +( char_ - '#' ) ]                 [ _val = trim_str_(_1) ];
            _data       = -( _first > -_second ) | -_comment;
        }

    private:
        qi::rule<Iterator>                                      _syms;
        qi::rule<Iterator>                                      _comment;
        qi::rule<Iterator, std::string()>                       _first;
        qi::rule<Iterator, std::string()>                       _second;
        qi::rule<Iterator, StdPair()>                           _data;
    };


} // namespace java_properties

Properties::Properties()
    : _data()
{

}

Properties::~Properties()
{

}

Properties Properties::load(std::istream &is)
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    Properties result;

    std::string buffer;
    java_properties::grammar<std::string::iterator> java_properties_grammar;

    while (is) {
        StdPair pair;
        std::getline(is, buffer);
        LOG_T << "buffer: " << buffer;
        qi::phrase_parse( buffer.begin(), buffer.end(), java_properties_grammar, ascii::space, pair );
        result._data.insert(pair);
        LOG_T << "'"<< pair.first << "' = '" << pair.second << "'";
    }

    return result;
}

void Properties::store(std::ostream &os) const
{
    for(MapType::const_iterator i = _data.begin(), end = _data.end(); i != end; ++i)
    {
        os << i->first << "=" << i->second << std::endl;
    }
}

void Properties::test_properties()
{
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

    std::istringstream iss(test_properties);
    piel::lib::Properties p = piel::lib::Properties::load(iss);

    LOG_T << "test prop1: "<< p["test prop1"];
    LOG_T << "test_prop2: "<< p["test_prop2"];
    LOG_T << "test_prop3: "<< p["test_prop3"];
    LOG_T << "test_prop4: "<< p["test_prop4"];
    LOG_T << "test_prop5: "<< p["test_prop5"];
}

} } //namespace piel::lib
