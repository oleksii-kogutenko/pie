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

#ifndef PIEL_INDEX_H_
#define PIEL_INDEX_H_

#include <iobjectsstorage.h>
#include <boost/format.hpp>

#include <iostream>
#include <map>
#include <set>

namespace piel { namespace lib {

class IndexesDiff;

class Index
{
public:
    typedef std::map<std::string, Asset> Content;
    typedef std::map<std::string, std::string> Attributes;
    typedef std::map<std::string, Attributes> ContentAttributes;

    Index();
    ~Index();

    bool insert(const std::string& index_path, const Asset& asset);
    const Content& content() const;

    const Asset& self() const;
    const Asset& parent() const;

    void set_parent(const Asset& parent);

    void set_(const std::string& attribute, const std::string& value);
    std::string get_(const std::string& attribute, const std::string& default_value = std::string()) const;

#define Index_DECLARE_ATTRIBUTE(x) \
    inline void set_ ## x ## _(const std::string& value)     { set_( #x , value); } \
    inline std::string get_ ## x ## _() const                { return get_( #x, std::string() ); } \

    Index_DECLARE_ATTRIBUTE(message)
    Index_DECLARE_ATTRIBUTE(author)
    Index_DECLARE_ATTRIBUTE(email)
    Index_DECLARE_ATTRIBUTE(commiter)
    Index_DECLARE_ATTRIBUTE(commiter_email)

#undef Index_DECLARE_ATTRIBUTE

    void set_attr_(const std::string& id, const std::string& attribute, const std::string& value);
    std::string get_attr_(const std::string& id, const std::string& attribute, const std::string& default_value = std::string()) const;

    void set_attrs_(const std::string& id, const Attributes& attrs);
    boost::optional<Attributes> get_attrs_(const std::string& id) const;

    // Serialization methods.
    void store(std::ostream& os) const;
    static Index load(std::istream& is, IObjectsStorage *storage = 0);
    static Index load(const Asset& asset, IObjectsStorage *storage = 0);

    // Get all assets including Index asset. Method will be used by storage.
    std::set<Asset> assets() const;

    // Get all paths
    std::set<std::string> paths() const;

private:
    friend class        IndexesDiff;

    mutable Asset       self_;
    Asset               parent_;
    Content             content_;
    Attributes          attributes_;
    ContentAttributes   content_attributes_;

};

//! Predefined attributes constants holder
struct PredefinedAttributes {
    // Asset type
    static const std::string asset_type;
    static const std::string asset_type__symlink;
    static const std::string asset_type__file;

    // Asset mode (unix mode)
    static const std::string asset_mode;

    // Fill predefined attributes
    static void fill_symlink_attrs(Index& index, const std::string& id, const boost::filesystem::path& file_path);
    static void fill_symlink_attrs(Index& index, const std::string& id, boost::shared_ptr<ZipEntry> entry);
    static void fill_file_attrs(Index& index, const std::string& id, const boost::filesystem::path& file_path);
    static void fill_file_attrs(Index& index, const std::string& id, boost::shared_ptr<ZipEntry> entry);

};

//! Indexers interface.
struct IIndexer {
    //! Build index.
    //! \param whatToIndex Path to directory or archive for index.
    Index build(const boost::filesystem::path& whatToIndex);
};

} } // namespace piel::lib

#endif /* PIEL_INDEX_H_ */
