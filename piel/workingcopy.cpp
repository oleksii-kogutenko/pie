/*
 * Copyright (c) 2018, diakovliev
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

#include <workingcopy.h>
#include <fsindexer.h>
#include <indextofsexporter.h>
#include <boost_filesystem_ext.hpp>
#include <logging.h>

namespace piel { namespace lib {

namespace fs = boost::filesystem;

namespace layout {

    struct L {
        static const std::string metadata_dir;
        static const std::string storage_dir;
        static const std::string reference_index_file;
        static const std::string config_file;
    };

    /*static*/ const std::string L::metadata_dir            = ".pie";
    /*static*/ const std::string L::storage_dir             = "storage";
    /*static*/ const std::string L::reference_index_file    = "index.json";
    /*static*/ const std::string L::config_file             = "config.properties";

};

/*static*/ const int WorkingCopy::local_storage_index = 0;

/*static*/ Properties::DefaultFromEnv PredefinedConfigs::author =
        Properties::Property("author",         "unknown").default_from_env("PIE_AUTHOR");

/*static*/ Properties::DefaultFromEnv PredefinedConfigs::email =
        Properties::Property("email",          "unknown").default_from_env("PIE_EMAIL");

/*static*/ Properties::DefaultFromEnv PredefinedConfigs::commiter =
        Properties::Property("commiter",       "unknown").default_from_env("PIE_COMMITER");

/*static*/ Properties::DefaultFromEnv PredefinedConfigs::commiter_email =
        Properties::Property("commiter_email", "unknown").default_from_env("PIE_COMMITER_EMAIL");

WorkingCopy::WorkingCopy(const boost::filesystem::path& working_dir)
    : working_dir_(working_dir)
    , storages_(local_storage_index + 1)
    , reference_index_()
    , current_index_()
{
    metadata_dir_           = working_dir_  / layout::L::metadata_dir;
    storage_dir_            = metadata_dir_ / layout::L::storage_dir;
    reference_index_file_   = metadata_dir_ / layout::L::reference_index_file;
    config_file_            = metadata_dir_ / layout::L::config_file;
}

WorkingCopy::~WorkingCopy()
{
}

void WorkingCopy::init_storages()
{
    storages_[local_storage_index] = IObjectsStorage::Ptr(new LocalDirectoryStorage(storage_dir_));
}

void WorkingCopy::init_filesystem()
{
    if (!fs::create_directories(metadata_dir_) || !fs::create_directories(storage_dir_))
    {
        throw errors::init_existing_working_copy();
    }

    if (fs::exists(reference_index_file_) || fs::exists(config_file_))
    {
        throw errors::init_existing_working_copy();
    }

    init_storages();
}

void WorkingCopy::attach_filesystem()
{
    if (!fs::exists(metadata_dir_) || !fs::exists(storage_dir_))
    {
        throw errors::attach_to_non_working_copy();
    }

    // Load config
    if (fs::exists(config_file_))
    {
        config_ = Properties::load(*boost::filesystem::istream(config_file_));
    }

    // Load reference index
    if (fs::exists(reference_index_file_))
    {
        reference_index_ = Index::load(*boost::filesystem::istream(reference_index_file_));
    }

    init_storages();
}

/*static*/ WorkingCopy WorkingCopy::init(const boost::filesystem::path& working_dir)
{
    WorkingCopy result(working_dir);
    result.init_filesystem();
    return result;
}

/*static*/ WorkingCopy WorkingCopy::attach(const boost::filesystem::path& working_dir)
{
    WorkingCopy result(working_dir);
    result.attach_filesystem();
    return result;
}

void WorkingCopy::set_config(const std::string& name, const std::string& value)
{
    config_.set(name, value);
    config_.store(*boost::filesystem::ostream(config_file_));
}

std::string WorkingCopy::get_config(const std::string& name, const std::string& default_value)
{
    if (fs::exists(config_file_))
    {
        config_ = Properties::load(*boost::filesystem::istream(config_file_));
    }

    return config_.get(name, default_value);
}


std::string WorkingCopy::checkout(const std::string& ref_to)
{
    IObjectsStorage::Ptr local_storage = storages_[local_storage_index];

    boost::optional<Index> ref_index = index_from_ref(local_storage, ref_to);
    if (!ref_index)
    {
        return "";
    }

    reference_index_ = *ref_index;

    IndexToFsExporter index_exporter(reference_index_);
    index_exporter.export_to(working_dir_);

    reference_index_.store(*boost::filesystem::ostream(reference_index_file_));

    return reference_index_.self().id().string();
}

void WorkingCopy::export_to(const boost::filesystem::path& directory)
{
    IndexToFsExporter index_exporter(reference_index_);
    index_exporter.export_to(directory);
}

std::string WorkingCopy::reset(const std::string& ref_to)
{
    return ref_to;
}

boost::optional<Index> WorkingCopy::index_from_ref(const IObjectsStorage::Ptr& storage, const std::string& ref) const
{
    AssetId ref_to_asset_id = storage->resolve(ref);
    if (ref_to_asset_id != AssetId::empty)
    {
        Asset ref_to_asset = storage->asset(ref_to_asset_id);
        return Index::load(ref_to_asset);
    }
    else
    {
        return boost::none;
    }
}

std::string WorkingCopy::commit(const std::string& message, const std::string& ref_to)
{
    IObjectsStorage::Ptr local_storage = storages_[local_storage_index];

    boost::optional<Index> ref_index = index_from_ref(local_storage, ref_to);
    if (ref_index)
    {
        reference_index_ = *ref_index;
    }

    current_index_ = FsIndexer::build(working_dir_, metadata_dir_);
    if (!reference_index_.empty())
    {
        current_index_.set_parent(reference_index_.self());
    }

    // Fill from config
    current_index_.set_author_(         config_.get(PredefinedConfigs::author).value());
    current_index_.set_email_(          config_.get(PredefinedConfigs::email).value());
    current_index_.set_commiter_(       config_.get(PredefinedConfigs::commiter).value());
    current_index_.set_commiter_email_( config_.get(PredefinedConfigs::commiter_email).value());

    // Set message
    current_index_.set_message_(message);

    // Put changes into local storage
    local_storage->put(current_index_.assets());
    local_storage->put(IObjectsStorage::Ref(ref_to, current_index_.self()));

    // Store index
    current_index_.store(*boost::filesystem::ostream(reference_index_file_));

    // Restore reference index
    reference_index_ = Index::load(*boost::filesystem::istream(reference_index_file_), local_storage.get());

    return reference_index_.self().id().string();
}

IndexesDiff WorkingCopy::diff(const std::string& ref_base) const
{
    IObjectsStorage::Ptr local_storage = storages_[local_storage_index];

    boost::optional<Index> ref_index = index_from_ref(local_storage, ref_base);
    if (ref_index)
    {
        return IndexesDiff::diff(*ref_index, FsIndexer::build(working_dir_, metadata_dir_));
    }
    else
    {
        return IndexesDiff::diff(reference_index_, FsIndexer::build(working_dir_, metadata_dir_));
    }
}

} } // namespace piel::lib
