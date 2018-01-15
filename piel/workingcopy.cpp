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
#include <boost_filesystem_ext.hpp>
#include <logging.h>

#include <checkout.h>
#include <assetsextractor.h>

namespace piel { namespace lib {

namespace fs = boost::filesystem;

namespace layout {

    struct L {
        static const std::string metadata_dir;
        static const std::string storage_dir;
        static const std::string current_tre_file;
        static const std::string current_tree_index_file;
        static const std::string config_file;
    };

    /*static*/ const std::string L::metadata_dir            = ".pie";
    /*static*/ const std::string L::storage_dir             = "storage";
    /*static*/ const std::string L::current_tre_file        = "reference";
    /*static*/ const std::string L::current_tree_index_file = "index.json";
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
    , current_tree_name_()
    , current_tree_index_()
{
}

WorkingCopy::WorkingCopy(const boost::filesystem::path& working_dir)
    : working_dir_(working_dir)
    , storages_(local_storage_index + 1)
    , current_tree_name_()
    , current_tree_index_()
{
    metadata_dir_           = working_dir_  / layout::L::metadata_dir;
    storage_dir_            = metadata_dir_ / layout::L::storage_dir;
    current_tree_file_      = metadata_dir_ / layout::L::current_tre_file;
    current_tree_index_file_= metadata_dir_ / layout::L::current_tree_index_file;
    config_file_            = metadata_dir_ / layout::L::config_file;
}

WorkingCopy::~WorkingCopy()
{
}

bool WorkingCopy::is_valid() const
{
    return !working_dir_.empty();
}

void WorkingCopy::init_storages(const std::string reference)
{
    init_local_storage();
    local_storage()->put(current_tree_index_.assets());
    local_storage()->create_reference(refs::Ref(reference, current_tree_index_.self().id()));
}

void WorkingCopy::attach_storages()
{
    init_local_storage();
}

void WorkingCopy::init_local_storage()
{
    storages_[local_storage_index] = IObjectsStorage::Ptr(new LocalDirectoryStorage(storage_dir_));
}

IObjectsStorage::Ptr WorkingCopy::local_storage() const
{
    return storages_[local_storage_index];
}

void WorkingCopy::init_filesystem(const std::string reference)
{
    if (!fs::create_directories(metadata_dir_) || !fs::create_directories(storage_dir_))
    {
        throw errors::init_existing_working_copy();
    }

    if (fs::exists(current_tree_file_) || fs::exists(current_tree_index_file_) || fs::exists(config_file_))
    {
        throw errors::init_existing_working_copy();
    }

    current_tree_index_.initial_for(reference);
    setup_current_tree(reference, current_tree_index_);

    init_storages(reference);
}

void WorkingCopy::attach_filesystem()
{
    if (!fs::exists(metadata_dir_) || !fs::exists(storage_dir_))
    {
        throw errors::attach_to_non_working_copy();
    }

    // Load reference
    if (fs::exists(current_tree_file_))
    {
        boost::shared_ptr<std::istream> pifs = boost::filesystem::istream(current_tree_file_);
        std::getline((*pifs), current_tree_name_);
    }
    else
    {
        throw errors::unable_to_find_reference_file();
    }

    // Load config
    if (fs::exists(config_file_))
    {
        config_ = Properties::load(*boost::filesystem::istream(config_file_));
    }

    attach_storages();

    // Load reference index
    if (fs::exists(current_tree_index_file_))
    {
        current_tree_index_ = TreeIndex::load(*boost::filesystem::istream(current_tree_index_file_), local_storage().get());
    }
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

const TreeIndex& WorkingCopy::current_tree_index() const
{
    return current_tree_index_;
}

TreeIndex WorkingCopy::current_index() const
{
    return FsIndexer::build(working_dir_, metadata_dir_);
}

/*static*/ WorkingCopy::Ptr WorkingCopy::init(const boost::filesystem::path& working_dir, const std::string reference)
{
    WorkingCopy::Ptr result(new WorkingCopy(working_dir));
    result->init_filesystem(reference);
    return result;
}

/*static*/ WorkingCopy::Ptr WorkingCopy::attach(const boost::filesystem::path& working_dir)
{
    boost::filesystem::path current_dir = working_dir;
    WorkingCopy::Ptr result;

    bool have_error = false;
    do
    {
        LOG_T << "Attempt to attach to " << current_dir;

        try {
            result = WorkingCopy::Ptr(new WorkingCopy(current_dir));
            result->attach_filesystem();
            have_error = false;
        }
        catch (const errors::attach_to_non_working_copy& e)
        {
            LOG_T << "Attaching to " << current_dir << " failed!";
            have_error = true;
        }
        catch (const errors::unable_to_find_reference_file& e)
        {
            LOG_T << "Attaching to " << current_dir << " failed! No reference file.";
            have_error = true;
        }

        current_dir = current_dir.parent_path();
    }
    while (!current_dir.empty() && have_error);

    if (have_error)
    {
        LOG_T << "Throw attaching exception!";
        throw errors::attach_to_non_working_copy();
    }

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

void WorkingCopy::set_current_tree_index(const TreeIndex& new_current_tree_index)
{
    new_current_tree_index.store(*boost::filesystem::ostream(current_tree_index_file_));
    current_tree_index_ = TreeIndex::load(*boost::filesystem::istream(current_tree_index_file_), local_storage().get());
}

std::string WorkingCopy::current_tree_name() const
{
    return current_tree_name_;
}

void WorkingCopy::set_current_tree(const std::string& new_current_tree)
{
    if (new_current_tree == current_tree_name_) return;

    // Store
    {
        boost::shared_ptr<std::ostream> pofs = boost::filesystem::ostream(current_tree_file_);
        (*pofs) << new_current_tree << std::endl;
    }

    // Load
    {
        boost::shared_ptr<std::istream> pifs = boost::filesystem::istream(current_tree_file_);
        std::getline((*pifs), current_tree_name_);
    }
}

void WorkingCopy::setup_current_tree(const std::string& new_reference, const TreeIndex& new_reference_index)
{
    set_current_tree(new_reference);
    set_current_tree_index(new_reference_index);
}

} } // namespace piel::lib
