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

#ifndef PIEL_LOCALDIRECTORYSTORAGE_H_
#define PIEL_LOCALDIRECTORYSTORAGE_H_

#include <boost_filesystem_ext.hpp>
#include <iobjectsstorage.h>
#include <properties.h>

namespace piel { namespace lib {

namespace errors {
    struct unable_to_create_directory {};
}

class LocalDirectoryStorage : public IObjectsStorage
{
public:
    LocalDirectoryStorage(const boost::filesystem::path& root_dir);
    virtual ~LocalDirectoryStorage();

    // Put readable asset(s) into storage.
    void put(const Asset& asset);
    void put(std::set<Asset> assets);
    void put(const IObjectsStorage::Ref& ref);

    // Check if readable asset available in storage.
    bool contains(const AssetId& id) const;

    // Make attempt to get readable asset from storage. Non readable Asset will be returned on fail.
    Asset asset(const AssetId& id) const;

    // Get input stream for reading asset data. Low level API used by Asset implementation.
    //External code must use get().istream() call sequense.
    boost::shared_ptr<std::istream> istream_for(const AssetId& id) const;

    AssetId resolve(const std::string& ref) const;
    std::set<IObjectsStorage::Ref> references() const;

protected:
    void init();

private:
    boost::filesystem::path root_dir_;
    boost::filesystem::path objects_;
    boost::filesystem::path references_;
    Properties refs_;
};

} } // namespace piel::lib

#endif /* PIEL_LOCALDIRECTORYSTORAGE_H_ */
