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
#include <boost/format.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>

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

namespace art { namespace lib {

namespace gavc {

    struct gavc_data {
        std::string group;
        std::string name;
        std::string version;        // empty == all by default
        std::string classifier;     // empty by default
        std::string extension;      // empty by default
    };

} // namespace gavc

} } // namespace art::lib

BOOST_FUSION_ADAPT_STRUCT(
    art::lib::gavc::gavc_data,
    (std::string, group)
    (std::string, name)
    (std::string, version)
    (std::string, classifier)
    (std::string, extension)
)

namespace art { namespace lib {

namespace gavc {

    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    template<typename Iterator>
    struct gavc_version_ops_grammar: qi::grammar<Iterator, std::vector<OpType>(), ascii::space_type> {
        gavc_version_ops_grammar(): gavc_version_ops_grammar::base_type(_ops)
        {
            using qi::char_;
            using qi::as_string;
            using qi::_1;
            using qi::_val;

            _op      = ops;
            _const   = as_string[ +( char_-_op ) ];
            _ops     = *( _op | _const );
        }

    private:
        struct ops_: qi::symbols<char,OpType> {
            ops_()
            {
                add_sym("*", Op_all);
                add_sym("+", Op_latest);
                add_sym("-", Op_oldest);
            }

            void add_sym(const std::string& sym, Ops op)
            {
                add(sym, OpType(op, sym));
            }
        } ops;

        qi::rule<Iterator, OpType()>                                    _op;        //!< Operation.
        qi::rule<Iterator, OpType()>                                    _const;     //!< Constanst.
        qi::rule<Iterator, std::vector<OpType>(), ascii::space_type>    _ops;
    };

    // req         :req         :opt              :opt           @opt
    // <group.spec>:<name-spec>[:][<version.spec>[:{[classifier][@][extension]}]]
    template<typename Iterator>
    struct gavc_grammar: qi::grammar<Iterator, gavc_data(), ascii::space_type> {
        gavc_grammar(): gavc_grammar::base_type(_gavc)
        {

            using qi::char_;
            using qi::skip;

            _body            = +( char_ - ":" );
            _group           = _body > skip[ ":" ];
            _name            = _body > -( skip[ ":" ] );
            _version         = _body > -( skip[ ":" ] );
            _classifier      = +( char_ - "@" ) > -( skip[ "@" ] );
            _extension       = +( char_ );

            _gavc   =  _group
                    >  _name
                    >  -_version
                    >  -_classifier
                    >  -_extension
                    ;

        }
    private:
        qi::rule<Iterator, std::string()>                   _body;          //!< Helper.
        qi::rule<Iterator, std::string()>                   _group;         //!< Consumer is gavc_data.group .
        qi::rule<Iterator, std::string()>                   _name;          //!< Consumer is gavc_data.name .
        qi::rule<Iterator, std::string()>                   _version;       //!< Consumer is gavc_data.version .
        qi::rule<Iterator, std::string()>                   _classifier;    //!< Consumer is gavc_data.classifier .
        qi::rule<Iterator, std::string()>                   _extension;     //!< Consumer is gavc_data.extension .
        qi::rule<Iterator, gavc_data(), ascii::space_type>  _gavc;          //!< Consumer is gavc_data.
    };

} // namespace gavc

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

boost::optional<GavcQuery> GavcQuery::parse(const std::string& gavc_str)
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    gavc::gavc_data                                     data;
    gavc::gavc_grammar<std::string::const_iterator>     grammar;

    try {
        qi::phrase_parse( gavc_str.begin(), gavc_str.end(), grammar, ascii::space, data );
    } catch (...) {
        return boost::none;
    }

    GavcQuery result;
    result._group       = data.group;
    result._name        = data.name;
    result._version     = data.version;
    result._classifier  = data.classifier;
    result._extension   = data.extension;

    LOG_T << "group: "      << result._group;
    LOG_T << "name: "       << result._name;
    LOG_T << "version: "    << result._version;
    LOG_T << "classifier: " << result._classifier;
    LOG_T << "extension: "  << result._extension;

    //result.query_version_ops();

    return result;
}

boost::optional<std::vector<gavc::OpType> > GavcQuery::query_version_ops() const
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    std::vector<gavc::OpType>                                   query_ops;
    gavc::gavc_version_ops_grammar<std::string::const_iterator> version_ops_grammar;

    try {
        qi::phrase_parse( _version.begin(), _version.end(), version_ops_grammar, ascii::space, query_ops );
    } catch (...) {
        return boost::none;
    }

    for (std::vector<gavc::OpType>::const_iterator i = query_ops.begin(), end = query_ops.end(); i != end; ++i)
    {
        LOG_T << "version query op: " << i->second;
    }

    return query_ops;
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
