#include "zipfile.h"

namespace piel { namespace lib {

ZipFile::EntryPtr ZipFile::entry(const std::string& entry_name)
{
    return ZipFile::EntryPtr(new ZipEntry(this, entry_name, fopen(entry_name)));
}

ZipFile::EntryPtr ZipFile::entry(zip_int64_t entry_index)
{
    return ZipFile::EntryPtr(new ZipEntry(this, get_name(entry_index), fopen(entry_index)));
}

zip_stat_t ZipFile::stat(zip_int64_t entry_index) const
{
    zip_stat_t result;
    ::zip_stat_init(&result);
    // TODO: errors processing
    ::zip_stat_index(_zip, entry_index, ZIP_FL_UNCHANGED, &result);
    return result;
}

zip_stat_t ZipFile::stat(const std::string& entry_name) const
{
    zip_stat_t result;
    ::zip_stat_init(&result);
    // TODO: errors processing
    ::zip_stat(_zip, entry_name.c_str(), ZIP_FL_UNCHANGED, &result);
    return result;
}

ZipEntryAttributes ZipFile::file_get_external_attributes(zip_int64_t entry_index) const
{
    ZipEntryAttributes result = { 0 };
    // TODO: errors processing
    ::zip_file_get_external_attributes(_zip, entry_index, ZIP_FL_UNCHANGED, &result.opsys, &result.attributes);
    return result;
}

ZipEntryAttributes ZipFile::file_get_external_attributes(const std::string& entry_name) const
{
    ZipEntryAttributes result = { 0 };
    // TODO: errors processing
    zip_stat_t zip_stat = stat(entry_name);
    ::zip_file_get_external_attributes(_zip, zip_stat.index, ZIP_FL_UNCHANGED, &result.opsys, &result.attributes);
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
    return _entry ? _entry->read(buffer, n) : -1;
}

} } // namespace piel::lib
