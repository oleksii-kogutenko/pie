/*
 * Copyright (c) 2017, <copyright holder> <email>
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
 * THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <fsindexer.h>
#include <baseindex.h>

#include <boost_filesystem_ext.hpp>
#include <checksumdigestbuilder.hpp>

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <queue>

namespace piel { namespace lib {

FsIndexer::FsIndexer()
{
}

FsIndexer::~FsIndexer()
{
}

std::string FsIndexer::fs_source(const fs::path& item) const
{
    return std::string("file://").append(fs::absolute(item).native());
}

BaseIndex FsIndexer::build_dir_index(const fs::path& dir) const
{
    BaseIndex result;

    if (!is_directory(dir)) {
        BOOST_LOG_TRIVIAL(debug) << dir << " is not a directory!";
        return result;
    }

    Sha256DigestBuilder sha256_digest_builder;

    std::queue<fs::path> directories;
    directories.push(dir);

    while(!directories.empty()) {
        fs::path p = directories.front();
        directories.pop();

        BOOST_LOG_TRIVIAL(trace) << "d " << p.generic_string();

        for (fs::directory_iterator i = fs::directory_iterator(p); i != fs::end(i); i++)
        {
            fs::directory_entry e = *i;

            if (fs::is_symlink(e.path()))
            {
                fs::path relative = fs::make_relative(dir, e.path());

                std::string name = relative.generic_string();
                std::string target = fs::read_symlink(e.path()).generic_string();
                std::string hash = sha256_digest_builder.str_digest_for(target);

                BOOST_LOG_TRIVIAL(trace) << "s " << name << " " << hash;

                result.put(name, hash, fs_source(e.path()));
            }
            else if (fs::is_regular_file(e.path()))
            {
                fs::path relative = fs::make_relative(dir, e.path());

                std::string name = relative.generic_string();
                std::ifstream target(e.path().c_str(), std::ifstream::in|std::ifstream::binary);
                std::string hash = sha256_digest_builder.str_digest_for(target);

                BOOST_LOG_TRIVIAL(trace) << "f " << name << " " << hash << std::endl;

                result.put(name, hash, fs_source(e.path()));
            }
            else if (fs::is_directory(e.path()))
            {
                directories.push(e.path());
            }
        }
    }
}

} } // namespace piel::lib
