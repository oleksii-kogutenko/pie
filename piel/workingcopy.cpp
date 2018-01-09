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

#include <checkout.h>

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

WorkingCopy::WorkingCopy()
    : working_dir_()
    , storages_(local_storage_index + 1)
    , reference_index_()
    , current_index_()
{
}

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

bool WorkingCopy::is_valid() const
{
    return !working_dir_.empty();
}

void WorkingCopy::init_storages()
{
    storages_[local_storage_index] = IObjectsStorage::Ptr(new LocalDirectoryStorage(storage_dir_));
}

IObjectsStorage::Ptr WorkingCopy::local_storage() const
{
    return storages_[local_storage_index];
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

fs::path WorkingCopy::working_dir() const
{
    return working_dir_;
}

fs::path WorkingCopy::metadata_dir() const
{
    return metadata_dir_;
}

fs::path WorkingCopy::storage_dir() const
{
    return storage_dir_;
}

fs::path WorkingCopy::reference_index_file() const
{
    return reference_index_file_;
}

fs::path WorkingCopy::config_file() const
{
    return config_file_;
}

Properties& WorkingCopy::config()
{
    return config_;
}

WorkingCopy::Storages& WorkingCopy::storages()
{
    return storages_;
}

const Index& WorkingCopy::reference_index() const
{
    return reference_index_;
}

Index WorkingCopy::current_index() const
{
    return FsIndexer::build(working_dir_, metadata_dir_);
}

/*static*/ WorkingCopy::Ptr WorkingCopy::init(const boost::filesystem::path& working_dir)
{
    WorkingCopy::Ptr result(new WorkingCopy(working_dir));
    result->init_filesystem();
    return result;
}

/*static*/ WorkingCopy::Ptr WorkingCopy::attach(const boost::filesystem::path& working_dir)
{
    WorkingCopy::Ptr result(new WorkingCopy(working_dir));
    result->attach_filesystem();
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

void WorkingCopy::set_reference_index(const Index& new_reference_index)
{
    new_reference_index.store(*boost::filesystem::ostream(reference_index_file_));
    reference_index_ = Index::load(*boost::filesystem::istream(reference_index_file_), local_storage().get());
}

} } // namespace piel::lib
