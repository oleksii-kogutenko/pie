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

#include <clean.h>
#include <fsindexer.h>
#include <indexesdiff.h>
#include <logging.h>

namespace piel { namespace cmd {

namespace fs = boost::filesystem;

Clean::Clean(const piel::lib::WorkingCopy::Ptr& working_copy)
    : WorkingCopyCommand(working_copy)
{
}

Clean::~Clean()
{
}

std::string Clean::operator()()
{
    piel::lib::Index current_index = piel::lib::FsIndexer::build(working_copy()->working_dir(), working_copy()->metadata_dir());

    piel::lib::IndexesDiff diff = piel::lib::IndexesDiff::diff(working_copy()->reference_index(), current_index);
    if (!diff.empty())
    {
        typedef piel::lib::IndexesDiff::ContentDiff::const_iterator ConstIter;
        for(ConstIter i = diff.content_diff().begin(), end = diff.content_diff().end(); i != end; ++i)
        {
            fs::path item_path = working_copy()->working_dir() / i->first;

            switch (i->second.first)
            {
            case piel::lib::IndexesDiff::ElementState_unmodified:
            break;
            case piel::lib::IndexesDiff::ElementState_removed:
            {
                boost::optional<piel::lib::Asset> oa = working_copy()->reference_index().asset(i->first);
                piel::lib::Asset asset = *oa;

                fs::path item_parent = item_path.parent_path();

                fs::create_directories(item_parent);

                LOG_T << "Restore removed file: " << item_path;

                // Create item and copy content into it
                boost::shared_ptr<std::istream> isp = asset.istream();
                if (!isp)
                {
                    LOG_F << "Non readable asset: " << asset.id().string();

                    throw errors::non_readable_asset();
                }

                boost::shared_ptr<std::ostream> osp = fs::ostream(item_path);

                if (asset.id().string() != fs::copy_into(osp, isp))
                {
                    LOG_F << "Corrupted data.";

                    throw errors::asset_data_is_corrupted();
                }
            }
            break;
            case piel::lib::IndexesDiff::ElementState_added:
            {
                LOG_T << "Remove added file: " << item_path;

                fs::remove(item_path);
                if (item_path.parent_path() != working_copy()->working_dir())
                {
                    // It will be succeesible if directory now empty
                    try {
                        fs::remove(item_path.parent_path());
                    } catch (const std::runtime_error& e) {}
                }
            }
            break;
            case piel::lib::IndexesDiff::ElementState_modified:
                // Keep modifications

                LOG_T << "Keep modifications in file: " << item_path;

            break;
            }
        }
    }

    return working_copy()->reference_index().self().id().string();
}

} } // namespace piel::cmd
