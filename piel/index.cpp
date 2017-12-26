/*
 * Copyright (c) 2017, diakovliev
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
 * THIS SOFTWARE IS PROVIDED BY diakovliev ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL diakovliev BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <index.h>
#include <logging.h>
#include <boost_property_tree_ext.hpp>

namespace pt = boost::property_tree;

namespace piel { namespace lib {

Index::Index()
    : self_(Asset::create_id(AssetId::base))
    , parent_(Asset::create_id(AssetId::base))
    , content_()
    , attributes_()
    , content_attributes_()
{
}

Index::~Index()
{
}

void Index::add(const std::string& index_path, const Asset& asset)
{
    content_.insert(std::make_pair(index_path, asset));
}

const Index::Content& Index::content() const
{
    return content_;
}

const Asset& Index::self() const
{
    std::ostringstream os;
    store(os);
    self_ = Asset::create_for(os.str());
    return self_;
}

const Asset& Index::parent() const
{
    return parent_;
}

void Index::set_(const std::string& attribute, const std::string& value)
{
    attributes_.insert(std::make_pair(attribute, value));
}

std::string Index::get_(const std::string& attribute, const std::string& default_value) const
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

void Index::set_attr_(const std::string& id, const std::string& attribute, const std::string& value)
{
    ContentAttributes::iterator objs_attrs_iter = content_attributes_.find(id);

    if (objs_attrs_iter == content_attributes_.end())
    {
        std::pair<ContentAttributes::iterator,bool> insert_result =
                content_attributes_.insert(std::make_pair(id, Attributes()));

        if (insert_result.second)
        {
            objs_attrs_iter = insert_result.first;
        }
    }

    if (objs_attrs_iter == content_attributes_.end())
    {
        // Fatal!
        LOG_F << "Can't get objects attributes collection!";
        return;
    }

    objs_attrs_iter->second.insert(std::make_pair(attribute, value));
}

std::string Index::get_attr_(const std::string& id, const std::string& attribute, const std::string& default_value) const
{
    ContentAttributes::const_iterator objs_attrs_iter = content_attributes_.find(id);

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

void Index::set_attrs_(const std::string& id, const Index::Attributes& attrs)
{
    content_attributes_.insert(std::make_pair(id, attrs));
}

boost::optional<Index::Attributes> Index::get_attrs_(const std::string& id) const
{
    ContentAttributes::const_iterator objs_attrs_iter = content_attributes_.find(id);

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
void Index::store(std::ostream& os) const
{
    pt::ptree tree;
    pt::ptree parent;
    pt::ptree content;
    pt::ptree attributes;
    pt::ptree objects_attributes;

    Asset::store(parent, parent_);

    for (Content::const_iterator i = content_.begin(), end = content_.end(); i != end; ++i)
    {
        pt::ptree item;
        Asset::store(item, i->second);
        content.insert(content.end(), std::make_pair(i->first, item));
    }

    for (Attributes::const_iterator i = attributes_.begin(), end = attributes_.end(); i != end; ++i)
    {
        attributes.insert(attributes.end(), std::make_pair(i->first, i->second));
    }

    for (ContentAttributes::const_iterator i = content_attributes_.begin(), end = content_attributes_.end(); i != end; ++i)
    {
        pt::ptree object_attributes;
        for (Attributes::const_iterator j = i->second.begin(), end2 = i->second.end(); j != end2; ++j)
        {
            object_attributes.insert(object_attributes.end(), std::make_pair(j->first, j->second));
        }
        objects_attributes.insert(objects_attributes.end(), std::make_pair(i->first, object_attributes));
    }

    tree.add_child(SerializationConstants::parent,              parent);
    tree.add_child(SerializationConstants::attributes,          attributes);
    tree.add_child(SerializationConstants::content,             content);
    tree.add_child(SerializationConstants::content_attributes,  objects_attributes);

    pt::write_json(os, tree, false);
}

/*static*/ Index Index::load(std::istream& is)
{
    Index result;

    pt::ptree tree;

    pt::read_json(is, tree);

    result.parent_ = Asset::load(tree.get_child(SerializationConstants::parent));

    pt::ptree attributes = tree.get_child(SerializationConstants::attributes);
    for(pt::ptree::const_iterator i = attributes.begin(), end = attributes.end(); i != end; ++i) {
        result.attributes_.insert(std::make_pair(i->first, i->second.data()));
    }

    pt::ptree content = tree.get_child(SerializationConstants::content);
    for(pt::ptree::const_iterator i = content.begin(), end = content.end(); i != end; ++i) {
        pt::ptree item = content.get_child(i->first);
        result.content_.insert(std::make_pair(i->first, Asset::load(item)));
    }

    pt::ptree objects_attributes = tree.get_child(SerializationConstants::content_attributes);
    for(pt::ptree::const_iterator i = objects_attributes.begin(), end = objects_attributes.end(); i != end; ++i) {
        Attributes obj_attrs;
        pt::ptree obj_attrs_tree = objects_attributes.get_child(i->first);
        for(pt::ptree::const_iterator j = obj_attrs_tree.begin(), end2 = obj_attrs_tree.end(); j != end2; ++j) {
            obj_attrs.insert(std::make_pair(j->first, j->second.data()));
        }
        result.set_attrs_(i->first, obj_attrs);
    }

    return result;
}

/*static*/ Index Index::load(const Asset& asset)
{
    boost::shared_ptr<std::istream> pis = asset.istream();
    if (pis)
    {
        return load(*pis);
    }
    else
    {
        return Index();
    }
}

// Get all assets including Index asset. Method will be used by storage.
std::set<Asset> Index::assets() const
{
    std::set<Asset> result;
    for (Content::const_iterator i = content_.begin(), end = content_.end(); i != end; ++i)
    {
        result.insert(i->second);
    }
    result.insert(self());
    return result;
}

// Get all paths
std::set<std::string> Index::paths() const
{
    std::set<std::string> result;
    for (Content::const_iterator i = content_.begin(), end = content_.end(); i != end; ++i)
    {
        result.insert(i->first);
    }
    return result;
}

} } // namespace piel::lib
