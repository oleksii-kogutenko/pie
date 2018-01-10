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

#define BOOST_TEST_MODULE WorkingCopyTests
#include <boost/test/unit_test.hpp>

#include <test_utils.hpp>

#include <workingcopy.h>
#include <indextofsexporter.h>

#include <commit.h>
#include <create.h>
#include <clean.h>
#include <checkout.h>

namespace cmd=piel::cmd;
namespace lib=piel::lib;

std::string ref_name_1 = "test_reference_1";
std::string ref_name_2 = "test_reference_2";

BOOST_AUTO_TEST_CASE(init_working_copy)
{
    lib::test_utils::TempFileHolder::Ptr wc_path = lib::test_utils::create_temp_dir(100);

    BOOST_CHECK_THROW(lib::WorkingCopy::attach(wc_path->first), lib::errors::attach_to_non_working_copy);

    lib::WorkingCopy::Ptr wc_initialized = lib::WorkingCopy::init(wc_path->first, ref_name_1);

    // Second attempt must throw exception
    BOOST_CHECK_THROW(lib::WorkingCopy::init(wc_path->first, ref_name_1), lib::errors::init_existing_working_copy);
}

BOOST_AUTO_TEST_CASE(initial_commit)
{
    lib::test_utils::TempFileHolder::Ptr wc_path = lib::test_utils::create_temp_dir(100);

    lib::WorkingCopy::Ptr wc = lib::WorkingCopy::init(wc_path->first, ref_name_1);

    cmd::Commit commit(wc);
    commit();
}

BOOST_AUTO_TEST_CASE(create_new_ref)
{
    lib::test_utils::TempFileHolder::Ptr wc_path = lib::test_utils::create_temp_dir(100);

    lib::WorkingCopy::Ptr wc = lib::WorkingCopy::init(wc_path->first, ref_name_1);

    cmd::Commit commit_1(wc);
    commit_1.set_message("Initial commit to " + ref_name_1);
    commit_1();

    // Random changes in dir
    lib::test_utils::update_temp_dir(wc_path->first, wc->metadata_dir());

    commit_1.set_message("Commit to " + ref_name_1);
    commit_1();

    cmd::Create create(wc, ref_name_2);
    create();

    cmd::Commit commit_2(wc);
    commit_2.set_message("Initial commit to " + ref_name_2);
    commit_2();

    cmd::Checkout checkout_1(wc, ref_name_1);
    checkout_1();

    cmd::Checkout checkout_2(wc, ref_name_2);
    checkout_2();

    // Random changes in dir
    lib::test_utils::update_temp_dir(wc_path->first, wc->metadata_dir());

    cmd::Clean clean(wc);
    clean();
}
