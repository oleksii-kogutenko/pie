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

namespace art { namespace lib {

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
    GavcQuery result;
    std::string::const_iterator end         = gavc_str.end();
    std::string::const_iterator sepa_begin  = gavc_str.begin();
    std::string::const_iterator sepa_end    = end;
 
    BOOST_LOG_TRIVIAL(trace) << "parce gavc query: " << gavc_str;
    
    // Group
    sepa_end = std::find_if(sepa_begin, end, boost::is_any_of(GavcConstants::delimiter));
    if (sepa_end == end) {
        return boost::none;
    }
    result._group.append(sepa_begin, sepa_end);
    BOOST_LOG_TRIVIAL(trace) << "group: " << result._group;
    sepa_begin = sepa_end;
    if (sepa_begin++ == end) {
        return boost::none;
    }

    // Name
    sepa_end = std::find_if(sepa_begin, end, boost::is_any_of(GavcConstants::delimiter));
    result._name.append(sepa_begin, sepa_end);
    BOOST_LOG_TRIVIAL(trace) << "name: " << result._name;
    sepa_begin = sepa_end;
    if (sepa_begin++ == end) {
        return result;
    }

    // Version
    sepa_end = std::find_if(sepa_begin, end, boost::is_any_of(GavcConstants::delimiter));
    result._version.append(sepa_begin, sepa_end);
    BOOST_LOG_TRIVIAL(trace) << "version: " << result._version;
    sepa_begin = sepa_end;
    if (sepa_begin++ == end) {
        return result;
    }

    // Classifier
    sepa_end = std::find_if(sepa_begin, end, boost::is_any_of(GavcConstants::extension_prefix));
    result._classifier.append(sepa_begin, sepa_end);
    BOOST_LOG_TRIVIAL(trace) << "classifier: " << result._classifier;
    sepa_begin = sepa_end;
    if (sepa_begin++ == end) {
        return result;
    }

    // Extension
    result._extension.append(sepa_begin, end);
    BOOST_LOG_TRIVIAL(trace) << "extension: " << result._extension;

    return result;
}

std::string GavcQuery::to_string() const 
{
    // TODO: implement
    return "";
}

} } // namespace art::lib
