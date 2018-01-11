/*
 * Copyright (c) 2018, diakovliev
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

#include <status.h>
#include <logging.h>
#include <fsindexer.h>
#include <indexesdiff.h>

#include <iostream>

namespace piel { namespace cmd {

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

    bool is_printable(piel::lib::IndexesDiff::ElementState s)
    {
        switch (s) {
        case piel::lib::IndexesDiff::ElementState_unmodified:return false;
        case piel::lib::IndexesDiff::ElementState_removed:   return true;
        case piel::lib::IndexesDiff::ElementState_added:     return true;
        case piel::lib::IndexesDiff::ElementState_modified:  return true;
        }
        return false;
    }

    std::string element_state(piel::lib::IndexesDiff::ElementState s)
    {
        switch (s) {
        case piel::lib::IndexesDiff::ElementState_unmodified:return " ";
        case piel::lib::IndexesDiff::ElementState_removed:   return "D";
        case piel::lib::IndexesDiff::ElementState_added:     return "A";
        case piel::lib::IndexesDiff::ElementState_modified:  return "M";
        }
        return " ";
    }

} // namespace fmt

/*static*/ const std::string Status::Status_clean = "clean";
/*static*/ const std::string Status::Status_dirty = "dirty";

Status::Status(const piel::lib::WorkingCopy::Ptr& working_copy)
    : WorkingCopyCommand(working_copy)
{
}

Status::~Status()
{
}

std::string Status::operator()()
{
    typedef piel::lib::IndexesDiff::ContentDiff::const_iterator ContentIter;
    typedef piel::lib::IndexesDiff::ContentAttributesDiff::const_iterator ContentAttrsIter;
    typedef piel::lib::IndexesDiff::AttributesDiff AttributesDiff;
    typedef piel::lib::IndexesDiff::AttributesDiff::const_iterator AttrsDiffIter;

    std::string final_status_str = Status_clean;

    piel::lib::IndexesDiff diff = piel::lib::IndexesDiff::diff(
            working_copy()->reference_index(), working_copy()->current_index());

    for (ContentIter i = diff.content_diff().begin(), end = diff.content_diff().end(); i != end; ++i)
    {
        ContentAttrsIter attributes_element_iter = diff.content_attributes_diff().find(i->first);

        bool has_attrubutes_changes = attributes_element_iter != diff.content_attributes_diff().end();

        if (fmt::is_printable(i->second.first) || has_attrubutes_changes)
        {
            if (fmt::is_printable(i->second.first))
            {
                std::cout << fmt::element_state(i->second.first) << " " << i->first << std::endl;

                final_status_str = Status_dirty;
            }

            AttributesDiff content_attributes_diff = diff.content_item_attributes_diff(attributes_element_iter);

            for (AttrsDiffIter j = content_attributes_diff.begin(), end1 = content_attributes_diff.end(); j != end1; ++j)
            {
                if (fmt::is_printable(j->second.first) && j->second.first != piel::lib::IndexesDiff::ElementState_removed)
                {
                    if (!fmt::is_printable(i->second.first))
                    {
                        std::cout << fmt::element_state(i->second.first) << " " << i->first << std::endl;
                    }

                    std::cout << fmt::tab(1)
                              << fmt::element_state(j->second.first)
                              << " attribute: "
                              << j->first
                              << " "
                              << j->second.second.first
                              << " -> "
                              << j->second.second.second
                              << std::endl;

                    final_status_str = Status_dirty;
                }
            }
        }
    }

    return final_status_str;
}

} } // namespace piel::cmd
