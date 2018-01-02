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

#ifndef BOOST_FILESYSTEM_EXT_HPP
#define BOOST_FILESYSTEM_EXT_HPP

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

#include <sstream>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace boost { namespace filesystem {

// Return path when appended to a_From will resolve to same as a_To
inline path make_relative( const path& from_arg, const path& to_arg )
{
    path result;

    path from    = absolute( from_arg );
    path to      = absolute( to_arg );

    path::const_iterator from_iter( from.begin() );
    path::const_iterator to_iter( to.begin() );

    // Find common base
    for( path::const_iterator to_end( to.end() ), from_end( from.end() ) ; from_iter != from_end && to_iter != to_end && *from_iter == *to_iter; ++from_iter, ++to_iter );

    // Navigate backwards in directory to reach previously found base
    for( path::const_iterator from_end( from.end() ); from_iter != from_end; ++from_iter )
    {
        if( (*from_iter) != "." )
            result /= "..";
    }

    // Now navigate down the directory branch
    for( ; to_iter != to.end() ; ++to_iter )
        result /= *to_iter;

    return result;
}

inline boost::shared_ptr<std::istream> istream( const path& from )
{
    return boost::shared_ptr<std::istream>(new std::ifstream(from.string().c_str(), std::ifstream::in|std::ifstream::binary));
}

inline boost::shared_ptr<std::ostream> ostream( const path& from )
{
    return boost::shared_ptr<std::ostream>(new std::ofstream(from.string().c_str(), std::ofstream::out|std::ifstream::binary));
}

// Test utils
inline std::string create_random_string()
{
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(5, 256);
    int count = index_dist(rng);
    std::ostringstream oss;
    for(int i = 0; i < count; ++i) {
        std::string::value_type ch = (std::string::value_type)index_dist(rng);
        oss << ch;
    }
    return oss.str();
}

inline std::pair<path,std::string> create_random_temp_file()
{
    boost::filesystem::path temp_file = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(5, 256);
    int count = index_dist(rng);
    boost::shared_ptr<std::ostream> ofs = boost::filesystem::ostream(boost::filesystem::path(temp_file));
    std::ostringstream oss;
    for(int i = 0; i < count; ++i) {
        std::string::value_type ch = (std::string::value_type)index_dist(rng);
        (*ofs) << ch;
        oss << ch;
    }
    return std::make_pair(temp_file, oss.str());
}

} } // namespace boost::filesystem

#endif // BOOST_FILESYSTEM_EXT_HPP
