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

#include <zipindexer.h>

#include <zipfile.h>
#include <checksumsdigestbuilder.hpp>
#include <boost/log/trivial.hpp>

namespace piel { namespace lib {

ZipIndexer::ZipIndexer()
{

}

ZipIndexer::~ZipIndexer()
{

}

std::string ZipIndexer::zip_source(const fs::path& zip_file, const std::string& entry_name) const
{
    return std::string("zip://").append(zip_file.native()).append("#").append(entry_name);
}

BaseIndex ZipIndexer::build(const fs::path& zip_file) const
{
    BaseIndex result;

    ChecksumsDigestBuilder digest_builder;

    ZipFile zip(zip_file.native());
    for (zip_int64_t i = 0; i < zip.num_entries(); i++) {
        boost::shared_ptr<ZipEntry> entry = zip.entry(i);

        ZipEntryAttributes attrs = entry->attributes();

        ChecksumsDigestBuilder::StrDigests checksums = digest_builder.str_digests_for(ZIP_ENTRY_ISTREAM(entry));

        std::string hash = checksums[Sha256::t::name()];

        BOOST_LOG_TRIVIAL(trace) << std::string(entry->symlink() ? "s " +  entry->target() + " " : "f ")
                                 << entry->name()
                                 << " os:"
                                 << boost::format("%1$08x") % (int)attrs.opsys
                                 << " attributes: "
                                 << boost::format("%1$08x") % (int)attrs.attributes
                                 << " mode: "
                                 << boost::format("%1$04o") % (int)(attrs.mode() & 0777)
                                 << " sha256: "
                                 << hash;

        result.put(entry->name(), hash, zip_source(zip_file, entry->name()));
    }

    return result;
}

} } // namespace piel::lib
