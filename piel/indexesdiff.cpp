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

#include <indexesdiff.h>

#include <logging.h>

namespace piel { namespace lib {

template<class DiffMap, class CompareMap>
struct MapDiffBuilder {

    typedef typename DiffMap::iterator                      DiffMapIterator;
    typedef typename CompareMap::const_iterator             ConstIter;
    typedef typename CompareMap::value_type::second_type    EmptyValueType;

    static DiffMap diff(const CompareMap& first_map, const CompareMap& second_map)
    {
        DiffMap result;

        ConstIter first_end    = first_map.end(),
                  second_end   = second_map.end();

        for (ConstIter i = first_map.begin(); i != first_end; ++i)
        {
            ConstIter second_iter_ = second_map.find(i->first);

            std::pair<DiffMapIterator, bool> insert_result;

            if (second_iter_ != second_end)
            {
                IndexesDiff::ElementState element_state = (i->second == second_iter_->second)
                        ?  IndexesDiff::ElementState_unmodified
                        :  IndexesDiff::ElementState_modified;


                insert_result = result.insert(
                        std::make_pair(i->first,
                                std::make_pair(element_state,
                                        std::make_pair(i->second, second_iter_->second))));
            }
            else
            {
                insert_result = result.insert(
                        std::make_pair(i->first,
                                std::make_pair(IndexesDiff::ElementState_removed,
                                        std::make_pair(i->second, EmptyValueType()))));
            }

            if (!insert_result.second)
            {
                LOG_F << "Unable on insert maps diff element into diff. Key: " << i->first;
            }
        }

        for (ConstIter i = second_map.begin(); i != second_end; ++i)
        {
            ConstIter first_iter = first_map.find(i->first);

            if (first_iter == first_end)
            {
                std::pair<DiffMapIterator, bool> insert_result;

                insert_result = result.insert(
                        std::make_pair(i->first,
                                std::make_pair(IndexesDiff::ElementState_added,
                                        std::make_pair(EmptyValueType(), i->second))));

                if (!insert_result.second)
                {
                    LOG_F << "Unable on insert maps diff element into diff. Key: " << i->first;
                }
            }
        }

        return result;
    }

};

struct FindChangedElement {

    template<class ValueType>
    bool operator()(ValueType val) const
    {
        return val.second.first != IndexesDiff::ElementState_unmodified;
    }

    template<class Map>
    static bool has_changed(const Map& map)
    {
        return std::find_if(map.begin(), map.end(), FindChangedElement()) != map.end();
    }

};

typedef MapDiffBuilder<IndexesDiff::ContentDiff,TreeIndex::Content>                     ContentDiffBuilder;
typedef MapDiffBuilder<IndexesDiff::AttributesDiff,TreeIndex::Attributes>               AttributesDiffBuilder;
typedef MapDiffBuilder<IndexesDiff::ContentAttributesDiff,TreeIndex::ContentAttributes> ContentAttributesDiffBuilder;

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
    return !different_content() &&
           !different_content_attributes();
}

bool IndexesDiff::different_content() const
{
    return FindChangedElement::has_changed(content_diff_);
}

bool IndexesDiff::different_attributes() const
{
    return FindChangedElement::has_changed(attributes_diff_);
}

bool IndexesDiff::different_content_attributes() const
{
    return FindChangedElement::has_changed(content_attributes_diff_);
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
        case IndexesDiff::ElementState_unmodified:return "<unmodified>";
        case IndexesDiff::ElementState_removed:   return "<removed>";
        case IndexesDiff::ElementState_added:     return "<added>";
        case IndexesDiff::ElementState_modified:  return "<modified>";
        }
        return "<unknown>";
    }

    std::string asset_id(const Asset& a)
    {
        return a.id().string();
    }

} // namespace fmt

IndexesDiff::AttributesDiff IndexesDiff::content_item_attributes_diff(const ContentAttributesDiff::const_iterator& element_iter) const
{
    return AttributesDiffBuilder::diff(element_iter->second.second.first, element_iter->second.second.second);
}

std::string IndexesDiff::format() const
{
    std::ostringstream oss;

    for (AttributesDiff::const_iterator i = attributes_diff_.begin(), end = attributes_diff_.end(); i != end; ++i)
    {
        int tl = 0;
        oss << fmt::tab(tl++) << "attribute: "  << i->first                             << std::endl;
        oss << fmt::tab(tl++) << "state: "      << fmt::element_state(i->second.first)  << std::endl;
        oss << fmt::tab(tl) << "first: "        << i->second.second.first               << std::endl;
        oss << fmt::tab(tl) << "second: "       << i->second.second.second              << std::endl;
    }

    for (ContentDiff::const_iterator i = content_diff_.begin(), end = content_diff_.end(); i != end; ++i)
    {
        int tl = 0;
        oss << fmt::tab(tl++)   << "path: "     << i->first                                 << std::endl;
        oss << fmt::tab(tl++)   << "state: "    << fmt::element_state(i->second.first)      << std::endl;
        oss << fmt::tab(tl)     << "first: "    << fmt::asset_id(i->second.second.first)    << std::endl;
        oss << fmt::tab(tl++)   << "second: "   << fmt::asset_id(i->second.second.second)   << std::endl;

        ContentAttributesDiff::const_iterator element_iter = content_attributes_diff_.find(i->first);

        if (element_iter != content_attributes_diff_.end())
        {
            AttributesDiff content_attributes_diff = content_item_attributes_diff(element_iter);

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

    return oss.str();
}

/*static*/ IndexesDiff IndexesDiff::diff(const TreeIndex& first_index, const TreeIndex& second_index)
{
    IndexesDiff result;

    result.content_diff_            = ContentDiffBuilder::diff(first_index.content_, second_index.content_);
    result.attributes_diff_         = AttributesDiffBuilder::diff(first_index.attributes_, second_index.attributes_);
    result.content_attributes_diff_ = ContentAttributesDiffBuilder::diff(first_index.content_attributes_, second_index.content_attributes_);

    return result;
}

} } // namespace piel::lib
