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

#include <downloadfilehandlers.h>

//      custom_header,    handle_header,  handle_input,   handle_output,  before_input,   before_output)
CURLH_T_(piel::lib::DownloadFileHandlers,\
        true,             false,          false,          true,           false,          false);

namespace piel { namespace lib {

DownloadFileHandlers::DownloadFileHandlers(const std::string& id, std::ostream* dest)
    : id_(id)
    , dest_(dest)
    , checksums_builder_()
{
    checksums_builder_.init();
}

CurlEasyHandlers::headers_type DownloadFileHandlers::custom_header()
{
    return CurlEasyHandlers::headers_type();
}

size_t DownloadFileHandlers::handle_header(char *ptr, size_t size)
{
    return -1;
}

size_t DownloadFileHandlers::handle_output(char *ptr, size_t size)
{
    if (dest_) dest_->write(ptr, size);
    checksums_builder_.update(ptr, size);
    return size;
}

size_t DownloadFileHandlers::handle_input(char *ptr, size_t size)
{
    return -1;
}

ChecksumsDigestBuilder::StrDigests DownloadFileHandlers::str_digests()
{
    return checksums_builder_.finalize<ChecksumsDigestBuilder::StrDigests>();
}

void DownloadFileHandlers::before_input()
{

}

void DownloadFileHandlers::before_output()
{

}

} } // namespace piel::lib
