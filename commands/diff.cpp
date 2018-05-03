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

#include <diff.h>
#include <logging.h>

namespace piel { namespace cmd {

Diff::Diff(const piel::lib::WorkingCopy::Ptr& working_copy, const boost::optional<piel::lib::refs::Range>& range)
    : WorkingCopyCommand(working_copy)
    , piel::lib::IOstreamsHolder()
    , range_(range)
{
}

Diff::~Diff()
{
}

piel::lib::TreeIndex::Ptr Diff::resolve_ref(const std::string& ref, const piel::lib::TreeIndex::Ptr& def_ref)
{
    piel::lib::TreeIndex::Ptr result;

    if (!ref.empty())
    {
        LOGT << "Attempt to resolve ref: " << ref << ELOG;
        result = piel::lib::TreeIndex::from_ref(working_copy()->local_storage(), ref);
    }

    if (!result && ref.empty())
    {
        LOGT << "Unable to resolve ref: " << ref << " use default ref: " << def_ref->str_id() << ELOG;
        result = def_ref;
    }

    if (result)
    {
        LOGT << "Result: " << result->str_id() << ELOG;
    }
    else
    {
        LOGE << "Unable to resolve ref: " << ref << ELOG;
    }

    return result;
}

void Diff::operator()()
{
    piel::lib::TreeIndex::Ptr from, to;

    if (range_)
    {
        from    = resolve_ref(range_->first,    working_copy()->current_tree_state());
        if (!from)
        {
            LOGE  << "Can't resolve non empty ref: " << range_->first << "!" <<  ELOG;
            throw errors::can_not_resolve_non_empty_reference(range_->first);
        }

        to      = resolve_ref(range_->second,   working_copy()->working_dir_state());
        if (!to)
        {
            LOGE  << "Can't resolve non empty ref: " << range_->second << "!" <<  ELOG;
            throw errors::can_not_resolve_non_empty_reference(range_->second);
        }

        LOGT << "Diff range { from: " << range_->first << " to:" << range_->second  << " }" << ELOG;
    }
    else
    {
        from    = working_copy()->current_tree_state();
        to      = working_copy()->working_dir_state();

        LOGT << "Diff range { from: HEAD  to: DIR }" << ELOG;
    }

    std::string from_str = "HEAD", to_str = "DIR";
    if (from != working_copy()->current_tree_state())
    {
        from_str = from->str_id();
    }
    if (to != working_copy()->working_dir_state())
    {
        to_str = to->str_id();
    }

    cout() << "# " << from_str << "..." << to_str << std::endl;

    piel::lib::IndexesDiff diff = piel::lib::IndexesDiff::diff(from, to);
}

} } // namespace piel::cmd
