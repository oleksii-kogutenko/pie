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

using namespace piel::lib;

BOOST_AUTO_TEST_CASE(init_working_copy)
{
    test_utils::TempFileHolder::Ptr wc_path = test_utils::create_temp_dir(100);

    BOOST_CHECK_THROW(WorkingCopy::attach(wc_path->first), errors::attach_to_non_working_copy);

    WorkingCopy wc_initialized = WorkingCopy::init(wc_path->first);

    // Second attempt must throw exception
    BOOST_CHECK_THROW(WorkingCopy::init(wc_path->first), errors::init_existing_working_copy);

    WorkingCopy wc_attached = WorkingCopy::attach(wc_path->first);

    wc_initialized.set_config(test_utils::generate_random_printable_string(), test_utils::generate_random_printable_string());
    wc_initialized.set_config(test_utils::generate_random_printable_string(), test_utils::generate_random_printable_string());
    wc_initialized.set_config(test_utils::generate_random_printable_string(), test_utils::generate_random_printable_string());
    wc_initialized.set_config(test_utils::generate_random_printable_string(), test_utils::generate_random_printable_string());

    std::cout << wc_initialized.diff().format();

    wc_initialized.commit("test commit message", "test_reference");

    //wc_initialized.diff().content_diff();
}
