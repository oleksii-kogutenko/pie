/*
 * Copyright (c) 2018, diakovliev
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
 * THIS SOFTWARE IS PROVIDED BY diakovliev ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL diakovliev BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <notificationsfile.h>

#include <fstream>

#include <sstream>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

std::string decode64(const std::string &val) {
    using namespace boost::archive::iterators;
    typedef transform_width<binary_from_base64<std::string::const_iterator>, 8, 6> It;
    std::ostringstream os;
    std::copy(It(val.begin()), It(val.end()), std::ostream_iterator<std::ostringstream::char_type>(os));
    return os.str();
}

std::string encode64(const std::string &val) {
    using namespace boost::archive::iterators;
    typedef base64_from_binary<transform_width<std::string::const_iterator, 6, 8>> It;
    std::string tmp = std::string(It(val.begin()), It(val.end()));
    return tmp.append((3 - val.size() % 3) % 3, '=');
}

namespace piel { namespace lib {

NotificationsFile::NotificationsFile(const std::string& filename)
    : filename_(filename)
{

}

NotificationsFile::~NotificationsFile()
{

}

void NotificationsFile::notify(const std::string& type, const std::string& data)
{
    if (filename_.empty()) return;

    std::ofstream fs;
    fs.open(filename_.c_str(), std::fstream::app);
    fs << type << " " << encode64(data) << std::endl;
    fs.close();
}

} }// namespace piel
