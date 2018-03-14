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
#include <pushcommand.h>
#include <push.h>

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

PushCommand::PushCommand(Application *app, int argc, char **argv)
    : ICommand(app)
    , argc_(argc)
    , argv_(argv)
    , server_url_()
    , server_api_access_token_()
    , server_repository_()
    , query_()
{
}

PushCommand::~PushCommand()
{
}

bool PushCommand::get_from_env(po::variables_map& vm,
                               const std::string& opt_name,
                               const std::string& env_var,
                               std::string& var)
{
    if (!vm.count(opt_name)) {
        const char *value = ::getenv(env_var.c_str());
        if (value)
        {
            LOGT << "Got " << env_var << " environment variable. Value: " << value << "." << ELOG;
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

void PushCommand::show_command_help_message(const po::options_description& desc)
{
    std::cerr << "Usage: gavc <gavc query> [options]" << std::endl;
    std::cout << "Note: Use specific version in the <gavc query>" << std::endl;
    std::cout << desc;
}

bool PushCommand::parse_arguments()
{
    po::options_description desc("Push options");

    desc.add_options()
        ("token,t",         po::value<std::string>(&server_api_access_token_),  "Token to access server remote api (required). Can be set using GAVC_SERVER_API_ACCESS_TOKEN environment variable.")
        ("server,s",        po::value<std::string>(&server_url_),               "Server url (required). Can be set using GAVC_SERVER_URL environment variable.")
        ("repository,r",    po::value<std::string>(&server_repository_),        "Server repository (required). Can be set using GAVC_SERVER_REPOSITORY environment variable.")
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
        std::cout << "Wrong gavc query: " << query_str << "!" << std::endl;
        show_command_help_message(desc);
        return false;
    }

    //[bool pie::app::PushCommand::parse_arguments()] query_:test_dir:dir2:4
    query_ = *parsed_query;

    try
    {
        int version;
        version = boost::lexical_cast<int>(query_.version());
    }
    catch(boost::bad_lexical_cast&)
    {
        std::cout << "'" << query_.version() << "' Version must be numeric value" << std::endl;
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

/*virtual*/ int PushCommand::perform()
{
    int result = -1;

    if (!parse_arguments()) {
        return result;
    }

    try
    {
        working_copy_ = piel::lib::WorkingCopy::attach(boost::filesystem::current_path());

        piel::cmd::Push push(working_copy_);

        push.set_server_url(server_url_);
        push.set_server_api_access_token(server_api_access_token_);
        push.set_server_repository(server_repository_);
        push.set_query(query_);

        push();

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
    catch (const piel::cmd::errors::nothing_to_push&)
    {
        std::cerr << "No changes!" << std::endl;
        return -1;
    }
    catch (const piel::cmd::errors::uploading_classifier_error& e)
    {
        std::cerr << "Classifier uploading error:" << e.error << std::endl;
        return -1;
    }
    catch (const piel::cmd::errors::uploading_pom_error& e)
    {
        std::cerr << "POM uploading error:" << e.error << std::endl;
        return -1;
    }

    if (!working_copy_->is_valid())
    {
        std::cerr << "Unknown error. Working copy state is invalid." << std::endl;
        return -1;
    }

    result = 0;

    return result;
}

} } // namespace pie::app
