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

#include <create.h>
#include <checkout.h>
#include <logging.h>

namespace piel { namespace cmd {

Create::Create(const piel::lib::WorkingCopy::Ptr& working_copy, const std::string& new_ref)
    : WorkingCopyCommand(working_copy)
    , new_ref_(new_ref)
{
}

Create::~Create()
{
}

void Create::operator()()
{
    if (piel::lib::AssetId::empty != working_copy()->local_storage()->resolve(new_ref_))
    {
        throw errors::non_empty_reference_already_exists();
    }

    // Check for non commit changes
    piel::lib::TreeIndex::Ptr current_index = working_copy()->working_dir_state();
    piel::lib::TreeIndex::Ptr reference_index = working_copy()->current_tree_state();

    piel::lib::IndexesDiff diff = piel::lib::IndexesDiff::diff(reference_index, current_index);
    if (!diff.empty())
    {
        LOGT << "There are non commit changes!" << ELOG;
        throw errors::there_are_non_commit_changes();
    }

    // Remove working directory content (exclude metadata)
    boost::filesystem::remove_directory_content(working_copy()->working_dir(), working_copy()->metadata_dir());

    piel::lib::TreeIndex::Ptr initial_tree_index(new piel::lib::TreeIndex());

    initial_tree_index->initial_for(new_ref_);
    working_copy()->local_storage()->put(initial_tree_index->assets());

    piel::lib::AssetId new_tree_id = initial_tree_index->self().id();
    working_copy()->local_storage()->create_reference(piel::lib::refs::Ref(new_ref_, new_tree_id));
    working_copy()->setup_current_tree(new_ref_, initial_tree_index);

    LOGT << "Created new empty tree: " << new_ref_  << ":" << new_tree_id.string() << ELOG;
}

} } // namespace piel::cmd
