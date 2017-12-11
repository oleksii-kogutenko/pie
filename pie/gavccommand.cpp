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

#include <boost/bind.hpp>
#include <boost_property_tree_ext.hpp>

namespace pt = boost::property_tree;
namespace po = boost::program_options;

GavcCommand::GavcCommand(Application *app, int argc, char **argv)
    : ICommand(app)
    , _argc(argc)
    , _argv(argv)
    , _server_url()
    , _server_api_access_token()
    , _server_repository()
    , _query()
    , _download_results(false)
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
        ("token,t",         po::value<std::string>(&_server_api_access_token),  "Token to access server remote api (required). Can be set using GAVC_SERVER_API_ACCESS_TOKEN environment variable.")
        ("server,s",        po::value<std::string>(&_server_url),               "Server url (required). Can be set using GAVC_SERVER_URL environment variable.")
        ("repository,r",    po::value<std::string>(&_server_repository),        "Server repository (required). Can be set using GAVC_SERVER_REPOSITORY environment variable.")
        ("download,d",                                                          "Download query results.")
        ;

    if (show_help(desc, _argc, _argv)) {
        return false;
    }

    // second argument is query
    if (_argc < 2) {
        show_command_help_message(desc);
        return false;
    }

    std::string query_str(_argv[1]);

    // Parce query
    LOG_T << "query to perform: " << query_str;

    boost::optional<art::lib::GavcQuery> parsed_query = art::lib::GavcQuery::parse(query_str);
    if (!parsed_query)
    {
        std::cout << "Wrong gavc query: " << query_str << "!" << std::endl;
        show_command_help_message(desc);
        return false;
    }

    _query = *parsed_query;

    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(_argc, _argv).options(desc).allow_unregistered().run();
    po::store(parsed, vm);
    try {
        po::notify(vm);
    } catch (...) {
        show_command_help_message(desc);
        return false;
    }

    bool get_env_flag = true;
    get_env_flag &= get_from_env(vm, "token",       "GAVC_SERVER_API_ACCESS_TOKEN", _server_api_access_token);
    get_env_flag &= get_from_env(vm, "server",      "GAVC_SERVER_URL",              _server_url);
    get_env_flag &= get_from_env(vm, "repository",  "GAVC_SERVER_REPOSITORY",       _server_repository);

    if (!get_env_flag) {
        show_command_help_message(desc);
        return false;
    }

    _download_results = vm.count("download");

    return true;
}

std::string GavcCommand::create_url() const
{
    std::string url = _server_url;
    url.append("/api/search/gavc");
    url.append("?r=").append(_server_repository);
    url.append("&g=").append(_query.group());
    url.append("&a=").append(_query.name());
    if (!_query.version().empty()) {
        url.append("&v=").append(_query.version());
    }
    if (!_query.classifier().empty()) {
        url.append("&c=").append(_query.classifier());
    }
    return url;
}

struct FindPropertyHelper
{
    std::string _prop_name;

    FindPropertyHelper(const std::string& prop_name)
        : _prop_name(prop_name)
    {
    }

    bool operator()(pt::ptree::value_type prop)
    {
        return prop.first == _prop_name;
    }
};

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
    boost::optional<std::string> op = pt::find_value(obj.second, FindPropertyHelper("downloadUri"));
    if (!op)
    {
        LOG_F << "Can't find downloadUri property!";
        return;
    }

    std::string download_uri = *op;
    LOG_T << "download_uri: " << download_uri;

    if (_download_results) {

        art::lib::ArtBaseDownloadHandlers download_handlers(_server_api_access_token);

        BeforeOutputCallback before_output;
        download_handlers.set_before_output_callback(&before_output);

        piel::lib::CurlEasyClient<art::lib::ArtBaseDownloadHandlers> download_client(download_uri, &download_handlers);

        std::cout << "Downloading file from: " << download_uri << std::endl;

        download_client.perform();

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

    //////////////////////////////////////////////////////////
    // Proptotyping code

    // Get maven metadata
    art::lib::ArtGavcHandlers download_metadata_handlers(_server_api_access_token);
    piel::lib::CurlEasyClient<art::lib::ArtGavcHandlers> get_metadata_client(
        _query.format_maven_metadata_url(_server_url, _server_repository), &download_metadata_handlers);
    get_metadata_client.perform();

    pt::ptree metadata_root;
    // Load the xml content into this ptree
    pt::read_xml(download_metadata_handlers.responce_stream(), metadata_root);

    pt::ptree metadata = metadata_root.get_child("metadata");
    boost::optional<std::string> op_group = pt::find_value(metadata, FindPropertyHelper("groupId"));
    boost::optional<std::string> op_name = pt::find_value(metadata, FindPropertyHelper("artifactId"));
    boost::optional<std::string> op_version = pt::find_value(metadata, FindPropertyHelper("version"));

    LOG_T << "metadata group: " << *op_group;
    LOG_T << "metadata name: " << *op_name;
    LOG_T << "metadata version: " << *op_version;

    pt::ptree versioning = metadata.get_child("versioning");
    boost::optional<std::string> op_latest = pt::find_value(versioning, FindPropertyHelper("latest"));
    boost::optional<std::string> op_release = pt::find_value(versioning, FindPropertyHelper("release"));
    boost::optional<std::string> op_last_updated = pt::find_value(versioning, FindPropertyHelper("lastUpdated"));

    LOG_T << "metadata versioning latest: " << *op_latest;
    LOG_T << "metadata versioning release: " << *op_release;
    LOG_T << "metadata versioning last updated: " << *op_last_updated;

    pt::ptree versions = versioning.get_child("versions");
    std::list<std::string> versions_list = pt::find_all_values(versions, FindPropertyHelper("version"));
    typedef std::list<std::string>::const_iterator Iter;
    for (Iter i = versions_list.begin(), end = versions_list.end(); i != end; ++i)
    {
        LOG_T << "metadata versioning versions version: " << *i;
    }
    //////////////////////////////////////////////////////////

    art::lib::ArtGavcHandlers api_handlers(_server_api_access_token);
    piel::lib::CurlEasyClient<art::lib::ArtGavcHandlers> client(create_url(), &api_handlers);
    client.perform();

    // Create a root
    pt::ptree root;

    // Load the json file into this ptree
    pt::read_json(api_handlers.responce_stream(), root);
    pt::each(root.get_child("results"), boost::bind(&GavcCommand::on_object, this, _1));
    
    result = 0;

    return result;
}