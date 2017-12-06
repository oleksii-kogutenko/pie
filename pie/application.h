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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>

// Forward
class Application;

struct ICommand {
    ICommand(Application *app) : _app(app) {}

    virtual int perform() = 0;

    Application *app() {
        return _app;
    }

protected:
    bool show_help(boost::program_options::options_description& desc, int argc, char **argv);
    virtual void show_command_help_message(const boost::program_options::options_description& desc);

private:
    Application *_app;
};

struct UnknownCommand: public ICommand {
    UnknownCommand(Application *app, int argc, char **argv);
    int perform();
private:
    int _argc;
    char **_argv;
};

struct ICommmandConstructor {
    virtual boost::shared_ptr<ICommand> create(Application *app, int argc, char **argv) const = 0;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
};

template<class Command>
class CommmandConstructor: public ICommmandConstructor {
public:
    CommmandConstructor(const std::string& name, const std::string& description)
        : _name(name)
        , _description(description)
    {}

    boost::shared_ptr<ICommand> create(Application *app, int argc, char **argv) const {
        return boost::shared_ptr<ICommand>(new Command(app, argc, argv));
    }    

    std::string name() const        { return _name;         }
    std::string description() const { return _description;  }

private:
    std::string _name;
    std::string _description;
};

class CommandsFactory {
public:    
    typedef std::map<std::string, boost::shared_ptr<ICommmandConstructor> > Constructors;

    CommandsFactory(Application *app);

    void register_command(ICommmandConstructor *constructor);
    void show_registered_commands() const;

    boost::shared_ptr<ICommand> create(int argc, char **argv);

private:
    Application *_app;
    Constructors _constructors;
};

class Application
{
public:
    Application(int argc, char **argv);
    ~Application();

    int run();
    void register_command(ICommmandConstructor *constructor);
    void show_registered_commands() const;

private:
    int _argc;
    char **_argv;
    CommandsFactory _commands_factory;
};

#endif // APPLICATION_H
