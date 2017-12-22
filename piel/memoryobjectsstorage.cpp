/*
 * Copyright (c) 2017, diakovliev
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

#include <boost/interprocess/streams/vectorstream.hpp>
#include <memoryobjectsstorage.h>

namespace piel { namespace lib {

typedef boost::interprocess::basic_ivectorstream<MemoryObjectsStorage::StorageItem> ivectorstream;
typedef boost::interprocess::basic_ovectorstream<MemoryObjectsStorage::StorageItem> ovectorstream;

MemoryObjectsStorage::MemoryObjectsStorage()
    : storage_()
{

}

MemoryObjectsStorage::~MemoryObjectsStorage()
{

}

// Put readable asset into storage.
void MemoryObjectsStorage::put(const Asset& asset)
{
    // Do not put AssetId::base Asset
    if (asset.id() == AssetId::base || contains(asset.id()))
    {
        return;
    }

    boost::shared_ptr<std::istream> asset_istream = asset.istream();
    ovectorstream os;

    if (asset_istream)
    {
        os << asset_istream->rdbuf();
    }

    storage_.insert(std::make_pair(asset.id(), os.vector()));
}

void MemoryObjectsStorage::put(std::set<Asset> assets)
{
    for(std::set<Asset>::const_iterator i = assets.begin(), end = assets.end(); i != end; ++i)
        put(*i);
}

// Check if readable asset available in storage.
bool MemoryObjectsStorage::contains(const AssetId& id) const
{
    return storage_.find(id) != storage_.end();
}

bool MemoryObjectsStorage::contains(const Asset& asset) const
{
    return contains(asset.id());
}

//// Make attempt to get readable asset from storage. Non readable Asset will be returned on fail.
Asset MemoryObjectsStorage::get(const AssetId& id) const
{
    if (contains(id))
    {
        return Asset::create_for(this, id);
    }
    else
    {
        return Asset::create_id(AssetId::base);
    }
}

Asset MemoryObjectsStorage::get(const Asset& asset) const
{
    return get(asset.id());
}

// Get input stream for reading asset data. Low level API used by Asset implementation.
//External code must use get().istream() call sequense.
boost::shared_ptr<std::istream> MemoryObjectsStorage::istream_for(const AssetId& id) const
{
    if (contains(id))
    {
        return boost::shared_ptr<std::istream>(new ivectorstream(storage_.at(id)));
    }
    else
    {
        return boost::shared_ptr<std::istream>();
    }
}

boost::shared_ptr<std::istream> MemoryObjectsStorage::istream_for(const Asset& asset) const
{
    return istream_for(asset.id());
}

} } // namespace piel::lib
