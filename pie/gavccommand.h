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

#ifndef GAVCCOMMAND_H
#define GAVCCOMMAND_H

#include <application.h>
#include <gavcquery.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/program_options.hpp>

class GavcCommand: public ICommand
{
public:
    GavcCommand(Application *app, int argc, char **argv);
    ~GavcCommand();

    virtual int perform();

    bool download_results() const { return _download_results; }

protected:
    bool parse_arguments();
    std::string create_url() const;
    void on_object(boost::property_tree::ptree::value_type obj);
    bool get_from_env(boost::program_options::variables_map& vm,
                      const std::string& opt_name,
                      const std::string& env_var,
                      std::string& var);

private:
    int _argc;
    char **_argv;

    std::string _server_url;
    std::string _server_api_access_token;
    std::string _server_repository;

    art::lib::GavcQuery _query;

    bool _download_results;

};

#endif // GAVCCOMMAND_H
