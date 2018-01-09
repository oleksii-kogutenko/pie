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

#include <commit.h>
#include <commitcommand.h>

#include <boost_filesystem_ext.hpp>

namespace pie { namespace app {

namespace po = boost::program_options;

CommitCommand::CommitCommand(Application *app, int argc, char **argv)
    : ICommand(app)
    , argc_(argc)
    , argv_(argv)
    , working_copy_()
{
}

CommitCommand::~CommitCommand()
{
}

void CommitCommand::show_command_help_message(const po::options_description& desc)
{
    std::cerr << "Usage: commit" << std::endl;
    std::cout << desc;
}

int CommitCommand::perform()
{
    po::options_description desc("Commit options");

    if (show_help(desc, argc_, argv_))
    {
        return -1;
    }

    try
    {
        working_copy_ = piel::lib::WorkingCopy::attach(boost::filesystem::current_path());
    }
    catch (piel::lib::errors::attach_to_non_working_copy e)
    {
        std::cerr << "Attempt to perform operation outside of working copy!" << std::endl;
        return -1;
    }

    if (!working_copy_->is_valid())
    {
        std::cerr << "Unknown error. Working copy state is invalid." << std::endl;
        return -1;
    }

    try
    {
        piel::cmd::Commit commit(working_copy_, "test_ref");

        commit.set_message("test message");

        std::string ref = commit();
        std::cout << "Commited: " << ref << std::endl;
    }
    catch (piel::cmd::errors::nothing_to_commit e)
    {
        std::cerr << "No changes!" << std::endl;
        return -1;
    }

    return 0;
}

} } // namespace pie::app
