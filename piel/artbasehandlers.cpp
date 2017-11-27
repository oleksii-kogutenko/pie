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

#include <artbasehandlers.h>

#include <boost/log/trivial.hpp>

namespace piel { namespace lib {

template<> const bool CurlEasyHandlersTraits<art::lib::ArtBaseHandlers>::have_handle_input    = false;
template<> const bool CurlEasyHandlersTraits<art::lib::ArtBaseHandlers>::have_custom_header   = true;
template<> const bool CurlEasyHandlersTraits<art::lib::ArtBaseHandlers>::have_handle_header   = false;
template<> const bool CurlEasyHandlersTraits<art::lib::ArtBaseHandlers>::have_handle_output   = true;
    
} } // namespace piel::lib

namespace art { namespace lib {

ArtBaseHandlers::ArtBaseHandlers(const std::string& api_token)
    : _api_token(api_token)
    , _response_buffer()
    , _stream()
{

}

ArtBaseHandlers::~ArtBaseHandlers()
{

}

piel::lib::CurlEasyHandlers::headers_type ArtBaseHandlers::custom_header()
{
    piel::lib::CurlEasyHandlers::headers_type result;
    result.push_back(std::string("X-JFrog-Art-Api:").append(_api_token));
    return result;
}

size_t ArtBaseHandlers::handle_header(char *ptr, size_t size)
{
    return -1;
}
    
size_t ArtBaseHandlers::handle_output(char *ptr, size_t size)
{
    _response_buffer.append(ptr, ptr + size);
    BOOST_LOG_TRIVIAL(trace) << "response: " << _response_buffer;        
    return size;
}

size_t ArtBaseHandlers::handle_input(char *ptr, size_t size)
{
    // Prepare command parameters
    return -1;
}

std::istringstream &ArtBaseHandlers::responce_stream()
{
    _stream = boost::shared_ptr<std::istringstream>(new std::istringstream(_response_buffer));
    return *_stream.get();
}

} } // namespace art::lib
