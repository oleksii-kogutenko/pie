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

#include <assetsextractor.h>

#include <logging.h>

namespace piel { namespace lib {

namespace fs = boost::filesystem;

AssetsExtractor::AssetsExtractor(const TreeIndex& index, ExtractPolitic politic)
    : index_(index)
    , politic_(politic)
{
}

AssetsExtractor::~AssetsExtractor()
{
}

void AssetsExtractor::create_parent_path(const boost::filesystem::path& item_path)
{
    fs::path parent_path    = item_path.parent_path();

    LOG_T << "Extract item: " << item_path << " parent: " << parent_path;

    // Create item parent directory
    if (fs::exists(parent_path))
    {
        if (!fs::is_directory(parent_path))
        {
            LOG_T << "Replace parent by directory: " << parent_path;

            fs::remove_all(parent_path);

            fs::create_directories(parent_path);
        }
    }
    else
    {
        LOG_T << "Create parent: " << parent_path;

        fs::create_directories(parent_path);
    }

    if (!fs::exists(parent_path) || !fs::is_directory(parent_path))
    {
        LOG_F << "No parent: " << parent_path;

        // No item parent
        throw errors::unable_to_create_item_parent();
    }
}

void AssetsExtractor::extract_asset_into(const boost::filesystem::path& item_path,
        const TreeIndex::Content::const_iterator& i)
{
    boost::shared_ptr<std::istream> isp = i->second.istream();
    if (!isp)
    {
        LOG_F << "Non readable asset: " << i->second.id().string();

        // Non readable asset
        throw errors::attempt_to_export_non_readable_asset();
    }

    create_parent_path(item_path);

    std::string asset_type = index_.get_attr_(i->first,
            PredefinedAttributes::asset_type, PredefinedAttributes::asset_type__file);

    std::string asset_mode_str = index_.get_attr_(i->first, PredefinedAttributes::asset_mode);

    int asset_mode = PredefinedAttributes::parse_asset_mode(asset_mode_str,
            PredefinedAttributes::default_asset_mode);

    if (asset_type == PredefinedAttributes::asset_type__file)
    {
        LOG_T << "Copy data from asset to file.";

        boost::shared_ptr<std::ostream> osp = fs::ostream(item_path);

        if (i->second.id().string() != fs::copy_into(osp, isp))
        {
            LOG_F << "Corrupted asset data.";

            throw errors::exported_data_is_corrupted();
        }

        fs::permissions(item_path, (fs::perms)asset_mode);
    }
    else if (asset_type == PredefinedAttributes::asset_type__symlink)
    {
        LOG_T << "Create symbolic link.";

        std::ostringstream* ossp = new std::ostringstream();
        boost::shared_ptr<std::ostream> oss(ossp);

        if (i->second.id().string() != fs::copy_into(oss, isp))
        {
            LOG_F << "Corrupted asset data.";

            throw errors::exported_data_is_corrupted();
        }

        std::string symlink_target = ossp->str();

        LOG_T << "Create symbolic link " << item_path << " -> " << symlink_target;

        fs::create_symlink(symlink_target, item_path);

        // TODO: permissions for symlinks
        //fs::permissions(item_path, fs::perms::symlink_perms|((fs::perms)asset_mode));
    }
    else
    {
        throw errors::unknown_asset_type();
    }
}

void AssetsExtractor::extract_into(const boost::filesystem::path& directory)
{
    if (!fs::exists(directory) || !fs::is_directory(directory))
    {
        LOG_F << "Attempt to extract data to non existing directory: " << directory;

        throw errors::attempt_to_export_to_non_existing_directory();
    }

    for (TreeIndex::Content::const_iterator i = index_.content().begin(), end = index_.content().end(); i != end; ++i)
    {
        fs::path item_path      = directory / i->first;

        if (fs::exists(item_path))
        {
            if (politic_ & ExtractPolicy__replace_existing)
            {
                LOG_T << "Replace existing file: " << item_path;

                fs::remove_all(item_path);
            }

            if (politic_ & ExtractPolicy__backup_existing)
            {
                LOG_T << "Backup existing file: " << item_path;

                fs::copy_file(item_path, item_path / (std::string(".backup.") + index_.self().id().string()));
            }

            if (politic_ & ExtractPolicy__put_new_with_suffix)
            {
                item_path /= std::string(".new.") + index_.self().id().string();

                LOG_T << "New item path: " << item_path;
            }

            if ((politic_ & ExtractPolicy__keep_existing) && !(politic_ & ExtractPolicy__put_new_with_suffix))
            {
                LOG_T << "Keep existing: " << item_path;

                continue;
            }
        }

        extract_asset_into(item_path, i);
    }
}

} } // namespace piel::lib
