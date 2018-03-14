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


#include "test_utils.hpp"

#include <workingcopy.h>
#include <assetsextractor.h>

#include <commit.h>
#include <create.h>
#include <clean.h>
#include <checkout.h>
#include <reset.h>

#include <treeenumerator.h>
#include <treeindexenumerator.h>
#include "test_utils.hpp"

#define DBOOST_AUTO_TEST_CASE(x) void x(void)

namespace cmd=piel::cmd;
namespace lib=piel::lib;
namespace fs=boost::filesystem;
namespace tst=lib::test_utils;

static std::string ref_name_1 = "test_reference_1";
static std::string ref_name_2 = "test_reference_2";

static const std::string zip_name = "arc.zip";

typedef std::map<std::string, tst::TempFileHolder::Ptr> mapa;
static const int files_count = 5;

BOOST_AUTO_TEST_CASE(Zip_PlainAPI)
{
    LOGI << "---start Zip_PlainAPI---" << ELOG;
    boost::filesystem::path exclude = "";

    tst::TempFileHolder::Ptr tmp_dir = tst::create_temp_dir(files_count);
    tst::DirState tmp_dir_state = tst::get_directory_state(tmp_dir->first, exclude);

    boost::filesystem::path zip_path = tmp_dir->first / zip_name;
    std::string path = zip_path.string();

    LOGT << "ARC name: " << path << ELOG;

    int errorp;
    zip_t *zip_create = ::zip_open(path.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorp);

    if (!zip_create) {
        zip_error_t error;
        ::zip_error_init_with_code(&error, errorp);
        LOGE << "Error: " << zip_error_strerror(&error) << ELOG;
        ::zip_error_fini(&error);
        return;
    }
    LOGT << " zip created" << ELOG;

    for (tst::DirState::const_iterator it = tmp_dir_state.begin(), end = tmp_dir_state.end(); it != end; ++it) {

        LOGT << "adding " << it->first << ":::" << it->second << ELOG;
        boost::filesystem::path file_path = tmp_dir->first / it->first;
        zip_source_t * zip_source_zip = ::zip_source_file(zip_create, file_path.string().c_str(), 0, -1);
        zip_int64_t ret = ::zip_file_add(zip_create, it->first.c_str(), zip_source_zip, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);

        if (ret < 0) {
            LOGE<< "Error on add zip file" << ELOG;
        }
        LOGT << "The file " << file_path.string() << " added as " << it->first << ELOG;
    }

    ::zip_close(zip_create);

    LOGT << "---check Zip_PlainAPI---" << ELOG;

    zip_t *zip_check = ::zip_open(path.c_str(), ZIP_RDONLY, &errorp);

    if (!zip_check) {
        zip_error_t error;
        ::zip_error_init_with_code(&error, errorp);
        LOGE << "Open zip file error: " << zip_error_strerror(&error) << ELOG;
        ::zip_error_fini(&error);
        return;
    }
    LOGT << " zip openned" << ELOG;

    zip_int64_t i, zip_entries = ::zip_get_num_entries(zip_check, ZIP_FL_UNCHANGED);
    zip_uint64_t entry_index = 0;
    BOOST_CHECK_EQUAL(zip_entries, tmp_dir_state.size());

    for (i = 0; i < zip_entries; i++) {
        entry_index = zip_uint64_t(i);
        zip_stat_t entry_status;

        ::zip_stat_init(&entry_status);
        int ret = ::zip_stat_index(zip_check, entry_index, ZIP_FL_UNCHANGED, &entry_status);

        if (ret < 0) {
            LOGE << "Error on read status of " << entry_index << " index" << ELOG;
            continue;
        }
        LOGT << "zip file[" << entry_index << "] " << entry_status.name << " has size " << entry_status.size << ELOG;

        zip_file_t* entry_file = ::zip_fopen_index(zip_check, entry_index, ZIP_FL_UNCHANGED);

        tst::DirState::const_iterator it = tmp_dir_state.find(entry_status.name);
        BOOST_CHECK(it != tmp_dir_state.end());

        std::string file_buf = it->second;
        zip_uint64_t file_size = file_buf.size();

        BOOST_CHECK_EQUAL(file_size, entry_status.size);

        std::vector<char> zip_buf(file_size);
        zip_buf.reserve(file_size);

        ::zip_fread(entry_file, zip_buf.data(), file_size);

        BOOST_CHECK_EQUAL_COLLECTIONS(file_buf.begin(), file_buf.end(), zip_buf.begin(), zip_buf.end());

        ::zip_fclose(entry_file);
    }

    ::zip_close(zip_check);

    LOGI << "---FINISH Zip_PlainAPI---" << ELOG;
}

BOOST_AUTO_TEST_CASE(Zip_CxxAPI_1)
{
    // Create archive
    LOGI << "+++START Zip_CxxAPI_1 +++" << ELOG;

    boost::filesystem::path exclude = "";

    tst::TempFileHolder::Ptr tmp_dir = tst::create_temp_dir(files_count);
    tst::DirState tmp_dir_state = tst::get_directory_state(tmp_dir->first, exclude);

    boost::filesystem::path zip_path = tmp_dir->first / zip_name;
    std::string path = zip_path.string();

    LOGT << "ARC name: " << path << ELOG;

    {
        lib::ZipFile::FilePtr zip = lib::ZipFile::create(path);

        for (tst::DirState::const_iterator it = tmp_dir_state.begin(), end = tmp_dir_state.end(); it != end; ++it) {
            boost::filesystem::path file_path = tmp_dir->first / it->first;

            zip->add_file(it->first, file_path.string());
            LOGT << "file " << it->first << " added as " << file_path.string() << ELOG;
        }
    }
    {
        lib::ZipFile::FilePtr zip = lib::ZipFile::open(path);

        for (zip_int64_t i = 0; i < zip->num_entries(); i++)
        {
            lib::ZipFile::EntryPtr   entry       = zip->entry( i );
            lib::ZipEntryAttributes  attrs       = entry->attributes();

            LOGT    << std::string( entry->symlink() ? "s " +  entry->target() + " " : "f " )
                    << entry->name()
                    << " os:"
                    << boost::format( "%1$08x" ) % static_cast<int>(attrs.opsys)
                    << " attributes: "
                    << boost::format( "%1$08x" ) % static_cast<int>(attrs.attributes)
                    << " mode: "
                    << boost::format( "%1$04o" ) % static_cast<int>( attrs.mode() & 0777 ) << ELOG;

            zip_stat_t entry_status = entry->stat();

            tst::DirState::const_iterator it = tmp_dir_state.find(entry_status.name);
            BOOST_CHECK(it != tmp_dir_state.end());

            std::string file_buf = it->second;
            zip_uint64_t file_size = file_buf.size();

            BOOST_CHECK_EQUAL(file_size, entry_status.size);

            std::vector<char> zip_buf(file_size);
            zip_buf.reserve(file_size);

            entry->read(zip_buf.data(), static_cast<zip_int64_t>(file_size));

            BOOST_CHECK_EQUAL_COLLECTIONS(file_buf.begin(), file_buf.end(), zip_buf.begin(), zip_buf.end());
        }

    }

    LOGI << "---FINISH Zip_CxxAPI_1 ---" << ELOG;
}

tst::DirState gen_dir_state()
{
    tst::DirState init_state;
    init_state["test_file_1"] = "test file 1 content 1";
    init_state["test_file_2"] = "test file 2 content 2";
    init_state["test_file_3"] = "test file 3 content 3";
    init_state["test_file_4"] = "test file 4 content 4";
    init_state["dir1/test_file_4"] = "dir1/test file 4 content 4";
    init_state["dir2/test_file_5"] = "dir2/test file 5 content 5";
    init_state["dir3/test_file_6"] = "dir3/test file 6 content 6";
    return  init_state;
}

lib::WorkingCopy::Ptr gen_wc(tst::TempFileHolder::Ptr wc_path)
{
    tst::DirState dir_state = gen_dir_state();
    // Init workspace
    lib::WorkingCopy::Ptr wc = lib::WorkingCopy::init(wc_path->first, ref_name_1);
    tst::make_directory_state(wc->working_dir(), wc->metadata_dir(), dir_state);

    cmd::Commit commit(wc);
    commit.set_message("Initial commit to " + ref_name_1);
    std::string initial_state_id = commit();
    return wc;
}

void gen_zip_from_wc(const std::string& path, lib::WorkingCopy::Ptr wc)
{
    lib::ZipFile::FilePtr zip = lib::ZipFile::create(path);

    lib::TreeEnumerator treeEnumerator(wc->local_storage(), wc->current_tree_state());
    while (treeEnumerator.next())
    {
        lib::TreeIndexEnumerator enumerator(treeEnumerator.index);
        while (enumerator.next())
        {
            LOGD << "enumerator.path: " << enumerator.path << ELOG;
            LOGT << lib::test_utils::istream_content(enumerator.asset.istream()) << ELOG;
            if (zip->add_file(enumerator.path, (wc->working_dir() / enumerator.path).string(), 0777) < 0) {
                LOGE << "Error to add " << enumerator.path << " file to zip!" << ELOG;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(enumerator_test)
{
    LOGI  << "---start enumerator_test---" << ELOG;

    // Generate zip file
    tst::TempFileHolder::Ptr wc_path = tst::create_temp_dir();
    tst::TempFileHolder::Ptr zip_path = tst::create_temp_dir(0);
    zip_path->first /= zip_name;
    lib::WorkingCopy::Ptr wc = gen_wc(wc_path);

    gen_zip_from_wc(zip_path->first.string(), wc);

    // Check zip file
    LOGT << "---Check zip file " << zip_path->first.string() << " ---" << ELOG;

    lib::ZipFile::FilePtr zip = lib::ZipFile::open(zip_path->first.string());

    lib::TreeEnumerator treeEnumerator(wc->local_storage(), wc->current_tree_state());
    while (treeEnumerator.next())
    {
        lib::TreeIndexEnumerator enumerator(treeEnumerator.index);
        LOGT << "treeEnumerator.index->self().id().string():" << treeEnumerator.index->self().id().string() << ELOG;
        while (enumerator.next())
        {
            LOGT << "\t" << enumerator.path << ":"
                    << enumerator.asset.id().string() << ELOG;

            LOGT << lib::test_utils::istream_content(enumerator.asset.istream()) << ELOG;

            lib::ZipFile::EntryPtr entry = zip->entry( enumerator.path );
            BOOST_CHECK(entry);
            if (!entry) {
                LOGE << "Emptry entry for " << enumerator.path << ELOG;
                continue;
            }

            boost::shared_ptr<std::istream> asset_istream_ptr = enumerator.asset.istream();
            size_t asset_stream_size = static_cast<size_t>(asset_istream_ptr->gcount());

            std::vector<char> zip_buf(asset_stream_size), asset_buf(asset_stream_size);
            zip_buf.reserve(asset_stream_size);
            asset_buf.reserve(asset_stream_size);

            entry->read(zip_buf.data(), static_cast<zip_int64_t>(asset_stream_size));
            asset_istream_ptr->read(asset_buf.data(), static_cast<std::streamsize>(asset_stream_size));

            BOOST_CHECK_EQUAL_COLLECTIONS(asset_buf.begin(), asset_buf.end(), zip_buf.begin(), zip_buf.end());
        }
    }
    LOGI  << "---FINISH enumerator_test---" << ELOG;
}

BOOST_AUTO_TEST_CASE(Zip_CxxAPI_buffer_1)
{
    // Create archive
    LOGI << "+++START Zip_CxxAPI_buffer_1 +++" << ELOG;

    tst::TempFileHolder::Ptr tmp_dir = tst::create_temp_dir();
    std::string path = tmp_dir->first.string() + zip_name;

    LOGT << "ARC name: " << path << ELOG;

    std::string test_str = "Test string";

    const char* fname = "buffer.txt";
    {
        lib::ZipFile::FilePtr zip = lib::ZipFile::create(path);

        zip->add_buffer(fname, test_str.c_str(), test_str.size());
    }

    {
        lib::ZipFile::FilePtr zip = lib::ZipFile::open(path);

        for (zip_int64_t i = 0; i < zip->num_entries(); i++)
        {
            lib::ZipFile::EntryPtr   entry       = zip->entry( i );

            zip_stat_t entry_status = entry->stat();

            BOOST_CHECK(strcmp(entry_status.name, fname) == 0);
            int ret = 0;
            if ((ret = strcmp(entry_status.name, fname)) != 0) {
                LOGE << entry_status.name << "!=" << fname << ELOG;
                LOGE << ret << "=" << ret << ELOG;
                break;
            }

            zip_uint64_t file_size = test_str.size();

            BOOST_CHECK_EQUAL(file_size, entry_status.size);
            if (file_size != entry_status.size) {
                LOGE << file_size << "!=" << entry_status.size << ELOG;
                break;
            }

            std::vector<char> zip_buf(file_size);
            zip_buf.reserve(file_size);

            entry->read(zip_buf.data(), static_cast<zip_int64_t>(file_size));

            BOOST_CHECK_EQUAL_COLLECTIONS(test_str.begin(), test_str.end(), zip_buf.begin(), zip_buf.end());
        }

    }
    LOGI  << "---FINISH Zip_CxxAPI_buffer_1---" << ELOG;
}

BOOST_AUTO_TEST_CASE(Zip_CxxAPI_buffer_istream)
{
    // Create archive
    LOGI << "+++START Zip_CxxAPI_buffer_istream +++" << ELOG;
    tst::TempFileHolder::Ptr tmp_dir = tst::create_temp_dir();
    std::string path = tmp_dir->first.string() + zip_name;

    LOGT << "ARC name: " << path << ELOG;

    std::string test_str = "Test string";
    lib::ZipFile::IStreamPtr test_stream_ptr = lib::ZipFile::IStreamPtr(new std::istringstream(test_str) );
    //std::istringstream test_stream(test_str);

    const char* fname = "buffer.txt";
    {
        lib::ZipFile::FilePtr zip = lib::ZipFile::create(path);

        zip->add_istream(fname, test_stream_ptr);
    }

    {
        lib::ZipFile::FilePtr zip = lib::ZipFile::open(path);

        for (zip_int64_t i = 0; i < zip->num_entries(); i++)
        {
            lib::ZipFile::EntryPtr   entry       = zip->entry( i );

            zip_stat_t entry_status = entry->stat();

            BOOST_CHECK(strcmp(entry_status.name, fname) == 0);
            int ret = 0;
            if ((ret = strcmp(entry_status.name, fname)) != 0) {
                LOGE << entry_status.name << "!=" << fname << ELOG;
                LOGE << ret << "=" << ret << ELOG;
                break;
            }

            zip_uint64_t file_size = test_str.size();

            BOOST_CHECK_EQUAL(file_size, entry_status.size);
            if (file_size != entry_status.size) {
                LOGE << file_size << "!=" << entry_status.size << ELOG;
                break;
            }

            std::vector<char> zip_buf(file_size);
            zip_buf.reserve(file_size);

            entry->read(zip_buf.data(), static_cast<zip_int64_t>(file_size));

            BOOST_CHECK_EQUAL_COLLECTIONS(test_str.begin(), test_str.end(), zip_buf.begin(), zip_buf.end());
        }

    }

    LOGI  << "---FINISH Zip_CxxAPI_buffer_istream---" << ELOG;
}

DBOOST_AUTO_TEST_CASE(Zip_created)
{
    // Create archive
    LOGI << "+++START Zip_created +++" << ELOG;

    tst::TempFileHolder::Ptr tmp_dir = tst::create_temp_dir();
    boost::filesystem::path zip_path = tmp_dir->first / zip_name;

    lib::ZipFile::FilePtr zip = lib::ZipFile::create(zip_path.string());

    LOGI  << "---FINISH Zip_created---" << ELOG;
}

DBOOST_AUTO_TEST_CASE(Zip_opened)
{
    // Create archive
    LOGI << "+++START Zip_opened +++" << ELOG;

    tst::TempFileHolder::Ptr tmp_dir = tst::create_temp_dir();
    boost::filesystem::path zip_path = tmp_dir->first / zip_name;

    lib::ZipFile::FilePtr zip = lib::ZipFile::open(zip_path.string());

    LOGI  << "---FINISH Zip_opened---" << ELOG;
}

DBOOST_AUTO_TEST_CASE(Zip_MemoryLeak)
{
    LOGI << "+++START Zip_MemoryLeak +++" << ELOG;

    tst::TempFileHolder::Ptr tmp_dir = tst::create_temp_dir();
    boost::filesystem::path zip_path = tmp_dir->first / zip_name;

    int errorp;
    zip_t *zip = ::zip_open(zip_path.string().c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorp);
    zip_close(zip);

    LOGI  << "---FINISH Zip_MemoryLeak---" << ELOG;
}

DBOOST_AUTO_TEST_CASE(Zip_NoMemoryLeak)
{
    LOGI << "+++START Zip_NoMemoryLeak +++" << ELOG;

    tst::TempFileHolder::Ptr tmp_dir = tst::create_temp_dir();
    boost::filesystem::path zip_path = tmp_dir->first / zip_name;

    int errorp;
    zip_t *zip = zip_open(zip_path.string().c_str(), ZIP_CREATE, &errorp);
    zip_close(zip);

    LOGI  << "---FINISH Zip_NoMemoryLeak---" << ELOG;
}

