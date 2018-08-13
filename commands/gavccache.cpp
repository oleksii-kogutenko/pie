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

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <gavc.h>
#include <gavccache.h>
#include <gavcconstants.h>
#include <gavcversionsfilter.h>
#include <gavcversionscomparator.h>
#include <artbaseconstants.h>
#include <artbasedownloadhandlers.h>
#include <artgavchandlers.h>
#include <logging.h>
#include <mavenmetadata.h>
#include <artbaseapihandlers.h>

#include <boost/bind.hpp>
#include <boost_property_tree_ext.hpp>
#include <boost/algorithm/string.hpp>

namespace al = art::lib;
namespace pl = piel::lib;
namespace fs = boost::filesystem;
namespace pt = boost::property_tree;
namespace po = boost::program_options;

namespace piel { namespace cmd {

GAVCCache::GAVCCache(const std::string& server_api_access_token
           , const std::string& server_url
           , const std::string& server_repository
           , const art::lib::GavcQuery& query
           , const bool have_to_download_results
           , const std::string &cache_path
           , const std::string& output_file)
    : pl::IOstreamsHolder()
    , server_url_(server_url)
    , server_api_access_token_(server_api_access_token)
    , server_repository_(server_repository)
    , query_(query)
    , path_to_download_()
    , have_to_download_results_(have_to_download_results)
    , list_of_actual_files_()
    , query_results_()
    , output_file_(output_file)
    , cache_path_(cache_path)
{
}

GAVCCache::~GAVCCache()
{
}

std::vector<std::string> GAVCCache::get_cached_versions(const std::string& path) const
{
    std::vector<std::string> result;
    if(!fs::is_directory(path)) {
        throw piel::cmd::errors::cache_folder_does_not_exist(path);
    }
    for(auto entry : boost::make_iterator_range(fs::directory_iterator(path), {})){
        LOGT << entry.path().filename().c_str() << ELOG;
        if(fs::is_directory(entry)) {
            result.push_back(entry.path().filename().c_str());
        }
    }

    boost::optional<std::vector<al::gavc::OpType> > ops_val = query_.query_version_ops();

    if (!ops_val) {
        LOGT << "Unable to get query operations list." << ELOG;
        return result;
    }

    std::vector<al::gavc::OpType> ops = *ops_val;

    al::GavcVersionsComparator comparator(ops);

    std::sort(result.begin(), result.end(), comparator);

    // Filter out versions what are not corresponding to query
    al::GavcVersionsFilter filter(ops);

    return filter.filtered(result);
}

GAVC::paths_list GAVCCache::get_cached_files_list(const std::vector<std::string>& versions_to_process, const std::string& path, bool do_print)
{
    std::string classifier_spec = query_.classifier();
    std::string query_name      = query_.name();

    std::vector<std::string> classifiers;

    boost::split(classifiers, classifier_spec, boost::is_any_of(","));

    for (auto ver = versions_to_process.begin(), end = versions_to_process.end(); ver != end; ++ver)
    {
        LOGT << "Version: " << *ver << ELOG;

        for (auto c = classifiers.begin(), cend = classifiers.end(); c != cend; ++c)
        {
            if (c->empty()) {
                LOGT << "empty classifier " << ELOG;
                continue;
            }

            std::string classifier_file_name = GAVC::get_classifier_file_name(query_name, *ver, *c);

            std::string file_path = path + fs::path::preferred_separator
                    + *ver + fs::path::preferred_separator +
                    classifier_file_name;

            LOGT << "Classifier: " << *c << " path: " << file_path << ELOG;


            if (!fs::is_regular_file(file_path)) {
                LOGT << "no file " << file_path << ELOG;
                continue;
            }

            std::map<std::string, std::string> checksum = GAVC::load_checksum(file_path);
            bool is_actual = GAVC::validate_local_file(file_path, checksum);

            if (!is_actual) {
                LOGT << "file " << file_path << " is not actual!!!" << ELOG;
                continue;
            }

            if (do_print) {
                cout() << "c " << classifier_file_name << std::endl;
            }

            if (std::find(list_of_actual_files_.begin(), list_of_actual_files_.end(), file_path) == list_of_actual_files_.end()) {
                list_of_actual_files_.push_back(file_path);
            }
        }
    }

    return list_of_actual_files_;
}

void GAVCCache::operator()()
{
    piel::cmd::GAVC gavc(
         server_api_access_token_,
         server_url_,
         server_repository_,
         query_,
         have_to_download_results_,
         "");

    gavc.set_cache_mode(true);

    std::string mm_path = cache_path_ + gavc.get_maven_metadata_path();

    LOGT << " gavc.get_maven_metadata_path:"    << gavc.get_maven_metadata_path()   << ELOG;
    LOGT << " cache_path_:"                     << cache_path_                      << ELOG;
    LOGT << " mm_path:"                         << mm_path                          << ELOG;

    std::vector<std::string> versions_to_process_remote;
    std::vector<std::string> versions_to_process_cache;

    bool use_cache      = false;
    bool empty_cache    = false;

    try {
        versions_to_process_remote  = gavc.get_versions_to_process();
    }
    catch (piel::cmd::errors::no_server_maven_metadata& e) {
        // No server metadata. Force use cache.
        use_cache = true;
    }

    try {
        versions_to_process_cache   = get_cached_versions(mm_path);
        empty_cache                 = versions_to_process_cache.empty();
    } catch (errors::cache_folder_does_not_exist& e) {
        // Don't see anything in cache.
        empty_cache = true;
    }

    if (use_cache && empty_cache) {
        throw errors::cache_no_queued_version(query_.to_string());
    }

    GAVC::paths_list list_files;

    list_files = get_cached_files_list(versions_to_process_cache, mm_path, false);

    use_cache |= (versions_to_process_remote == versions_to_process_cache) && (!empty_cache && !list_files.empty());

    LOGT << " use_cache: " << use_cache << ELOG;

    if (use_cache) {

        for (auto ver = versions_to_process_cache.begin(), end = versions_to_process_cache.end(); ver != end; ++ver) {
            cout() << "Version: " << *ver << std::endl;
        }

        if (have_to_download_results_) {
            list_files = get_cached_files_list(versions_to_process_cache, mm_path, use_cache);

            for (GAVC::paths_list::iterator f = list_files.begin(), end = list_files.end(); f != end; ++f) {

                fs::path path        = output_file_.empty()         ?   fs::path(*f).filename() : output_file_                          ;
                fs::path object_path = path_to_download_.empty()    ?   path                    : path_to_download_ / path.filename()   ;

                LOGT << *f << "->" << object_path << ELOG;

                cout() << "+ " << object_path.filename().string() << std::endl;

                fs::copy_file(*f, object_path, fs::copy_option::overwrite_if_exists);
            }
        }

    } else {

        // Download files to cache
        for (auto i = versions_to_process_remote.begin(), end = versions_to_process_remote.end(); i != end; ++i) {
            LOGT << "Version: " << *i << ELOG;

            std::string path = mm_path + "/" + *i;

            LOGT << "path:"     << path << ELOG;
            boost::filesystem::create_directories(path);

            gavc.set_path_to_download(path);
            gavc.process_version(*i);
        }

        if (have_to_download_results_) {
            list_files = gavc.get_list_of_actual_files();

            for (GAVC::paths_list::iterator f = list_files.begin(), end = list_files.end(); f != end; ++f) {

                fs::path path        = output_file_.empty()         ?   fs::path(*f).filename() : output_file_                          ;
                fs::path object_path = path_to_download_.empty()    ?   path                    : path_to_download_ / path.filename()   ;

                LOGT << *f << "->" << object_path << ELOG;

                cout() << "+ " << object_path.filename().string() << std::endl;

                fs::copy_file(*f, object_path, fs::copy_option::overwrite_if_exists);
            }
        }
    }
}

void GAVCCache::set_path_to_download(const boost::filesystem::path& path)
{
    path_to_download_ = path;
}

boost::filesystem::path GAVCCache::get_path_to_download() const
{
    return path_to_download_;
}

} } // namespace piel::cmd
