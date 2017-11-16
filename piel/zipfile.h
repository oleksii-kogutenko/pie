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

struct ZipEntry;

struct ZipEntryAttributes {
/*
#define ZIP_OPSYS_DOS	  	0x00u
#define ZIP_OPSYS_AMIGA	 	0x01u
#define ZIP_OPSYS_OPENVMS	0x02u
#define ZIP_OPSYS_UNIX	  	0x03u
#define ZIP_OPSYS_VM_CMS	0x04u
#define ZIP_OPSYS_ATARI_ST	0x05u
#define ZIP_OPSYS_OS_2		0x06u
#define ZIP_OPSYS_MACINTOSH	0x07u
#define ZIP_OPSYS_Z_SYSTEM	0x08u
#define ZIP_OPSYS_CPM	  	0x09u
#define ZIP_OPSYS_WINDOWS_NTFS	0x0au
#define ZIP_OPSYS_MVS	  	0x0bu
#define ZIP_OPSYS_VSE	  	0x0cu
#define ZIP_OPSYS_ACORN_RISC	0x0du
#define ZIP_OPSYS_VFAT	  	0x0eu
#define ZIP_OPSYS_ALTERNATE_MVS	0x0fu
#define ZIP_OPSYS_BEOS	  	0x10u
#define ZIP_OPSYS_TANDEM	0x11u
#define ZIP_OPSYS_OS_400	0x12u
#define ZIP_OPSYS_OS_X	  	0x13u
#define ZIP_OPSYS_DEFAULT	ZIP_OPSYS_UNIX
*/
    zip_uint8_t opsys;
    zip_uint32_t attributes;

    mode_t mode() const {
        return attributes >> 16;
    }

    bool symlink() const {
        return (ZIP_OPSYS_UNIX == opsys) & S_ISLNK(mode());
    }
};

//! Data source used for ZipEntrySource::istream.
class ZipEntrySource
{
public:
    typedef char char_type;
    typedef boost::iostreams::source_tag category;
    typedef boost::iostreams::stream<ZipEntrySource> istream; //!< ZipEntry input stream.
    typedef boost::shared_ptr<istream> istream_ptr;

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

class ZipFile
{
public:

    typedef boost::shared_ptr<ZipEntry> EntryPtr;

    ZipFile(const std::string& filename)
        : _error(0)
        , _zip()
    {
        _zip = ::zip_open(filename.c_str(), ZIP_RDONLY, &_error);
    }

    ~ZipFile()
    {
        ::zip_close(_zip);
    }

    zip_int64_t num_entries() const {
        return ::zip_get_num_entries(_zip, ZIP_FL_UNCHANGED);
    }

    std::string entry_name(zip_int64_t entryIndex) const {
        return ::zip_get_name(_zip, entryIndex, ZIP_FL_ENC_GUESS);
    }

    EntryPtr entry(const std::string& entry_name);

    EntryPtr entry(zip_int64_t entry_index);

protected:
    friend class ZipEntry;

    std::string get_name(zip_int64_t entry_index) const {
        return ::zip_get_name(_zip, entry_index, ZIP_FL_ENC_GUESS);
    }

    zip_file_t* fopen(const std::string& entry_name) const {
        return ::zip_fopen(_zip, entry_name.c_str(), ZIP_FL_UNCHANGED);
    }

    zip_file_t* fopen(zip_int64_t entry_index) const {
        return ::zip_fopen_index(_zip, entry_index, ZIP_FL_UNCHANGED);
    }

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

    zip_int64_t fread(zip_file_t* zip_file, void *buf, zip_int64_t size) const {
        return ::zip_fread(zip_file, buf, size);
    }

    zip_stat_t stat(zip_int64_t entry_index) const;

    zip_stat_t stat(const std::string& entry_name) const;

    ZipEntryAttributes file_get_external_attributes(zip_int64_t entry_index) const;

    ZipEntryAttributes file_get_external_attributes(const std::string& entry_name) const;

private:
    int _error;
    zip_t *_zip;
};


struct ZipEntry {

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
    std::string name() const {
        return _name;
    }

    //! Zip entry stat.
    zip_stat_t stat() const {
        return _owner->stat(_name);
    }

    //! Get entry extended attributes.
    ZipEntryAttributes attributes() const {
        return _owner->file_get_external_attributes(_name);
    }

    //! Get unix mode.
    mode_t mode() const {
        ZipEntryAttributes attrs = attributes();
        return attrs.mode();
    }

    //! Check if the entry is symlink.
    bool symlink() const {
        ZipEntryAttributes attrs = attributes();
        return attrs.symlink();
    }

    //! Read symlink target.
    //! \sa bool symlink() const
    std::string target();

    //! Reopen file.
    bool reopen() {
        _owner->fclose(_zip_file);
        _zip_file = _owner->fopen(_name);
    }

    //! Construct input stream.
    ZipEntrySource::istream_ptr istream();

protected:
    friend class ZipFile;

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

#define ZIP_ENTRY_ISTREAM(entry) *((entry)->istream().get())

#endif // ZIPFILE_H
