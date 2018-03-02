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
#include <uploadcommand.h>
#include <upload.h>

#include <artgavchandlers.h>
#include <logging.h>
#include <mavenmetadata.h>

#include <boost/bind.hpp>
#include <boost_property_tree_ext.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <vector>

namespace pie { namespace app {

namespace pt = boost::property_tree;
namespace po = boost::program_options;
namespace al = art::lib;

const int empty_classifiers_count_allowed = 1;

UploadCommand::UploadCommand(Application *app, int argc, char **argv)
    : ICommand(app)
    , argc_(argc)
    , argv_(argv)
    , server_url_()
    , server_api_access_token_()
    , server_repository_()
    , query_()
    , classifier_vector_()
{
}

UploadCommand::~UploadCommand()
{
}

bool UploadCommand::get_from_env(po::variables_map& vm,
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

void UploadCommand::show_command_help_message(const po::options_description& desc)
{
    std::cerr << "Usage: gavc <gavc query> [options]" << std::endl;
    std::cout << "Note: Use specific version in the <gavc query>" << std::endl;
    std::cout << desc;
}

bool UploadCommand::check_empty_classifiers_count(const art::lib::ufs::UFSVector& c)
{
    bool ret_val = true;
    int count_empty_classifier = 0;
    for (art::lib::ufs::UFSVector::const_iterator it = c.begin(), end = c.end(); it != end && ret_val; ++it)
    {
        if (it->classifier.empty()) {
            count_empty_classifier++;
            std::cout << "Find empty classifier: " << art::lib::ufs::to_string(*it) << "(" << count_empty_classifier << ")" << std::endl;
        }
        ret_val &= count_empty_classifier <= empty_classifiers_count_allowed;
    }
    return ret_val;
}

bool UploadCommand::parse_arguments()
{
    po::options_description desc("Upload options");
    std::string  classifiers_str;

    desc.add_options()
        ("token,t",         po::value<std::string>(&server_api_access_token_),  "Token to access server remote api (required). Can be set using GAVC_SERVER_API_ACCESS_TOKEN environment variable.")
        ("server,s",        po::value<std::string>(&server_url_),               "Server url (required). Can be set using GAVC_SERVER_URL environment variable.")
        ("repository,r",    po::value<std::string>(&server_repository_),        "Server repository (required). Can be set using GAVC_SERVER_REPOSITORY environment variable.")
        ("filelist,f",      po::value<std::string>(&classifiers_str)->required(),    "List of files to upload (required). Use as: classifier1:file1,classifier2:file2,...")
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

    //[bool pie::app::UploadCommand::parse_arguments()] query_:test_dir:dir2:4
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

    // Gen file list
    LOGT << "classifier_vector_str_:" << classifiers_str << ELOG;

    al::UploadFileSpec spec;

    boost::optional<al::UploadFileSpec> result_parse = spec.parse(classifiers_str);
    if (result_parse)
    {
        classifier_vector_ = result_parse->get_data();
        if (!check_empty_classifiers_count(classifier_vector_)) {
            std::cout << "Empty classifiers more then " << empty_classifiers_count_allowed << ", it's wrong!" << std::endl;
            show_command_help_message(desc);
            return false;
        }
    } else {
        std::cout << "Wrong parse <filelist> argument!" << std::endl;
        show_command_help_message(desc);
        return false;
    }

    return true;
}

/*virtual*/ int UploadCommand::perform()
{
    int result = -1;

    if (!parse_arguments()) {
        return result;
    }

    try
    {
        piel::cmd::Upload upload;

        upload.set_server_url(server_url_);
        upload.set_server_api_access_token(server_api_access_token_);
        upload.set_server_repository(server_repository_);
        upload.set_query(query_);
        upload.set_classifiers(classifier_vector_);

        upload();
    }
    catch (const piel::cmd::errors::nothing_to_upload&)
    {
        std::cerr << "No changes!" << std::endl;
        return -1;
    }

    result = 0;

    return result;
}

} } // namespace pie::app
