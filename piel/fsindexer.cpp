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

#include <fsindexer.h>
#include <logging.h>
#include <boost_filesystem_ext.hpp>

#include <iostream>
#include <fstream>
#include <queue>

namespace piel { namespace lib {

namespace fs = boost::filesystem;

FsIndexer::FsIndexer()
{
}

FsIndexer::~FsIndexer()
{
}

/*static*/ TreeIndex::Ptr FsIndexer::build(const fs::path& dir, const fs::path& exclude)
{
    if (!is_directory(dir)) {
        LOG_F << dir << " is not a directory!";
        return TreeIndex::Ptr(new TreeIndex());
    }

    TreeIndex::Ptr          result(new TreeIndex());
    std::queue<fs::path>    directories;

    directories.push( dir );

    while( !directories.empty() )
    {
        fs::path p = directories.front();
        directories.pop();

        LOG_T << "d " << p.generic_string();

        for (fs::directory_iterator i = fs::directory_iterator(p), end = fs::directory_iterator(); i != end; i++)
        {
            fs::directory_entry e           = *i;
            fs::path            relative    = fs::make_relative( dir, e.path() );
            std::string         name        = relative.generic_string();

            if ( fs::is_symlink( e.path() ) )
            {
                std::string target = fs::read_symlink( e.path() ).generic_string();

                LOG_T << "s " << name;

                if (result->insert_path(name, Asset::create_for(target)))
                {
                    PredefinedAttributes::fill_symlink_attrs(result, name, e.path());
                }
                else
                {
                    LOG_F << "Can't insert element " << name << " into index! Probably index already have element with such name.";
                }
            }
            else if ( fs::is_regular_file( e.path() ) )
            {
                LOG_T << "f " << name;

                if (result->insert_path(name, Asset::create_for(e.path())))
                {
                    PredefinedAttributes::fill_file_attrs(result, name, e.path());
                }
                else
                {
                    LOG_F << "Can't insert element " << name << " into index! Probably index already have element with such name.";
                }
            }
            else if ( fs::is_directory(e.path()) )
            {
                if ( exclude.empty() || e.path() != exclude )
                {
                    directories.push( e.path() );
                }
                else
                {
                    LOG_T << "exclude directory:" << e.path();
                }
            }
        }
    }

    return result;
}

} } // namespace piel::lib
