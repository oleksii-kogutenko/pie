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

#ifndef PIEL_ASSET_H_
#define PIEL_ASSET_H_

#include <assetid.h>
#include <boost/filesystem.hpp>
#include <boost_property_tree_ext.hpp>

#include <zipfile.h>

namespace piel { namespace lib {

class AssetImpl;
class IObjectsStorage;

class Asset
{
    Asset(AssetImpl *impl);
public:
    Asset();
    Asset(const Asset& src);
    ~Asset();

    void operator=(const Asset& src);
    bool operator!=(const Asset& src) const;
    bool operator==(const Asset& src) const;
    bool operator<(const Asset& src) const;
    operator AssetId() const;

    const AssetId& id() const;
    boost::shared_ptr<std::istream> istream() const;

    static Asset create_id(const AssetId& id);

    static Asset create_for(const IObjectsStorage *storage, const AssetId& id);
    static Asset create_for(const std::string& str_data);
    static Asset create_for(const boost::filesystem::path& file_path);
    static Asset create_for(boost::shared_ptr<std::istream> is);
    static Asset create_for(boost::shared_ptr<ZipEntry> entry);

    static void store(boost::property_tree::ptree& tree, const Asset& asset);
    static Asset load(const boost::property_tree::ptree& tree);

private:
    AssetImpl *impl_;
};

} } // namespace piel::lib

#endif /* PIEL_ASSET_H_ */
