#ifndef ZIPINDEXER_H
#define ZIPINDEXER_H

#include <baseindex.h>
#include <boost/filesystem.hpp>

namespace piel { namespace lib {

namespace fs = boost::filesystem;

class ZipIndexer : public IIndexer
{
public:
    ZipIndexer();
    ~ZipIndexer();

    //! Build zip file index.
    //! \param zip_archive indexed archive.
    //! \return an index.
    BaseIndex build(const fs::path& zip_file) const;

protected:

    //! Format source specification for the zip file entry.
    //! \param zip_file zip archive.
    //! \param entry_name zip archive entry name.
    //! \return zip archive source specification in form: 'zip:\\' + zip_archive + '#' + entry_name.
    std::string zip_source(const fs::path& zip_file, const std::string& entry_name) const;

};

} } // namespace piel::lib

#endif // ZIPINDEXER_H
