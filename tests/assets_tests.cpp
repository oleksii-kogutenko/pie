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

#include <checksumsdigestbuilder.hpp>
#include <asset.h>
#include <index.h>
#include <indexesdiff.h>
#include <memoryobjectsstorage.h>
#include <localdirectorystorage.h>

using namespace piel::lib;

BOOST_AUTO_TEST_CASE(Basic_Assets)
{
    AssetId emptyId;
    BOOST_CHECK(AssetId::base == emptyId);
    BOOST_CHECK(AssetId::base != AssetId::create(""));
    BOOST_CHECK(AssetId::base == AssetId::create("<base>"));

    std::string helloContent = "hello";
    Asset helloAsset = Asset::create_for(helloContent);

    ChecksumsDigestBuilder digestBuilder;
    ChecksumsDigestBuilder::StrDigests str_digests = digestBuilder.str_digests_for(helloContent);

    BOOST_CHECK(AssetId::create(str_digests[Sha256::t::name()]) == helloAsset.id());
    BOOST_CHECK(AssetId::base != helloAsset.id());

    std::string hello;
    (*helloAsset.istream()) >> hello;
    BOOST_CHECK_EQUAL("hello", hello);

    std::string emptyStrContent = "";
    Asset emptyStrAsset = Asset::create_for(emptyStrContent);
    str_digests = digestBuilder.str_digests_for(emptyStrContent);

    BOOST_CHECK(AssetId::create(str_digests[Sha256::t::name()]) == emptyStrAsset.id());
    BOOST_CHECK(AssetId::base != emptyStrAsset.id());

    Index index;

    index.insert("test_path_1/1", helloAsset);
    index.insert("test_path_2/2", emptyStrAsset);

    index.set_attr_("test_path_1/1", "test.attr.1.1", "test_val_1_1");
    index.set_attr_("test_path_1/1", "test.attr.1.2", "test_val_1_2");

    index.set_attr_("test_path_2/2", "test.attr.2.1", "test_val_2_1");
    index.set_attr_("test_path_2/2", "test.attr.2.2", "test_val_2_2");

    BOOST_CHECK_EQUAL("test_val_1_1", index.get_attr_("test_path_1/1", "test.attr.1.1"));
    BOOST_CHECK_EQUAL("test_val_1_2", index.get_attr_("test_path_1/1", "test.attr.1.2"));
    BOOST_CHECK_EQUAL("default_1",    index.get_attr_("test_path_1/1", "test.attr.1.3", "default_1"));

    BOOST_CHECK_EQUAL("test_val_2_1", index.get_attr_("test_path_2/2", "test.attr.2.1"));
    BOOST_CHECK_EQUAL("test_val_2_2", index.get_attr_("test_path_2/2", "test.attr.2.2"));
    BOOST_CHECK_EQUAL("default_2",    index.get_attr_("test_path_2/2", "test.attr.2.3", "default_2"));

    index.set_message_("test message");
    index.set_author_("test_user");
    index.set_email_("author@email");
    index.set_commiter_("test_commiter");
    index.set_commiter_email_("commiter@email");

    index.set_("test attribute 1", "test attribute value 1");
    index.set_("test attribute 2", "test attribute value 2");

    BOOST_CHECK_EQUAL("test message",               index.get_message_());
    BOOST_CHECK_EQUAL("test_user",                  index.get_author_());
    BOOST_CHECK_EQUAL("author@email",               index.get_email_());
    BOOST_CHECK_EQUAL("test_commiter",              index.get_commiter_());
    BOOST_CHECK_EQUAL("commiter@email",             index.get_commiter_email_());

    BOOST_CHECK_EQUAL("test attribute value 1",     index.get_("test attribute 1"));
    BOOST_CHECK_EQUAL("test attribute value 2",     index.get_("test attribute 2"));
    BOOST_CHECK_EQUAL(std::string(),                index.get_("test attribute 3"));
    BOOST_CHECK_EQUAL("test default value 3",       index.get_("test attribute 3", "test default value 3"));

    std::ostringstream test_os;
    index.store(test_os);
    std::string serialized_index = test_os.str();

    std::cout << "index self: " << index.self().id().presentation() << std::endl;
    std::cout << "index data: " << serialized_index << std::endl;

    std::istringstream test_is(serialized_index);

    Index index1 = Index::load(test_is);

    BOOST_CHECK(index1.content().at("test_path_1/1").id() == index.content().at("test_path_1/1").id());
    BOOST_CHECK(index1.content().at("test_path_2/2").id() == index.content().at("test_path_2/2").id());

    BOOST_CHECK_EQUAL("test message",   index1.get_message_());
    BOOST_CHECK_EQUAL("test_user",      index1.get_author_());
    BOOST_CHECK_EQUAL("author@email",   index1.get_email_());
    BOOST_CHECK_EQUAL("test_commiter",  index1.get_commiter_());
    BOOST_CHECK_EQUAL("commiter@email", index1.get_commiter_email_());

    BOOST_CHECK_EQUAL("test attribute value 1",     index1.get_("test attribute 1"));
    BOOST_CHECK_EQUAL("test attribute value 2",     index1.get_("test attribute 2"));
    BOOST_CHECK_EQUAL(std::string(),                index1.get_("test attribute 3"));
    BOOST_CHECK_EQUAL("test default value 3",       index1.get_("test attribute 3", "test default value 3"));

    std::ostringstream test_os1;
    index1.store(test_os1);
    std::string serialized_index1 = test_os1.str();

    std::cout << "index1 self: " << index1.self().id().presentation() << std::endl;
    std::cout << "index1 data: " << serialized_index1 << std::endl;

    BOOST_CHECK_EQUAL(serialized_index1, serialized_index);

    LocalDirectoryStorage storage(boost::filesystem::path("/tmp/test_local_storage"));
    storage.put(index.assets());
    storage.put(std::make_pair("test_reference", index.self().id()));

    Asset s0_asset = storage.asset(helloAsset.id());

    (*s0_asset.istream()) >> hello;
    BOOST_CHECK_EQUAL("hello", hello);

    Asset s1_asset = storage.asset(emptyStrAsset.id());
    Asset i_asset = storage.asset(index1.self().id());

    std::ostringstream osi;
    osi << i_asset.istream()->rdbuf();
    std::string osi_str = osi.str();

    std::cout << "i_asset data: " << osi_str << std::endl;

    BOOST_CHECK_EQUAL(serialized_index1, osi_str);

    Index index2 = Index::load(i_asset);
    std::ostringstream test_os2;
    index2.store(test_os2);
    std::string serialized_index2 = test_os2.str();

    std::cout << "index2 self: " << index2.self().id().presentation() << std::endl;
    std::cout << "index2 data: " << serialized_index2 << std::endl;

    BOOST_CHECK_EQUAL(serialized_index1, serialized_index2);

    BOOST_CHECK_EQUAL("test_val_1_1", index2.get_attr_("test_path_1/1", "test.attr.1.1"));
    BOOST_CHECK_EQUAL("test_val_1_2", index2.get_attr_("test_path_1/1", "test.attr.1.2"));
    BOOST_CHECK_EQUAL("default_1",    index2.get_attr_("test_path_1/1", "test.attr.1.3", "default_1"));

    BOOST_CHECK_EQUAL("test_val_2_1", index2.get_attr_("test_path_2/2", "test.attr.2.1"));
    BOOST_CHECK_EQUAL("test_val_2_2", index2.get_attr_("test_path_2/2", "test.attr.2.2"));
    BOOST_CHECK_EQUAL("default_2",    index2.get_attr_("test_path_2/2", "test.attr.2.3", "default_2"));
}

BOOST_AUTO_TEST_CASE(Indexes_Diff)
{
    std::string helloContent = "hello";
    Asset helloAsset = Asset::create_for(helloContent);

    std::string emptyStrContent = "";
    Asset emptyStrAsset = Asset::create_for(emptyStrContent);

    Index index;

    index.insert("test_path_1/1", helloAsset);
    index.set_attr_("test_path_1/1", "test.attr.1.1", "test_val_1_1");
    index.set_attr_("test_path_1/1", "test.attr.1.2", "test_val_1_2");

    index.insert("test_path_2/2", emptyStrAsset);
    index.set_attr_("test_path_2/2", "test.attr.2.1", "test_val_2_1");
    index.set_attr_("test_path_2/2", "test.attr.2.2", "test_val_2_2");

    index.set_message_("test message");
    index.set_author_("test_user");
    index.set_email_("author@email");
    index.set_commiter_("test_commiter");
    index.set_commiter_email_("commiter@email");
    index.set_("test attribute 1", "test attribute value 1");
    index.set_("test attribute 2", "test attribute value 2");

    Index index1;

    index1.insert("test_path_1/1", helloAsset);
    index1.set_attr_("test_path_1/1", "test.attr.1.1", "test_val_1_1");
    index1.set_attr_("test_path_1/1", "test.attr.1.2", "test_val_1_2");

    index1.insert("test_path_2/2", emptyStrAsset);
    index1.set_attr_("test_path_2/2", "test.attr.2.1", "test_val_2_1");
    index1.set_attr_("test_path_2/2", "test.attr.2.2", "test_val_2_2");

    index1.set_message_("test message");
    index1.set_author_("test_user");
    index1.set_email_("author@email");
    index1.set_commiter_("test_commiter");
    index1.set_commiter_email_("commiter@email");
    index1.set_("test attribute 1", "test attribute value 1");
    index1.set_("test attribute 2", "test attribute value 2");

    IndexesDiff diff1 = IndexesDiff::diff(index, index1);
    BOOST_CHECK(diff1.empty());
}

BOOST_AUTO_TEST_CASE(Indexes_DiffAttrs)
{
    Index index;

    index.set_message_("test message");
    index.set_author_("test_user");
    index.set_email_("author@email");
    index.set_commiter_("test_commiter");
    index.set_commiter_email_("commiter@email");

    Index index1;

    index1.set_message_("test message1");
    index1.set_author_("test_user");
    index1.set_email_("author@email");
    index1.set_commiter_("test_commiter2");
    index1.set_commiter_email_("commiter@email");
    index1.set_("test attribute 1", "test attribute value 1");
    index1.set_("test attribute 2", "test attribute value 2");

    IndexesDiff diff1 = IndexesDiff::diff(index, index1);
    BOOST_CHECK(!diff1.empty());

    std::cout << "Begin diff >>" << std::endl;
    std::cout << diff1.format();
    std::cout << "<< End diff" << std::endl;
}

BOOST_AUTO_TEST_CASE(Indexes_DiffContent)
{
    std::string helloContent = "hello";
    Asset helloAsset = Asset::create_for(helloContent);

    std::string helloContent1 = "hello1";
    Asset helloAsset1 = Asset::create_for(helloContent1);

    std::string emptyStrContent = "";
    Asset emptyStrAsset = Asset::create_for(emptyStrContent);

    Index index;

    index.insert("hello", helloAsset);

    index.set_attr_("hello", "test-attr0-1", "test-val_1");
    index.set_attr_("hello", "test-attr0-2", "test-val_2");

    index.insert("empty", emptyStrAsset);

    index.set_message_("test message");
    index.set_author_("test_user");
    index.set_email_("author@email");
    index.set_commiter_("test_commiter");
    index.set_commiter_email_("commiter@email");

    Index index1;

    index1.insert("hello", helloAsset1);
    index1.insert("empty", emptyStrAsset);
    index1.insert("empty1", emptyStrAsset);

    index1.set_attr_("empty1", "test-eattr-1", "test-eval_1");
    index1.set_attr_("empty1", "test-eattr-2", "test-eval_2");
    index1.set_attr_("hello", "test-attr-1", "test-val_1");
    index1.set_attr_("hello", "test-attr-2", "test-val_2");

    std::ostringstream test_os;
    index1.store(test_os);
    std::string serialized_index1 = test_os.str();

    std::cout << serialized_index1;

    index1.set_message_("test message1");
    index1.set_author_("test_user");
    index1.set_email_("author@email");
    index1.set_commiter_("test_commiter2");
    index1.set_commiter_email_("commiter@email");
    index1.set_("test attribute 1", "test attribute value 1");
    index1.set_("test attribute 2", "test attribute value 2");

    IndexesDiff diff1 = IndexesDiff::diff(index, index1);
    BOOST_CHECK(!diff1.empty());

    std::cout << "Begin diff >>" << std::endl;
    std::cout << diff1.format();
    std::cout << "<< End diff" << std::endl;

    IndexesDiff diff2 = IndexesDiff::diff(index1, index);
    BOOST_CHECK(!diff2.empty());

    std::cout << "Begin diff >>" << std::endl;
    std::cout << diff2.format();
    std::cout << "<< End diff" << std::endl;

}




