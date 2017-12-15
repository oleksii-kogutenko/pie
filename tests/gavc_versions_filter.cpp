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

#define BOOST_TEST_MODULE GavcVersionsFilter
#include <boost/test/unit_test.hpp>

#include <gavcquery.h>
#include <gavcversionsfilter.h>
#include <gavcversionsmatcher.h>

using namespace art::lib;

void init_versions(std::vector<std::string>& versions)
{
    versions.push_back("1234");
    versions.push_back("1223");
    versions.push_back("12");
    versions.push_back("16.6.123");
    versions.push_back("16.6.536");
}

BOOST_AUTO_TEST_CASE(BasicFiltering_filter)
{
    boost::optional<GavcQuery> op = GavcQuery::parse("test:test:16.6.*");

    BOOST_CHECK(op);

    GavcQuery q = *op;

    GavcVersionsFilter f(*(q.query_version_ops()));

    std::vector<std::string> versions;
    init_versions(versions);

    std::vector<std::string> filtered       = f.filtered(versions);

    BOOST_CHECK_EQUAL(2,            filtered.size());
    BOOST_CHECK_EQUAL("16.6.123",   filtered[0]);
    BOOST_CHECK_EQUAL("16.6.536",   filtered[1]);

    std::vector<std::string> filtered_out   = f.filtered_out(versions);

    BOOST_CHECK_EQUAL(3,            filtered_out.size());
    BOOST_CHECK_EQUAL("1234",       filtered_out[0]);
    BOOST_CHECK_EQUAL("1223",       filtered_out[1]);
    BOOST_CHECK_EQUAL("12",         filtered_out[2]);
}

BOOST_AUTO_TEST_CASE(BasicFiltering_filter_out_all)
{
    boost::optional<GavcQuery> op = GavcQuery::parse("test:test:12.6.*");

    BOOST_CHECK(op);

    GavcQuery q = *op;

    GavcVersionsFilter f(*(q.query_version_ops()));

    std::vector<std::string> versions;
    init_versions(versions);

    std::vector<std::string> filtered       = f.filtered(versions);

    BOOST_CHECK(filtered.empty());

    std::vector<std::string> filtered_out   = f.filtered_out(versions);

    BOOST_CHECK_EQUAL(5,            filtered_out.size());
    BOOST_CHECK_EQUAL("1234",       filtered_out[0]);
    BOOST_CHECK_EQUAL("1223",       filtered_out[1]);
    BOOST_CHECK_EQUAL("12",         filtered_out[2]);
    BOOST_CHECK_EQUAL("16.6.123",   filtered_out[3]);
    BOOST_CHECK_EQUAL("16.6.536",   filtered_out[4]);
}

BOOST_AUTO_TEST_CASE(BasicFiltering_trivial_query)
{
    boost::optional<GavcQuery> op = GavcQuery::parse("test:test:*");

    BOOST_CHECK(op);

    GavcQuery q = *op;

    GavcVersionsFilter f(*(q.query_version_ops()));

    std::vector<std::string> versions;
    init_versions(versions);

    std::vector<std::string> filtered       = f.filtered(versions);

    BOOST_CHECK_EQUAL(5,            filtered.size());
    BOOST_CHECK_EQUAL("1234",       filtered[0]);
    BOOST_CHECK_EQUAL("1223",       filtered[1]);
    BOOST_CHECK_EQUAL("12",         filtered[2]);
    BOOST_CHECK_EQUAL("16.6.123",   filtered[3]);
    BOOST_CHECK_EQUAL("16.6.536",   filtered[4]);

    std::vector<std::string> filtered_out   = f.filtered_out(versions);

    BOOST_CHECK(filtered_out.empty());
}

BOOST_AUTO_TEST_CASE(BasicFiltering_significant_parts)
{
    boost::optional<GavcQuery> op = GavcQuery::parse("test:test:12.6.*.123.+.*");

    BOOST_CHECK(op);

    GavcQuery q = *op;

    GavcVersionsMatcher marcher(*(q.query_version_ops()));
    std::vector<std::string> sparts = marcher.significant_parts("12.6.ods.123.4453.987");

    BOOST_CHECK_EQUAL(3,            sparts.size());
    BOOST_CHECK_EQUAL("ods",        sparts[0]);
    BOOST_CHECK_EQUAL("4453",       sparts[1]);
    BOOST_CHECK_EQUAL("987",        sparts[2]);
}
