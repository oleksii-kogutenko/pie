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

const Asset& Index::self()
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

// Serialization methods.
void Index::store(std::ostream& os) const
{
    // TODO: Rework!
    //
    //  As the only one valid ID is data checksum, here I must have fully own storing objects implementation.
    // Serialization result must be fully predictable and must not depends from platform, third party library
    // or from something else.

    pt::ptree tree;
    pt::ptree parent;
    pt::ptree content;
    pt::ptree attributes;

    Asset::store(parent, parent_);
    for (Content::const_iterator i = content_.begin(), end = content_.end(); i != end; ++i)
    {
        pt::ptree item;
        Asset::store(item, i->second);
        content.add_child(i->first, item);
    }
    for (Attributes::const_iterator i = attributes_.begin(), end = attributes_.end(); i != end; ++i)
    {
        attributes.add(i->first, i->second);
    }

    tree.add_child("parent", parent);
    tree.add_child("content", content);
    tree.add_child("attributes", attributes);

    pt::write_json(os, tree);

}

void Index::load(std::istream& is)
{
    pt::ptree tree;

    pt::read_json(is, tree);

    parent_ = Asset::load(tree.get_child("parent"));

    content_.clear();
    pt::ptree content = tree.get_child("content");
    for(pt::ptree::const_iterator i = content.begin(), end = content.end(); i != end; ++i) {
        pt::ptree item = content.get_child(i->first);
        content_.insert(std::make_pair(i->first, Asset::load(item)));
    }

    attributes_.clear();
    pt::ptree attributes = tree.get_child("attributes");
    for(pt::ptree::const_iterator i = attributes.begin(), end = attributes.end(); i != end; ++i) {
        attributes_.insert(std::make_pair(i->first, i->second.data()));
    }
}

// Get all assets including Index asset. Method will be used by storage.
std::set<Asset> Index::assets() const
{

}

} } // namespace piel::lib
