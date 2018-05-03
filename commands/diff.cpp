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

Diff::Diff(const piel::lib::WorkingCopy::Ptr& working_copy, const piel::lib::refs::Range& range)
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

    if (!result)
    {
        LOGT << "Unable to resolve ref: " << ref << " use default ref: " << def_ref->self().id().string() << ELOG;
        result = def_ref;
    }

    LOGT << "Result: " << result->self().id().string() << ELOG;

    return result;
}

void Diff::operator()()
{
    cout() << "# " << range_.first << "..." << range_.second << std::endl;

    piel::lib::TreeIndex::Ptr from = resolve_ref(range_.first, working_copy()->current_tree_state());
    piel::lib::TreeIndex::Ptr to = resolve_ref(range_.second, working_copy()->working_dir_state());

    LOGT << "Diff range { from: " << range_.first << " to:" << range_.second  << " }" << ELOG;

    piel::lib::IndexesDiff diff = piel::lib::IndexesDiff::diff(from, to);
}

} } // namespace piel::cmd
