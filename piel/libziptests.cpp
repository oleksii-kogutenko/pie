#include "libziptests.h"

#include <zip.h>
#include <map>
#include <checksumdigestbuilder.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/log/trivial.hpp>

#include <iosfwd>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/concepts.hpp>

LibzipTests::LibzipTests()
{

}

struct ZipEntry;
struct ZipFile {

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

    boost::shared_ptr<ZipEntry> entry(const std::string& entry_name);

    boost::shared_ptr<ZipEntry> entry(zip_int64_t entry_index);

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

//    zip_int8_t fseek(zip_file_t* zip_file, zip_int64_t offset, int whence) const {
//        return ::zip_fseek(zip_file, offset, whence);
//    }

//    zip_int64_t ftell(zip_file_t* zip_file) const {
//        return ::zip_ftell(zip_file);
//    }

    zip_int64_t fread(zip_file_t* zip_file, void *buf, zip_int64_t size) const {
        return ::zip_fread(zip_file, buf, size);
    }

private:
    int _error;
    zip_t *_zip;
};

struct ZipEntry {

    ZipEntry(ZipFile *owner, const std::string& name, zip_file_t* zip_file)
        : _owner(owner)
        , _name(name)
        , _zip_file(zip_file)
    {}

    ~ZipEntry() {
        _owner->fclose(_zip_file);
    }

    zip_int64_t read(void *buf, zip_int64_t size) const {
        return _owner->fread(_zip_file, buf, size);
    }

//    zip_int8_t seek(zip_int64_t offset, int whence) const {
//        return _owner->fseek(_zip_file, offset, whence);
//    }

//    zip_int64_t tell() const {
//        return _owner->ftell(_zip_file);
//    }

    std::string name() const {
        return _name;
    }

private:
    ZipFile* _owner;
    std::string _name;
    zip_file_t* _zip_file;
};

namespace boost {
    namespace iostreams {
        class ZipEntrySource
        {
        public:
            typedef char char_type;
            typedef source_tag category;
            ZipEntrySource(ZipEntry *const entry)
                : _entry(entry)
            {}

            std::streamsize read(char* buffer, std::streamsize n)
            {
                return _entry ? _entry->read(buffer, n) : -1;
            }

        private:
            ZipEntry *const _entry;
        };
    }
}

boost::shared_ptr<ZipEntry> ZipFile::entry(const std::string& entry_name) {
    return boost::shared_ptr<ZipEntry>(new ZipEntry(this, entry_name, fopen(entry_name)));
}

boost::shared_ptr<ZipEntry> ZipFile::entry(zip_int64_t entry_index) {
    return boost::shared_ptr<ZipEntry>(new ZipEntry(this, get_name(entry_index), fopen(entry_index)));
}

void LibzipTests::test_read_archive_content(const std::string& afile)
{
    using namespace piel::lib;

    Sha256Context sha256_context;
    ShaContext sha_context;
    Md5Context md5_context;

    MultiChecksumsDigestBuilder digest_builder;

    ZipFile zip_file(afile);
    for (zip_int64_t i = 0; i < zip_file.num_entries(); i++) {
        boost::shared_ptr<ZipEntry> entry = zip_file.entry(i);
        //BOOST_LOG_TRIVIAL(trace) << entry->name() << std::endl;

        namespace io = boost::iostreams;
        io::stream<io::ZipEntrySource> target(entry.get());

        std::map<std::string, std::string> checksums = digest_builder.str_digests_for(target);
        std::string hash = checksums[sha256_context.name()];

        BOOST_LOG_TRIVIAL(trace) << entry->name() << "," << hash;

//        std::string sha = checksums[sha_context.name()];
//        std::string md5 = checksums[md5_context.name()];
//        BOOST_LOG_TRIVIAL(trace) << "sha: " << sha << " md5: " << md5;
    }
}
