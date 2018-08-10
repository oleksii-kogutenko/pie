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
    , cache_path_ (cache_path)
{
    LOGT << " " << __FUNCTION__ << ":" << __LINE__ << ELOG;
}

GAVCCache::~GAVCCache()
{
    LOGT << " " << __FUNCTION__ << ":" << __LINE__ << ELOG;
}


std::vector<std::string> GAVCCache::get_cache_versions(const std::string& path) const
{
    std::vector<std::string> result;
    if(!fs::is_directory(path)) {
        throw piel::cmd::errors::cache_folder_does_not_exist(path);
    }
    for(auto& entry : boost::make_iterator_range(fs::directory_iterator(path), {})){
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
    //return result;
}

std::string GAVCCache::get_clessifier_file_name(const std::string& query_name, const std::string& ver, const std::string& classifier)
{
    return query_name + al::GavcConstants::oldest_version + ver + al::GavcConstants::oldest_version + classifier;
}

GAVC::paths_list GAVCCache::get_cached_file_list(const std::vector<std::string>& versions_to_process, const std::string& path, bool do_print)
{
    std::string classifier_spec = query_.classifier();
    std::string query_name = query_.name();
    std::vector<std::string> classifiers;
    fs::path output_file(output_file_);
    std::string ext = query_.extension().empty() ? output_file.extension().generic_string() : query_.extension();

    boost::split(classifiers, classifier_spec, boost::is_any_of(","));

    for (std::vector<std::string>::const_iterator ver = versions_to_process.begin(), end = versions_to_process.end(); ver != end; ++ver)
    {
        if (do_print)
        {
            std::cout << "Version: " << *ver << std::endl;
        }
        LOGT << "Version: " << *ver << ELOG;
        for (std::vector<std::string>::const_iterator c = classifiers.begin(), cend = classifiers.end(); c != cend; ++c)
        {

            std::string classifier_file_name = get_clessifier_file_name(query_name, *ver, *c);
            std::string file_path = path + fs::path::preferred_separator
                    + *ver + fs::path::preferred_separator +
                    classifier_file_name + (ext.empty() ? "" : ext);
            LOGT << "Classifier: " << *c << " path: " << file_path << ELOG;

            if (fs::is_regular_file(file_path)) {
                std::map<std::string, std::string> checksum = GAVC::load_checksum(file_path);
                bool is_actual = GAVC::validate_local_file(file_path, checksum);
                if (is_actual) {
                    if (do_print) {
                        std::cout << "+ " << classifier_file_name << std::endl;
                    }
                    list_of_actual_files_.push_back(file_path);
                } else {
                    LOGW << "file " << file_path << " is not actual!!!" << ELOG;
                }
            } else {
                LOGW << "no file " << file_path << ELOG;
            }
        }
    }

    return list_of_actual_files_;
}

void GAVCCache::copy_file_list(GAVC::paths_list &file_list)
{
    //if (output_file_.empty()) return;
    for (GAVC::paths_list::iterator f = file_list.begin(), end = file_list.end(); f != end; ++f) {
        LOGT << *f << "->" << output_file_ << ELOG;
        fs::copy_file(*f, output_file_, fs::copy_option::overwrite_if_exists);
    }
}

void GAVCCache::operator()()
{
    LOGT << " " << __FUNCTION__ << ":" << __LINE__ << ELOG;

    std::string output_file;
    if (cache_path_.empty()) {
        output_file = output_file_;
    }

        piel::cmd::GAVC gavc(
             server_api_access_token_,
             server_url_,
             server_repository_,
             query_,
             have_to_download_results_,
             output_file);

        std::string mm_path = cache_path_ + gavc.get_maven_metadata_path();
        LOGT << "   " << __FUNCTION__ << ":" << __LINE__ << ELOG;
        LOGT << " gavc.get_maven_metadata_path:" << gavc.get_maven_metadata_path() << ELOG;
        LOGT << " cache_path_:" << cache_path_ << ELOG;
        LOGT << " mm_path:" << mm_path << ELOG;

        std::vector<std::string> versions_to_process;
        std::vector<std::string> versions_to_process_cache;
        bool use_cache = false;
        bool empty_cache = cache_path_.empty();

        try {
            if (!cache_path_.empty()) {
                versions_to_process_cache = get_cache_versions(mm_path);
            }
        } catch (errors::cache_folder_does_not_exist& e) {
            empty_cache = true;
        }

        try {
            versions_to_process = gavc.get_versions_to_process();
        }
        catch (piel::cmd::errors::unable_to_parse_maven_metadata& e) {
            std::cerr << "Error on parsing maven metadata. Server response has non expected format." << std::endl;
            throw e;
        }
        catch (piel::cmd::errors::error_processing_version& e) {
            std::cerr << "Error on processing version: " << e.ver << "!"    << std::endl;
            std::cerr <<  e.error << std::endl;
            throw e;
        }
        catch (piel::cmd::errors::cant_get_maven_metadata& e) {
            std::cerr << "Can't retrieve maven metadata!" << std::endl;
            throw e;
        }
        catch (piel::cmd::errors::cant_find_version_for_query& e) {
            std::cerr << "Can't find any version for query!" << std::endl;
            throw e;
        }
        catch (piel::cmd::errors::no_server_maven_metadata& e) {
            std::cerr << "Error on requesting maven metadata." << std::endl;
            std::cerr << e.error << std::endl;
            if (cache_path_.empty() || empty_cache) {
                throw e;
            }
            use_cache = true;
        }


        GAVC::paths_list list_files;
        list_files = get_cached_file_list(versions_to_process_cache, mm_path, use_cache);

        use_cache |= (versions_to_process == versions_to_process_cache) && (!empty_cache && !list_files.empty());

        LOGT << " use_cache: " << use_cache << ELOG;

        if (!use_cache) {
            try {
                for (std::vector<std::string>::const_iterator i = versions_to_process.begin(), end = versions_to_process.end(); i != end; ++i)
                {
                    //std::cout << "Version: " << *i << ELOG;

                    LOGT << "Version: " << *i << ELOG;
                    if (!cache_path_.empty()) {
                        std::string path = mm_path + "/" + *i;
                        LOGT << "path:" << path << ELOG;
                        boost::filesystem::create_directories(path);
                        gavc.set_path_to_download(path);
                    }

                    gavc.process_version(*i);
                }
            }
            catch (piel::cmd::errors::gavc_download_file_error& e) {
                std::cerr << "Can't find any version for query!" << std::endl;
                throw e;
            }
            list_files = gavc.get_list_of_actual_files();
        }

        if (!output_file_.empty() && have_to_download_results_ && !list_files.empty()) {
            copy_file_list(list_files);
        } else {
            LOGT << "Nothing to copy ..." << ELOG;
        }


    LOGT << "---" << __FUNCTION__ << ":" << __LINE__ << ELOG;
}

} } // namespace piel::cmd
