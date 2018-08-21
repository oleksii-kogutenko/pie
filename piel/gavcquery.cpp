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
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>

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

            _op      = ops;
            _const   = as_string[ +( char_-_op ) ];
            _ops     = *( (_op >> !_op) | _const );
        }

    private:
        struct ops_: qi::symbols<char,OpType> {
            ops_()
            {
                add_sym(GavcConstants::all_versions, Op_all);
                add_sym(GavcConstants::latest_version, Op_latest);
                add_sym(GavcConstants::oldest_version, Op_oldest);
            }

            void add_sym(const std::string& sym, Ops op)
            {
                add(sym, OpType(op, sym));
            }

            void add_sym(char ch, Ops op)
            {
                std::string sym;
                sym.push_back(ch);
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

            _body            = +( char_ - GavcConstants::delimiter );
            _group           = _body > skip[ GavcConstants::delimiter ];
            _name            = _body > -( skip[ GavcConstants::delimiter ] );
            _version         = _body > -( skip[ GavcConstants::delimiter ] );
            _classifier      = +( char_ - GavcConstants::extension_prefix ) > -( skip[ GavcConstants::extension_prefix ] );
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
    : data_()
{
}

GavcQuery::~GavcQuery()
{
}

boost::optional<GavcQuery> GavcQuery::parse(const std::string& gavc_str)
{
    if (gavc_str.empty())
        return boost::none;

    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    GavcQuery                                       result;
    gavc::gavc_grammar<std::string::const_iterator> grammar;

    try {
        qi::phrase_parse( gavc_str.begin(), gavc_str.end(), grammar, ascii::space, result.data_ );
    } catch (...) {
        return boost::none;
    }

    LOGT << "group: "      << result.group()        << ELOG;
    LOGT << "name: "       << result.name()         << ELOG;
    LOGT << "version: "    << result.version()      << ELOG;
    LOGT << "classifier: " << result.classifier()   << ELOG;
    LOGT << "extension: "  << result.extension()    << ELOG;

    // Attempt to parse versions query
    boost::optional<std::vector<gavc::OpType> > ops = result.query_version_ops();
    if (!ops) {
        return boost::none;
    }

    return result;
}

boost::optional<std::vector<gavc::OpType> > GavcQuery::query_version_ops() const
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    std::vector<gavc::OpType> result;
    if (data_.version.empty()) {
        result.push_back(gavc::OpType(gavc::Op_all, GavcConstants::all_versions));
        return result;
    }

    gavc::gavc_version_ops_grammar<std::string::const_iterator> version_ops_grammar;

    try {
        qi::phrase_parse( data_.version.begin(), data_.version.end(), version_ops_grammar, ascii::space, result );
    } catch (...) {
        return boost::none;
    }

    for (std::vector<gavc::OpType>::iterator i = result.begin(), end = result.end(); i != end; ++i )
    {
        LOGT << "version query op: " << i->second << ELOG;
    }

    if (!data_.version.empty() & result.empty())
    {
        LOGE << "version query: " << data_.version << " has wrong syntax!" << ELOG;
        return boost::none;
    }

    return result;
}

struct QueryOpsFinder_exact_version_query {
    bool operator()(const gavc::OpType& op) {
        return op.first > gavc::Op_const;
    }
};

bool GavcQuery::is_exact_version_query() const
{
    boost::optional<std::vector<gavc::OpType> > pops = query_version_ops();
    if (!pops) return false;

    return std::find_if(pops->begin(), pops->end(), QueryOpsFinder_exact_version_query()) == pops->end();
}

bool GavcQuery::is_single_version_query() const
{
    boost::optional<std::vector<gavc::OpType> > pops = query_version_ops();
    if (!pops) return false;

    bool is_last_op_all = false;
    for(std::vector<gavc::OpType>::const_iterator i = pops->begin(), end = pops->end(); i != end; ++i)
    {
        if (i->first != gavc::Op_const)
            is_last_op_all = i->first == gavc::Op_all;
    }

    return !is_last_op_all;
}

std::string GavcQuery::to_string() const
{
    std::ostringstream result;

    if (!group().empty())
    {
        result << group();
    }
    if (!name().empty())
    {
        result << GavcConstants::delimiter;
        result << name();
    }
    if (!version().empty())
    {
        result << GavcConstants::delimiter;
        result << version();
    }
    if (!classifier().empty())
    {
        result << GavcConstants::delimiter;
        result << classifier();
    }
    if (!extension().empty())
    {
        result << GavcConstants::extension_prefix;
        result << extension();
    }

    LOGT << result.str() << ELOG;

    return result.str();
}

std::string GavcQuery::group_path() const
{
    std::string g = group();
    std::vector<std::string> parts;
    boost::split(parts, g, boost::is_any_of("."));
    return boost::join(parts, "/");
}

std::string GavcQuery::format_maven_metadata_url(const std::string& server_url, const std::string& repository) const
{
    LOGT << "Build url for maven metadata. server_url: " << server_url << " repository: " << repository << ELOG;

    std::string group_path = group();
    std::replace(group_path.begin(), group_path.end(), GavcConstants::group_delimiter, GavcConstants::path_delimiter);
    std::string result = boost::str(boost::format( "%2$s%1$c%3$s%1$c%4$s%1$c%5$s%1$c%6$s" )
        % GavcConstants::path_delimiter % server_url % repository % group_path % name() % GavcConstants::maven_metadata_filename);

    LOGT << "Maven metadata url: " << result << ELOG;
    return result;
}

std::string GavcQuery::format_maven_metadata_path(const std::string& repository) const
{
    LOGT << "Build path for maven metadata. repository: " << repository << ELOG;

    std::string group_path = group();
    std::replace(group_path.begin(), group_path.end(), GavcConstants::group_delimiter, GavcConstants::path_delimiter);
    std::string result = boost::str(boost::format( "%1$c%2$s%1$c%3$s%1$c%4$s" )
        % GavcConstants::path_delimiter % repository % group_path % name());

    LOGT << "Cache path to metadata url: " << result << ELOG;
    return result;
}

} } // namespace art::lib
