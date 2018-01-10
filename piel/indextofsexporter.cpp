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

#include <indextofsexporter.h>
#include <logging.h>

namespace piel { namespace lib {

IndexToFsExporter::IndexToFsExporter(const Index& index, ExportingPolitic politic)
    : index_(index)
    , politic_(politic)
{
}

IndexToFsExporter::~IndexToFsExporter()
{
}

void IndexToFsExporter::export_to(const boost::filesystem::path& directory)
{
    namespace fs = boost::filesystem;

    if (!fs::exists(directory) || !fs::is_directory(directory))
    {
        LOG_F << "Attempt to export data to non existing directory: " << directory;

        throw errors::attempt_to_export_to_non_existing_directory();
    }

    for (Index::Content::const_iterator i = index_.content().begin(), end = index_.content().end(); i != end; ++i)
    {
        boost::shared_ptr<std::istream> isp = i->second.istream();
        if (isp)
        {
            fs::path item_path      = directory / i->first;
            fs::path parent_path    = item_path.parent_path();

            LOG_T << "Export item path: " << item_path << " parent: " << parent_path;

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

            if (fs::exists(item_path))
            {
                if (politic_ & ExportPolicy__replace_existing)
                {
                    LOG_T << "Replace existing file: " << item_path;

                    fs::remove_all(item_path);
                }

                if (politic_ & ExportPolicy__backup_existing)
                {
                    LOG_T << "Backup existing file: " << item_path;

                    fs::copy_file(item_path, item_path / (std::string(".backup.") + index_.self().id().string()));
                }

                if (politic_ & ExportPolicy__put_new_with_suffix)
                {
                    item_path /= std::string(".new.") + index_.self().id().string();

                    LOG_T << "New item path: " << item_path;
                }

                if ((politic_ & ExportPolicy__keep_existing) && !(politic_ & ExportPolicy__put_new_with_suffix))
                {
                    LOG_T << "Keep existing: " << item_path;

                    continue;
                }
            }

            LOG_T << "Copy data from asset.";

            // Create item and copy content into it
            boost::shared_ptr<std::ostream> osp = fs::ostream(item_path);

            if (i->second.id().string() != fs::copy_into(osp, isp))
            {
                LOG_F << "Corrupted data.";

                throw errors::exported_data_is_corrupted();
            }

            // TODO: Process file attributes
        }
        else
        {
            LOG_F << "Non readable asset: " << i->second.id().string();

            // Non readable asset
            throw errors::attempt_to_export_non_readable_asset();
        }
    }
}

} } // namespace piel::lib
