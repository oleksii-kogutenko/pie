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

#include <tree.h>

namespace piel { namespace cmd {

Tree::Tree(const piel::lib::WorkingCopy::Ptr& working_copy)
    : WorkingCopyCommand(working_copy)
    , piel::lib::IOstreamsHolder()
    , show_all_(false)
    , verbose_(false)
{
}

Tree::~Tree()
{
}

const Tree* Tree::set_show_all(bool flag)
{
    show_all_ = flag;
    return this;
}

const Tree* Tree::set_verbose(bool flag)
{
    verbose_ = flag;
    return this;
}

void Tree::operator()()
{
    if (!show_all_)
    {
        cout() << working_copy()->current_tree_name();

        if (verbose_)
            cout() << ":" << working_copy()->current_tree_state()->self().id().string();

        cout() << std::endl;

        return;
    }

    std::set<piel::lib::refs::Ref> all_refs = working_copy()->local_storage()->references();

    for(std::set<piel::lib::refs::Ref>::const_iterator i = all_refs.begin(), end = all_refs.end(); i != end; ++i)
    {
        if (working_copy()->current_tree_name() == i->first)
            cout() << "*" << i->first;
        else
            cout() << " " << i->first;

        if (verbose_)
            cout() << ":" << i->second.string();

        cout() << std::endl;
    }
}

} } // namespace piel::cmd
