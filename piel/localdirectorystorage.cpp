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
#include <boost/algorithm/string.hpp>

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

    /*static*/ const std::string   L::objects                        = "objects";
    /*static*/ const std::string   L::references                     = "references.properties";
    /*static*/ const unsigned      L::objects_storage_depth          = 3;
    /*static*/ const unsigned      L::objects_subdirs_names_length   = 2;

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
            LOGT << "Path part: " << part << " pos: " << pos << ELOG;
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
            LOGT << "Path part: " << *i << ELOG;

            result /= *i;
        }

        return result;
    }

    fs::path asset_path(const fs::path& root_dir, const Asset& asset)
    {
        return asset_path(root_dir, asset.id());
    }

};

namespace constants {
    struct C {
        static const std::string ref_ids_delimiter;
    };

    /*static*/ const std::string C::ref_ids_delimiter = ",";
};

LocalDirectoryStorage::LocalDirectoryStorage(const boost::filesystem::path& root_dir)
    : IObjectsStorage()
    , root_dir_(root_dir)
{
    objects_    = root_dir_ / layout::L::objects;
    references_ = root_dir_ / layout::L::references;

    LOGT << "Objects path: " << objects_ << " References: " << references_ << ELOG;

    if (!fs::exists(objects_) || !fs::exists(references_))
    {
        LOGT << "Init. Objects path: " << objects_ << " References: " << references_ << ELOG;
        init();
    }
    else
    {
        LOGT << "Attach. Objects path: " << objects_ << " References: " << references_ << ELOG;
        attach();
    }
}

LocalDirectoryStorage::~LocalDirectoryStorage()
{
}

void LocalDirectoryStorage::attach()
{
    LOGT << "Loading references from: " << references_ << ELOG;

    refs_ = Properties::load(*fs::istream(references_).get());
}

void LocalDirectoryStorage::init()
{
    if (!fs::exists(objects_) && !fs::create_directories(objects_))
    {
        LOGF << "Unable to create objects directory: " << objects_ << ELOG;

        throw errors::unable_to_create_directory();
    }

    LOGT << "Init empty references: " << references_ << ELOG;

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

    LOGT << "Asset path: " << asset_path << " Parent path: " << asset_parent_path << ELOG;

    boost::shared_ptr<std::istream> isp = asset.istream();

    if (isp)
    {
        if (!fs::exists(asset_parent_path))
        {
            if (!fs::create_directories(asset_parent_path))
            {
                LOGF << "Unable to create parent directory: " << asset_parent_path << " for the asset: " << asset.id().string() << ELOG;

                throw errors::unable_to_create_directory();
            }
        }

        boost::shared_ptr<std::ostream> osp = fs::ostream(asset_path);

        boost::filesystem::copy_into(osp, isp);
    }
    else
    {
        LOGF << "Asset: " << asset.id().string() << " is not readable!" << ELOG;

        throw errors::attempt_to_put_non_readable_asset();
    }
}

void LocalDirectoryStorage::put(std::set<Asset> assets)
{
    typedef std::set<Asset>::const_iterator ConstIter;
    for(ConstIter i = assets.begin(), end = assets.end(); i != end; ++i)
    {
        LOGT << "Put asset: " << i->id().string() << ELOG;
        put(*i);
    }
}

void LocalDirectoryStorage::create_reference(const refs::Ref& ref)
{
    if (!refs_.data().insert(std::make_pair(ref.first, ref.second.string())).second)
    {
        throw errors::unable_to_insert_new_reference();
    }
    refs_.store(*fs::ostream(references_).get());
}

void LocalDirectoryStorage::destroy_reference(const refs::Ref::first_type& ref_name)
{
    refs_.data().erase(ref_name);
    refs_.store(*fs::ostream(references_).get());
}

void LocalDirectoryStorage::update_reference(const refs::Ref& ref)
{
    std::string prev_value = refs_.data().at(ref.first);
    refs_.data()[ref.first] = ref.second.string() + constants::C::ref_ids_delimiter + prev_value;
    refs_.store(*fs::ostream(references_).get());
}

// Check if readable asset available in storage.
bool LocalDirectoryStorage::contains(const AssetId& id) const
{
    return fs::exists(layout::asset_path(objects_, id));
}

// Make attempt to get readable asset from storage. Non readable Asset will be returned on fail.
Asset LocalDirectoryStorage::asset(const IObjectsStorage::Ptr& storage, const AssetId& id) const
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
        std::string ref_indexes_list = i->second;

        std::vector<std::string> ids;
        boost::split(ids, ref_indexes_list, boost::is_any_of(constants::C::ref_ids_delimiter));

        LOGT << "Ref: " << ref << " resolved to id: " << ids[0] << ELOG;

        return AssetId::create(ids[0]);
    }
    else if (ref.length() == AssetId::str_digest_len)
    {
        AssetId id = AssetId::create(ref);
        if (id != AssetId::empty && contains(id))
        {
            LOGT << "Resolved to id: " << id.string() << ELOG;
            return id;
        }
        else
        {
            LOGT << "Uknown id: " << ref << ELOG;
            return AssetId::empty;
        }
    }
    else
    {
        LOGT << "Uknown string: " << ref << ELOG;
        return AssetId::empty;
    }
}

std::set<refs::Ref> LocalDirectoryStorage::references() const
{
    std::set<refs::Ref> result;
    for(Properties::MapType::const_iterator i = refs_.data().begin(), end = refs_.data().end(); i != end; ++i)
    {
        result.insert(*i);
    }
    return result;
}

} } // namespace piel::lib
