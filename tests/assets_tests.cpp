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
}
