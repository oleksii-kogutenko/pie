#ifndef BASEINDEX_H
#define BASEINDEX_H

#include <string>
#include <map>
#include <boost/filesystem.hpp>

namespace piel { namespace lib {

//! Base data index.
class BaseIndex {
public:
    //! Constructor.
    BaseIndex();

    //! Copy constructor.
    BaseIndex(const BaseIndex& src);

    //! Put element into index.
    //! \param name an index item identifier.
    //! \param hash an index item data checksum (sha256 by default).
    //! \param source an index item source specification.
    void put(const std::string& name, const std::string& hash, const std::string& source);

    //! Check if index is empty.
    //! \return true if the index empty.
    bool empty() const;

    //! Add index into this index.
    BaseIndex& operator+(const BaseIndex& index);

private:

    //! An index data element.
    struct IndexItem {
        std::string name;   //!< An index item name.
        std::string hash;   //!< An index item data hash (sha256 by default).
        std::string source; //!< An index item source specification.
    };

    std::map<std::string, IndexItem> _index; //!< An index data.
};

//! Indexers interface.
struct IIndexer {
    //! Build index.
    //! \param whatToIndex Path to directory or archive for index.
    BaseIndex build(const boost::filesystem::path& whatToIndex);
};

} } // namespace piel::lib

#endif // BASEINDEX_H
