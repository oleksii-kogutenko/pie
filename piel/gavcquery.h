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

#ifndef GAVCQUERY_H
#define GAVCQUERY_H

#include <vector>
#include <string>
#include <boost/optional.hpp>

namespace art { namespace lib {

namespace gavc {

    enum Ops {
        Op_const,
        Op_all,
        Op_latest,
        Op_oldest,
    };

    struct OpType: public std::pair<Ops,std::string>
    {
        OpType()                                : std::pair<Ops,std::string>(Op_const, "")      {}
        OpType(const std::string& val)          : std::pair<Ops,std::string>(Op_const, val)     {}
        OpType(Ops op, const std::string& val)  : std::pair<Ops,std::string>(op, val)           {}
    };

} // namespace gavc

class GavcQuery
{
public:
    GavcQuery();
    ~GavcQuery();

    static boost::optional<GavcQuery> parse(const std::string& gavc_str);
    std::string to_string() const;

    std::string group() const           { return _group; }
    std::string name() const            { return _name; }
    std::string version() const         { return _version; }
    std::string classifier() const      { return _classifier; }
    std::string extension() const       { return _extension; }

    boost::optional<std::vector<gavc::OpType> > query_version_ops() const;
    std::string format_maven_metadata_url(const std::string& server_url, const std::string& repository) const;

private:
    std::string _group;
    std::string _name;
    std::string _version;
    std::string _classifier;
    std::string _extension;

};

} } // namespace art::lib

#endif // GAVCQUERY_H
