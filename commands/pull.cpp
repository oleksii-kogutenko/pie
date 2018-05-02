/*
 * Copyright (c) 2017-2018
 *
 *  Dmytro Iakovliev daemondzk@gmail.com
 *  Oleksii Kogutenko https://github.com/oleksii-kogutenko
 *
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

#include <pull.h>
#include <gavc.h>

#include <zipindexer.h>

#include <logging.h>
#include <fsindexer.h>
#include <treeindexenumerator.h>
#include <assetsextractor.h>

#include <boost_filesystem_ext.hpp>

namespace al = art::lib;
namespace pl = piel::lib;
namespace fs = boost::filesystem;

namespace piel { namespace cmd {

namespace constants {

    static const std::string pom_extention = ".pom";
    static const char classifier_delimiter = '-';

};

Pull::Pull(const std::string& server_api_access_token
     , const std::string& server_url
     , const std::string& server_repository
     , const art::lib::GavcQuery& query)
    : piel::lib::IOstreamsHolder()
    , working_copy_()
    , server_url_(server_url)
    , server_api_access_token_(server_api_access_token)
    , server_repository_(server_repository)
    , query_(query)
    , path_to_download_()
{
}

Pull::~Pull()
{
}

std::vector<std::string> Pull::split(const std::string &s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, delim)) {
        LOGT << "split item: " << item << ELOG;
        result.push_back(item);
    }
    return result;
}

std::string Pull::get_classifier_from_filename(const fs::path& fn)
{
    std::string classifier;
    std::vector<std::string> res = split(fn.stem().generic_string(), constants::classifier_delimiter);

    if (res.size() != 3) {
        throw errors::invalid_downloaded_artifact_name(fn.stem().generic_string());
    }

    classifier = res[2];
    if (fn.has_extension()) {
        classifier = classifier.substr(0, classifier.find(fn.extension().generic_string(), 0));
    }

    return classifier;
}

void Pull::operator()()
{
    fs::path wc_path = (path_to_download_.empty()) ? fs::current_path() : path_to_download_;

    std::string ref;

    working_copy_ = piel::lib::WorkingCopy::init(wc_path, ref);

    if (!working_copy_->is_valid())
    {
        throw errors::invalid_working_copy();
    }

    fs::path archives_path = working_copy_->archives_dir().generic_string();

    GAVC gavc(server_api_access_token_,
              server_url_,
              server_repository_,
              query_,
              true);

    gavc.set_path_to_download(archives_path);
    gavc();

    GAVC::paths_list list = gavc.get_list_of_downloaded_files();

    LOGD << "List of download file..." << ELOG;

    for(GAVC::paths_list::const_iterator it = list.begin(), end = list.end(); it != end; ++it)
    {
        if (it->extension() == constants::pom_extention) {
            LOGD << "Skip POM file." << ELOG;
            continue;
        }

        std::string classifier = get_classifier_from_filename(*it);
        LOGD << "classifier:" << classifier << ELOG;

        cout() << "Import tree: " << classifier;

        pl::TreeIndex::Ptr zip_index = pl::ZipIndexer::build(*it);
        working_copy_->local_storage()->put(zip_index->assets());

        piel::lib::AssetId new_tree_id = zip_index->self().id();
        working_copy_->local_storage()->create_reference(piel::lib::refs::Ref(classifier, new_tree_id));
        working_copy_->setup_current_tree(classifier, zip_index);

        cout() << " COMPLETE" << std::endl;
    }

    pl::TreeIndex::Ptr current_tree = piel::lib::TreeIndex::from_ref(working_copy_->local_storage(), classifier_to_checkout_);
    if (!classifier_to_checkout_.empty() && !current_tree)
    {
        cout() << "Can't find requested tree to checkout! Latest imported will be used." << std::endl;
    }

    if (classifier_to_checkout_.empty() || !current_tree)
    {
        classifier_to_checkout_ = working_copy_->current_tree_name();
        current_tree            = working_copy_->current_tree_state();
    }
    else
    {
        working_copy_->setup_current_tree(classifier_to_checkout_, current_tree);
    }

    cout() << "Checkout " << classifier_to_checkout_;

    pl::AssetsExtractor index_exporter(current_tree, pl::ExtractPolicy__replace_existing);
    index_exporter.extract_into(working_copy_->working_dir());

    cout() << " COMPLETE" << std::endl;
}

} } // namespace piel::cmd
