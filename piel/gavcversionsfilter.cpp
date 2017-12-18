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
#include <gavcversionscomparator.h>

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

typedef std::pair< std::string, std::vector<std::string> > SpartsTableElement;
typedef std::list< SpartsTableElement > SpartsTable;

struct SpartsTableComparator
{
    SpartsTableComparator(GavcVersionsComparator *comparator, const gavc::OpType& op, std::vector<std::string>::size_type field_index)
        : comparator_(comparator), op_(op), field_index_(field_index)
    {
    }

    bool operator()(const SpartsTableElement& lhs, const SpartsTableElement& rhs) const
    {
        switch (op_.first)
        {
        case gavc::Op_const:
        case gavc::Op_all:
            return false;
        case gavc::Op_latest:
            return comparator_->compare_part(lhs.second[field_index_], rhs.second[field_index_]);
        case gavc::Op_oldest:
            return !comparator_->compare_part(lhs.second[field_index_], rhs.second[field_index_]);
        }
    }

private:
    GavcVersionsComparator              *comparator_;
    gavc::OpType                        op_;
    std::vector<std::string>::size_type field_index_;

};

struct SpartsTableValuesFilter
{
    SpartsTableValuesFilter(std::vector<std::string>::size_type field_index, const std::string& value)
        : field_index_(field_index)
        , value_(value)
    {
    }

    bool operator()(const SpartsTableElement& val) const
    {
        val.second[field_index_] != value_;
    }

private:
    std::vector<std::string>::size_type field_index_;
    std::string                         value_;

};

std::vector<std::string> GavcVersionsFilter::filtered(const std::vector<std::string>& versions)
{
    std::vector<std::string> result = versions;
    if (is_trivial())
    {
        return versions;
    }

    // 1. Filter out all non matched versions.
    Match predicate(query_ops_);
    result.erase(std::remove_if(result.begin(), result.end(), Match::not_(predicate)), result.end());

    GavcVersionsMatcher     matcher     = query_ops_;
    GavcVersionsComparator  comparator  = query_ops_;

    // 2. Process signed parts. Build table <version, [significant parts]>
    SpartsTable sparts_table;
    for (std::vector<std::string>::const_iterator i = result.begin(), end = result.end(); i != end; ++i)
    {
        LOG_T << "Table item for: " << *i;
        sparts_table.push_back(std::make_pair(*i, matcher.significant_parts(*i)));
    }

    // 3. For each query_ops_ filter out elements from sparts_table according to op
    std::vector<std::string>::size_type field_index = 0;
    for (std::vector<gavc::OpType>::const_iterator i = query_ops_.begin(), end = query_ops_.end(); i != end; ++i)
    {
        if (i->first != gavc::Op_const && i->first != gavc::Op_all)
        {
            LOG_T << "Filtering for field: " << field_index;

            SpartsTableComparator table_comparator(&comparator, *i, field_index);
            SpartsTable::iterator element_to_keep = std::max_element(sparts_table.begin(), sparts_table.end(), table_comparator);

            if (element_to_keep != sparts_table.end()) {
                SpartsTableValuesFilter sparts_remove_filter(field_index, element_to_keep->second[field_index]);
                sparts_table.erase(std::remove_if(sparts_table.begin(), sparts_table.end(), sparts_remove_filter), sparts_table.end());
            }
            else
            {
                LOG_F << "Can't find max element for field: " << field_index;
            }
        }
        else
        {
            LOG_T << "Skip filtering for field: " << field_index;
        }

        if (i->first != gavc::Op_const) {
            ++field_index;
        }
    }

    // 4. Copy results into result
    result.erase(result.begin(),result.end());
    for (SpartsTable::const_iterator i = sparts_table.begin(), end = sparts_table.end(); i != end; ++i)
    {
        result.push_back(i->first);
    }

    return result;
}

struct VectorContains {

    VectorContains(const std::vector<std::string>& collection)
        : collection_(collection)
    {
    }

    bool operator()(const std::string& val) const
    {
        return std::find(collection_.begin(), collection_.end(), val) != collection_.end();
    }

private:
    std::vector<std::string> collection_;

};

std::vector<std::string> GavcVersionsFilter::filtered_out(const std::vector<std::string>& versions)
{
    std::vector<std::string> result = versions;

    VectorContains filtered_contains_predicate(filtered(versions));
    result.erase(std::remove_if(result.begin(), result.end(), filtered_contains_predicate), result.end());

    return result;
}

} } // namespace art::lib
