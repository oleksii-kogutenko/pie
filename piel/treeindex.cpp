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

#include <treeindex.h>

#include <logging.h>
#include <boost_property_tree_ext.hpp>
#include <boost_filesystem_ext.hpp>
#include <boost/lexical_cast.hpp>

namespace pt = boost::property_tree;

namespace piel { namespace lib {

namespace constants {
    struct C {
        static const std::string initial_for;
    };

    /*static*/ const std::string C::initial_for = "initial_for";
};

/*static*/ const std::string PredefinedAttributes::asset_type           = "atype";
/*static*/ const std::string PredefinedAttributes::asset_type__symlink  = "symlink";
/*static*/ const std::string PredefinedAttributes::asset_type__file     = "file";
/*static*/ const std::string PredefinedAttributes::asset_mode           = "amode";
/*static*/ const int PredefinedAttributes::asset_mode_mask              = 0777;
/*static*/ const int PredefinedAttributes::default_asset_mode           = 0666;

TreeIndex::TreeIndex()
    : self_(Asset::create_id(AssetId::not_calculated))
    , parent_()
    , content_()
    , attributes_()
    , content_attributes_()
{
}

TreeIndex::~TreeIndex()
{
}


bool TreeIndex::empty() const
{
    return  parent_.id() == AssetId::empty &&
            content_.empty() &&
            attributes_.empty() &&
            content_attributes_.empty();
}

bool TreeIndex::insert_path(const std::string& index_path, const Asset& asset)
{
    if (asset.id() != AssetId::empty)
    {
        return content_.insert(std::make_pair(index_path, asset)).second;
    }
    else
    {
        // insert can return false
        return false;
    }
}

void TreeIndex::replace_path(const std::string& index_path, const Asset& asset)
{
    if (asset.id() != AssetId::empty)
    {
        content_[index_path] = asset;
    }
    else
    {
        throw errors::attempt_to_add_empty_asset_into_index();
    }
}

bool TreeIndex::contains_path(const std::string& index_path) const
{
    return content_.find(index_path) != content_.end();
}

void TreeIndex::remove_path(const std::string& index_path)
{
    content_.erase(index_path);
}

const TreeIndex::Content& TreeIndex::content() const
{
    return content_;
}

const Asset& TreeIndex::self() const
{
    std::ostringstream os;
    store(os);
    self_ = Asset::create_for(os.str());
    return self_;
}

const Asset& TreeIndex::parent() const
{
    return parent_;
}

void TreeIndex::set_parent(const Asset& parent)
{
    parent_ = parent;
}

void TreeIndex::set_(const std::string& attribute, const std::string& value)
{
    attributes_[attribute] = value;
}

bool TreeIndex::contains_(const std::string& attribute) const
{
    return attributes_.find(attribute) != attributes_.end();
}

bool TreeIndex::is_initial_index() const
{
    return contains_(constants::C::initial_for);
}

void TreeIndex::initial_for(const std::string& new_ref_name)
{
    return set_(constants::C::initial_for, new_ref_name);
}

std::string TreeIndex::get_(const std::string& attribute, const std::string& default_value) const
{
    if (attributes_.find(attribute) == attributes_.end())
    {
        return default_value;
    }
    else
    {
        return attributes_.at(attribute);
    }
}

void TreeIndex::set_attr_(const std::string& index_path, const std::string& attribute, const std::string& value)
{
    ContentAttributes::iterator objs_attrs_iter = content_attributes_.find(index_path);

    if (objs_attrs_iter == content_attributes_.end())
    {
        std::pair<ContentAttributes::iterator,bool> insert_result =
                content_attributes_.insert(std::make_pair(index_path, Attributes()));

        if (insert_result.second)
        {
            objs_attrs_iter = insert_result.first;
        }
    }

    if (objs_attrs_iter == content_attributes_.end())
    {
        // Actually this code never should be reached. But the STL API semantic make it possible theoretically.
        LOG_F << "Can't get objects attributes collection!";

        throw errors::unable_to_get_path_attributes_map();
    }

    objs_attrs_iter->second[attribute] = value;
}

std::string TreeIndex::get_attr_(const std::string& index_path, const std::string& attribute, const std::string& default_value) const
{
    ContentAttributes::const_iterator objs_attrs_iter = content_attributes_.find(index_path);

    if (objs_attrs_iter == content_attributes_.end())
    {
        return default_value;
    }

    Attributes::const_iterator attrs_iter = objs_attrs_iter->second.find(attribute);

    if (attrs_iter == objs_attrs_iter->second.end())
    {
        return default_value;
    }
    else
    {
        return objs_attrs_iter->second.at(attribute);
    }
}

void TreeIndex::set_attrs_(const std::string& index_path, const TreeIndex::Attributes& attrs)
{
    content_attributes_[index_path] = attrs;
}

boost::optional<TreeIndex::Attributes> TreeIndex::get_attrs_(const std::string& index_path) const
{
    ContentAttributes::const_iterator objs_attrs_iter = content_attributes_.find(index_path);

    if (objs_attrs_iter == content_attributes_.end())
    {
        return boost::none;
    }
    else
    {
        return objs_attrs_iter->second;
    }
}

struct SerializationConstants {
    static const std::string parent;
    static const std::string attributes;
    static const std::string content;
    static const std::string content_attributes;
};

const std::string SerializationConstants::parent                = "parent";
const std::string SerializationConstants::attributes            = "attributes";
const std::string SerializationConstants::content               = "content";
const std::string SerializationConstants::content_attributes    = "content_attributes";

// Serialization methods.
void TreeIndex::store(std::ostream& os) const
{
    pt::ptree tree;
    pt::ptree parent;
    pt::ptree content;
    pt::ptree objects_attributes;
    pt::ptree attributes;

    Asset::store(parent, parent_);

    for (Content::const_iterator i = content_.begin(), end = content_.end(); i != end; ++i)
    {
        pt::ptree item;
        Asset::store(item, i->second);
        content.insert(content.end(), std::make_pair(i->first, item));
    }

    for (Attributes::const_iterator i = attributes_.begin(), end = attributes_.end(); i != end; ++i)
    {
        attributes.insert(attributes.end(), std::make_pair(i->first, pt::ptree(i->second)));
    }

    for (ContentAttributes::const_iterator i = content_attributes_.begin(), end = content_attributes_.end(); i != end; ++i)
    {
        pt::ptree object_attributes;
        for (Attributes::const_iterator j = i->second.begin(), end2 = i->second.end(); j != end2; ++j)
        {
            object_attributes.insert(object_attributes.end(), std::make_pair(j->first, pt::ptree(j->second)));
        }
        objects_attributes.insert(objects_attributes.end(), std::make_pair(i->first, object_attributes));
    }

    tree.add_child(SerializationConstants::parent,              parent);
    tree.add_child(SerializationConstants::attributes,          attributes);
    tree.add_child(SerializationConstants::content,             content);
    tree.add_child(SerializationConstants::content_attributes,  objects_attributes);

    pt::write_json(os, tree, false);
    //pt::write_json(os, tree, true);
}

/*static*/ TreeIndex::Ptr TreeIndex::load(std::istream& is, IObjectsStorage *storage)
{
    TreeIndex::Ptr result(new TreeIndex());

    pt::ptree tree;

    pt::read_json(is, tree);

    result->parent_ = Asset::load(tree.get_child(SerializationConstants::parent), storage);

    pt::ptree attributes = tree.get_child(SerializationConstants::attributes);
    for(pt::ptree::const_iterator i = attributes.begin(), end = attributes.end(); i != end; ++i) {
        result->attributes_.insert(std::make_pair(i->first, i->second.data()));
    }

    pt::ptree content = tree.get_child(SerializationConstants::content);
    for(pt::ptree::const_iterator i = content.begin(), end = content.end(); i != end; ++i) {
        pt::ptree item = i->second;
        result->content_.insert(std::make_pair(i->first, Asset::load(item, storage)));
    }

    pt::ptree objects_attributes = tree.get_child(SerializationConstants::content_attributes);
    for(pt::ptree::const_iterator i = objects_attributes.begin(), end = objects_attributes.end(); i != end; ++i) {
        Attributes obj_attrs;
        pt::ptree obj_attrs_tree = i->second;
        for(pt::ptree::const_iterator j = obj_attrs_tree.begin(), end2 = obj_attrs_tree.end(); j != end2; ++j) {
            obj_attrs.insert(std::make_pair(j->first, j->second.data()));
        }
        result->set_attrs_(i->first, obj_attrs);
    }

    return result;
}

/*static*/ TreeIndex::Ptr TreeIndex::load(const Asset& asset, IObjectsStorage *storage)
{
    boost::shared_ptr<std::istream> pis = asset.istream();
    if (pis)
    {
        return load(*pis, storage);
    }
    else
    {
        return TreeIndex::Ptr(new TreeIndex());
    }
}

/*static*/ TreeIndex::Ptr TreeIndex::from_ref(const IObjectsStorage::Ptr& storage, const std::string& ref)
{
    AssetId ref_to_asset_id = storage->resolve(ref);
    if (ref_to_asset_id != AssetId::empty)
    {
        Asset ref_to_asset = storage->asset(ref_to_asset_id);
        return TreeIndex::load(ref_to_asset, storage.get());
    }
    else
    {
        return TreeIndex::Ptr();
    }
}


// Get all assets including Index asset. Method will be used by storage.
std::set<Asset> TreeIndex::assets() const
{
    std::set<Asset> result;
    for (Content::const_iterator i = content_.begin(), end = content_.end(); i != end; ++i)
    {
        result.insert(i->second);
    }
    result.insert(self());
    return result;
}

// Get asset by path
boost::optional<Asset> TreeIndex::asset(const std::string& index_path) const
{
    if (content_.find(index_path) != content_.end())
    {
        return content_.at(index_path);
    }
    else
    {
        return boost::none;
    }
}

// Get all paths
std::set<std::string> TreeIndex::index_paths() const
{
    std::set<std::string> result;
    for (Content::const_iterator i = content_.begin(), end = content_.end(); i != end; ++i)
    {
        if (!result.insert(i->first).second)
        {
            LOG_F << "Not valid index! Content map contains several elements (" << content_.count(i->first) << ") for key " << i->first;

            throw errors::index_has_several_equals_paths();
        }
    }
    return result;
}

/*static*/ std::string PredefinedAttributes::format_asset_mode(int mode)
{
    return boost::lexical_cast<std::string>(mode & asset_mode_mask);
}

/*static*/ int PredefinedAttributes::parse_asset_mode(std::string mode_str, int default_value)
{
    try {
        return boost::lexical_cast<int>(mode_str) & asset_mode_mask;
    }
    catch (const boost::bad_lexical_cast&)
    {
        return default_value & asset_mode_mask;
    }
}

/*static*/ void PredefinedAttributes::fill_symlink_attrs(TreeIndex::Ptr& index, const std::string& index_path, const boost::filesystem::path& file_path)
{
    index->set_attr_(index_path, PredefinedAttributes::asset_type, PredefinedAttributes::asset_type__symlink);

    boost::filesystem::file_status s = boost::filesystem::symlink_status(file_path);
    index->set_attr_(index_path, PredefinedAttributes::asset_mode, format_asset_mode(s.permissions()));
}

/*static*/ void PredefinedAttributes::fill_symlink_attrs(TreeIndex::Ptr& index, const std::string& index_path, boost::shared_ptr<ZipEntry> entry)
{
    index->set_attr_(index_path, PredefinedAttributes::asset_type, PredefinedAttributes::asset_type__symlink);

    index->set_attr_(index_path, PredefinedAttributes::asset_mode, format_asset_mode(entry->attributes().mode()));
}

/*static*/ void PredefinedAttributes::fill_file_attrs(TreeIndex::Ptr& index, const std::string& index_path, const boost::filesystem::path& file_path)
{
    index->set_attr_(index_path, PredefinedAttributes::asset_type, PredefinedAttributes::asset_type__file);

    boost::filesystem::file_status s = boost::filesystem::status(file_path);
    index->set_attr_(index_path, PredefinedAttributes::asset_mode, format_asset_mode(s.permissions()));
}

/*static*/ void PredefinedAttributes::fill_file_attrs(TreeIndex::Ptr& index, const std::string& index_path, boost::shared_ptr<ZipEntry> entry)
{
    index->set_attr_(index_path, PredefinedAttributes::asset_type, PredefinedAttributes::asset_type__file);

    index->set_attr_(index_path, PredefinedAttributes::asset_mode, format_asset_mode(entry->attributes().mode()));
}

} } // namespace piel::lib
