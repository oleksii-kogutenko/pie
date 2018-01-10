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
#include <reset.h>

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
    lib::test_utils::update_temp_dir(wc->working_dir(), wc->metadata_dir());

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
    lib::test_utils::update_temp_dir(wc->working_dir(), wc->metadata_dir());

    cmd::Clean clean(wc);
    clean();
}

BOOST_AUTO_TEST_CASE(reset_workspace)
{
    lib::test_utils::DirState init_state;
    init_state["test_file_1"] = "test file 1 content 1";
    init_state["test_file_2"] = "test file 1 content 2";
    init_state["test_file_3"] = "test file 1 content 3";
    init_state["test_file_4"] = "test file 1 content 4";
    init_state["dir1/test_file_4"] = "test file 1 content 4";
    init_state["dir2/test_file_4"] = "test file 1 content 4";
    init_state["dir3/test_file_4"] = "test file 1 content 4";

    lib::test_utils::TempFileHolder::Ptr wc_path = lib::test_utils::create_temp_dir();

    // Init workspace
    lib::WorkingCopy::Ptr wc = lib::WorkingCopy::init(wc_path->first, ref_name_1);
    lib::test_utils::make_directory_state(wc->working_dir(), wc->metadata_dir(), init_state);

    cmd::Commit commit(wc);
    commit.set_message("Initial commit to " + ref_name_1);
    std::string initial_state_id = commit();

    // Random changes in dir
    lib::test_utils::update_temp_dir(wc->working_dir(), wc->metadata_dir());

    lib::test_utils::DirState random_state = lib::test_utils::get_directory_state(wc->working_dir(), wc->metadata_dir());

    BOOST_CHECK(init_state != random_state);

    cmd::Reset reset(wc, ref_name_1);
    reset();

    lib::test_utils::DirState after_reset_state = lib::test_utils::get_directory_state(wc->working_dir(), wc->metadata_dir());

    BOOST_CHECK(init_state == after_reset_state);

    lib::test_utils::DirState state_1;
    state_1["test_file_1"] = "test file 1 mcontent 1";
    state_1["test_file_2"] = "test file 1 content 2";
    state_1["test_file_3"] = "test file 1 content 3";
    state_1["test_file_4"] = "test file 1 content 4";
    state_1["dir1/test_file_4"] = "test file 1 mcontent 4";
    state_1["dir2/test_file_4"] = "test file 1 content 4";
    state_1["dir3/test_file_4"] = "test file 1 content 4";
    BOOST_CHECK(init_state != state_1);

    lib::test_utils::make_directory_state(wc->working_dir(), wc->metadata_dir(), state_1);

    commit.set_message("Commit to " + ref_name_1);
    commit();

    // Random changes in dir
    lib::test_utils::update_temp_dir(wc->working_dir(), wc->metadata_dir());

    random_state = lib::test_utils::get_directory_state(wc->working_dir(), wc->metadata_dir());

    BOOST_CHECK(init_state != random_state);
    BOOST_CHECK(state_1 != random_state);

    reset();

    after_reset_state = lib::test_utils::get_directory_state(wc->working_dir(), wc->metadata_dir());

    BOOST_CHECK(state_1 == after_reset_state);

    cmd::Reset reset_to_initial(wc, initial_state_id);
    reset_to_initial();

    after_reset_state = lib::test_utils::get_directory_state(wc->working_dir(), wc->metadata_dir());

    BOOST_CHECK(init_state == after_reset_state);
}
