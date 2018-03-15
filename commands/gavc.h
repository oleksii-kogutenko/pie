/*
 * Copyright (c) 2018, diakovliev
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
 * THIS SOFTWARE IS PROVIDED BY diakovliev ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL diakovliev BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef GAVC_H_
#define GAVC_H_

#include <gavcquery.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/program_options.hpp>

namespace piel { namespace cmd {

namespace errors {
    struct fail_to_parse_maven_metadata {};
    struct fail_on_request_maven_metadata {
        fail_on_request_maven_metadata(const std::string& e)
            : error(e)
        {}
        std::string error;
    };
    struct error_processing_version {
        error_processing_version(const std::string& e, const std::string& v)
            : error(e)
            , ver(v)
        {}
        std::string error;
        std::string ver;
    };
    struct cant_receive_metadata {};
};

class GAVC
{
public:
    GAVC(  const std::string& server_api_access_token
         , const std::string& server_url
         , const std::string& server_repository
         , const art::lib::GavcQuery& query
         , const bool have_to_download_results);
    virtual ~GAVC();

    void operator()();

    void set_path_to_download(const std::string& path)
    {
        path_to_download_ = path;
    }

    std::string get_path_to_download() const { return path_to_download_; }

protected:
    std::string create_url(const std::string& version_to_query) const;
    void on_object(boost::property_tree::ptree::value_type obj);
    std::map<std::string,std::string> get_server_checksums(const boost::property_tree::ptree& obj_tree, const std::string& section) const;
private:
    std::string server_url_;
    std::string server_api_access_token_;
    std::string server_repository_;
    art::lib::GavcQuery query_;
    std::string path_to_download_;
    bool have_to_download_results_;
};

} } // namespace piel::cmd

#endif /* GAVC_H_ */
