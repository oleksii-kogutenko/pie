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
#include "logging.h"

namespace piel { namespace lib {

struct ZipSource;
struct ZipSourceIStream;

typedef boost::shared_ptr<ZipSourceIStream> SourceIStreamPtr;   //!< Pointer to ZipSourceEntry.
typedef boost::weak_ptr<ZipSourceIStream>   SourceIStreamWeakPtr;   //!< Pointer to ZipSourceEntry.

struct ZipSourceIStream {

    ~ZipSourceIStream() {
        LOGI << __PRETTY_FUNCTION__ << ELOG;
    }

    zip_source_t* init(zip_t *zip, ZipFile::IStreamWeakPtr stream, SourceIStreamWeakPtr owner)
    {
        stream_ = stream.lock();
        size_ = 0;
        LOGI << __PRETTY_FUNCTION__ << ELOG;
        size_ = 0;
        if (stream_) {
            LOGI << "stream_:" << stream_.get() << ELOG;
            std::streamsize pos = stream_->tellg();
            stream_->seekg(0, std::ios_base::end);
            size_ = stream_->tellg();
            stream_->seekg(pos, std::ios_base::beg);
        }
        owner_ = owner.lock();

        return ::zip_source_function(zip, read_data_callback, this);
    }

protected:
    friend struct ZipSource;
    ZipSourceIStream()
        : stream_()
        , size_(0)
        , owner_()
        , libzip_error_()
    {
        LOGI << __PRETTY_FUNCTION__ << ELOG;
    }

    static zip_int64_t read_data_callback(void *user_data, void *data, zip_uint64_t len, zip_source_cmd_t cmd);
    void lock()
    {
        LOGI << __PRETTY_FUNCTION__ << ELOG;
    }
    void unlock()
    {
        LOGI << __PRETTY_FUNCTION__ << ELOG;
        owner_.reset();
    }
private:
    ZipFile::IStreamPtr     stream_;
    std::streamsize         size_;
    SourceIStreamPtr        owner_;
    zip_error_t             libzip_error_;    //!< libzip last error code.
};

//! libzip C api wrapper. Encapsulated api to work with archive entry.
struct ZipSource {

    //! Destructor.
    ~ZipSource() {
        if (to_be_freed_) {
            zip_source_free(source_);
        }
    }

#ifdef ZIP_ENABLE_SEEK_TELL
    zip_int8_t seek(zip_int64_t offset, int whence) const {
        return owner_->fseek(zip_file_, offset, whence);
    }

    zip_int64_t tell() const {
        return owner_->ftell(zip_file_);
    }
#endif//ZIP_ENABLE_SEEK_TELL

    //! Get entry name.
    //! \return Archive entry name.
    std::string name() const {
        return entry_name_;
    }

    //! Get entry file name.
    //! \return Archive entry file name.
    std::string file_name() const {
        return file_name_;
    }

    //! Zip entry stat.
    //! \return libzip zip_stat_t
    zip_stat_t stat() const {
        return owner_->stat(entry_name_);
    }

    //! Get entry extended attributes.
    //! \return ZipEntryAttribute struct with extenden attributes.
    ZipEntryAttributes attributes() const {
        return owner_->file_get_external_attributes(entry_name_);
    }

    //! Get entry extended attributes.
    //! \return ZipEntryAttribute struct with extenden attributes.
    ZipEntryAttributes set_attributes() const {
        return owner_->file_get_external_attributes(entry_name_);
    }

    //! Get unix mode.
    //! \return unix file mode.
    mode_t mode() const {
        ZipEntryAttributes attrs = attributes();
        return attrs.mode();
    }

    //! Check if the entry is symlink.
    //! \return true if the entry is unix symlink, false otherwise.
    bool symlink() const {
        ZipEntryAttributes attrs = attributes();
        return attrs.symlink();
    }

    //! Read symlink target.
    //! \return The path symlink points to.
    //! \sa bool symlink() const
    std::string target();

    //! Construct input stream.
    //! \return Pointer to archive entry istream.
    ZipEntrySource::istream_ptr istream();

protected:
    void set_to_be_freed(bool b) {
        to_be_freed_ = b;
    }

    zip_source_t *source() const {
        return source_;
    }

    // Frienship is used for control ownership model. ZipEntry instance can be
    //created by ZipFile instance only.
    friend class ZipFile;

    //! Constructor.
    //! \param owner ZipFile instance.
    //! \param name zip archive entry name.
    //! \param zip_file zip_file_t libzip handle.
    ZipSource(ZipFile::WeakFilePtr owner, const std::string& entry_name, const std::string& file_name)
        : owner_(owner.lock())
        , entry_name_(entry_name)
        , file_name_(file_name)
        , to_be_freed_(true)
        , stream_ptr_()
    {
        source_ = ::zip_source_file(owner_->zip_, file_name.c_str(), 0, -1);
    }

    ZipSource(ZipFile::WeakFilePtr owner, const std::string& entry_name, const void *data, zip_uint64_t len, int freep = 0)
        : owner_(owner.lock())
        , entry_name_(entry_name)
        , file_name_()
        , to_be_freed_(true)
        , stream_ptr_()
    {
        source_ = ::zip_source_buffer(owner_->zip_, data, len, freep);
    }

    ZipSource(ZipFile::WeakFilePtr owner, const std::string& entry_name, ZipFile::IStreamWeakPtr stream_ptr);

private:
    ZipFile::FilePtr        owner_;            //!< Reference to zip file.
    std::string             entry_name_;                  //!< Entry name.
    std::string             file_name_;                  //!< Entry name.
    zip_source_t            *source_;
    bool                    to_be_freed_;
    SourceIStreamPtr        stream_ptr_;
};
/*--------------------------------*/
zip_int64_t
ZipSourceIStream::read_data_callback(void *user_data, void *data, zip_uint64_t len, zip_source_cmd_t cmd)
{
    ZipSourceIStream *ctx = reinterpret_cast<ZipSourceIStream*>(user_data);
    zip_int64_t ret_val = 0;

    switch (cmd) {
        case ZIP_SOURCE_OPEN:
            break;
        case ZIP_SOURCE_CLOSE:
            break;
        case ZIP_SOURCE_FREE:
            ctx->unlock();
            break;
        case ZIP_SOURCE_READ:
            if (len > ZIP_INT64_MAX) {
                ::zip_error_set(&ctx->libzip_error_, ZIP_ER_INVAL, 0);
                ret_val = -1;
                break;
            }
            ctx->stream_->read(static_cast<std::ios::char_type*>(data), static_cast<std::streamsize>(len));
            ret_val = static_cast<zip_int64_t>(ctx->stream_->gcount());
            break;
        case ZIP_SOURCE_STAT:
            zip_stat_t *st;

            if (len < sizeof(*st)) {
                ::zip_error_set(&ctx->libzip_error_, ZIP_ER_INVAL, 0);
                ret_val = -1;
                break;
            }

            st = static_cast<zip_stat_t *>(data);

            ::zip_stat_init(st);
            st->mtime = time(NULL);
            st->size = static_cast<zip_uint64_t>(ctx->size_);
            st->comp_size = st->size;
            st->comp_method = ZIP_CM_STORE;
            st->encryption_method = ZIP_EM_NONE;
            st->valid = ZIP_STAT_MTIME|ZIP_STAT_SIZE|ZIP_STAT_COMP_SIZE|ZIP_STAT_COMP_METHOD|ZIP_STAT_ENCRYPTION_METHOD;

            ret_val = sizeof(*st);
            break;
        case ZIP_SOURCE_SUPPORTS:
            ret_val = ::zip_source_make_command_bitmap(
                        ZIP_SOURCE_READ,
                        ZIP_SOURCE_OPEN,
                        ZIP_SOURCE_CLOSE,
                        ZIP_SOURCE_FREE,
                        ZIP_SOURCE_STAT, -1);
            break;
        default:
            LOGW << "Do not processed command: " << cmd << ELOG;
            ret_val = -1;
    }
    return ret_val;
}

ZipSource::ZipSource(ZipFile::WeakFilePtr owner, const std::string& entry_name, ZipFile::IStreamWeakPtr stream_weak_ptr)
    : owner_(owner.lock())
    , entry_name_(entry_name)
    , file_name_()
    , to_be_freed_(true)
    , stream_ptr_(SourceIStreamPtr(new ZipSourceIStream))
{
    source_ = stream_ptr_->init(owner_->zip_, stream_weak_ptr, stream_ptr_);
}

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

ZipFile::SourcePtr ZipFile::create_file_source(const std::string& entry_name, const std::string &file_name)
{
    return ZipFile::SourcePtr(
            new ZipSource(
                    entries_owner_, entry_name, file_name));
}

ZipFile::SourcePtr ZipFile::create_buffer_source(const std::string& entry_name, const void *data, zip_uint64_t len, int freep)
{
    return ZipFile::SourcePtr(
            new ZipSource(
                    entries_owner_, entry_name, data, len, freep));
}

ZipFile::SourcePtr ZipFile::create_istream_source(const std::string& entry_name, IStreamWeakPtr source_ptr)
{
    return ZipFile::SourcePtr(
            new ZipSource(
                    entries_owner_, entry_name, source_ptr));
}


zip_int64_t ZipFile::add_source(const SourcePtr& source, int mode)
{
    LOGD << source->entry_name_ << " " << source->file_name_ << " --> " << filename_ << ELOG;

    zip_int64_t entry_index = ::zip_file_add(zip_, source->name().c_str(), source->source(), ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
    source->set_to_be_freed(false);
    if ( mode >=0 ) {
        ZipEntryAttributes attr;
        // TODO: errors processing
        ::zip_file_get_external_attributes(zip_, zip_uint64_t(entry_index), ZIP_FL_UNCHANGED, &attr.opsys, &attr.attributes);
        attr.set_mode(mode);
        ::zip_file_set_external_attributes(zip_, zip_uint64_t(entry_index), ZIP_FL_UNCHANGED, attr.opsys, attr.attributes);
    }
    return entry_index;
}

zip_stat_t ZipFile::stat(zip_int64_t entry_index) const
{
    zip_stat_t result;
    ::zip_stat_init(&result);
    // TODO: errors processing
    ::zip_stat_index(zip_, static_cast<zip_uint64_t>(entry_index), ZIP_FL_UNCHANGED, &result);
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
    ZipEntryAttributes result;
    // TODO: errors processing
    ::zip_file_get_external_attributes(zip_, static_cast<zip_uint64_t>(entry_index), ZIP_FL_UNCHANGED, &result.opsys, &result.attributes);
    return result;
}

ZipEntryAttributes ZipFile::file_get_external_attributes(const std::string& entry_name) const
{
    ZipEntryAttributes result;
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
