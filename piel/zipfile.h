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

#ifndef ZIPFILE_H
#define ZIPFILE_H

#include <string>
#include <zip.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <queue>

//#define ZIP_ENABLE_SEEK_TELL

namespace piel { namespace lib {

// Forward
struct ZipEntry;
struct ZipSource;

//! ZipEntry extended attributes.
struct ZipEntryAttributes {

    zip_uint8_t opsys;          //!< See libzip zip_file_get_external_attributes function docs for the details.
    zip_uint32_t attributes;    //!< See libzip zip_file_get_external_attributes function docs for the details.

    //! Get entry unix mode.
    //! \return unix mode value.
    //! \sa man stat.
    mode_t mode() const {
        return attributes >> 16;
    }

    //! Check is the entry is a unix symlink.
    //! \return true if the entry is unix symlink, false otherwise.
    bool symlink() const {
        return (ZIP_OPSYS_UNIX == opsys) & S_ISLNK(mode());
    }
};

//! Data source used for ZipEntrySource::istream.
class ZipEntrySource
{
public:
    typedef char char_type;                                     //!< Stream char_type. See boost::istreams docs for the details.
    typedef boost::iostreams::source_tag category;              //!< Stream category. See boost::istreams docs for the details.
    typedef boost::iostreams::stream<ZipEntrySource> istream;   //!< ZipEntry input stream.
    typedef boost::shared_ptr<istream> istream_ptr;             //!< Pointer to ZipEntry input stream.

    //! Constructor.
    //! \param entry ZipEntry for reading data.
    ZipEntrySource(const ZipEntry* entry)
        : entry_(entry)
    {}

    //! Read entry data.
    //! \param buf Data buffer.
    //! \param size Data buffer size.
    //! \return number of readed bytes.
    std::streamsize read(char* buffer, std::streamsize n);

private:
    const ZipEntry* entry_;
};

//! libzip C api wrapper. Encapsulated api to work with whole archive.
class ZipFile
{
public:

    typedef boost::shared_ptr<ZipSource> SourcePtr;   //!< Pointer to ZipSourceEntry.
    typedef boost::shared_ptr<ZipEntry>  EntryPtr;   //!< Pointer to ZipEntry.
    typedef boost::shared_ptr<ZipFile>   FilePtr;    //!< Pointer to ZipFile.
    typedef boost::weak_ptr<ZipFile>     WeakFilePtr;    //!< Pointer to ZipFile.
    typedef std::queue<SourcePtr>        SourceQueue;    //!< Pointer to ZipFile.

    //! Constructor.
    //! Will init internal libzip handle (zip_t*) to archive.
    //! \sa libzip zip_open docs.
    //! \param filename Path to archive.
    ZipFile()
        : libzip_error_(0)
        , zip_()
        , filename_()
        , entries_owner_()
        , flags_()
        , source_queue_()
    {
    }

    //! Copy constructor.
    ZipFile(const ZipFile& src)
        : libzip_error_(src.libzip_error_)
        , zip_(src.zip_)
        , filename_(src.filename_)
        , entries_owner_(src.entries_owner_)
        , flags_(src.flags_)
        , source_queue_(src.source_queue_)
    {
    }

    static FilePtr create(const std::string& filename)
    {
        FilePtr ret_val = FilePtr(new ZipFile(filename));
        ret_val->flags_ = ZIP_CREATE;// | ZIP_TRUNCATE;
        ret_val->zip_ = ::zip_open(ret_val->filename_.c_str(), ret_val->flags_, &ret_val->libzip_error_);
        ret_val->entries_owner_ = ret_val;
        return ret_val;
    }

    static FilePtr open(const std::string& filename)
    {
        FilePtr ret_val = FilePtr(new ZipFile(filename));
        ret_val->flags_ = ZIP_RDONLY;
        ret_val->zip_ = ::zip_open(ret_val->filename_.c_str(), ret_val->flags_, &ret_val->libzip_error_);
        ret_val->entries_owner_ = ret_val;
        return ret_val;
    }

    //! Destructor.
    //! Will free internal libzip handle (zip_t*) to archive.
    //! \sa libzip zip_close docs.
    ~ZipFile()
    {
        ::zip_close(zip_);
    }

    //! Retrieve number of archive entries.
    //! \return Number of archive entries.
    zip_int64_t num_entries() const {
        return ::zip_get_num_entries(zip_, ZIP_FL_UNCHANGED);
    }

    //! Get archive entry name by entry index.
    //! \return archive entry name.
    std::string entry_name(zip_int64_t entryIndex) const {
        return ::zip_get_name(zip_, entryIndex, ZIP_FL_ENC_GUESS);
    }

    //! Construct ZipEntry using entry name.
    //! \return Pointer to ZipEntry.
    EntryPtr entry(const std::string& entry_name);

    //! Construct ZipEntry using entry index.
    //! \return Pointer to ZipEntry.
    EntryPtr entry(zip_int64_t entry_index);

    //! Construct ZipEntry using entry name.
    //! \return Pointer to ZipEntry.
    SourcePtr file_entry(const std::string& entry_name, const std::string& file_name);

    //! Construct ZipEntry using entry index.
    //! \return Pointer to ZipEntry.
    SourcePtr file_entry(zip_int64_t entry_index);

    bool add(const SourcePtr& source);

    void add_file(const std::string& entry_name, const std::string& file_name)
    {
        add(file_entry(entry_name, file_name));
    }
protected:
    //! Constructor.
    //! Will init internal libzip handle (zip_t*) to archive.
    //! \sa libzip zip_open docs.
    //! \param filename Path to archive.
    ZipFile(const std::string& filename)
        : libzip_error_(0)
        , zip_()
        , filename_(filename)
        , entries_owner_()
        , flags_()
        , source_queue_()
    {
    }

    // Frienship is used for proper C api encapsulation. Api related to work
    //with the entries data must be accesible by ZipEntry instance only.
    friend struct ZipEntry;
    friend struct ZipSource;

    //! Low level api. Open libzip zip_file_t handle by entry name.
    //! \param entry_name Archive entry name.
    //! \return libzip zip_file_t handle.
    //! \sa libzip zip_fopen docs.
    zip_file_t* fopen(const std::string& entry_name) const {
        return ::zip_fopen(zip_, entry_name.c_str(), ZIP_FL_UNCHANGED);
    }

    //! Low level api. Open libzip zip_file_t handle by entry index.
    //! \param entry_name Archive entry index.
    //! \return libzip zip_file_t handle.
    //! \sa libzip zip_fopen docs.
    zip_file_t* fopen(zip_int64_t entry_index) const {
        return ::zip_fopen_index(zip_, entry_index, ZIP_FL_UNCHANGED);
    }

    //! Low level api. Close libzip zip_file_t.
    //! \param zip_file Handle to close.
    //! \sa libzip zip_fclose docs, zip_file_t* fopen(zip_int64_t entry_index) const, zip_file_t* fopen(const std::string& entry_name) const
    int fclose(zip_file_t* zip_file) const {
        return ::zip_fclose(zip_file);
    }

#ifdef ZIP_ENABLE_SEEK_TELL
    zip_int8_t fseek(zip_file_t* zip_file, zip_int64_t offset, int whence) const {
        return ::zip_fseek(zip_file, offset, whence);
    }

    zip_int64_t ftell(zip_file_t* zip_file) const {
        return ::zip_ftell(zip_file);
    }
#endif//ZIP_ENABLE_SEEK_TELL

    //! Low level api. Read archive entry data.
    //! \param zip_file zip_file_t handle to read.
    //! \param buf Data buffer.
    //! \param size Data buffer size.
    //! \return number of readed bytes.
    //! \sa libzip zip_read docs.
    zip_int64_t fread(zip_file_t* zip_file, void *buf, zip_int64_t size) const {
        return ::zip_fread(zip_file, buf, size);
    }

    //! Low level api. Get archive entry stat info by entry index.
    //! \param entry_index Archive entry index.
    //! \return libzip zip_stat_t
    zip_stat_t stat(zip_int64_t entry_index) const;

    //! Low level api. Get archive entry stat info by entry name.
    //! \param entry_name Archive entry name.
    //! \return libzip zip_stat_t
    zip_stat_t stat(const std::string& entry_name) const;

    //! Low level api. Get archive entry extended attributes by entry index.
    //! \param entry_name Archive entry index.
    //! \return ZipEntryAttribute struct with extenden attributes.
    ZipEntryAttributes file_get_external_attributes(zip_int64_t entry_index) const;

    //! Low level api. Get archive entry extended attributes by entry name.
    //! \param entry_name Archive entry name.
    //! \return ZipEntryAttribute struct with extenden attributes.
    ZipEntryAttributes file_get_external_attributes(const std::string& entry_name) const;

private:
    int                     libzip_error_;    //!< libzip last error code.
    zip_t                   *zip_;            //!< libzip archive handle.
    std::string             filename_;        //!< stored filename
    ZipFile::WeakFilePtr    entries_owner_;   //!< owner for entries;
    int                     flags_;
    SourceQueue             source_queue_;
};

//! libzip C api wrapper. Encapsulated api to work with archive entry.
struct ZipEntry {

    //! Destructor.
    ~ZipEntry() {
        owner_->fclose(zip_file_);
    }

    //! Read entry data.
    //! \param buf Data buffer.
    //! \param size Data buffer size.
    //! \return number of readed bytes.
    zip_int64_t read(void *buf, zip_int64_t size) const {
        return owner_->fread(zip_file_, buf, size);
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
        return name_;
    }

    //! Zip entry stat.
    //! \return libzip zip_stat_t
    zip_stat_t stat() const {
        return owner_->stat(name_);
    }

    //! Get entry extended attributes.
    //! \return ZipEntryAttribute struct with extenden attributes.
    ZipEntryAttributes attributes() const {
        return owner_->file_get_external_attributes(name_);
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

    // Frienship is used for control ownership model. ZipEntry instance can be
    //created by ZipFile instance only.
    friend class ZipFile;

    //! Reopen file.
    void reopen() {
        owner_->fclose(zip_file_);
        zip_file_ = owner_->fopen(name_);
    }

    //! Constructor.
    //! \param owner ZipFile instance.
    //! \param name zip archive entry name.
    //! \param zip_file zip_file_t libzip handle.
    ZipEntry(ZipFile::WeakFilePtr owner, const std::string& name, zip_file_t* zip_file)
        : owner_(owner.lock())
        , name_(name)
        , zip_file_(zip_file)
    {

    }

private:
    ZipFile::FilePtr owner_;            //!< Reference to zip file.
    std::string name_;                  //!< Entry name.
    zip_file_t* zip_file_;              //!< Internal libzip handle.
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

    zip_source_t *source() const {
        return source_;
    }

    void set_to_be_freed(bool b) {
        to_be_freed_ = b;
    }
protected:

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
    {
        source_ = zip_source_file(owner_->zip_, file_name.c_str(), 0, -1);
    }

private:
    ZipFile::FilePtr        owner_;            //!< Reference to zip file.
    std::string             entry_name_;                  //!< Entry name.
    std::string             file_name_;                  //!< Entry name.
    zip_source_t            *source_;
    bool                    to_be_freed_;
};

} } // namespace piel::lib

//! Helper to create ZipEntry istream ant obtain reference to it.
#define ZIP_ENTRY_ISTREAM(entry) *((entry)->istream().get())

#endif // ZIPFILE_H
