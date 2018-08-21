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

#include <iostream>
#include <cstdlib>
#include <gavccommand.h>
#include <gavccacheinitcommand.h>
#include "gavccacheinit.h"
#include <gavccache.h>
#include <logging.h>
#include <mavenmetadata.h>

#include <boost/bind.hpp>
#include <boost_property_tree_ext.hpp>
#include <boost/filesystem.hpp>

namespace pie { namespace app {

namespace pt = boost::property_tree;
namespace po = boost::program_options;

const int default_age = 180;

GavcCacheInitCommand::GavcCacheInitCommand(Application *app, int argc, char **argv)
    : ICommand(app)
    , argc_(argc)
    , argv_(argv)
    , cache_path_(utils::get_default_cache_path())
{
}

GavcCacheInitCommand::~GavcCacheInitCommand()
{
}

void GavcCacheInitCommand::show_command_help_message(const po::options_description& desc)
{
    std::cerr << "Usage: gavccacheclean [options]" << std::endl;
    std::cout << desc;
}

bool GavcCacheInitCommand::parse_arguments()
{
    po::options_description desc("Query options");
    desc.add_options()
        ("cache-path", po::value<std::string>(&cache_path_), (std::string("Cache path. Can be set using GAVC_CACHE environment variable. Default: ") + utils::get_default_cache_path()).c_str())
        ;

    if (show_help(desc, argc_, argv_)) {
        return false;
    }

    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(argc_, argv_).options(desc).allow_unregistered().run();
    po::store(parsed, vm);
    try {
        po::notify(vm);
    } catch (...) {
        show_command_help_message(desc);
        return false;
    }

    get_from_env(vm, "cache",       "GAVC_CACHE",                   cache_path_);

    return true;
}

/*virtual*/ int GavcCacheInitCommand::perform()
{
    int result = -1;

    if (!parse_arguments()) {
        return result;
    }

    try {
        {
            piel::cmd::GAVCCacheInit gavccachecheinit(
                             cache_path_);

            gavccachecheinit();
        }

    }
    catch (piel::cmd::errors::cant_find_version_for_query& ) {
        std::cerr << "Can't find any version for query!" << std::endl;
        return -1;
    }
    catch (piel::cmd::errors::gavc_download_file_error& ) {
        std::cerr << "Can't download file!" << std::endl;
        return -1;
    }
    catch (piel::cmd::errors::cache_folder_does_not_exist& e) {
        std::cerr << "cache_folder_does_not_exist: " << e.folder << "!" << std::endl;
        return -1;
    }

    result = 0;

    return result;
}

} } // namespace pie::app
