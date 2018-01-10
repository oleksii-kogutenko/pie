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

    piel::lib::Index empty_index;

    working_copy()->local_storage()->put(empty_index.assets());
    working_copy()->local_storage()->put(piel::lib::refs::Ref(new_ref_, piel::lib::Asset()));
    working_copy()->update_reference(new_ref_, empty_index);

    LOG_T << "Created new empty reference: " << new_ref_;
}

} } // namespace piel::cmd
