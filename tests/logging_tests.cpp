/*
 * Copyright (c) 2018, Oleksii Kogutenko oleksii.kogutenko@gmail.com
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

#define BOOST_TEST_MODULE LoggingTests
#include <boost/test/unit_test.hpp>
#include "test_utils.hpp"

#include "logger/dispatcher/logdispatcher.h"
#include "logger/app/logapp.h"
#include "logger/dispatcher/logging.h"

#include <locale>
#include <time.h>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <string>

using namespace piel::lib;
using namespace logger_app;
using namespace logger_dispatcher;


BOOST_AUTO_TEST_CASE(logging_simple)
{
    std::cout << "start test+++\n";

    LogAppPtr log1 = Logging::create_logger("test1.1");
    LogAppPtr log2 = Logging::create_logger("test1.2");

    log1->info("info\n");
    log1->debug("debug\n");
    log2->error("error\n");
    log2->warn("warn\n");

    std::cout << "end test+++\n";
}

BOOST_AUTO_TEST_CASE(logging_simple_2)
{
    std::cout << "start 2 test+++\n";

    LogAppPtr log1 = Logging::create_logger("test2");
    log1 << "t" << 5 << trace;
    log1 << "d" << 4 << debug;
    log1 << "i" << 3 << info;
    log1 << "w" << 2 << warn;
    log1 << "e" << 1 << error;
    log1 << "f" << 0 << fatal;

    std::cout << "end 2 test+++\n";
}

BOOST_AUTO_TEST_CASE(logging_simple_3)
{
    std::cout << "start 3 test+++\n";
    {
        LogAppPtr log1 = Logging::create_logger("test3");
        log1 << "t" << 5 << trace;
        log1 << "d" << 4 << debug;
        log1 << "i" << 3 << info;
        log1 << "w" << 2 << warn;
        log1 << "e" << 1 << error;
        log1 << "f" << 0 << fatal;
    }
    {
        LogAppPtr log2 = Logging::create_logger("test3");
        log2 << "2t" << 5 << trace;
        log2 << "2d" << 4 << debug;
        log2 << "2i" << 3 << info;
        log2 << "2w" << 2 << warn;
        log2 << "2e" << 1 << error;
        log2 << "2f" << 0 << fatal;
    }
    std::cout << "end 3 test+++\n";
}

BOOST_AUTO_TEST_CASE(logging_simple_4)
{
    std::cout << "start 4 test+++\n";

    LogAppPtr log1 = Logging::create_logger("test4");

    for (int i=0; i < 10000; i++) {
        log1 << "t" << 5 << "i:" << i << trace;
        log1 << "d" << 4 << "i:" << i << debug;
        log1 << "i" << 3 << "i:" << i << info;
        log1 << "w" << 2 << "i:" << i << warn;
        log1 << "e" << 1 << "i:" << i << error;
        log1 << "f" << 0 << "i:" << i << fatal;
    }

    std::cout << "end 4 test+++\n";
}

