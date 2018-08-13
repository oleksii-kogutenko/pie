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

#define BOOST_TEST_MODULE GavcQueryParsing
#include <boost/test/unit_test.hpp>

#include <gavccache.h>
#include <cstdlib>
#include "test_utils.hpp"
#include "logging.h"

namespace al=art::lib;
namespace cmd=piel::cmd;
namespace lib=piel::lib;
namespace fs=boost::filesystem;
namespace tst=lib::test_utils;

std::string server_url        = "http://server/art";
std::string server_repository = "test-repo-local";
std::string server_token      = "";
std::string gavc_cache        = "";
std::string query_str         = " valhalla.dev.charter.humaxwb20.third_parties.linux.arm:expat-2.1.1:44:package";

std::string getEnv(const std::string& env)
{
    std::string ret;
    const char * val = ::getenv(env.c_str());
    if (val) {
        ret = val;
    }
    return ret;
}

void init()
{
    server_url        = getEnv("GAVC_SERVER_URL");
    server_repository = getEnv("GAVC_SERVER_REPOSITORY");
    server_token      = getEnv("GAVC_SERVER_API_ACCESS_TOKEN");

    LOGT << "server_url:" << server_url << ELOG;
    LOGT << "server_repository:" << server_repository << ELOG;
    //gavc_cache        = ::getenv("GAVC_CACHE");
}

BOOST_AUTO_TEST_CASE(Check_classifier_without_cache)
{
    init();

    boost::optional<al::GavcQuery> op = al::GavcQuery::parse(query_str);
    BOOST_CHECK(op);
    al::GavcQuery q = *op;

    LOGT << "---" << q.to_string() << ELOG;

    piel::cmd::GAVCCache gavccache(server_token, server_url, server_repository, q, false, "", "");

    try {
        gavccache();
    }
    catch (...) {
        BOOST_CHECK(false);
    }
}

BOOST_AUTO_TEST_CASE(Check_classifier_without_cache_wrong)
{
    init();

    boost::optional<al::GavcQuery> op = al::GavcQuery::parse(query_str + "AAA");
    BOOST_CHECK(op);
    al::GavcQuery q = *op;

    LOGT << "---" << q.to_string() << ELOG;

    piel::cmd::GAVCCache gavccache(server_token, server_url, server_repository, q, false, "", "");

    try {
        gavccache();
    }
    catch (...) {
        BOOST_CHECK(true);
    }
}

BOOST_AUTO_TEST_CASE(Check_classifier_with_cache)
{
    init();
    tst::TempFileHolder::Ptr tmp_dir = tst::create_temp_dir(0);

    boost::optional<al::GavcQuery> op = al::GavcQuery::parse(query_str);
    BOOST_CHECK(op);
    al::GavcQuery q = *op;

    LOGT << "---" << q.to_string() << ELOG;

    piel::cmd::GAVCCache gavccache(server_token, server_url, server_repository, q, false, tmp_dir->first.string(), "");

    try {
        gavccache();
    }
    catch (...) {
        BOOST_CHECK(false);
    }
}

BOOST_AUTO_TEST_CASE(Check_classifier_with_cache_download)
{
    boost::filesystem::path exclude = "";
    init();
    tst::TempFileHolder::Ptr cache_dir = tst::create_temp_dir(0);

    boost::optional<al::GavcQuery> op = al::GavcQuery::parse(query_str);
    BOOST_CHECK(op);
    al::GavcQuery q = *op;

    LOGT << "---" << q.to_string() << ELOG;

    piel::cmd::GAVCCache gavccache(server_token, server_url, server_repository, q, true, cache_dir->first.string(), "");

    try {
        gavccache();
    }
    catch (...) {
        BOOST_CHECK(false);
    }

    tst::DirState cache_dir_state = tst::get_directory_state(cache_dir->first, exclude);

    for (tst::DirState::const_iterator it = cache_dir_state.begin(), end = cache_dir_state.end(); it != end; ++it) {
        boost::filesystem::path file_path = cache_dir->first / it->first;

        LOGT << "file " << it->first << " added as " << file_path.string() << ELOG;
    }

    boost::filesystem::path file_path =
            cache_dir->first /
            q.format_maven_metadata_path(server_repository) /
            q.version() /
            (gavccache.get_classifier_file_name(q.name(), q.version(), q.classifier()) + ".tbz2");
    LOGT << "PATH: " << q.format_maven_metadata_path(server_repository) << ELOG;
    LOGT << "path: " << file_path << ELOG;
    LOGT << "is: " << fs::is_regular_file(file_path) << ELOG;

    BOOST_CHECK(fs::is_regular_file(file_path));
}

BOOST_AUTO_TEST_CASE(Check_classifier_with_cache_download_and_copy)
{
    boost::filesystem::path exclude = "";
    init();
    tst::TempFileHolder::Ptr cache_dir = tst::create_temp_dir(0);
    tst::TempFileHolder::Ptr to_dir = tst::create_temp_dir(0);

    boost::optional<al::GavcQuery> op = al::GavcQuery::parse(query_str);
    BOOST_CHECK(op);
    al::GavcQuery q = *op;

    LOGT << "---" << q.to_string() << ELOG;

    fs::path art_file = to_dir->first / "art";

    piel::cmd::GAVCCache gavccache(server_token, server_url, server_repository, q, true, cache_dir->first.string(), art_file.string());

    try {
        gavccache();
    }
    catch (...) {
        BOOST_CHECK(false);
    }

    tst::DirState cache_dir_state = tst::get_directory_state(cache_dir->first, exclude);

    for (tst::DirState::const_iterator it = cache_dir_state.begin(), end = cache_dir_state.end(); it != end; ++it) {
        boost::filesystem::path file_path = cache_dir->first / it->first;

        LOGT << "file " << it->first << " added as " << file_path.string() << ELOG;
    }

    boost::filesystem::path file_path =
            cache_dir->first /
            q.format_maven_metadata_path(server_repository) /
            q.version() /
            (gavccache.get_classifier_file_name(q.name(), q.version(), q.classifier()) + ".tbz2");
    LOGT << "PATH: " << q.format_maven_metadata_path(server_repository) << ELOG;
    LOGT << "path: " << file_path << ELOG;
    LOGT << "is: " << fs::is_regular_file(file_path) << ELOG;
    LOGT << "is_art: " << fs::is_regular_file(art_file) << ELOG;

    BOOST_CHECK(fs::is_regular_file(file_path));
    BOOST_CHECK(fs::is_regular_file(art_file));
}

BOOST_AUTO_TEST_CASE(Check_check_cache)
{
    boost::filesystem::path exclude = "";
    init();
    tst::TempFileHolder::Ptr cache_dir = tst::create_temp_dir(0);
    tst::TempFileHolder::Ptr to_dir = tst::create_temp_dir(0);

    boost::optional<al::GavcQuery> op = al::GavcQuery::parse(query_str);
    BOOST_CHECK(op);
    al::GavcQuery q = *op;

    LOGT << "---" << q.to_string() << ELOG;

    fs::path art_file = to_dir->first / "art.tbz2";

    piel::cmd::GAVCCache gavccache(server_token, server_url, server_repository, q, true, cache_dir->first.string());

    try {
        gavccache();
    }
    catch (...) {
        BOOST_CHECK(false);
    }

    piel::cmd::GAVCCache gavccache1(server_token, server_url + "AAA", server_repository, q, true, cache_dir->first.string(), art_file.string());
    try {
        gavccache1();
    }
    catch (...) {
        BOOST_CHECK(false);
    }

    tst::DirState cache_dir_state = tst::get_directory_state(cache_dir->first, exclude);

    for (tst::DirState::const_iterator it = cache_dir_state.begin(), end = cache_dir_state.end(); it != end; ++it) {
        boost::filesystem::path file_path = cache_dir->first / it->first;

        LOGT << "file " << it->first << " added as " << file_path.string() << ELOG;
    }

    boost::filesystem::path file_path =
            cache_dir->first /
            q.format_maven_metadata_path(server_repository) /
            q.version() /
            (gavccache1.get_classifier_file_name(q.name(), q.version(), q.classifier()) + ".tbz2");
    LOGT << "PATH: " << q.format_maven_metadata_path(server_repository) << ELOG;
    LOGT << "path: " << file_path << ELOG;
    LOGT << "is: " << fs::is_regular_file(file_path) << ELOG;
    LOGT << "is_art: " << fs::is_regular_file(art_file) << ELOG;

    BOOST_CHECK(fs::is_regular_file(file_path));
    BOOST_CHECK(fs::is_regular_file(art_file));
}
