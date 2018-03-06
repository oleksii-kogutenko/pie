#define BOOST_TEST_MODULE ZipTests
#include <boost/test/unit_test.hpp>

//#include <fsindexer.h>
//#include <zipindexer.h>
#include <zip.h>
#include <map>
#include <vector>
#include <zipfile.h>
#include "fstream"
#include "logging.h"

#include <boost/format.hpp>

using namespace piel::lib;

static const char* zip_path = "/home/okogutenko/projects/333_zip/";
static const char* zip_file_path = "/home/okogutenko/projects/333_zip/arc.zip";
static const char* zip_file2_path = "/home/okogutenko/projects/333_zip/arc2.zip";
static const char* zip_file3_path = "/home/okogutenko/projects/333_zip/arc3.zip";
static const char* zip_file4_path = "/home/okogutenko/projects/333_zip/arc4.zip";
static const char* zip_file5_path = "/home/okogutenko/projects/333_zip/arc5.zip";
static const char* zip_file6_path = "/home/okogutenko/projects/333_zip/arc6.zip";
typedef std::map<std::string, std::string> mapa;
static mapa files =
{
    {"file1.txt", "/home/okogutenko/projects/333_zip/file1.txt"},
    {"file2.txt", "/home/okogutenko/projects/333_zip/file2.txt"},
    {"file3.txt", "/home/okogutenko/projects/333_zip/file3.txt"}
};

bool check_zip_file_ptr(const ZipFile::FilePtr& zip, const mapa files)
{
    bool ret_val = true;
    for (zip_int64_t i = 0; i < zip->num_entries(); i++)
    {
        ZipFile::EntryPtr   entry       = zip->entry( i );
        ZipEntryAttributes  attrs       = entry->attributes();

        LOGT    << std::string( entry->symlink() ? "s " +  entry->target() + " " : "f " )
                << entry->name()
                << " os:"
                << boost::format( "%1$08x" ) % ( int )attrs.opsys
                << " attributes: "
                << boost::format( "%1$08x" ) % ( int )attrs.attributes
                << " mode: "
                << boost::format( "%1$04o" ) % ( int )( attrs.mode() & 0777 ) << ELOG;

        zip_stat_t entry_status = entry->stat();

        mapa::const_iterator it = files.find(entry_status.name);
        BOOST_CHECK(it != files.end());

        std::ifstream f;
        f.open(it->second.c_str());
        long file_size = f.seekg(0, f.end).tellg();
        f.seekg(0);

        BOOST_CHECK_EQUAL(file_size, entry_status.size);

        std::vector<char> file_buf(file_size), zip_buf(file_size);
        file_buf.reserve(file_size);
        zip_buf.reserve(file_size);

        f.read(file_buf.data(), file_size);
        entry->read(zip_buf.data(), file_size);

        BOOST_CHECK_EQUAL_COLLECTIONS(file_buf.begin(), file_buf.end(), zip_buf.begin(), zip_buf.end());

        f.close();
    }
    return ret_val;
}

BOOST_AUTO_TEST_CASE(Zip_PlainAPI)
{
    LOGI << "---start Zip_PlainAPI---" << ELOG;
    const char* path = zip_file_path;
    int errorp;
    zip_t *zip_create = zip_open(path, ZIP_CREATE | ZIP_TRUNCATE, &errorp);

    if (!zip_create) {
        zip_error_t error;
        zip_error_init_with_code(&error, errorp);
        LOGE << "Error: " << zip_error_strerror(&error) << ELOG;
        zip_error_fini(&error);
        return;
    }
    LOGT << " zip created" << ELOG;

    for (mapa::const_iterator it = files.begin(), end = files.end(); it != end; ++it) {
        zip_source_t * zip_source_zip = zip_source_file(zip_create, it->second.c_str(), 0, -1);
        zip_int64_t ret = zip_file_add(zip_create, it->first.c_str(), zip_source_zip, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
        //zip_source_free(zip_source_zip);
        if (ret < 0) {
            LOGE<< "Error on add zip file" << ELOG;
        }
        LOGT << "The file " << it->second << " added as " << it->first << ELOG;
    }

    zip_close(zip_create);

    LOGI << "---check Zip_PlainAPI---" << ELOG;

    zip_t *zip_check = zip_open(path, ZIP_RDONLY, &errorp);

    if (!zip_check) {
        zip_error_t error;
        zip_error_init_with_code(&error, errorp);
        LOGE << "Open zip file error: " << zip_error_strerror(&error) << ELOG;
        zip_error_fini(&error);
        return;
    }
    LOGT << " zip openned" << ELOG;

    zip_int64_t i, zip_entries = zip_get_num_entries(zip_check, ZIP_FL_UNCHANGED);
    zip_uint64_t entry_index = 0;
    BOOST_CHECK_EQUAL(zip_entries, files.size());

    for (i = 0; i < zip_entries; i++) {
        entry_index = zip_uint64_t(i);
        zip_file_t* entry_file = zip_fopen_index(zip_check, entry_index, ZIP_FL_UNCHANGED);
        zip_stat_t entry_status;

        zip_stat_init(&entry_status);
        int ret = zip_stat_index(zip_check, entry_index, ZIP_FL_UNCHANGED, &entry_status);

        if (ret < 0) {
            LOGE << "Error on read status of " << entry_index << " index" << ELOG;
            continue;
        }
        LOGT << "zip file[" << entry_index << "] " << entry_status.name << " has size " << entry_status.size << ELOG;

        mapa::const_iterator it = files.find(entry_status.name);
        BOOST_CHECK(it != files.end());

        std::ifstream f;
        f.open(it->second.c_str());
        long file_size = f.seekg(0, f.end).tellg();
        f.seekg(0);

        BOOST_CHECK_EQUAL(file_size, entry_status.size);

        std::vector<char> file_buf(file_size), zip_buf(file_size);
        file_buf.reserve(file_size);
        zip_buf.reserve(file_size);

        f.read(file_buf.data(), file_size);
        zip_fread(entry_file, zip_buf.data(), file_size);

        BOOST_CHECK_EQUAL_COLLECTIONS(file_buf.begin(), file_buf.end(), zip_buf.begin(), zip_buf.end());

        f.close();
    }

    zip_close(zip_check);

    LOGI << "---FINISH Zip_PlainAPI---" << ELOG;
}

BOOST_AUTO_TEST_CASE(Zip_CxxAPI_1)
{
    // Create archive
    LOGI << "+++START Zip_CxxAPI_1 +++" << ELOG;
    {
        ZipFile::FilePtr zip = ZipFile::create(zip_file2_path);

        for (mapa::const_iterator it = files.begin(), end = files.end(); it != end; ++it) {
            zip->add(zip->file_entry(it->first, it->second));
            LOGT << "file " << it->second << " added as " << it->first << ELOG;
        }
    }
    {
        ZipFile::FilePtr zip = ZipFile::open(zip_file2_path);
        check_zip_file_ptr(zip, files);
    }
    LOGI << "---FINISH Zip_CxxAPI_1 ---" << ELOG;
}

BOOST_AUTO_TEST_CASE(Zip_CxxAPI_2)
{
    // Create archive
    LOGI << "+++START Zip_CxxAPI_2 +++" << ELOG;
    {
        ZipFile::FilePtr zip = ZipFile::create(zip_file3_path);

        for (mapa::const_iterator it = files.begin(), end = files.end(); it != end; ++it) {
            zip->add_file(it->first, it->second);
        }
    }
    {
        ZipFile::FilePtr zip = ZipFile::open(zip_file3_path);
        check_zip_file_ptr(zip, files);
    }
    std::cout << "---FINISH Zip_CxxAPI_2---" << std::endl;
}

/*
BOOST_AUTO_TEST_CASE(Zip_created)
{
    // Create archive
    std::cout << "+++START free+++" << __LINE__<< std::endl;

    ZipFile::FilePtr zip = ZipFile::create(zip_file4_path);

    std::cout << "---FINISH free---" << __LINE__<< std::endl;
}
/*
BOOST_AUTO_TEST_CASE(Zip_opened)
{
    // Create archive
    std::cout << "+++START openned+++" << __LINE__<< std::endl;

    ZipFile::FilePtr zip = ZipFile::open(zip_file4_path);

    std::cout << "---FINISH openned---" << __LINE__<< std::endl;
}

BOOST_AUTO_TEST_CASE(Zip_MemoryLeak)
{
    const char* path = zip_file6_path;
    int errorp;
    zip_t *zip = zip_open(path, ZIP_CREATE | ZIP_TRUNCATE, &errorp);
    zip_close(zip);

    std::cout << "---FINISH Leak---" << __LINE__<< std::endl;
}

BOOST_AUTO_TEST_CASE(Zip_NoMemoryLeak)
{
    const char* path = zip_file6_path;
    int errorp;
    zip_t *zip = zip_open(path, ZIP_CREATE, &errorp);
    zip_close(zip);

    std::cout << "---FINISH No Leak---" << __LINE__<< std::endl;
}
*/
