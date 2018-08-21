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

#include <treeindexenumerator.h>

namespace piel { namespace lib {

TreeIndexEnumerator::TreeIndexEnumerator(const TreeIndex::Ptr &index)
    : path()
    , asset()
    , attributes(boost::none)
    , index_(index)
{
    iterator_   = index_->content().begin();
    end_        = index_->content().end();
}

TreeIndexEnumerator::~TreeIndexEnumerator()
{

}

bool TreeIndexEnumerator::next()
{
    bool ret = is_valid();
    if (ret)
    {
        update_values();
        ++iterator_;
    }
    return ret;
}

bool TreeIndexEnumerator::is_valid() const
{
    return iterator_ != end_;
}

void TreeIndexEnumerator::update_values()
{
    if (is_valid())
    {
        path        = iterator_->first;
        asset       = index_->content().at(iterator_->first);
        attributes  = index_->get_attrs_(iterator_->first);
    }
    else
    {
        path        = TreeIndex::Content::value_type::first_type();
        asset       = Asset();
        attributes  = boost::none;
    }
}

} } // namespace piel::lib
