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

#include <artconstants.h>
#include <artbaseapihandlers.h>

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>

namespace piel { namespace lib {

template<> const bool CurlEasyHandlersTraits<art::lib::ArtBaseApiHandlers>::have_handle_input    = false;
template<> const bool CurlEasyHandlersTraits<art::lib::ArtBaseApiHandlers>::have_custom_header   = true;
template<> const bool CurlEasyHandlersTraits<art::lib::ArtBaseApiHandlers>::have_handle_header   = false;
template<> const bool CurlEasyHandlersTraits<art::lib::ArtBaseApiHandlers>::have_handle_output   = true;
template<> const bool CurlEasyHandlersTraits<art::lib::ArtBaseApiHandlers>::have_before_input    = false;
template<> const bool CurlEasyHandlersTraits<art::lib::ArtBaseApiHandlers>::have_before_output   = false;

} } // namespace piel::lib

namespace art { namespace lib {

ArtBaseApiHandlers::ArtBaseApiHandlers(const std::string& api_token)
    : _api_token(api_token)
    , _response_buffer()
    , _stream()
    , _before_input_callback(0)
    , _before_output_callback(0)
{

}

/*virtual*/ ArtBaseApiHandlers::~ArtBaseApiHandlers()
{

}

/*virtual*/ piel::lib::CurlEasyHandlers::headers_type ArtBaseApiHandlers::custom_header()
{
    piel::lib::CurlEasyHandlers::headers_type result;
    result.push_back(std::string(ArtConstants::rest_api_header__access_key).append(_api_token));
    return result;
}

/*virtual*/ size_t ArtBaseApiHandlers::handle_header(char *ptr, size_t size)
{
    std::string headers(ptr, size);
    BOOST_LOG_TRIVIAL(trace) << "headers: " << headers;

    std::string::iterator sepa = std::find_if(headers.begin(), headers.end(), boost::is_any_of(":"));
    if (sepa == headers.end()) {
        return size;
    }

    std::string name;
    name.append(headers.begin(), sepa);
    boost::trim(name);
    
    std::string value;
    ++sepa;
    if (sepa != headers.end()) {
        value.append(sepa, headers.end());
        boost::trim(value);
    }

    BOOST_LOG_TRIVIAL(trace) << name << "= " << value;
    _headers.insert(std::make_pair(name,value));

    return size;
}

/*virtual*/ void ArtBaseApiHandlers::before_input()
{
    // Call & reset callback pointer
    if (_before_input_callback) {
        _before_input_callback->callback(this);
        _before_input_callback = 0;
    }
}

/*virtual*/ void ArtBaseApiHandlers::before_output()
{
    // Call & reset callback pointer
    if (_before_output_callback) {
        _before_output_callback->callback(this);
        _before_output_callback = 0;
    }
}

void ArtBaseApiHandlers::set_before_input_callback(ArtBaseApiHandlers::IBeforeCallback *callback)
{
    _before_input_callback = callback;
}

void ArtBaseApiHandlers::set_before_output_callback(ArtBaseApiHandlers::IBeforeCallback *callback)
{
    _before_output_callback = callback;
}

std::map<std::string, std::string>& ArtBaseApiHandlers::headers()
{
    return _headers;
}

/*virtual*/ size_t ArtBaseApiHandlers::handle_output(char *ptr, size_t size)
{
    _response_buffer.append(ptr, ptr + size);
    BOOST_LOG_TRIVIAL(trace) << "response: " << _response_buffer;
    return size;
}

/*virtual*/ size_t ArtBaseApiHandlers::handle_input(char *ptr, size_t size)
{
    // Prepare command parameters
    return -1;
}

/*virtual*/ std::istringstream &ArtBaseApiHandlers::responce_stream()
{
    _stream = boost::shared_ptr<std::istringstream>(new std::istringstream(_response_buffer));
    return *_stream.get();
}

} } // namespace art::lib
