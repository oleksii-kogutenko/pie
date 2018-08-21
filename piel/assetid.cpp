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

#include <assetid.h>
#include <checksumsdigestbuilder.hpp>

namespace piel { namespace lib {

const AssetId AssetId::not_calculated("<not calculated>");
const AssetId AssetId::empty("<empty>");

const std::string AssetId::digest_algo      = Sha256::t::name();
const unsigned int AssetId::str_digest_len  = Sha256::t::len() * 2;

AssetId::AssetId()
    : id_(AssetId::empty.id_)
{
}

AssetId::AssetId(const std::string& id)
    : id_(id)
{
}

AssetId::AssetId(std::istream& is)
    : id_()
{
    ChecksumsDigestBuilder digestBuilder;
    ChecksumsDigestBuilder::StrDigests str_digests = digestBuilder.str_digests_for(is);
    id_ = str_digests[AssetId::digest_algo];
}

AssetId::AssetId(const AssetId& src)
    : id_(src.id_)
{
}

AssetId::~AssetId()
{
}

bool AssetId::operator==(const AssetId& src) const
{
    return id_ == src.id_;
}

bool AssetId::operator!=(const AssetId& src) const
{
    return id_ != src.id_;
}

bool AssetId::operator<(const AssetId& src) const
{
    return id_ < src.id_;
}

AssetId AssetId::create_for(std::istream& is)
{
    return AssetId(is);
}

AssetId AssetId::create(const std::string& id)
{
    return AssetId(id);
}

std::string AssetId::string() const
{
    return id_;
}

} } // namespace piel::lib
