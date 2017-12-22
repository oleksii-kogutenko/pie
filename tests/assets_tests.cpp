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
#include <memoryobjectsstorage.h>

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
    index.add("test_path_1/1", helloAsset);
    index.add("test_path_2/2", emptyStrAsset);

    index.set_message_("test message");
    index.set_author_("test_user");
    index.set_email_("author@email");
    index.set_commiter_("test_commiter");
    index.set_commiter_email_("commiter@email");

    index.set_("test attribute 1", "test attribute value 1");
    index.set_("test attribute 2", "test attribute value 2");

    BOOST_CHECK_EQUAL("test message",   index.get_message_());
    BOOST_CHECK_EQUAL("test_user",      index.get_author_());
    BOOST_CHECK_EQUAL("author@email",   index.get_email_());
    BOOST_CHECK_EQUAL("test_commiter",  index.get_commiter_());
    BOOST_CHECK_EQUAL("commiter@email", index.get_commiter_email_());

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

    MemoryObjectsStorage storage;
    storage.put(index1.assets());

    Asset s0_asset = storage.get(helloAsset.id());

    (*s0_asset.istream()) >> hello;
    BOOST_CHECK_EQUAL("hello", hello);

    Asset s1_asset = storage.get(emptyStrAsset.id());
    Asset i_asset = storage.get(index1.self().id());

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
}




