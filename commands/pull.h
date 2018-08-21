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

#ifndef PULL_H_
#define PULL_H_

#include <workingcopycommand.h>
#include <indexesdiff.h>
#include <gavcquery.h>
#include <list>
#include <iostreamsholder.h>

#include <boost/filesystem.hpp>

namespace piel { namespace cmd {

namespace errors {
    struct invalid_working_copy {};
    struct invalid_downloaded_artifact_name {
        invalid_downloaded_artifact_name(const std::string& n) : name(n) {}
        std::string name;
    };
};

class Pull: public piel::lib::IOstreamsHolder
{
public:
    Pull(  const std::string& server_api_access_token
         , const std::string& server_url
         , const std::string& server_repository
         , const art::lib::GavcQuery& query);
    virtual ~Pull();

    void operator()();

    void set_path_to_download(const boost::filesystem::path& path) { path_to_download_ = path; }
    boost::filesystem::path get_path_to_download() const { return path_to_download_; }

    void set_classifier_to_checkout(const std::string& c) { classifier_to_checkout_ = c; }
    std::string get_classifier_to_checkout() const { return classifier_to_checkout_; }

private:
    piel::lib::WorkingCopy::Ptr working_copy_;
    std::string server_url_;
    std::string server_api_access_token_;
    std::string server_repository_;
    art::lib::GavcQuery query_;
    boost::filesystem::path path_to_download_;
    std::string classifier_to_checkout_;
};

} } // namespace piel::cmd

#endif /* PULL_H_ */
