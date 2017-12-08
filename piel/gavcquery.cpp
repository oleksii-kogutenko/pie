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
#include <logging.h>
#include <gavcconstants.h>

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

#include <boost/format.hpp>

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
    struct Parser {

        std::string _group;
        std::string _name;
        std::vector<std::string> _version;      // empty == all by default
        std::string _classifier;                // empty by default
        std::string _extension;                 // empty by default

        bool parse(Iterator begin, Iterator end)
        {
            using qi::char_;
            using qi::skip;
            using qi::lexeme;
            using qi::_1;
            using qi::phrase_parse;
            using boost::phoenix::ref;
            using boost::phoenix::push_back;
            using boost::spirit::as_string;

            qi::rule<Iterator, std::string()> group, name, version, version_body, 
                classifier, extension, version_ops, version_op, version_const, gavc;

            version_body = +( char_-':' );

            group           = as_string[ lexeme[ +( char_ - ':' ) ] ]               [ ref(_group) = _1 ];
            name            = as_string[ lexeme[ skip[ ':' ] >> +( char_-':' ) ] ]  [ ref(_name) = _1 ];
            classifier      = as_string[ lexeme[ skip[ ':' ] >> +( char_-'@' ) ] ]  [ ref(_classifier) = _1 ];
            extension       = as_string[ lexeme[ skip[ '@' ] >> +( char_ ) ] ]      [ ref(_extension) = _1 ];
            version_ops     = char_('*')|char_('+')|char_('-');
            version_op      = as_string[ lexeme[ version_ops ] ]                    [ push_back( ref(_version), _1 ) ];
            version_const   = as_string[ lexeme[ +( char_-version_ops ) ] ]         [ push_back( ref(_version), _1 ) ];

            gavc =
                group
                    > name
                        > *( (version_op | version_const)
                            > *(classifier
                                > *extension) );

            bool result = true;
            try {
                phrase_parse( begin, end, gavc, ascii::space );
            } catch (...) {
                result = false;
            }

            LOG_T << "* group: "    << _group;
            LOG_T << "* name: "     << _name;
            LOG_T << "classifier: " << _classifier;
            LOG_T << "extension: "  << _extension;

            for (std::vector<std::string>::const_iterator i = _version.begin(), end = _version.end(); i != end; ++i)
            {
                LOG_T << "version op: " << *i;
            }

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
    gavc::Parser<std::string::const_iterator> parser;
    if (!parser.parse(gavc_str.begin(), gavc_str.end()))
        return boost::none;

    GavcQuery result;
    result._group       = parser._group;
    result._name        = parser._name;
    result._classifier  = parser._classifier;
    result._extension   = parser._extension;

    if (!parser._version.empty()) {
        // TODO: Implement metadata query execution plan.
        result._version     = parser._version[0];
    }

    return result;
}

std::string GavcQuery::to_string() const 
{
    // TODO: implement
    return "";
}

std::string GavcQuery::format_maven_metadata_url(const std::string& server_url, const std::string& repository) const
{
    LOG_T << "Build url for maven metadata. server_url: " << server_url << " repository: " << repository;

    std::string group_path = _group;
    std::replace(group_path.begin(), group_path.end(), GavcConstants::group_delimiter, GavcConstants::path_delimiter);
    std::string result = boost::str(boost::format( "%2$s%1$c%3$s%1$c%4$s%1$c%5$s%1$c%6$s" )
        % GavcConstants::path_delimiter % server_url % repository % group_path % _name % GavcConstants::maven_metadata_filename);

    LOG_T << "Maven metadata url: " << result;
    return result;
}

} } // namespace art::lib
