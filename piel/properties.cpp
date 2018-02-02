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

struct JavaPropertiesConstants {
    static const char comment;
    static const char assign;
    static const char endl;
};

const char JavaPropertiesConstants::comment = '#';
const char JavaPropertiesConstants::assign = '=';
const char JavaPropertiesConstants::endl = '\n';

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

            _syms       = (char_(JavaPropertiesConstants::assign)|char_(JavaPropertiesConstants::comment));
            _comment    = skip[JavaPropertiesConstants::comment > -( +( char_ ) ) ];
            _first      = as_string[ +( char_ - _syms ) > skip[ JavaPropertiesConstants::assign ] ]         [ _val = trim_str_(_1) ];
            _second     = as_string[ +( char_ - JavaPropertiesConstants::comment ) ]                        [ _val = trim_str_(_1) ];
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
    : data_()
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
        LOGT << "buffer: " << buffer << ELOG;
        qi::phrase_parse( buffer.begin(), buffer.end(), java_properties_grammar, ascii::space, pair );
        result.data_[pair.first] = pair.second;
        LOGT << "'"<< pair.first << "' = '" << pair.second << "'" << ELOG;
    }

    return result;
}

void Properties::store(std::ostream &os) const
{
    for(MapType::const_iterator i = data_.begin(), end = data_.end(); i != end; ++i)
    {
        if (!i->first.empty() && !i->second.empty())
        {
            os << i->first << JavaPropertiesConstants::assign << i->second << JavaPropertiesConstants::endl;
        }
    }
}

void Properties::set(const Property::name_type& name, const Property::value_type& value)
{
    data_[name] = value;
}

Properties::Property::value_type Properties::get(const Property::name_type& name, const Property::value_type& default_value) const
{
    if (data_.find(name) == data_.end())
    {
        return default_value;
    }
    else
    {
        return data_.at(name);
    }
}

const Properties::MapType& Properties::data() const
{
    return data_;
}

Properties::MapType& Properties::data()
{
    return data_;
}

Properties::MapType::mapped_type& Properties::operator[](const Properties::MapType::key_type& key)
{
    return data_[key];
}

void Properties::clear()
{
    data_.clear();
}

} } //namespace piel::lib
