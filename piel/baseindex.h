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

    std::map<std::string, IndexItem> index_; //!< An index data.
};

//! Indexers interface.
struct IIndexer {
    //! Build index.
    //! \param whatToIndex Path to directory or archive for index.
    BaseIndex build(const boost::filesystem::path& whatToIndex);
};

} } // namespace piel::lib

#endif // BASEINDEX_H
