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

#ifndef PIEL_IOBJECTSSTORAGE_H_
#define PIEL_IOBJECTSSTORAGE_H_

#include <boost/shared_ptr.hpp>
#include <asset.h>

#include <string>
#include <iostream>

namespace piel { namespace lib {

// Pseudocode:
//
// Asset asset;
// asset.os << "asset data";
// std::cout << asset.id();
//
// Asset file_asset = Asset::from_file("path/to/file");
// std::string w;
// file_asset.is >> w;
// std::cout << w;
// std::cout << asset.id();
//
// Asset remote_http_asset = Asset::from_url("http://url/to/file");
//
// AssedId parentId = AssedId::base;
//
// // Index is Asset too
// Index index(parentId);
//
// index.add("path/1", asset);
// index.add("path/2", file_asset);
// index.add("path/3", http_asset);
// // Predefined attributes
// index.set_message("Test message");
// index.set_author("author");
// index.set_email("email");
// index.set_commiter("author");
// index.set_commiter_email("email");
// // Custom attributes
// index.set_attribute("test-attribute-1", "test attribute value-1");
// index.set_attribute("test-attribute-2", "test attribute value-2");
// index.set_attribute("test-attribute-3", "test attribute value-3");
//
// Storage storage = Storage::create("abstract path");
// storage.add(asset);
// storage.add(file_asset);
// storage.add(remote_http_asset);
// storage.add(index);
// // Optionally can be created symbolic reference
// AssetRef index_ref = storage.ref(index, "index_ref");
// storage.add_ref(index_ref);
//
// storage.commit();
// storage.get("index_ref");
// storage.get(index.id);
// storage.checkout();
//
// Storage remote_storage = Storage::create(...)
// Replicator replicator(remote_storage, storage)
//
// replicator.push();          // local -> remote
// replicator.push(index_ref); //
//
// replicator.fetch();         // remote -> local
// replicator.fetch(index_ref);//
//
// Index index1 = storage.get("index_ref");
// Index index2 = storage.get(index.id);
// asset(index1.id == index.id);
// asset(index2.id == index.id);
//

class IObjectsStorage
{
public:
    IObjectsStorage();
    virtual ~IObjectsStorage();

    // Put readable asset into storage.
    virtual void put(const Asset& asset) = 0;

    // Check if readable asset available in storage.
    virtual bool contains(const AssetId& id) const = 0;
    virtual bool contains(const Asset& asset) const = 0;

    // Make attempt to get readable asset from storage. Non readable Asset will be returned on fail.
//    virtual Asset get(const AssetId& id) const = 0;
//    virtual Asset get(const Asset& asset) const = 0;

    // Get input stream for reading asset data. Low level API used by Asset implementation.
    //External code must use get().istream() call sequense.
    virtual boost::shared_ptr<std::istream> istream_for(const AssetId& id) const = 0;
    virtual boost::shared_ptr<std::istream> istream_for(const Asset& asset) const = 0;

};

} } // namespace piel::lib

#endif /* PIEL_IOBJECTSSTORAGE_H_ */
