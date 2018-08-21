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

#ifndef PUSH_H_
#define PUSH_H_

#include <workingcopycommand.h>
#include <indexesdiff.h>
#include <gavcquery.h>
#include <uploadfilesspec.h>
#include <list>
#include <artdeployartifacthandlers.h>
#include <upload.h>
#include <iostreamsholder.h>

namespace piel { namespace cmd {

namespace errors {
    struct nothing_to_push {};
    struct uploading_classifier_error
    {
        uploading_classifier_error(const std::string& e) : error(e) {}
        std::string error;
    };
    struct uploading_pom_error
    {
        uploading_pom_error(const std::string& e) : error(e) {}
        std::string error;
    };
};

class Push: public WorkingCopyCommand, public piel::lib::IOstreamsHolder
{
public:
    Push(const piel::lib::WorkingCopy::Ptr& working_copy);
    virtual ~Push();

    void operator()();

    const Push* set_server_url(const std::string& url);
    const Push* set_server_api_access_token(const std::string& token);
    const Push* set_server_repository(const std::string& repo);
    const Push* set_query(const art::lib::GavcQuery& query);

protected:
    bool upload(const std::string& classifier, const std::string& file_name);
    void deploy_pom(const boost::filesystem::path& path_to_save_pom);

private:
    std::string server_url_;
    std::string server_api_access_token_;
    std::string server_repository_;
    art::lib::GavcQuery query_;
    std::list<std::string> zip_list_;
};

} } // namespace piel::cmd

#endif /* PUSH_H_ */
