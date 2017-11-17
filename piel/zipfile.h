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
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//#define ZIP_ENABLE_SEEK_TELL

namespace piel { namespace lib {

// Forward
struct ZipEntry;

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
        : _entry(entry)
    {}

    //! Read entry data.
    //! \param buf Data buffer.
    //! \param size Data buffer size.
    //! \return number of readed bytes.
    std::streamsize read(char* buffer, std::streamsize n);

private:
    const ZipEntry* _entry;
};

//! libzip C api wrapper. Encapsulated api to work with whole archive.
class ZipFile
{
public:

    typedef boost::shared_ptr<ZipEntry> EntryPtr;   //!< Pointer to ZipEntry.

    //! Constructor.
    //! Will init internal libzip handle (zip_t*) to archive.
    //! \sa libzip zip_open docs.
    //! \param filename Path to archive.
    ZipFile(const std::string& filename)
        : _libzip_error(0)
        , _zip()
    {
        _zip = ::zip_open(filename.c_str(), ZIP_RDONLY, &_libzip_error);
    }

    //! Destructor.
    //! Will free internal libzip handle (zip_t*) to archive.
    //! \sa libzip zip_close docs.
    ~ZipFile()
    {
        ::zip_close(_zip);
    }

    //! Retrieve number of archive entries.
    //! \return Number of archive entries.
    zip_int64_t num_entries() const {
        return ::zip_get_num_entries(_zip, ZIP_FL_UNCHANGED);
    }

    //! Get archive entry name by entry index.
    //! \return archive entry name.
    std::string entry_name(zip_int64_t entryIndex) const {
        return ::zip_get_name(_zip, entryIndex, ZIP_FL_ENC_GUESS);
    }

    //! Construct ZipEntry using entry name.
    //! \return Pointer to ZipEntry.
    EntryPtr entry(const std::string& entry_name);

    //! Construct ZipEntry using entry index.
    //! \return Pointer to ZipEntry.
    EntryPtr entry(zip_int64_t entry_index);

protected:

    // Frienship is used for proper C api encapsulation. Api related to work
    //with the entries data must be accesible by ZipEntry instance only.
    friend class ZipEntry;

    //! Low level api. Open libzip zip_file_t handle by entry name.
    //! \param entry_name Archive entry name.
    //! \return libzip zip_file_t handle.
    //! \sa libzip zip_fopen docs.
    zip_file_t* fopen(const std::string& entry_name) const {
        return ::zip_fopen(_zip, entry_name.c_str(), ZIP_FL_UNCHANGED);
    }

    //! Low level api. Open libzip zip_file_t handle by entry index.
    //! \param entry_name Archive entry index.
    //! \return libzip zip_file_t handle.
    //! \sa libzip zip_fopen docs.
    zip_file_t* fopen(zip_int64_t entry_index) const {
        return ::zip_fopen_index(_zip, entry_index, ZIP_FL_UNCHANGED);
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
    int _libzip_error;  //!< libzip last error code.
    zip_t *_zip;        //!< libzip archive handle.
};

//! libzip C api wrapper. Encapsulated api to work with archive entry.
struct ZipEntry {

    //! Destructor.
    ~ZipEntry() {
        _owner->fclose(_zip_file);
    }

    //! Read entry data.
    //! \param buf Data buffer.
    //! \param size Data buffer size.
    //! \return number of readed bytes.
    zip_int64_t read(void *buf, zip_int64_t size) const {
        return _owner->fread(_zip_file, buf, size);
    }

#ifdef ZIP_ENABLE_SEEK_TELL
    zip_int8_t seek(zip_int64_t offset, int whence) const {
        return _owner->fseek(_zip_file, offset, whence);
    }

    zip_int64_t tell() const {
        return _owner->ftell(_zip_file);
    }
#endif//ZIP_ENABLE_SEEK_TELL

    //! Get entry name.
    //! \return Archive entry name.
    std::string name() const {
        return _name;
    }

    //! Zip entry stat.
    //! \return libzip zip_stat_t
    zip_stat_t stat() const {
        return _owner->stat(_name);
    }

    //! Get entry extended attributes.
    //! \return ZipEntryAttribute struct with extenden attributes.
    ZipEntryAttributes attributes() const {
        return _owner->file_get_external_attributes(_name);
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
        _owner->fclose(_zip_file);
        _zip_file = _owner->fopen(_name);
    }

    //! Constructor.
    //! \param owner ZipFile instance.
    //! \param name zip archive entry name.
    //! \param zip_file zip_file_t libzip handle.
    ZipEntry(ZipFile *owner, const std::string& name, zip_file_t* zip_file)
        : _owner(owner)
        , _name(name)
        , _zip_file(zip_file)
    {}

private:
    ZipFile* _owner;        //!< Reference to zip file.
    std::string _name;      //!< Entry name.
    zip_file_t* _zip_file;  //!< Internal libzip handle.
};

} } // namespace piel::lib

//! Helper to create ZipEntry istream ant obtain reference to it.
#define ZIP_ENTRY_ISTREAM(entry) *((entry)->istream().get())

#endif // ZIPFILE_H
