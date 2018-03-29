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

#include <push.h>
#include <logging.h>
#include <fsindexer.h>
#include <artbaseconstants.h>
#include <treeindexenumerator.h>
#include <mavenpom.h>
#include <artdeployartifacthandlers.h>
#include <artdeployartifactchecksumhandlers.h>
#include <boost_filesystem_ext.hpp>

namespace al = art::lib;
namespace pl = piel::lib;
namespace fs = boost::filesystem;

namespace piel { namespace cmd {

namespace constants {

    static const std::string zip_extention = ".zip";
    static const std::string pom_extention = ".pom";

};


Push::Push(const piel::lib::WorkingCopy::Ptr& working_copy)
    : WorkingCopyCommand(working_copy)
    , server_url_()
    , server_api_access_token_()
    , server_repository_()
    , query_()
    , zip_list_()
{
}

Push::~Push()
{
}

const Push* Push::set_server_url(const std::string& url)
{
    server_url_ = url;
    return this;
}

const Push* Push::set_server_api_access_token(const std::string& token)
{
    server_api_access_token_ = token;
    return this;
}

const Push* Push::set_server_repository(const std::string& repo)
{
    server_repository_ = repo;
    return this;
}

const Push* Push::set_query(const art::lib::GavcQuery &query)
{
    query_ = query;
    return this;
}

void Push::deploy_pom(const boost::filesystem::path &path_to_save_pom)
{
    pl::MavenPom pom;
    pom.set_group(query_.group());
    pom.set_name(query_.name());
    pom.set_version(query_.version());

    std::ostringstream os;
    pom.store(os);

    boost::filesystem::path pom_path = path_to_save_pom / constants::pom_extention;
    {
        std::ofstream osf(pom_path.generic_string());
        pom.store(osf);
    }

    art::lib::ArtDeployArtifactHandlers deploy_handlers(server_api_access_token_);

    deploy_handlers.set_url(server_url_);
    deploy_handlers.set_repo(server_repository_);
    deploy_handlers.set_path(query_.group_path());
    deploy_handlers.set_name(query_.name());
    deploy_handlers.set_version(query_.version());
    deploy_handlers.set_classifier(art::lib::ArtBaseConstants::pom_classifier);
    deploy_handlers.push_input_stream(boost::shared_ptr<std::istream>(new std::istringstream(os.str())));

    piel::lib::CurlEasyClient<art::lib::ArtDeployArtifactHandlers> upload_client(deploy_handlers.gen_uri(), &deploy_handlers);

    LOGD << "Upload pom to: " << deploy_handlers.gen_uri() << ELOG;

    if (!upload_client.perform())
    {
        LOGE << "Error on upload pom!"                      << ELOG;
        LOGE << upload_client.curl_error().presentation()   << ELOG;
        throw errors::uploading_pom_error(upload_client.curl_error().presentation());
    }

    Upload::upload_checksums_for(&deploy_handlers, art::lib::ArtBaseConstants::checksums_md5);
    Upload::upload_checksums_for(&deploy_handlers, art::lib::ArtBaseConstants::checksums_sha1);
    Upload::upload_checksums_for(&deploy_handlers, art::lib::ArtBaseConstants::checksums_sha256);
}

bool Push::upload(const std::string& classifier, const std::string& file_name)
{
    bool no_errors = true;

    LOGD << query_.to_string() << ELOG;
    {
        art::lib::ArtDeployArtifactHandlers deploy_handlers(server_api_access_token_);

        deploy_handlers.set_url(server_url_);
        deploy_handlers.set_repo(server_repository_);
        deploy_handlers.set_path(query_.group_path());
        deploy_handlers.set_name(query_.name());
        deploy_handlers.set_version(query_.version());
        deploy_handlers.set_classifier(classifier);
        deploy_handlers.file(file_name);

        piel::lib::CurlEasyClient<art::lib::ArtDeployArtifactHandlers> upload_client(deploy_handlers.gen_uri(), &deploy_handlers);

        LOGD << "Upload: " << file_name << " as " << classifier << " to: " << deploy_handlers.gen_uri() << ELOG;

        if (!(no_errors &= upload_client.perform()))
        {
            LOGE << "Error on upload file!"                     << ELOG;
            LOGE << upload_client.curl_error().presentation()   << ELOG;
            throw errors::uploading_classifier_error(upload_client.curl_error().presentation());
        }

        Upload::upload_checksums_for(&deploy_handlers, art::lib::ArtBaseConstants::checksums_md5);
        Upload::upload_checksums_for(&deploy_handlers, art::lib::ArtBaseConstants::checksums_sha1);
        Upload::upload_checksums_for(&deploy_handlers, art::lib::ArtBaseConstants::checksums_sha256);
    }
    return no_errors;
}

void Push::operator()()
{
    bool no_errors = true;
    std::set<piel::lib::refs::Ref> all_refs = working_copy()->local_storage()->references();

    zip_list_.clear();

    if (!all_refs.size()) {
        LOGW << "No references found!" << ELOG;
        throw errors::nothing_to_push();
    }

    boost::filesystem::path version_dir = working_copy()->archives_dir() / query_.version();
    fs::create_directories(version_dir);

    for(std::set<piel::lib::refs::Ref>::const_iterator i = all_refs.begin(), end = all_refs.end(); i != end; ++i)
    {
        std::string log_str;
        if (working_copy()->current_tree_name() == i->first)
            log_str  = "*" + i->first;
        else
            log_str  = " " + i->first;

        LOGD << log_str << ":" << i->second.string() << ELOG;

        piel::lib::TreeIndex::Ptr reference_index = piel::lib::TreeIndex::from_ref(working_copy()->local_storage(), i->first);
        piel::lib::TreeIndexEnumerator enumerator(reference_index);

        LOGD << "reference_index->self().id().string():" << reference_index->self().id().string() << ELOG;

        boost::filesystem::path zip_path_fs = version_dir / (i->first + constants::zip_extention);

        std::string zip_path = zip_path_fs.generic_string();
        zip_list_.push_back(zip_path);
        {
            lib::ZipFile::FilePtr zip = lib::ZipFile::create(zip_path);

            while (enumerator.next())
            {
                LOGD << "\t" << enumerator.path << ":"
                        << enumerator.asset.id().string() << ELOG;

                std::string asset_mode_str = reference_index->get_attr_(enumerator.path, pl::PredefinedAttributes::asset_mode);

                int asset_mode = pl::PredefinedAttributes::parse_asset_mode(asset_mode_str,
                        pl::PredefinedAttributes::default_asset_mode);

                zip->add_istream(enumerator.path, enumerator.asset.istream(), asset_mode);
            }
        }
        no_errors &= upload(i->first + constants::zip_extention, zip_path);
    }

    if (no_errors)
    {
        deploy_pom(version_dir);
    }
}

} } // namespace piel::cmd
