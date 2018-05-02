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

#include <setconfig.h>
#include <configcommand.h>

namespace po = boost::program_options;

namespace pie { namespace app {

ConfigCommand::ConfigCommand(Application *app, int argc, char **argv)
    : ICommand(app)
    , argc_(argc)
    , argv_(argv)
    , working_copy_()
{

}

ConfigCommand::~ConfigCommand()
{

}

void ConfigCommand::show_command_help_message(const po::options_description& desc)
{
    std::cerr << "Usage: config --<parameter name> <parameter value>" << std::endl;
    std::cout << desc;
}

int ConfigCommand::perform()
{
    std::string value_to_set_;

    po::options_description desc("Set configuration parameter value");

    typedef std::map<std::string,std::string>::const_iterator ConstIter;
    for (ConstIter iter = piel::cmd::SetConfig::supported().begin(), end = piel::cmd::SetConfig::supported().end(); iter != end; ++iter)
    {
        desc.add_options()(iter->first.c_str(), po::value<std::string>(&value_to_set_), iter->second.c_str());
    }

    if (show_help(desc, argc_, argv_))
    {
        return -1;
    }

    po::variables_map vm;
    po::parsed_options parsed
        = po::command_line_parser(argc_, argv_).options(desc).allow_unregistered().run();
    po::store(parsed, vm);
    po::notify(vm);

    try
    {
        working_copy_ = piel::lib::WorkingCopy::attach(boost::filesystem::current_path());

        for (ConstIter iter = piel::cmd::SetConfig::supported().begin(), end = piel::cmd::SetConfig::supported().end(); iter != end; ++iter)
        {
            if (vm.count(iter->first))
            {
                piel::cmd::SetConfig set_config(working_copy_);

                set_config.set_name(iter->first);
                set_config.set_value(value_to_set_);

                std::cout << "Set " << iter->first << ": " << value_to_set_ << std::endl;

                set_config();
            }
        }
    }
    catch (const piel::cmd::errors::attempt_to_set_unsupported_config&)
    {
        std::cerr << "Attempt to set unsupported configuration parameter!" << std::endl;
        return -1;
    }

    if (!working_copy_->is_valid())
    {
        std::cerr << "Unknown error. Working copy state is invalid." << std::endl;
        return -1;
    }

    return 0;
}


} } // namespace pie::app
