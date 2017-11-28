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

#include <application.h>
#include <iostream>

bool ICommand::show_help(boost::program_options::options_description &desc, int argc, char **argv)
{
    boost::program_options::options_description help_desc("Help options");

    help_desc.add_options()
        ("help,h", "Show help message.")
        ;

    boost::program_options::variables_map vm;
    boost::program_options::parsed_options parsed =
        boost::program_options::command_line_parser(argc, argv).options(help_desc).allow_unregistered().run();
    boost::program_options::store(parsed, vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cout << help_desc;
        std::cout << desc;
        return true;
    }

    return false;
}

UnknownCommand::UnknownCommand(Application *app, int argc, char **argv)
    : ICommand(app)
    , _argc(argc)
    , _argv(argv)
{
}

int UnknownCommand::perform()
{
    std::cerr << "Incorrect command line or unknown command."               << std::endl;

    std::cout << "Usage:"                                                   << std::endl;
    std::cout << "\t" << _argv[0] << " <command name> [command args]"       << std::endl;
    std::cout << "Help:"                                                    << std::endl;
    std::cout << "\t" << _argv[0] << " <command name> --help|-h"            << std::endl;
    std::cout << "\t\t- Will out detailed list of the command arguments."   << std::endl;
    app()->show_registered_commands();
    return -1;
}

CommandsFactory::CommandsFactory(Application *app)
    : _app(app)
    , _constructors()
{
}

void CommandsFactory::register_command(ICommmandConstructor *constructor)
{
    _constructors.insert(std::make_pair(constructor->name(), boost::shared_ptr<ICommmandConstructor>(constructor)));
}

boost::shared_ptr<ICommand> CommandsFactory::create(int argc, char **argv)
{
    // second argument is command name
    if (argc < 2) {
        return boost::shared_ptr<ICommand>(new UnknownCommand(_app, argc, argv));
    }

    Constructors::iterator cmd_iter = _constructors.find(std::string(argv[1]));
    if (cmd_iter == _constructors.end()) {
        return boost::shared_ptr<ICommand>(new UnknownCommand(_app, argc, argv));
    }

    return cmd_iter->second->create(_app, argc - 1, argv + 1);
}

void CommandsFactory::show_registered_commands() const
{
    std::cout << "Commands:" << std::endl;
    Constructors::const_iterator cmd_iter = _constructors.begin();
    Constructors::const_iterator end      = _constructors.end();
    for (;cmd_iter != end; ++cmd_iter) {
        std::cout << "\t" << cmd_iter->second->name()
                  << ":\t" << cmd_iter->second->description()
                  << std::endl;
    }
}

Application::Application(int argc, char **argv)
    : _argc(argc)
    , _argv(argv)
    , _commands_factory(this)
{
}

Application::~Application()
{
}

int Application::run()
{
    boost::shared_ptr<ICommand> command = _commands_factory.create(_argc, _argv);
    return command->perform();
}

void Application::register_command(ICommmandConstructor *constructor)
{
    _commands_factory.register_command(constructor);
}

void Application::show_registered_commands() const
{
    _commands_factory.show_registered_commands();
}
