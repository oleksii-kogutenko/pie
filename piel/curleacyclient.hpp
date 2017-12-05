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

#ifndef CURLEACYCLIENT_H
#define CURLEACYCLIENT_H

#include <curl/curl.h>

#include <vector>
#include <string>

namespace piel { namespace lib {

template<class CurlEasyHandlers>
struct CurlEasyHandlersTraits {
    static const bool have_custom_header;
    static const bool have_handle_header;
    static const bool have_handle_input;
    static const bool have_handle_output;
    static const bool have_before_input;
    static const bool have_before_output;
};

struct CurlEasyHandlers {
    typedef std::vector<std::string> headers_type;
    headers_type custom_header()                    { return headers_type(); }
    size_t handle_header(char *ptr, size_t size)    { return CURLE_WRITE_ERROR; }
    size_t handle_output(char *ptr, size_t size)    { return CURLE_WRITE_ERROR; }
    size_t handle_input(char *ptr, size_t size)     { return CURLE_READ_ERROR; }
    void before_input()                             { }
    void before_output()                            { }
};

template<class Handlers>
class CurlEasyClient {
public:
    typedef CurlEasyClient* CurlEasyClientPtr;
    typedef Handlers* HandlersPtr;

    CurlEasyClient(const std::string& url, HandlersPtr handlers)
        : _url(url)
        , _handlers(handlers)
    {
        _curl = ::curl_easy_init();
    }

    ~CurlEasyClient()
    {
        ::curl_easy_cleanup(_curl);
    }

    CURLcode perform();

protected:
    static size_t handle_header(char *ptr, size_t size, size_t count, void* ctx);
    static size_t handle_write(char *ptr, size_t size, size_t count, void* ctx);
    static size_t handle_read(char *ptr, size_t size, size_t count, void* ctx);

private:
    std::string _url;       //!< url.
    ::CURL *_curl;          //!< libcurl handle.
    HandlersPtr _handlers;  //!< data provider.
};

template<class Handlers>
size_t CurlEasyClient<Handlers>::handle_header(char *ptr, size_t size, size_t count, void* ctx)
{
    CurlEasyClientPtr thiz = static_cast<CurlEasyClientPtr>(ctx);
    return thiz->_handlers->handle_header(ptr, size*count);
}

template<class Handlers>
size_t CurlEasyClient<Handlers>::handle_write(char *ptr, size_t size, size_t count, void* ctx)
{
    CurlEasyClientPtr thiz = static_cast<CurlEasyClientPtr>(ctx);
    if (CurlEasyHandlersTraits<Handlers>::have_before_output) {
        thiz->_handlers->before_output();
    }
    return thiz->_handlers->handle_output(ptr, size*count);
}

template<class Handlers>
size_t CurlEasyClient<Handlers>::handle_read(char *ptr, size_t size, size_t count, void* ctx)
{
    CurlEasyClientPtr thiz = static_cast<CurlEasyClientPtr>(ctx);
    if (CurlEasyHandlersTraits<Handlers>::have_before_input) {
        thiz->_handlers->before_input();
    }
    return thiz->_handlers->handle_input(ptr, size*count);
}

template<class Handlers>
CURLcode CurlEasyClient<Handlers>::perform()
{
    ::curl_easy_setopt(_curl, CURLOPT_URL, _url.c_str());
    if (CurlEasyHandlersTraits<Handlers>::have_custom_header) {
        ::curl_slist *chunk = 0;
        CurlEasyHandlers::headers_type headers = _handlers->custom_header();
        typedef CurlEasyHandlers::headers_type::const_iterator Iter;
        for (Iter i = headers.begin(); i != headers.end(); ++i)
        {
            chunk = ::curl_slist_append(chunk, (*i).c_str());
        }
        // TODO: process errors
        ::curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, chunk);
    }
    if (CurlEasyHandlersTraits<Handlers>::have_handle_header) {
        ::curl_easy_setopt(_curl, CURLOPT_HEADERDATA, this);
        ::curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, handle_header);
    }
    if (CurlEasyHandlersTraits<Handlers>::have_handle_output) {
        ::curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
        ::curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, handle_write);
    }
    if (CurlEasyHandlersTraits<Handlers>::have_handle_input) {
        ::curl_easy_setopt(_curl, CURLOPT_READDATA, this);
        ::curl_easy_setopt(_curl, CURLOPT_READFUNCTION, handle_read);
        ::curl_easy_setopt(_curl, CURLOPT_UPLOAD, 1L);
    }
    curl_easy_setopt(_curl, CURLOPT_VERBOSE, 1L);
    return ::curl_easy_perform(_curl);
}

} } // namespace piel::lib

#endif // CURLEACYCLIENT_H
