/*
 * Copyright (c) 2018, Dmytro Iakovliev daemondzk@gmail.com
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

#include <iostreamsholder.h>

namespace piel { namespace lib {

IOstreamsHolder::IOstreamsHolder()
    : cout_(0), cerr_(0), cin_(0)
{
}

IOstreamsHolder::~IOstreamsHolder()
{
}

std::ostream& IOstreamsHolder::cout() const
{
    if (cout_)
        return *cout_;
    else
        return std::cout;
}

std::ostream& IOstreamsHolder::cerr() const
{
    if (cerr_)
        return *cerr_;
    else
        return std::cerr;
}

std::istream& IOstreamsHolder::cin() const
{
    if (cin_)
        return *cin_;
    else
        return std::cin;
}

void IOstreamsHolder::setup_iostreams(std::ostream *cout, std::ostream *cerr, std::istream *cin)
{
    cout_   = cout;
    cerr_   = cerr;
    cin_    = cin;
}

} } // namespace piel::lib
