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

#define BOOST_TEST_MODULE AssetsTests
#include <boost/test/unit_test.hpp>

#include "test_utils.hpp"
#include <checksumsdigestbuilder.hpp>

#include <asset.h>
#include <treeindex.h>

//#include <indexesdiff.h>
//#include <memoryobjectsstorage.h>
//#include <localdirectorystorage.h>

using namespace piel::lib;

BOOST_AUTO_TEST_CASE(empty_asset)
{
    Asset empty;

    BOOST_CHECK(AssetId::empty          == empty);
    BOOST_CHECK(AssetId::not_calculated != empty);
    BOOST_CHECK(AssetId::empty          != AssetId::create(""));
    BOOST_CHECK(AssetId::not_calculated != AssetId::create(""));
}

BOOST_AUTO_TEST_CASE(not_calculated_asset)
{
    Asset not_calculated_asset = Asset::create_id(AssetId::not_calculated);

    BOOST_CHECK_THROW(not_calculated_asset.id(), errors::unable_to_calculate_asset_id);
}

BOOST_AUTO_TEST_CASE(strings_assets)
{
    ChecksumsDigestBuilder digestBuilder;

    for (int i = 0; i < 100; ++i)
    {
        std::string asset_content = test_utils::generate_random_string();
        Asset string_asset = Asset::create_for(asset_content);
        BOOST_CHECK(AssetId::not_calculated != string_asset);

        ChecksumsDigestBuilder::StrDigests str_digests = digestBuilder.str_digests_for(asset_content);
        BOOST_CHECK_EQUAL(str_digests[AssetId::digest_algo], string_asset.id().string());
    }
}

BOOST_AUTO_TEST_CASE(files_assets)
{
    ChecksumsDigestBuilder digestBuilder;

    for (int i = 0; i < 100; ++i)
    {
        test_utils::TempFileHolder::Ptr test_file = test_utils::create_random_temp_file();

        //std::cout << "Random file: " << test_file->first.string() << std::endl;
        //std::cout << "Random content: " << test_file->second << std::endl;

        Asset file_asset = Asset::create_for(test_file->first);
        Asset string_asset = Asset::create_for(test_file->second);

        BOOST_CHECK(AssetId::not_calculated != file_asset);
        BOOST_CHECK(AssetId::not_calculated != string_asset);

        BOOST_CHECK_EQUAL(file_asset.id().string(), string_asset.id().string());

        ChecksumsDigestBuilder::StrDigests str_digests = digestBuilder.str_digests_for(test_file->second);
        BOOST_CHECK_EQUAL(str_digests[AssetId::digest_algo], string_asset.id().string());
    }
}

BOOST_AUTO_TEST_CASE(index_content_insert_contains_replace_remove)
{
    TreeIndex index;

    std::string index_path;
    Asset asset_0  = Asset::create_for(test_utils::generate_random_string());
    Asset asset_1;

    bool inserted = false;
    do {

        index_path = test_utils::generate_random_printable_string();
        asset_1    = Asset::create_for(test_utils::generate_random_string());
        inserted   = index.insert_path(index_path, asset_0);

    } while (!inserted || asset_1 == asset_0);

    BOOST_CHECK(inserted);
    BOOST_CHECK(asset_1 != asset_0);
    BOOST_CHECK(index.contains_path(index_path));
    BOOST_CHECK(!index.insert_path(index_path, asset_1));

    BOOST_CHECK(index.content().at(index_path) == asset_0);

    index.replace_path(index_path, asset_1);

    BOOST_CHECK(index.content().at(index_path) == asset_1);

    index.remove_path(index_path);

    BOOST_CHECK(!index.contains_path(index_path));
}

BOOST_AUTO_TEST_CASE(index_attributes)
{
    TreeIndex index;

    for (int i = 0; i < 100; ++i)
    {
        std::string attr_name = test_utils::generate_random_printable_string();
        std::string attr_value = test_utils::generate_random_printable_string();
        std::string attr_new = attr_value + test_utils::generate_random_printable_string();

        BOOST_CHECK_EQUAL("", index.get_(attr_name));
        BOOST_CHECK_EQUAL(attr_value, index.get_(attr_name, attr_value));

        index.set_(attr_name, attr_value);

        BOOST_CHECK_EQUAL(attr_value, index.get_(attr_name));

        index.set_(attr_name, attr_new);

        BOOST_CHECK_EQUAL(attr_new, index.get_(attr_name));
    }

    // Add some random string assets
    for (int i = 0; i < 100; ++i)
    {
        std::string index_path = test_utils::generate_random_printable_string();

        index.replace_path(index_path, Asset::create_for(test_utils::generate_random_string()));

        std::string attr_name = test_utils::generate_random_printable_string();
        std::string attr_value = test_utils::generate_random_printable_string();
        std::string attr_new = attr_value + test_utils::generate_random_printable_string();

        BOOST_CHECK_EQUAL("", index.get_attr_(index_path, attr_name));
        BOOST_CHECK_EQUAL(attr_value, index.get_attr_(index_path, attr_name, attr_value));

        index.set_attr_(index_path, attr_name, attr_value);

        BOOST_CHECK_EQUAL(attr_value, index.get_attr_(index_path, attr_name));

        index.set_attr_(index_path, attr_name, attr_new);

        BOOST_CHECK_EQUAL(attr_new, index.get_attr_(index_path, attr_name));
    }
}

BOOST_AUTO_TEST_CASE(index_content)
{
    TreeIndex index;
    std::map<std::string, test_utils::TempFileHolder::Ptr> temp_files_holder;

    // Initial state
    BOOST_CHECK(AssetId::empty == index.parent().id());
    BOOST_CHECK(AssetId::empty != index.self().id());

    std::string empty_path = test_utils::generate_random_printable_string();
    BOOST_CHECK_THROW(index.replace_path(empty_path, Asset()), errors::attempt_to_add_empty_asset_into_index);
    BOOST_CHECK(!index.insert_path(empty_path, Asset()));
    BOOST_CHECK(!index.contains_path(empty_path));

    std::set<std::string> added_paths;

    // Add some random string assets
    for (int i = 0; i < 100; ++i)
    {
        std::string index_path = test_utils::generate_random_printable_string();
        index.replace_path(index_path, Asset::create_for(test_utils::generate_random_string()));
        added_paths.insert(index_path);
    }

    // Add some random file assets
    for (int i = 0; i < 100; ++i)
    {
        std::string index_path = test_utils::generate_random_printable_string();
        test_utils::TempFileHolder::Ptr test_file = test_utils::create_random_temp_file();

        // This prevents for delete temp files until leave test scope
        temp_files_holder[index_path] = test_file;

        index.replace_path(index_path, Asset::create_for(test_file->first));
        added_paths.insert(index_path);
    }

    // Check if we do not have empty or not initialized assets
    std::set<Asset> assets = index.assets();
    for (std::set<Asset>::const_iterator i = assets.begin(), end = assets.end(); i != end; ++i)
    {
        BOOST_CHECK(i->id() != AssetId::empty);
        BOOST_CHECK(i->id() != AssetId::not_calculated);
    }

    // Check if we have readable assets for all paths
    std::set<std::string> index_paths = index.index_paths();
    for (std::set<std::string>::const_iterator i = index_paths.begin(), end = index_paths.end(); i != end; ++i)
    {
        BOOST_CHECK(index.asset(*i));
    }

    // Check if we have all what we added
    for (std::set<std::string>::const_iterator i = added_paths.begin(), end = added_paths.end(); i != end; ++i)
    {
        BOOST_CHECK(index.contains_path(*i));
    }

    // Basic serialization
    std::ostringstream oss;
    index.store(oss);
    std::string index_stored_data = oss.str();
    std::istringstream iss(index_stored_data);

    // These 2 indexes contains non readable assets
    TreeIndex index_loaded_from_asset = TreeIndex::load(Asset::create_for(index_stored_data));
    TreeIndex index_loaded_from_stream = TreeIndex::load(iss);

    BOOST_CHECK(index_loaded_from_asset.self() == index_loaded_from_stream.self());

    for (std::set<std::string>::const_iterator i = added_paths.begin(), end = added_paths.end(); i != end; ++i)
    {
        BOOST_CHECK(index_loaded_from_asset.contains_path(*i));
        BOOST_CHECK(index_loaded_from_asset.asset(*i));
        BOOST_CHECK(!index_loaded_from_asset.asset(*i)->istream());

        BOOST_CHECK(index_loaded_from_stream.contains_path(*i));
        BOOST_CHECK(index_loaded_from_stream.asset(*i));
        BOOST_CHECK(!index_loaded_from_stream.asset(*i)->istream());
    }
}
