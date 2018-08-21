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

#include <log.h>
#include <logging.h>

#include <treeenumerator.h>

namespace piel { namespace cmd {

Log::Log(const piel::lib::IObjectsStorage::Ptr& storage,
        const piel::lib::TreeIndex::Ptr& ref_index, const piel::lib::refs::Range& range)
    : IObjectsStorageCommand(storage)
    , piel::lib::IOstreamsHolder()
    , range_(range)
    , ref_index_(ref_index)
{
}

Log::~Log()
{
}

void Log::format_log_element(const piel::lib::TreeIndex::Ptr& index) const
{
    cout() << "--------------------------------------------------------------------------------" << std::endl;
    cout() << "author: " << index->get_author_()                                                 << std::endl;
    cout() << "email: " << index->get_email_()                                                   << std::endl;
    cout() << "id: "<< index->self().id().string()                                               << std::endl;
    cout()                                                                                       << std::endl;
    cout() << index->get_message_()                                                              << std::endl;
}

void Log::operator()()
{
    piel::lib::TreeIndex::Ptr from(new piel::lib::TreeIndex()), to = ref_index_;

    if (!range_.first.empty())
    {
        piel::lib::AssetId fromId = storage()->resolve(range_.first);
        if (piel::lib::AssetId::empty != fromId)
        {
            from = piel::lib::TreeIndex::from_ref(storage(), range_.first);
        }
    }

    if (!range_.second.empty())
    {
        piel::lib::AssetId toId = storage()->resolve(range_.second);
        if (piel::lib::AssetId::empty != toId)
        {
            to = piel::lib::TreeIndex::from_ref(storage(), range_.second);
        }
    }

    if (from->self().id() == to->self().id())
    {
        return;
    }

    LOGT << "from: " << from->self().id().string()
        << " to: " << to->self().id().string() << ELOG;

    piel::lib::TreeEnumerator tree_enumerator(storage(), to);
    while (tree_enumerator.next())
    {
        if (tree_enumerator.index->self().id() == from->self().id() || tree_enumerator.index->is_initial_index())
        {
            break;
        }

        format_log_element(tree_enumerator.index);
    }
}

} } // namespace piel::cmd
