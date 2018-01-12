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

#include <checkoutcommand.h>
#include <checkout.h>

#include <boost_filesystem_ext.hpp>

namespace pie { namespace app {

namespace po = boost::program_options;

CheckoutCommand::CheckoutCommand(Application *app, int argc, char **argv)
    : ICommand(app)
    , argc_(argc)
    , argv_(argv)
    , working_copy_()
    , ref_()
{
}

CheckoutCommand::~CheckoutCommand()
{
}

void CheckoutCommand::show_command_help_message(const po::options_description& desc)
{
    std::cerr << "Usage: checkout [--create] <ref>" << std::endl;
    std::cout << desc;
}

int CheckoutCommand::perform()
{
    po::options_description ref_opts;
    ref_opts.add_options()
        ("ref",         po::value<std::string>(&ref_),    "Reference name to checkout.");
        ;

    po::options_description opts;
    opts.add_options()
        ("create,b",    po::value<std::string>(&ref_),    "Create new tree based on current state.");
        ;

    po::options_description desc("Checkout reference options");
    desc.add(ref_opts).add(opts);

    if (show_help(desc, argc_, argv_))
    {
        return -1;
    }

    po::variables_map vm;
    po::parsed_options parsed
        = po::command_line_parser(argc_, argv_).options(opts).allow_unregistered().run();
    po::store(parsed, vm);
    po::notify(vm);

    if (!vm.count("create"))
    {
        po::positional_options_description pos_desc;
        pos_desc.add("ref",     1);

        parsed = po::command_line_parser(argc_, argv_).options(ref_opts).positional(pos_desc).allow_unregistered().run();
        po::store(parsed, vm);
        po::notify(vm);
    }

    try
    {
        working_copy_ = piel::lib::WorkingCopy::attach(boost::filesystem::current_path());

        piel::cmd::Checkout checkout(working_copy_, ref_);

        checkout.create_new_branch(vm.count("create"));

        std::string checkout_result = checkout();

        std::cout << ref_ <<  ":" << checkout_result << std::endl;
    }
    catch (const piel::lib::errors::attach_to_non_working_copy& e)
    {
        std::cerr << "No working copy detected at " <<  boost::filesystem::current_path() << "!" << std::endl;
        return -1;
    }
    catch (const piel::lib::errors::unable_to_find_reference_file& e)
    {
        std::cerr << "Unable to find reference file at working copy!" << std::endl;
        return -1;
    }
    catch (const piel::cmd::errors::no_such_reference& e)
    {
        std::cerr << "Can't find reference " << ref_ << " for checkout! Please use -b option if you want to create new tree based on current state." << std::endl;
        return -1;
    }
    catch (const piel::cmd::errors::there_are_non_commited_changes& e)
    {
        std::cerr << "Can't checkout " << ref_ << ". There are non commit changes in workspace!" << std::endl;
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
