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

#ifndef PIEL_IOBJECTSSTORAGE_H_
#define PIEL_IOBJECTSSTORAGE_H_

#include <asset.h>

#include <boost/shared_ptr.hpp>
#include <set>
#include <string>
#include <iostream>

namespace piel { namespace lib {

namespace errors {
    // Common put error
    struct attempt_to_put_non_readable_asset {};
}

namespace refs {
    typedef std::pair<std::string, AssetId>             Ref;
    typedef std::pair<Ref,Ref>                          RefsRange;
    typedef std::pair<Ref::first_type,Ref::first_type>  Range;
}

class IObjectsStorage
{
public:
    typedef boost::shared_ptr<IObjectsStorage> Ptr;

    virtual ~IObjectsStorage();

    // Put readable asset into storage.
    virtual void put(const Asset& asset) = 0;
    virtual void put(std::set<Asset> assets) = 0;

    // Check if readable asset available in storage.
    virtual bool contains(const AssetId& id) const = 0;

    // Make attempt to get readable asset from storage. Non readable Asset will be returned on fail.
    virtual Asset asset(const AssetId& id) const = 0;

    // Get input stream for reading asset data. Low level API used by Asset implementation.
    //External code must use get().istream() call sequense.
    virtual boost::shared_ptr<std::istream> istream_for(const AssetId& id) const = 0;

    // References related API
    //typedef std::pair<std::string, AssetId> Ref;

    virtual void update_reference(const refs::Ref& ref) = 0;
    virtual AssetId resolve(const std::string& refName) const = 0;
    virtual std::set<refs::Ref> references() const = 0;

};

} } // namespace piel::lib

#endif /* PIEL_IOBJECTSSTORAGE_H_ */
