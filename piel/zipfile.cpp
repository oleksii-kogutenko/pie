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

#include <zipfile.h>

namespace piel { namespace lib {

ZipFile::EntryPtr ZipFile::entry(const std::string& entry_name)
{
    return ZipFile::EntryPtr(
            new ZipEntry(
                    entries_owner_, entry_name, fopen(entry_name)));
}

ZipFile::EntryPtr ZipFile::entry(zip_int64_t entry_index)
{
    return ZipFile::EntryPtr(
            new ZipEntry(
                    entries_owner_, entry_name(entry_index), fopen(entry_index)));
}

zip_stat_t ZipFile::stat(zip_int64_t entry_index) const
{
    zip_stat_t result;
    ::zip_stat_init(&result);
    // TODO: errors processing
    ::zip_stat_index(zip_, entry_index, ZIP_FL_UNCHANGED, &result);
    return result;
}

zip_stat_t ZipFile::stat(const std::string& entry_name) const
{
    zip_stat_t result;
    ::zip_stat_init(&result);
    // TODO: errors processing
    ::zip_stat(zip_, entry_name.c_str(), ZIP_FL_UNCHANGED, &result);
    return result;
}

ZipEntryAttributes ZipFile::file_get_external_attributes(zip_int64_t entry_index) const
{
    ZipEntryAttributes result = { 0 };
    // TODO: errors processing
    ::zip_file_get_external_attributes(zip_, entry_index, ZIP_FL_UNCHANGED, &result.opsys, &result.attributes);
    return result;
}

ZipEntryAttributes ZipFile::file_get_external_attributes(const std::string& entry_name) const
{
    ZipEntryAttributes result = { 0 };
    // TODO: errors processing
    zip_stat_t zip_stat = stat(entry_name);
    ::zip_file_get_external_attributes(zip_, zip_stat.index, ZIP_FL_UNCHANGED, &result.opsys, &result.attributes);
    return result;
}

std::string ZipEntry::target()
{
    // To be sure what we will be able to read data.
    reopen();
    std::string result;
    ZipEntrySource::istream is(this);
    std::getline(is, result);
    return result;
}

ZipEntrySource::istream_ptr ZipEntry::istream()
{
    // To be sure what we will be able to read data.
    reopen();
    return ZipEntrySource::istream_ptr(new ZipEntrySource::istream(this));
}

std::streamsize ZipEntrySource::read(char* buffer, std::streamsize n)
{
    return entry_ ? entry_->read(buffer, n) : -1;
}

} } // namespace piel::lib
