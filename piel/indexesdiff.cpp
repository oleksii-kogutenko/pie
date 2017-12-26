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

#include <indexesdiff.h>
#include <logging.h>

namespace piel { namespace lib {

IndexesDiff::IndexesDiff()
    : content_diff_()
    , attributes_diff_()
    , content_attributes_diff_()
{
}

IndexesDiff::~IndexesDiff()
{
}

bool IndexesDiff::empty() const
{
    return content_diff_.empty() &&
           attributes_diff_.empty() &&
           content_attributes_diff_.empty();
}

bool IndexesDiff::different_content() const
{
    return !content_diff_.empty();
}

bool IndexesDiff::different_attributes() const
{
    return !attributes_diff_.empty();
}

bool IndexesDiff::different_content_attributes() const
{
    return !content_attributes_diff_.empty();
}

const IndexesDiff::ContentDiff& IndexesDiff::content_diff() const
{
    return content_diff_;
}

const IndexesDiff::AttributesDiff& IndexesDiff::attributes_diff() const
{
    return attributes_diff_;
}

const IndexesDiff::ContentAttributesDiff& IndexesDiff::content_attributes_diff() const
{
    return content_attributes_diff_;
}

namespace fmt {

    std::string tab(int count)
    {
        std::ostringstream oss;
        for (int i = 0; i < count; ++i)
        {
            oss << "\t";
        }
        return oss.str();
    }

    std::string element_state(IndexesDiff::ElementState s)
    {
        switch (s) {
        case IndexesDiff::ElementState_removed:   return "<removed>";
        case IndexesDiff::ElementState_added:     return "<added>";
        case IndexesDiff::ElementState_modified:  return "<modified>";
        }
        return "<unknown>";
    }

    std::string asset_id(const Asset& a)
    {
        return a.id().presentation();
    }

} // namespace fmt

template<class DiffMap, class CompareMap>
DiffMap diff_maps(const CompareMap& first_map, const CompareMap& second_map)
{
    DiffMap result;

    typename CompareMap::const_iterator first_begin  = first_map.begin(),
                                        second_begin = second_map.begin();

    typename CompareMap::const_iterator first_end    = first_map.end(),
                                        second_end   = second_map.end();

    typedef typename CompareMap::value_type::second_type EmptyValueType;

    for (typename CompareMap::const_iterator i = first_begin, end = first_end; i != end; ++i)
    {

        typename CompareMap::const_iterator second_content_iterator_ = second_map.find(i->first);

        if (second_content_iterator_ != second_end)
        {
            if (i->second != second_content_iterator_->second)
            {
                result.insert(
                        std::make_pair(i->first,
                                std::make_pair(IndexesDiff::ElementState_modified,
                                        std::make_pair(i->second, second_content_iterator_->second))));
            }
        }
        else
        {
            result.insert(
                    std::make_pair(i->first,
                            std::make_pair(IndexesDiff::ElementState_removed,
                                    std::make_pair(i->second, EmptyValueType()))));
        }
    }

    for (typename CompareMap::const_iterator i = second_begin, end = second_end; i != end; ++i)
    {
        typename CompareMap::const_iterator first_content_iterator_ = first_map.find(i->first);

        if (first_content_iterator_ == first_end)
        {
            result.insert(
                    std::make_pair(i->first,
                            std::make_pair(IndexesDiff::ElementState_added,
                                    std::make_pair(EmptyValueType(), i->second))));
        }
    }

    return result;
}

std::string IndexesDiff::format() const
{
    if (empty()) return "";

    std::ostringstream oss;

    if (different_attributes())
    {
        for (AttributesDiff::const_iterator i = attributes_diff_.begin(), end = attributes_diff_.end(); i != end; ++i)
        {
            int tl = 0;
            oss << fmt::tab(tl++) << "attribute: "  << i->first                             << std::endl;
            oss << fmt::tab(tl++) << "state: "      << fmt::element_state(i->second.first)  << std::endl;
            oss << fmt::tab(tl) << "first: "        << i->second.second.first               << std::endl;
            oss << fmt::tab(tl) << "second: "       << i->second.second.second              << std::endl;
        }
    }

    if (different_content())
    {
        for (ContentDiff::const_iterator i = content_diff_.begin(), end = content_diff_.end(); i != end; ++i)
        {
            int tl = 0;
            oss << fmt::tab(tl++)   << "path: "     << i->first                                 << std::endl;
            oss << fmt::tab(tl++)   << "state: "    << fmt::element_state(i->second.first)      << std::endl;
            oss << fmt::tab(tl)     << "first: "    << fmt::asset_id(i->second.second.first)    << std::endl;
            oss << fmt::tab(tl++)   << "second: "   << fmt::asset_id(i->second.second.second)   << std::endl;

            if (!different_content_attributes()) continue;

            ContentAttributesDiff::const_iterator element_iter = content_attributes_diff_.find(i->first);

            if (element_iter != content_attributes_diff_.end())
            {
                AttributesDiff content_attributes_diff = diff_maps<AttributesDiff,Index::Attributes>(element_iter->second.second.first, element_iter->second.second.second);

                for (AttributesDiff::const_iterator j = content_attributes_diff.begin(), end1 = content_attributes_diff.end(); j != end1; ++j)
                {
                    int tll = 0;
                    oss << fmt::tab(tl + tll++)   << "attribute: "  << j->first                             << std::endl;
                    oss << fmt::tab(tl + tll++)   << "state: "      << fmt::element_state(j->second.first)  << std::endl;
                    oss << fmt::tab(tl + tll)     << "first: "      << j->second.second.first               << std::endl;
                    oss << fmt::tab(tl + tll)     << "second: "     << j->second.second.second              << std::endl;
                }
            }
        }
    }

    return oss.str();
}

/*static*/ IndexesDiff IndexesDiff::diff(const Index& first_index, const Index& second_index)
{
    IndexesDiff result;

    result.content_diff_            = diff_maps<ContentDiff,Index::Content>(first_index.content_, second_index.content_);
    result.attributes_diff_         = diff_maps<AttributesDiff,Index::Attributes>(first_index.attributes_, second_index.attributes_);
    result.content_attributes_diff_ = diff_maps<ContentAttributesDiff,Index::ContentAttributes>(first_index.content_attributes_, second_index.content_attributes_);

    return result;
}

} } // namespace piel::lib
