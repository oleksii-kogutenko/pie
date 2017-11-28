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

#include <application.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/log/trivial.hpp>
#include <artbaseapihandlers.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class TestBaseGavc: public ICommand {
public:
    TestBaseGavc(Application *app, int argc, char **argv)
        : ICommand(app)
        , _argc(argc)
        , _argv(argv)
        , _server_url()
        , _server_api_access_token()
        , _server_repository()
        , _query_group()
        , _query_artifact()
        , _query_version()
        , _query_classifier()
    {}

    int perform()
    {
        int result = -1;

        if (!parse_arguments()) {
            return result;
        }

        namespace pt = boost::property_tree;

        art::lib::ArtBaseApiHandlers apiHandlers(_server_api_access_token);
        piel::lib::CurlEasyClient<art::lib::ArtBaseApiHandlers> client(create_url(), &apiHandlers);
        client.perform();

        // Create a root
        pt::ptree root;

        // Load the json file in this ptree
        pt::read_json(apiHandlers.responce_stream(), root);

        typedef pt::ptree::const_iterator Iter;

        //results = root.get_child("results");
        pt::ptree results = root.get_child("results");
        //root.get_value();

        for(Iter i = results.begin(); i != results.end(); ++i) {

            pt::ptree::value_type pair = (*i);

            //BOOST_LOG_TRIVIAL(trace) << std::string(pair.first) << ": " << std::string(pair.second.data());

            pt::ptree val = pair.second;

            for(Iter v = val.begin(); v != val.end(); ++v) {

                pt::ptree::value_type p = (*v);

                BOOST_LOG_TRIVIAL(trace) << std::string(p.first) << ": " << std::string(p.second.data());

            }
        }

        return result;
    }

protected:
    bool parse_arguments()
    {
        namespace po = boost::program_options;

        po::options_description desc("Query options");

        desc.add_options()
            ("token,t",         po::value<std::string>(&_server_api_access_token)->required(),  "Token to access server remote api (required).")
            ("server,s",        po::value<std::string>(&_server_url)->required(),               "Server url (required).")
            ("repository,r",    po::value<std::string>(&_server_repository)->required(),        "Server repository (required).")
            ("group,g",         po::value<std::string>(&_query_group)->required(),              "Query group (required).")
            ("artifact,a",      po::value<std::string>(&_query_artifact)->required(),           "Query artifact (required).")
            ("version,v",       po::value<std::string>(&_query_version)->default_value(""),     "Query version (default: all versions).")
            ("classifier,c",    po::value<std::string>(&_query_classifier)->default_value(""),  "Query classifier (default: all artifact classifiers).")
            ;

        if (show_help(desc, _argc, _argv)) {
            return false;
        }

        po::variables_map vm;
        po::parsed_options parsed = po::command_line_parser(_argc, _argv).options(desc).allow_unregistered().run();
        po::store(parsed, vm);
        po::notify(vm);

        // Check if all requared arguments are set
        if (!( vm.count("server")
            && vm.count("repository")
            && vm.count("group")
            && vm.count("artifact")
            && vm.count("token")
        )) {
            std::cerr << "Please specify all required arguments." << std::endl;
            std::cout << desc;
            return false;
        }

        return true;
    }

    std::string create_url() const {
        std::string url = _server_url;
        url.append("/api/search/gavc");
        url.append("?r=").append(_server_repository);
        url.append("&g=").append(_query_group);
        url.append("&a=").append(_query_artifact);
        if (!_query_version.empty()) {
            url.append("&v=").append(_query_version);
        }
        if (!_query_classifier.empty()) {
            url.append("&c=").append(_query_classifier);
        }
        return url;
    }

private:
    int _argc;
    char **_argv;

    std::string _server_url;
    std::string _server_api_access_token;
    std::string _server_repository;
    std::string _query_group;
    std::string _query_artifact;
    std::string _query_version;
    std::string _query_classifier;
};

int main(int argc, char **argv)
{
    Application application(argc, argv);

    application.register_command(new CommmandConstructor<TestBaseGavc>("gavc_tst", "Basic GAVC query implementation"));

    return application.run();
}
