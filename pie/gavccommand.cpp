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

#include <iostream>
#include <cstdlib>
#include <gavccommand.h>
#include <artbasedownloadhandlers.h>
#include <artgavchandlers.h>
#include <logging.h>
#include <mavenmetadata.h>

#include <boost/bind.hpp>
#include <boost_property_tree_ext.hpp>

namespace pt = boost::property_tree;
namespace po = boost::program_options;

GavcCommand::GavcCommand(Application *app, int argc, char **argv)
    : ICommand(app)
    , argc_(argc)
    , argv_(argv)
    , server_url_()
    , server_api_access_token_()
    , server_repository_()
    , query_()
    , have_to_download_results_(false)
{
}

GavcCommand::~GavcCommand()
{
}

bool GavcCommand::get_from_env(po::variables_map& vm,
                               const std::string& opt_name,
                               const std::string& env_var,
                               std::string& var)
{
    if (!vm.count(opt_name)) {
        const char *value = ::getenv(env_var.c_str());
        if (value)
        {
            LOG_T << "Got " << env_var << " environment variable. Value: " << value << ".";
            var = std::string(value);
            return true;
        }
        else
        {
            return false;
        }
    }
    return true;
}

void GavcCommand::show_command_help_message(const po::options_description& desc)
{
    std::cerr << "Usage: gavc <gavc query> [options]" << std::endl;
    std::cout << desc;
}

bool GavcCommand::parse_arguments()
{
    po::options_description desc("Query options");
    desc.add_options()
        ("token,t",         po::value<std::string>(&server_api_access_token_),  "Token to access server remote api (required). Can be set using GAVC_SERVER_API_ACCESS_TOKEN environment variable.")
        ("server,s",        po::value<std::string>(&server_url_),               "Server url (required). Can be set using GAVC_SERVER_URL environment variable.")
        ("repository,r",    po::value<std::string>(&server_repository_),        "Server repository (required). Can be set using GAVC_SERVER_REPOSITORY environment variable.")
        ("download,d",                                                          "Download query results.")
        ;

    if (show_help(desc, argc_, argv_)) {
        return false;
    }

    // second argument is query
    if (argc_ < 2) {
        show_command_help_message(desc);
        return false;
    }

    std::string query_str(argv_[1]);

    // Parce query
    LOG_T << "query to perform: " << query_str;

    boost::optional<art::lib::GavcQuery> parsed_query = art::lib::GavcQuery::parse(query_str);
    if (!parsed_query)
    {
        std::cout << "Wrong gavc query: " << query_str << "!" << std::endl;
        show_command_help_message(desc);
        return false;
    }

    query_ = *parsed_query;

    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(argc_, argv_).options(desc).allow_unregistered().run();
    po::store(parsed, vm);
    try {
        po::notify(vm);
    } catch (...) {
        show_command_help_message(desc);
        return false;
    }

    bool get_env_flag = true;
    get_env_flag &= get_from_env(vm, "token",       "GAVC_SERVER_API_ACCESS_TOKEN", server_api_access_token_);
    get_env_flag &= get_from_env(vm, "server",      "GAVC_SERVER_URL",              server_url_);
    get_env_flag &= get_from_env(vm, "repository",  "GAVC_SERVER_REPOSITORY",       server_repository_);

    if (!get_env_flag) {
        show_command_help_message(desc);
        return false;
    }

    have_to_download_results_ = vm.count("download");

    return true;
}

std::string GavcCommand::create_url(const std::string& version_to_query) const
{
    std::string url = server_url_;
    url.append("/api/search/gavc");
    url.append("?r=").append(server_repository_);
    url.append("&g=").append(query_.group());
    url.append("&a=").append(query_.name());
    if (!version_to_query.empty()) {
        url.append("&v=").append(version_to_query);
    }
    if (!query_.classifier().empty()) {
        url.append("&c=").append(query_.classifier());
    }
    return url;
}

struct BeforeOutputCallback: public art::lib::ArtBaseApiHandlers::IBeforeCallback
{
    virtual void callback(art::lib::ArtBaseApiHandlers *handlers)
    {
        std::string output_filename = handlers->headers()["X-Artifactory-Filename"];
        LOG_T << "Artifactory filename: " << output_filename;

        _dest = boost::shared_ptr<std::ofstream>(new std::ofstream(output_filename.c_str()));

        dynamic_cast<art::lib::ArtBaseDownloadHandlers*>(handlers)->set_destination(_dest.get());
    }
private:
    boost::shared_ptr<std::ofstream> _dest;
};

void GavcCommand::on_object(pt::ptree::value_type obj)
{
    boost::optional<std::string> op = pt::find_value(obj.second, pt::FindPropertyHelper("downloadUri"));
    if (!op)
    {
        LOG_F << "Can't find downloadUri property!";
        return;
    }

    std::string download_uri = *op;
    LOG_T << "download_uri: " << download_uri;

    if (have_to_download_results_) {

        art::lib::ArtBaseDownloadHandlers download_handlers(server_api_access_token_);

        BeforeOutputCallback before_output;
        download_handlers.set_before_output_callback(&before_output);

        piel::lib::CurlEasyClient<art::lib::ArtBaseDownloadHandlers> download_client(download_uri, &download_handlers);

        std::cout << "Downloading file from: " << download_uri << std::endl;

        if (!download_client.perform())
        {
            LOG_E << "Error on downloading file attempt!";
            LOG_E << download_client.curl_error().presentation();
        }

    } else {

        std::cout << "Download url: " << download_uri << std::endl;

    }
}

/*virtual*/ int GavcCommand::perform()
{
    int result = -1;

    if (!parse_arguments()) {
        return result;
    }

    // Get maven metadata
    art::lib::ArtGavcHandlers download_metadata_handlers(server_api_access_token_);
    piel::lib::CurlEasyClient<art::lib::ArtGavcHandlers> get_metadata_client(
        query_.format_maven_metadata_url(server_url_, server_repository_), &download_metadata_handlers);

    if (!get_metadata_client.perform())
    {
        LOG_E << "Error on requesting maven metadata.";
        LOG_E << get_metadata_client.curl_error().presentation();
        return result;
    }

    // Try to parse server response.
    boost::optional<art::lib::MavenMetadata> metadata_op = boost::none;
    try
    {
        metadata_op = art::lib::MavenMetadata::parse(download_metadata_handlers.responce_stream());
    }
    catch (...)
    {
        LOG_E << "Error on parsing maven metadata. Server response has non expected format.";
    }

    if (!metadata_op) {
        LOG_E << "Can't retrieve maven metadata!";
        return result;
    }

    art::lib::MavenMetadata metadata = *metadata_op;

    std::vector<std::string> versions_to_process = metadata.versions_for(query_);

    for (std::vector<std::string>::const_iterator i = versions_to_process.begin(), end = versions_to_process.end(); i != end; ++i)
    {
        LOG_T << "Process version: " << *i;

        art::lib::ArtGavcHandlers api_handlers(server_api_access_token_);
        piel::lib::CurlEasyClient<art::lib::ArtGavcHandlers> client(create_url(*i), &api_handlers);

        if (!client.perform())
        {
            LOG_E << "Error on processing version: " << *i << "!";
            LOG_E << client.curl_error().presentation();
            return result;
        }

        // Create a root
        pt::ptree root;

        // Load the json file into this ptree
        pt::read_json(api_handlers.responce_stream(), root);
        pt::each(root.get_child("results"), boost::bind(&GavcCommand::on_object, this, _1));
    }

    result = 0;

    return result;
}
