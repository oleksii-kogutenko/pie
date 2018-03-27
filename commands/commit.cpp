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

#include <commit.h>
#include <logging.h>
#include <fsindexer.h>

namespace piel { namespace cmd {

/*static*/ piel::lib::Properties::DefaultFromEnv PredefinedConfigs::author =
        piel::lib::Properties::Property("author",         "unknown").default_from_env("PIE_AUTHOR");

/*static*/ piel::lib::Properties::DefaultFromEnv PredefinedConfigs::email =
        piel::lib::Properties::Property("email",          "unknown").default_from_env("PIE_EMAIL");

/*static*/ piel::lib::Properties::DefaultFromEnv PredefinedConfigs::commiter =
        piel::lib::Properties::Property("commiter",       "unknown").default_from_env("PIE_COMMITER");

/*static*/ piel::lib::Properties::DefaultFromEnv PredefinedConfigs::commiter_email =
        piel::lib::Properties::Property("commiter_email", "unknown").default_from_env("PIE_COMMITER_EMAIL");

Commit::Commit(const piel::lib::WorkingCopy::Ptr& working_copy)
    : WorkingCopyCommand(working_copy)
{
}

Commit::~Commit()
{
}

const Commit* Commit::set_message(const std::string& message)
{
    message_ = message;
    return this;
}

piel::lib::IndexesDiff Commit::diff(const piel::lib::TreeIndex::Ptr& current_index) const
{
    LOGT << "Calculate diff " << working_copy()->current_tree_state()->self().id().string() << " <-> CDIR" << ELOG;
    return piel::lib::IndexesDiff::diff(working_copy()->current_tree_state(), current_index);
}

std::string Commit::operator()()
{
    piel::lib::IObjectsStorage::Ptr ls  = working_copy()->local_storage();
    piel::lib::TreeIndex::Ptr current_index  = working_copy()->working_dir_state();

    piel::lib::IndexesDiff indexes_diff = diff(current_index);
    if (indexes_diff.empty())
    {
        LOGT << "Diff is empty!" << ELOG;
        throw errors::nothing_to_commit();
    }

    LOGT << "Non empty diff:" << ELOG;
    LOGT << indexes_diff.format() << ELOG;

    piel::lib::TreeIndex::Ptr reference_index    = working_copy()->current_tree_state();

    piel::lib::TreeIndex::Ptr ref_index = piel::lib::TreeIndex::from_ref(ls, working_copy()->current_tree_name());
    if (ref_index)
    {
        reference_index = ref_index;
    }

    if (!reference_index->empty())
    {
        current_index->set_parent(reference_index->self());
    }

    // Fill from config
    current_index->set_author_(         working_copy()->config().get(PredefinedConfigs::author).value());
    current_index->set_email_(          working_copy()->config().get(PredefinedConfigs::email).value());
    current_index->set_commiter_(       working_copy()->config().get(PredefinedConfigs::commiter).value());
    current_index->set_commiter_email_( working_copy()->config().get(PredefinedConfigs::commiter_email).value());

    // Set message
    current_index->set_message_(message_);

    // Put changes into local storage
    ls->put(current_index->assets());
    ls->update_reference(piel::lib::refs::Ref(working_copy()->current_tree_name(), current_index->self()));

    working_copy()->setup_current_tree(working_copy()->current_tree_name(), current_index);
    return working_copy()->current_tree_state()->self().id().string();
}

} } // namespace piel::cmd
