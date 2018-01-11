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

#include <boost/interprocess/streams/vectorstream.hpp>
#include <memoryobjectsstorage.h>

namespace piel { namespace lib {

typedef boost::interprocess::basic_ivectorstream<MemoryObjectsStorage::Object> ivectorstream;
typedef boost::interprocess::basic_ovectorstream<MemoryObjectsStorage::Object> ovectorstream;

MemoryObjectsStorage::MemoryObjectsStorage()
    : assets_()
    , refs_()
{
}

MemoryObjectsStorage::~MemoryObjectsStorage()
{
}

// Put readable asset into storage.
void MemoryObjectsStorage::put(const Asset& asset)
{
    // Do not put AssetId::empty Asset
    if (asset.id() == AssetId::empty || contains(asset.id()))
    {
        return;
    }

    boost::shared_ptr<std::istream> asset_istream = asset.istream();
    ovectorstream os;

    if (asset_istream)
    {
        os << asset_istream->rdbuf();
        assets_.insert(std::make_pair(asset.id(), os.vector()));
    }
    else
    {
        throw errors::attempt_to_put_non_readable_asset();
    }
}

void MemoryObjectsStorage::put(std::set<Asset> assets)
{
    for(std::set<Asset>::const_iterator i = assets.begin(), end = assets.end(); i != end; ++i)
    {
        put(*i);
    }
}

void MemoryObjectsStorage::update_reference(const refs::Ref& ref)
{
    refs_.insert(ref);
}

// Check if readable asset available in storage.
bool MemoryObjectsStorage::contains(const AssetId& id) const
{
    return assets_.find(id) != assets_.end();
}

//// Make attempt to get readable asset from storage. Non readable Asset will be returned on fail.
Asset MemoryObjectsStorage::asset(const AssetId& id) const
{
    if (contains(id))
    {
        return Asset::create_for(this, id);
    }
    else
    {
        return Asset();
    }
}

// Get input stream for reading asset data. Low level API used by Asset implementation.
//External code must use get().istream() call sequense.
boost::shared_ptr<std::istream> MemoryObjectsStorage::istream_for(const AssetId& id) const
{
    if (contains(id))
    {
        return boost::shared_ptr<std::istream>(new ivectorstream(assets_.at(id)));
    }
    else
    {
        return boost::shared_ptr<std::istream>();
    }
}

AssetId MemoryObjectsStorage::resolve(const std::string& ref) const
{
    if (refs_.find(ref) != refs_.end())
    {
        return refs_.at(ref);
    }
    else if (ref.length() == AssetId::str_digest_len)
    {
        // Attempt to resolve AssetId by string representation.
        AssetId id = AssetId::create(ref);
        if (id != AssetId::empty && contains(id))
        {
            return id;
        }
        else
        {
            return AssetId::empty;
        }
    }
    else
    {
        return AssetId::empty;
    }
}

std::set<refs::Ref> MemoryObjectsStorage::references() const
{
    std::set<refs::Ref> result;
    for (References::const_iterator i = refs_.begin(), end = refs_.end(); i != end; ++i)
    {
        result.insert(*i);
    }
    return result;
}

} } // namespace piel::lib
