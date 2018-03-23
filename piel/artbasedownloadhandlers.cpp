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
#include <artconstants.h>
#include <artbasedownloadhandlers.h>
#include <logging.h>

//      custom_header,    handle_header,  handle_input,   handle_output,  before_input,   before_output)
CURLH_T_(art::lib::ArtBaseDownloadHandlers,\
        true,             true,           false,          true,           false,          true);

namespace art { namespace lib {

ArtBaseDownloadHandlers::ArtBaseDownloadHandlers(const std::string& api_token)
    : ArtBaseApiHandlers(api_token)
    , id_()
    , dest_(0)
    , checksums_builder_()
{
    checksums_builder_.init();
}

ArtBaseDownloadHandlers::ArtBaseDownloadHandlers(const std::string& api_token, std::ostream *dest)
    : ArtBaseApiHandlers(api_token)
    , id_()
    , dest_(dest)
    , checksums_builder_()
{
    checksums_builder_.init();
}

/*virtual*/ ArtBaseDownloadHandlers::~ArtBaseDownloadHandlers()
{

}

void ArtBaseDownloadHandlers::set_destination(std::ostream *dest)
{
    dest_ = dest;
}

/*virtual*/ size_t ArtBaseDownloadHandlers::handle_output(char *ptr, size_t size)
{
    LOGT << "recieved buffer for id: " << id_ << " at:" << ptr << " size: " << size << LOGE;

    if (dest_)
    {
        dest_->write(ptr, size);
    }
    checksums_builder_.update(ptr, size);

    on_buffer_({
        .id = id_,
        .size = size,
    });

    return size;
}

/*virtual*/ piel::lib::ChecksumsDigestBuilder::StrDigests ArtBaseDownloadHandlers::str_digests()
{
    return checksums_builder_.finalize<piel::lib::ChecksumsDigestBuilder::StrDigests>();
}

} } // namespace art::lib
