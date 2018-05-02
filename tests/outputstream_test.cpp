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

#define BOOST_TEST_MODULE OutPutStreamTest

#include <boost/test/unit_test.hpp>
#include "test_utils.hpp"

#include <logging.h>
#include <datasequencecutter.h>

#include <sstream>
#include <fstream>
#include <vector>

using namespace art::lib;
using namespace piel::lib;

BOOST_AUTO_TEST_CASE(outputstream_check_string_pattern1)
{
    LOGT << "outputstream_check_pattern1" << ELOG;

    DataSequenceCutter os;

    const std::string pattern = "ABCDEFGH";
    boost::shared_ptr<std::istream> ss_ptr1 (new std::istringstream(pattern));

    std::vector<char> buffer(pattern.size());
    buffer.reserve(pattern.size());

    os.push_input_stream(ss_ptr1);

    size_t ret_val = 0;
    while( (ret_val = os.putto(buffer.data(), buffer.size())) ) {
        //LOGT << ret_val << ":" << buffer.data() << " pattern:" << pattern << ELOG;
        BOOST_CHECK(pattern.compare(buffer.data()));
    }
}

BOOST_AUTO_TEST_CASE(outputstream_check_string_patterns)
{
    LOGT << "outputstream_check_string_patterns" << ELOG;

    DataSequenceCutter os;

    const std::string pattern = "ABCDEFGH";
    size_t read_size = pattern.size() -1;

    std::queue<std::string> check_patterns;
    check_patterns.push("ABCDEF");
    check_patterns.push("HABCDE");
    check_patterns.push("FHABCD");
    check_patterns.push("EFH");

    boost::shared_ptr<std::istream> ss_ptr1 (new std::istringstream(pattern));
    boost::shared_ptr<std::istream> ss_ptr2 (new std::istringstream(pattern));
    boost::shared_ptr<std::istream> ss_ptr3 (new std::istringstream(pattern));

    std::vector<char> buffer(read_size);
    buffer.reserve(read_size);

    os.push_input_stream(ss_ptr1);
    os.push_input_stream(ss_ptr2);
    os.push_input_stream(ss_ptr3);
    size_t ret_val = 0;
    while( (ret_val = os.putto(buffer.data(), buffer.size())) ) {
        std::string check_val = check_patterns.front();
        BOOST_CHECK(check_val.compare(buffer.data()));
        check_patterns.pop();
    }
}

BOOST_AUTO_TEST_CASE(outputstream_check_file)
{
    LOGT << "outputstream_check_file" << ELOG;

    DataSequenceCutter os;

    test_utils::TempFileHolder::Ptr test_file = test_utils::create_random_temp_file();


    boost::shared_ptr<std::istream> ios_ptr(new std::ifstream(test_file->first.generic_string()));

    std::vector<char> buffer(test_file->second.size());
    buffer.reserve(test_file->second.size());

    os.push_input_stream(ios_ptr);

    size_t ret_val = 0;
    while( (ret_val = os.putto(buffer.data(), buffer.size())) ) {
        //BOOST_CHECK(test_file->second.compare(buffer.data()));
        BOOST_CHECK_EQUAL_COLLECTIONS(test_file->second.begin(), test_file->second.end(), buffer.begin(), buffer.begin() + ret_val);
    }
}


BOOST_AUTO_TEST_CASE(outputstream_check_files)
{
    LOGT << "outputstream_check_files" << ELOG;

    DataSequenceCutter os;

    test_utils::TempFileHolder::Ptr test_file1 = test_utils::create_random_temp_file();
    test_utils::TempFileHolder::Ptr test_file2 = test_utils::create_random_temp_file();

    boost::shared_ptr<std::istream> ios_ptr1(new std::ifstream(test_file1->first.generic_string()));
    boost::shared_ptr<std::istream> ios_ptr2(new std::ifstream(test_file2->first.generic_string()));

    std::size_t buffer_size = (test_file1->second.size() + test_file2->second.size()) / 5;
    std::vector<char> buffer(buffer_size);
    buffer.reserve(buffer_size);

    os.push_input_stream(ios_ptr1);
    os.push_input_stream(ios_ptr2);

    std::string pattern_to_check = test_file1->second + test_file2->second;

    size_t ret_val = 0;
    size_t offset = 0;
    while( (ret_val = os.putto(buffer.data(), buffer.size())) ) {
        std::string cutted_str = pattern_to_check.substr(offset, ret_val);
        //std::vector<char> cutted_vector(cutted_str.begin(), cutted_str.end());
        //cutted_vector.reserve(ret_val);
        /*if (!cutted_str.compare(buffer.data())) {
            LOGT << "cutted_str:" << cutted_str << ELOG;
            LOGT << "    buffer:" << buffer.data() << ELOG;
        }
        BOOST_CHECK(cutted_str.compare(buffer.data()));
        */
        BOOST_CHECK_EQUAL_COLLECTIONS(cutted_str.begin(), cutted_str.end(), buffer.begin(), buffer.begin() + ret_val);
        //BOOST_TEST(cutted_vector == buffer);
        offset += ret_val;
    }
}

BOOST_AUTO_TEST_CASE(outputstream_check_string_files)
{
    LOGT << "outputstream_check_string_files" << ELOG;

    DataSequenceCutter os;

    const std::string pattern = "ABCDEFGH";
    test_utils::TempFileHolder::Ptr test_file1 = test_utils::create_random_temp_file();
    test_utils::TempFileHolder::Ptr test_file2 = test_utils::create_random_temp_file();

    boost::shared_ptr<std::istream> ss_ptr1 (new std::istringstream(pattern));
    boost::shared_ptr<std::istream> ss_ptr2 (new std::istringstream(pattern));
    boost::shared_ptr<std::istream> ios_ptr1(new std::ifstream(test_file1->first.generic_string()));
    boost::shared_ptr<std::istream> ios_ptr2(new std::ifstream(test_file2->first.generic_string()));

    std::size_t buffer_size = 5;
    std::vector<char> buffer(buffer_size);
    buffer.reserve(buffer_size);

    os.push_input_stream(ss_ptr1);
    os.push_input_stream(ios_ptr1);
    os.push_input_stream(ios_ptr2);
    os.push_input_stream(ss_ptr2);
    std::string pattern_to_check = pattern + test_file1->second + test_file2->second + pattern;

    size_t ret_val = 0;
    size_t offset = 0;
    while( (ret_val = os.putto(buffer.data(), buffer.size())) ) {
        std::string cutted_str = pattern_to_check.substr(offset, ret_val);
        BOOST_CHECK_EQUAL_COLLECTIONS(cutted_str.begin(), cutted_str.end(), buffer.begin(), buffer.begin() + ret_val);
        //BOOST_CHECK(cutted_str.compare(buffer.data()));
        offset += ret_val;
    }
}

