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

template<class DataProvider>
struct DataProviderTraits {
    static const bool have_input;
    static const bool have_output;
};

struct EmptyDataProvider {
    size_t get_from(char *ptr, size_t size);
    size_t put_into(char *ptr, size_t size);
};
template<> const bool DataProviderTraits<EmptyDataProvider>::have_input = false;
template<> const bool DataProviderTraits<EmptyDataProvider>::have_output = false;

template<class DataProvider>
class HttpxClient {
public:
    typedef HttpxClient* HttpxClientPtr;
    typedef DataProvider* DataProviderPtr;

    HttpxClient(const std::string& url, DataProviderPtr data_provider)
        : _url(url)
        , _data_provider(data_provider)
    {
        _curl = ::curl_easy_init();
    }

    ~HttpxClient()
    {
        ::curl_easy_cleanup(_curl);
    }

    CURLcode perform() {
        ::curl_easy_setopt(_curl, CURLOPT_URL, _url.c_str());
        if (DataProviderTraits<DataProvider>::have_output) {
            ::curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
            ::curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, handle_write);

        }
        if (DataProviderTraits<DataProvider>::have_input) {
            ::curl_easy_setopt(_curl, CURLOPT_READDATA, this);
            ::curl_easy_setopt(_curl, CURLOPT_READFUNCTION, handle_read);
            ::curl_easy_setopt(_curl, CURLOPT_UPLOAD, 1L);
        }
        return ::curl_easy_perform(_curl);
    }

protected:
    static size_t handle_write(char *ptr, size_t size, size_t count, void* ctx);
    static size_t handle_read(char *ptr, size_t size, size_t count, void* ctx);

private:
    std::string _url;                   //!< url.
    ::CURL *_curl;                      //!< libcurl handle.
    DataProviderPtr _data_provider;     //!< data provider.
};

template<class DataProvider>
size_t HttpxClient<DataProvider>::handle_write(char *ptr, size_t size, size_t count, void* ctx) {
    HttpxClientPtr thiz = static_cast<HttpxClientPtr>(ctx);
    return thiz->_data_provider->get_from(ptr, size*count);
}

template<class DataProvider>
size_t HttpxClient<DataProvider>::handle_read(char *ptr, size_t size, size_t count, void* ctx) {
    HttpxClientPtr thiz = static_cast<HttpxClientPtr>(ctx);
    return thiz->_data_provider->put_into(ptr, size*count);
}

struct DownloadToOstream_DataProvider {

    DownloadToOstream_DataProvider(std::ostream& dest)
        : _dest(dest)
        , _checksums_builder()
    {
        _checksums_builder.init();
    }

    size_t get_from(char *ptr, size_t size)
    {
        _dest.write(ptr, size);
        _checksums_builder.update(ptr, size);
        return size;
    }

    size_t put_into(char *ptr, size_t size) { return -1; }

    piel::lib::MultiChecksumsDigestBuilder::StrDigests str_digests()
    {
        return _checksums_builder.finalize<piel::lib::MultiChecksumsDigestBuilder::StrDigests>();
    }

private:
    std::ostream& _dest;    //!< destination stream.
    piel::lib::MultiChecksumsDigestBuilder _checksums_builder;
};
template<> const bool DataProviderTraits<DownloadToOstream_DataProvider>::have_output = true;
template<> const bool DataProviderTraits<DownloadToOstream_DataProvider>::have_input = false;

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

    std::ofstream out(argv[2], std::ofstream::out);
    DownloadToOstream_DataProvider ostream_provider(out);
    HttpxClient<DownloadToOstream_DataProvider> client(argv[1], &ostream_provider);
    client.perform();

    piel::lib::MultiChecksumsDigestBuilder::StrDigests str_digests = ostream_provider.str_digests();

    BOOST_LOG_TRIVIAL(trace) << piel::lib::Sha256::t::name() << ": " << str_digests[piel::lib::Sha256::t::name()];
    BOOST_LOG_TRIVIAL(trace) << piel::lib::Sha::t::name()    << ": " << str_digests[piel::lib::Sha::t::name()];
    BOOST_LOG_TRIVIAL(trace) << piel::lib::Md5::t::name()    << ": " << str_digests[piel::lib::Md5::t::name()];

    return result;
}
