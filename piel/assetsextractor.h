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

#ifndef PIEL_ASSETSEXTRACTOR_H_
#define PIEL_ASSETSEXTRACTOR_H_

#include <boost_filesystem_ext.hpp>
#include <treeindex.h>

namespace piel { namespace lib {

namespace errors {
    struct attempt_to_export_to_non_existing_directory {};
    struct attempt_to_export_non_readable_asset {};
    struct unable_to_create_item_parent {};
    struct exported_data_is_corrupted {};
    struct unknown_asset_type {};
};

enum ExtractPolitic
{
    ExtractPolicy__keep_existing         = 0x01,
    ExtractPolicy__replace_existing      = 0x02,
    ExtractPolicy__backup_existing       = 0x04,
    ExtractPolicy__put_new_with_suffix   = 0x08,
};

class AssetsExtractor
{
public:
    AssetsExtractor(const TreeIndex::Ptr& index, ExtractPolitic politic = ExtractPolicy__replace_existing);
    ~AssetsExtractor();

    void extract_into(const boost::filesystem::path& directory, const std::string& prefix_only = std::string());
    void extract_asset_into(const boost::filesystem::path& item_path,
            const TreeIndex::Content::const_iterator& i);

private:
    void create_parent_path(const boost::filesystem::path& item_path);

private:
    TreeIndex::Ptr          index_;
    ExtractPolitic          politic_;

};

} } // namespace piel

#endif /* PIEL_ASSETSEXTRACTOR_H_ */
