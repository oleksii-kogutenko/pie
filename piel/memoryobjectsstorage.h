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

#ifndef PIEL_MEMORYOBJECTSSTORAGE_H_
#define PIEL_MEMORYOBJECTSSTORAGE_H_

#include <iobjectsstorage.h>
#include <vector>
#include <map>

namespace piel { namespace lib {

class MemoryObjectsStorage: public IObjectsStorage
{
public:
    typedef char Byte;
    typedef std::vector<Byte> StorageItem;
    typedef std::map<AssetId,StorageItem> Storage;

    MemoryObjectsStorage();
    virtual ~MemoryObjectsStorage();

    // Put readable asset into storage.
    void put(const Asset& asset);

    // Check if readable asset available in storage.
    bool contains(const AssetId& id) const;
    bool contains(const Asset& asset) const;

    // Make attempt to get readable asset from storage. Non readable Asset will be returned on fail.
    Asset get(const AssetId& id) const;
    Asset get(const Asset& asset) const;

    // Get input stream for reading asset data. Low level API used by Asset implementation.
    //External code must use get().istream() call sequense.
    boost::shared_ptr<std::istream> istream_for(const AssetId& id) const;
    boost::shared_ptr<std::istream> istream_for(const Asset& asset) const;

private:
    Storage storage_;

};

} } // namespace piel::lib

#endif /* PIEL_MEMORYOBJECTSSTORAGE_H_ */
