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
#include <fsindexer.h>
#include <zipindexer.h>
#include <baseindex.h>

//****
#include <checksumdigestbuilder.hpp>
#include <curl/curl.h>
#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#include <fstream>

struct CurlDownloader {

    typedef char char_type;

    CurlDownloader(const std::string& url, std::ostream& dest)
        : _url(url)
        , _dest(dest)
        , _curl(0)
    {
        _curl = ::curl_easy_init();
    }

    ~CurlDownloader()
    {
        ::curl_easy_cleanup(_curl);
    }

    CURLcode download() {
        ::curl_easy_setopt(_curl, CURLOPT_URL, _url.c_str());
        ::curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
        ::curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, handle_data);
        return ::curl_easy_perform(_curl);
    }

private:
    static size_t handle_data(char *ptr, size_t size, size_t offset, void* ctx);

private:
    std::string _url;       //!< href to download data.
    std::ostream& _dest;    //!< destination stream.
    ::CURL *_curl;          //!< libcurl handle.
};

size_t CurlDownloader::handle_data(char *ptr, size_t size, size_t count, void* ctx)
{
    CurlDownloader *thiz = static_cast<CurlDownloader*>(ctx);

    BOOST_LOG_TRIVIAL(trace) << " handle new buffer"
                             << boost::format(" ptr: %1$p") % (void*)ptr
                             << " size: " << size
                             << " count: " << count;

    thiz->_dest.write(ptr, size*count);
    return size*count;
}

int main(int argc, char **argv) {

    int result = -1;

    if (argc < 2) {
        return result;
    }


    //piel::lib::FsIndexer indexer;
    //piel::lib::ZipIndexer indexer;
    //piel::lib::BaseIndex index = indexer.build(argv[1]);

    //if (!index.empty())
    //{
    //    result = 0;
    //}

    piel::lib::MultiChecksumsDigestBuilder digest_builder;
    piel::lib::Sha256Context sha256_context;

    std::ofstream out(argv[2], std::ofstream::out);
    CurlDownloader downloader(argv[1], out);
    downloader.download();

    //CurlDownloader::istream is(argv[1]);

    //piel::lib::MultiChecksumsDigestBuilder::StrDigests digests = digest_builder.str_digests_for(is);
    //std::string sha256 = digests[sha256_context.name()];

    //BOOST_LOG_TRIVIAL(trace) << "sha256: "
                             //<< sha256;


    return result;
}
