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

#ifndef ARTBASEAPIHANDLERS_H
#define ARTBASEAPIHANDLERS_H

#include <string>
#include <sstream>
#include <map>
#include <boost/shared_ptr.hpp>
#include <curleacyclient.hpp>

namespace art { namespace lib {

class ArtBaseApiHandlers
{
public:
    struct IBeforeCallback {
        virtual void callback(ArtBaseApiHandlers *handlers) = 0;
    };

    ArtBaseApiHandlers(const std::string& api_token);
    virtual ~ArtBaseApiHandlers();

    virtual piel::lib::CurlEasyHandlers::headers_type custom_header();

    virtual size_t handle_header(char *ptr, size_t size);

    virtual size_t handle_output(char *ptr, size_t size);

    virtual size_t handle_input(char *ptr, size_t size);

    virtual std::istringstream &responce_stream();

    virtual void before_input();

    virtual void before_output();

    void set_before_input_callback(IBeforeCallback *callback);

    void set_before_output_callback(IBeforeCallback *callback);

    std::map<std::string, std::string>& headers();

private:
    std::string _api_token;
    std::string _response_buffer;
    boost::shared_ptr<std::istringstream> _stream;
    std::map<std::string, std::string> _headers;

    IBeforeCallback *_before_input_callback;
    IBeforeCallback *_before_output_callback;
};

} } // namespace art::lib


#endif // ARTBASEAPIHANDLERS_H
