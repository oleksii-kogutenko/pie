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

#define BOOST_TEST_MODULE GavcQueryParsing
#include <boost/test/unit_test.hpp>

#include <logging.h>
#include <artdeployartifacthandlers.h>
#include <artcreatedirectoryhandlers.h>

using namespace art::lib;

const std::string server_url = "some.server.com";
const std::string server_repository = "some-repo";
const std::string create_directory_server_path = "test_dir";
/*--------------*/
const std::string deploy1 = "test_dir/dir1/1/dir1-1-file.txt";
const std::string deploy2 = "test_dir/dir1/1/dir1-1.pom";

const std::string deploy1_fname = "file.txt";
const std::string deploy2_fname = "file.pom";

const std::string deploy1_md5 = "1181c1834012245d785120e3505ed169";
const std::string deploy2_md5 = "02153619a43e56474b66d11840004880";

const std::string deploy1_sha1 = "63bea2e3b0c7cd2d1f98bc5b7a9951eafcfead0f";
const std::string deploy2_sha1 = "2f94a61f12ba15821860034f03b49e7e1e3d416d";
/*--------------*/
const std::string path_1 = "test_dir";

const std::string group_1 = "dir2";
const std::string version_1 = "4";

const std::string classifier_1_1 = "file.txt";
const std::string classifier_1_1_fn = "file.txt";
const std::string classifier_1_2 = ".pom";
const std::string classifier_1_2_fn = ".pom";
/*--------------*/
const std::string server_api_access_token_="token";

/*
BOOST_AUTO_TEST_CASE(art_test_deploy_artifact_1)
{

    LOGT << "art_test_deploy_artifact test" << ELOG;

    art::lib::ArtBaseApiDeployArtifactHandlers deploy_handlers(server_api_access_token_);
    deploy_handlers.set_url(server_url);
    deploy_handlers.set_repo(server_repository);
    deploy_handlers.set_path(path_1);
    deploy_handlers.set_name(group_1);
    deploy_handlers.set_version(version_1);
    deploy_handlers.set_classifier(classifier_1_1);
    //deploy_handlers.file(classifier_1_1_fn);
    deploy_handlers.generate_pom();

    return;
    //deploy_handlers.md5(deploy1_md5);
    //deploy_handlers.sha1(deploy1_sha1);

    LOGT << "--" << __LINE__ << "--" << ELOG;

    piel::lib::CurlEasyClient<art::lib::ArtBaseApiDeployArtifactHandlers> art1_client(deploy_handlers.gen_uri(), &deploy_handlers);
    LOGT << "--" << __LINE__ << "--" << ELOG;

    std::cout << "art1 here: " << deploy_handlers.gen_uri() << std::endl;

    if (!art1_client.perform())
    {
        LOGE << "Error on downloading file attempt!"        << ELOG;
        LOGE << art1_client.curl_error().presentation() << ELOG;
    }
    LOGT << "--" << __LINE__ << "--" << ELOG;

    art::lib::ArtBaseApiDeployArtifactHandlers deploy_handlers2(server_api_access_token_);
    deploy_handlers2.set_url(server_url);
    deploy_handlers2.set_repo(server_repository);
    deploy_handlers2.set_path(path_1);

    deploy_handlers2.set_name(group_1);
    deploy_handlers2.set_version(version_1);
    deploy_handlers2.set_classifier(classifier_1_2);

    deploy_handlers2.file(classifier_1_2_fn);
    //deploy_handlers2.md5(deploy2_md5);
    //deploy_handlers2.sha1(deploy2_sha1);
    piel::lib::CurlEasyClient<art::lib::ArtBaseApiDeployArtifactHandlers> art2_client(deploy_handlers2.gen_uri(), &deploy_handlers2);
    LOGT << "--" << __LINE__ << "--" << ELOG;

    std::cout << "art2 here: " << deploy_handlers2.gen_uri() << std::endl;

    if (!art2_client.perform())
    {
        LOGE << "Error on downloading file attempt!"        << ELOG;
        LOGE << art2_client.curl_error().presentation() << ELOG;
    }

}
*/

/*
BOOST_AUTO_TEST_CASE(art_test_deploy_artifact)
{

    LOGT << "art_test_deploy_artifact test" << ELOG;

    art::lib::ArtBaseApiDeployArtifactHandlers deploy_handlers(server_api_access_token_);
    deploy_handlers.set_url(server_url);
    deploy_handlers.set_repo(server_repository);
    deploy_handlers.set_path(deploy1);
    deploy_handlers.md5(deploy1_md5);
    deploy_handlers.sha1(deploy1_sha1);
    deploy_handlers.file(deploy1_fname);

    LOGT << "--" << __LINE__ << "--" << ELOG;

    piel::lib::CurlEasyClient<art::lib::ArtBaseApiDeployArtifactHandlers> art1_client(deploy_handlers.gen_uri(), &deploy_handlers);
    LOGT << "--" << __LINE__ << "--" << ELOG;

    std::cout << "art1 here: " << deploy_handlers.gen_uri() << std::endl;

    if (!art1_client.perform())
    {
        LOGE << "Error on downloading file attempt!"        << ELOG;
        LOGE << art1_client.curl_error().presentation() << ELOG;
    }
    LOGT << "--" << __LINE__ << "--" << ELOG;

    art::lib::ArtBaseApiDeployArtifactHandlers deploy_handlers2(server_api_access_token_);
    deploy_handlers2.set_url(server_url);
    deploy_handlers2.set_repo(server_repository);
    deploy_handlers2.set_path(deploy2);
    deploy_handlers2.md5(deploy2_md5);
    deploy_handlers2.sha1(deploy2_sha1);
    deploy_handlers2.file(deploy2_fname);
    piel::lib::CurlEasyClient<art::lib::ArtBaseApiDeployArtifactHandlers> art2_client(deploy_handlers2.gen_uri(), &deploy_handlers2);
    LOGT << "--" << __LINE__ << "--" << ELOG;

    std::cout << "art2 here: " << deploy_handlers2.gen_uri() << std::endl;

    if (!art2_client.perform())
    {
        LOGE << "Error on downloading file attempt!"        << ELOG;
        LOGE << art2_client.curl_error().presentation() << ELOG;
    }

}
*/


BOOST_AUTO_TEST_CASE(art_test)
{
    LOGT << "MkDir test" << ELOG;

    art::lib::ArtCreateDirectoryHandlers mkdir_handlers(server_api_access_token_);
    mkdir_handlers.set_url("url");
    mkdir_handlers.set_repo("repo");
    mkdir_handlers.set_path("test_dir/dir3");

    piel::lib::CurlEasyClient<art::lib::ArtCreateDirectoryHandlers> mkdir_client(mkdir_handlers.gen_uri(), &mkdir_handlers);

    std::cout << "MkDir here: " << mkdir_handlers.gen_uri() << std::endl;

    if (!mkdir_client.perform())
    {
        LOGE << "Error on downloading file attempt!"        << ELOG;
        LOGE << mkdir_client.curl_error().presentation() << ELOG;
    }
}

