/*
 * Copyright (c) 2017, Dmytro Iakovliev <email>
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
 * THIS SOFTWARE IS PROVIDED BY Dmytro Iakovliev <email> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Dmytro Iakovliev <email> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef LOGGING_H
#define LOGGING_H

#include <boost/log/trivial.hpp>

#define LOG_(x) BOOST_LOG_TRIVIAL(x) << "[" << __PRETTY_FUNCTION__ << ":" << __LINE__ << "] "
#define LOG_T   LOG_(trace)
#define LOG_D   LOG_(debug)
#define LOG_I   LOG_(info)
#define LOG_W   LOG_(warning)
#define LOG_E   LOG_(error)
#define LOG_F   LOG_(fatal)

#define LOG_T_(x)   LOG_(trace) << x
#define LOG_D_(x)   LOG_(debug) << x
#define LOG_I_(x)   LOG_(info) << x
#define LOG_W_(x)   LOG_(warning) << x
#define LOG_E_(x)   LOG_(error) << x
#define LOG_F_(x)   LOG_(fatal) << x


class Logging
{
};

#endif // LOGGING_H
