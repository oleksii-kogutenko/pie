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
#include <boost_filesystem_ext.hpp>
#include <checksumdigestbuilder.hpp>

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <queue>

namespace piel { namespace lib {

FsIndexer::FsIndexer()
{
}

FsIndexer::~FsIndexer()
{
}

void FsIndexer::enumerate_dir(const fs::path& dir) const
{
    Sha256DigestBuilder sha256_digest_builder;

    if (!is_directory(dir))
        return;

    std::queue<fs::path> directories;
    directories.push(dir);

    while(!directories.empty()) {
        fs::path p = directories.front();
        directories.pop();
        for (fs::directory_iterator i = fs::directory_iterator(p); i != fs::end(i); i++)
        {
            fs::directory_entry e = *i;

            if (fs::is_symlink(e.path()))
            {
                fs::path relative = fs::make_relative(dir, e.path());
                std::string target = fs::read_symlink(e.path()).generic_string();
                //std::cout << "s " << relative.generic_string() << " " << sha256_digest_builder.str_digest_for(target) << std::endl;
                std::cout << relative.generic_string() << "," << sha256_digest_builder.str_digest_for(target) << std::endl;
            }
            else if (fs::is_regular_file(e.path()))
            {
                fs::path relative = fs::make_relative(dir, e.path());
                std::ifstream target(e.path().c_str(), std::ifstream::in|std::ifstream::binary);
                //std::cout << "f " << relative.generic_string() << " " << sha256_digest_builder.str_digest_for(target) << std::endl;
                std::cout << relative.generic_string() << "," << sha256_digest_builder.str_digest_for(target) << std::endl;
            }
            else if (fs::is_directory(e.path()))
            {
                directories.push(e.path());
            }
        }
    }
}

} } // namespace piel::lib
