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

#ifndef PIEL_MEMORYOBJECTSSTORAGE_H_
#define PIEL_MEMORYOBJECTSSTORAGE_H_

#include <iobjectsstorage.h>
#include <vector>
#include <map>

namespace piel { namespace lib {

class MemoryObjectsStorage: public IObjectsStorage
{
public:
    typedef char                                                    Byte;
    typedef std::vector<Byte>                                       Object;
    typedef std::map<AssetId,Object>                                Storage;
    typedef std::map<refs::Ref::first_type,refs::Ref::second_type>  References;

    MemoryObjectsStorage();
    virtual ~MemoryObjectsStorage();

    // Put readable asset(s) into storage.
    void put(const Asset& asset);
    void put(std::set<Asset> assets);
    void put(const refs::Ref& ref);

    // Check if readable asset available in storage.
    bool contains(const AssetId& id) const;

    // Make attempt to get readable asset from storage. Non readable Asset will be returned on fail.
    Asset asset(const AssetId& id) const;

    // Get input stream for reading asset data. Low level API used by Asset implementation.
    //External code must use get().istream() call sequense.
    boost::shared_ptr<std::istream> istream_for(const AssetId& id) const;

    AssetId resolve(const std::string& ref) const;
    std::set<refs::Ref> references() const;

private:
    Storage     assets_;
    References  refs_;
};

} } // namespace piel::lib

#endif /* PIEL_MEMORYOBJECTSSTORAGE_H_ */
