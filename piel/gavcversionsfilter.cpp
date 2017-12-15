/*
 * Copyright (c) 2017, diakovliev
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
 * THIS SOFTWARE IS PROVIDED BY diakovliev ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL diakovliev BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <gavcversionsfilter.h>
#include <gavcversionsmatcher.h>

#include <logging.h>

#include <algorithm>
#include <sstream>

namespace art { namespace lib {

template<class Predicate, typename Arg>
struct Not {
    Not(Predicate& pred)
        : pred_(pred)
    {
    }

    bool operator()(Arg arg)
    {
        bool result = !pred_(arg);

        LOG_T << "negated value: " << result;

        return result;
    }

private:
    Predicate& pred_;

};

struct Match {

    typedef Not<Match, const std::string& > not_;

    Match(const std::vector<gavc::OpType>& query_ops)
        : matcher_(query_ops)
    {
    }

    bool operator()(const std::string& val)
    {
        bool result = matcher_.match(val);

        LOG_T << "match value: " << val << " result: " << result;

        return result;
    }

private:
    GavcVersionsMatcher matcher_;

};

GavcVersionsFilter::GavcVersionsFilter(const std::vector<gavc::OpType>& query_ops)
    : query_ops_(query_ops)
{

}

GavcVersionsFilter::~GavcVersionsFilter()
{

}

bool GavcVersionsFilter::is_trivial() const
{
    bool result = true;

    for(std::vector<gavc::OpType>::const_iterator i = query_ops_.begin(), end = query_ops_.end(); i != end && result; ++i)
        result = !(i->first == gavc::Op_const);

    return result;
}

std::vector<std::string> GavcVersionsFilter::filtered(const std::vector<std::string>& versions)
{
    std::vector<std::string> result = versions;
    if (is_trivial())
    {
        return versions;
    }

    Match predicate(query_ops_);
    result.erase(std::remove_if(result.begin(), result.end(), Match::not_(predicate)), result.end());

    return result;
}

std::vector<std::string> GavcVersionsFilter::filtered_out(const std::vector<std::string>& versions)
{
    std::vector<std::string> result;
    if (is_trivial())
    {
        return result;
    }

    result = versions;
    Match predicate(query_ops_);
    result.erase(std::remove_if(result.begin(), result.end(), predicate), result.end());

    return result;
}

} } // namespace art::lib
