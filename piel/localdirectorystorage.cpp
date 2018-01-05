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

#include <localdirectorystorage.h>
#include <logging.h>

#include <fstream>

namespace piel { namespace lib {

namespace fs = boost::filesystem;

namespace layout {

    struct L {
        static const std::string objects;
        static const std::string references;
        static const unsigned objects_storage_depth;
        static const unsigned objects_subdirs_names_length;
    };

    const std::string   L::objects                        = "objects";
    const std::string   L::references                     = "references.properties";
    const unsigned      L::objects_storage_depth          = 3;
    const unsigned      L::objects_subdirs_names_length   = 2;

    std::vector<std::string> storage_dir_parts_for(const AssetId& asset_id)
    {
        std::vector<std::string> result;

        if (asset_id == AssetId::not_calculated)
        {
            // TODO: Error
            return result;
        }

        std::string asset_id_str = asset_id.string();
        //std::string::size_type size = asset_id.size();

        unsigned pos = 0;
        for (
            ; pos < L::objects_storage_depth * L::objects_subdirs_names_length
            ; pos += L::objects_subdirs_names_length
            )
        {
            std::string part = asset_id_str.substr(pos, L::objects_subdirs_names_length);
            LOG_T << "Path part: " << part << " pos: " << pos;
            result.push_back(part);
        }

        result.push_back(asset_id_str);

        return result;
    }

    fs::path asset_path(const fs::path& root_dir, const AssetId& asset_id)
    {
        fs::path result = root_dir;

        typedef std::vector<std::string>::const_iterator ConstIter;

        std::vector<std::string> path_parts = storage_dir_parts_for(asset_id);

        for (ConstIter i = path_parts.begin(), end = path_parts.end(); i != end; ++i)
        {
            LOG_T << "Path part: " << *i;

            result /= *i;
        }

        return result;
    }

    fs::path asset_path(const fs::path& root_dir, const Asset& asset)
    {
        return asset_path(root_dir, asset.id());
    }

};

LocalDirectoryStorage::LocalDirectoryStorage(const boost::filesystem::path& root_dir)
    : IObjectsStorage()
    , root_dir_(root_dir)
{
    objects_    = root_dir_ / layout::L::objects;
    references_ = root_dir_ / layout::L::references;

    LOG_T << "Objects path: " << objects_ << " References: " << references_;

    init();
}

LocalDirectoryStorage::~LocalDirectoryStorage()
{
}

void LocalDirectoryStorage::init()
{
    if (!fs::exists(objects_) && !fs::create_directories(objects_))
    {
        LOG_F << "Unable to create objects directory: " << objects_;

        throw errors::unable_to_create_directory();
    }

    refs_.store(*fs::ostream(references_).get());
}

// Put readable asset(s) into storage.
void LocalDirectoryStorage::put(const Asset& asset)
{
    if (asset.id() == AssetId::empty || contains(asset.id()))
    {
        return;
    }

    // 1. Create parent directory for the asset
    fs::path asset_path             = layout::asset_path(objects_, asset);
    fs::path asset_parent_path      = asset_path.parent_path();

    LOG_T << "Asset path: " << asset_path << " Parent path: " << asset_parent_path;

    boost::shared_ptr<std::istream> isp = asset.istream();

    if (isp)
    {
        if (!fs::exists(asset_parent_path))
        {
            if (!fs::create_directories(asset_parent_path))
            {
                LOG_F << "Unable to create parent directory: " << asset_parent_path << " for the asset: " << asset.id().string();

                throw errors::unable_to_create_directory();
            }
        }

        boost::shared_ptr<std::ostream> osp = fs::ostream(asset_path);

        boost::filesystem::copy_into(osp, isp);
    }
    else
    {
        LOG_F << "Asset: " << asset.id().string() << " is not readable!";

        throw errors::attempt_to_put_non_readable_asset();
    }
}

void LocalDirectoryStorage::put(std::set<Asset> assets)
{
    typedef std::set<Asset>::const_iterator ConstIter;
    for(ConstIter i = assets.begin(), end = assets.end(); i != end; ++i)
    {
        LOG_T << "Put asset: " << i->id().string();
        put(*i);
    }
}

void LocalDirectoryStorage::put(const IObjectsStorage::Ref& ref)
{
    refs_[ref.first] = ref.second.string();
    refs_.store(*fs::ostream(references_).get());
}

// Check if readable asset available in storage.
bool LocalDirectoryStorage::contains(const AssetId& id) const
{
    return fs::exists(layout::asset_path(objects_, id));
}

// Make attempt to get readable asset from storage. Non readable Asset will be returned on fail.
Asset LocalDirectoryStorage::asset(const AssetId& id) const
{
    fs::path asset_path = layout::asset_path(objects_, id);
    if (fs::exists(asset_path))
    {
        return Asset::create_for(asset_path);
    }
    else
    {
        return Asset();
    }
}

// Get input stream for reading asset data. Low level API used by Asset implementation.
//External code must use get().istream() call sequense.
boost::shared_ptr<std::istream> LocalDirectoryStorage::istream_for(const AssetId& id) const
{
    boost::shared_ptr<std::istream> result;

    fs::path asset_path = layout::asset_path(objects_, id);
    if (fs::exists(asset_path))
    {
        return result = fs::istream(asset_path);
    }

    return result;
}

AssetId LocalDirectoryStorage::resolve(const std::string& ref) const
{
    Properties::MapType::const_iterator i = refs_.data().find(ref);
    if (i != refs_.data().end())
    {
        return AssetId::create(i->second);
    }
    else
    {
        return AssetId::empty;
    }
}

std::set<IObjectsStorage::Ref> LocalDirectoryStorage::references() const
{
    std::set<IObjectsStorage::Ref> result;
    for(Properties::MapType::const_iterator i = refs_.data().begin(), end = refs_.data().end(); i != end; ++i)
    {
        result.insert(*i);
    }
    return result;
}

} } // namespace piel::lib
