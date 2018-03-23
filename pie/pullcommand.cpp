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
#include <pullcommand.h>
#include <pull.h>
#include <gavc.h>

#include <artgavchandlers.h>
#include <logging.h>
#include <mavenmetadata.h>

#include <boost/bind.hpp>
#include <boost_property_tree_ext.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

namespace pie { namespace app {

namespace pt = boost::property_tree;
namespace po = boost::program_options;
namespace al = art::lib;

PullCommand::PullCommand(Application *app, int argc, char **argv)
    : ICommand(app)
    , argc_(argc)
    , argv_(argv)
    , server_url_()
    , server_api_access_token_()
    , server_repository_()
    , query_()
    , path_to_download_()
    , classifier_to_checkout_()
{
}

PullCommand::~PullCommand()
{
}

void PullCommand::show_command_help_message(const po::options_description& desc)
{
    std::cerr << "Usage: pull <gavc query> [options]" << std::endl;
    std::cout << "Note: Use specific version in the <gavc query>" << std::endl;
    std::cout << desc;
}

bool PullCommand::parse_arguments()
{
    po::options_description desc("Pull options");

    desc.add_options()
        ("token,t",         po::value<std::string>(&server_api_access_token_),  "Token to access server remote api (required). Can be set using GAVC_SERVER_API_ACCESS_TOKEN environment variable.")
        ("server,s",        po::value<std::string>(&server_url_),               "Server url (required). Can be set using GAVC_SERVER_URL environment variable.")
        ("repository,r",    po::value<std::string>(&server_repository_),        "Server repository (required). Can be set using GAVC_SERVER_REPOSITORY environment variable.")
        ("path,p",          po::value<std::string>(&path_to_download_),         "New working copy directory.")
        ("classifier,c",    po::value<std::string>(&classifier_to_checkout_),   "Tree name what will be checkout after pulling data.")
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
    LOGT << "query to perform: " << query_str << ELOG;

    boost::optional<art::lib::GavcQuery> parsed_query = art::lib::GavcQuery::parse(query_str);
    if (!parsed_query)
    {
        std::cerr << "Wrong gavc query: " << query_str << "!" << std::endl;
        show_command_help_message(desc);
        return false;
    }

    query_ = *parsed_query;

    if (!query_.is_single_version_query())
    {
        std::cerr << "Single version query is needed for the pull command!" << std::endl;
        return false;
    }

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

    return true;
}

/*virtual*/ int PullCommand::perform()
{
    int result = -1;

    if (!parse_arguments()) {
        return result;
    }

    try
    {
        piel::cmd::Pull pull(server_api_access_token_,
                             server_url_,
                             server_repository_,
                             query_);

        pull.set_path_to_download(path_to_download_);
        pull.set_classifier_to_checkout(classifier_to_checkout_);

        pull();
    }
    catch (const piel::lib::errors::init_existing_working_copy&)
    {
        std::cerr << "Attempt to initialize already initialized working copy!" << std::endl;
        return -1;
    }
    catch (const piel::lib::errors::archives_directory_not_exists&)
    {
        std::cerr << "Archives folder cannot be created!" << std::endl;
        return -1;
    }
    catch (const piel::lib::errors::attach_to_non_working_copy&)
    {
        std::cerr << "Attempt to perform operation outside of working copy!" << std::endl;
        return -1;
    }
    catch (piel::cmd::errors::fail_to_parse_maven_metadata&) {
        std::cerr << "Error on parsing maven metadata. Server response has non expected format." << std::endl;
        return -1;
    }
    catch (piel::cmd::errors::fail_on_request_maven_metadata& e) {
        std::cerr << "Error on requesting maven metadata." << std::endl;
        std::cerr << e.error << std::endl;
        return -1;
    }
    catch (piel::cmd::errors::error_processing_version& e) {
        std::cerr << "Error on processing version: " << e.ver << "!" << std::endl;
        std::cerr << e.error << std::endl;
        return -1;
    }
    catch (piel::cmd::errors::cant_receive_metadata& ) {
        std::cerr << "Can't retrieve maven metadata!" << std::endl;
        return -1;
    }
    catch (piel::cmd::errors::invalid_working_copy& ) {
        std::cerr << "Unknown error. Working copy state is invalid." << std::endl;
        return -1;
    }
    catch (piel::cmd::errors::invalid_downloaded_artifact_name& e) {
        std::cerr << "Unexpected downloaded file name: " << e.name  << "!" << std::endl;
        return -1;
    }
    catch (const piel::lib::errors::unable_to_find_reference_file& e)
    {
        std::cerr << "Unable to find reference file at working copy!" << std::endl;
        return -1;
    }

    result = 0;

    return result;
}

} } // namespace pie::app
