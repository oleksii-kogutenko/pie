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

#include <gavcquery.h>
#include <gavcconstants.h>

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/qi.hpp>

namespace art { namespace lib {

//! Versions based queries
//
//  '*' - all
//  '+' - latest
//  '-' - oldest
// 
// prefix(+|-|*\.)+suffix
//  - calculation from left to right
//    (+|-|*\.)(+|-) == (+|-) (single element)
//    (+|-|*\.)* == * (set)
//
// Pairs conversion matrix:
//     -------------
//     | + | - | * |
// -----------------
// | + | + | - | + |
// -----------------
// | - | - | - | - |
// -----------------
// | * | + | - | * |
// -----------------
namespace gavc {

    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    template<typename Iterator>
    struct parser {

        std::string _group;
        std::string _name;
        std::string _version;       // all by default
        std::string _classifier;    // empty by default
        std::string _extension;     // empty by default

        bool parse(Iterator begin, Iterator end)
        {
            using qi::char_;
            using qi::skip;
            using qi::lexeme;
            using qi::_1;
            using qi::phrase_parse;
            using boost::phoenix::ref;
            using boost::spirit::as_string;

            qi::rule<Iterator, std::string()> group, name, version, version_body, classifier, extension, gavc;

            version_body = +(char_-':');

            group       = as_string[lexeme[+(char_-':')]]               [ref(_group) = _1];
            name        = as_string[lexeme[skip[':'] >> +(char_-':')]]  [ref(_name) = _1];
            version     = as_string[lexeme[skip[':'] >> version_body]]  [ref(_version) = _1];
            classifier  = as_string[lexeme[skip[':'] >> +(char_-'@')]]  [ref(_classifier) = _1];
            extension   = as_string[lexeme[skip['@'] >> +(char_)]]      [ref(_extension) = _1];

            gavc =
                group
                    > name
                        > *(version
                            > *(classifier
                                > *extension) );

            bool result = true;
            try {
                phrase_parse(begin, end, gavc, ascii::space);
            } catch (...) {
                result = false;
            }

            BOOST_LOG_TRIVIAL(trace) << "* group: "      << _group;
            BOOST_LOG_TRIVIAL(trace) << "* name: "       << _name;
            BOOST_LOG_TRIVIAL(trace) << "version: "    << _version;
            BOOST_LOG_TRIVIAL(trace) << "classifier: " << _classifier;
            BOOST_LOG_TRIVIAL(trace) << "extension: "  << _extension;

            return result;
        }

    };
}

GavcQuery::GavcQuery()
    : _group()
    , _name()
    , _version()
    , _classifier()
    , _extension()
{
}

GavcQuery::~GavcQuery()
{
}

// req         :req         :opt              :opt           @opt
// <group.spec>:<name-spec>[:][<version.spec>[:{[classifier][@][extension]}]]
boost::optional<GavcQuery> GavcQuery::parse(const std::string& gavc_str)
{
    gavc::parser<std::string::const_iterator> parser;
    if (!parser.parse(gavc_str.begin(), gavc_str.end()))
        return boost::none;

    GavcQuery result;
    result._group       = parser._group;
    result._name        = parser._name;
    result._version     = parser._version;
    result._classifier  = parser._classifier;
    result._extension   = parser._extension;

    return result;
}

std::string GavcQuery::to_string() const 
{
    // TODO: implement
    return "";
}

} } // namespace art::lib
