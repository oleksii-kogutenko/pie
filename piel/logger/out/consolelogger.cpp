#include "consolelogger.h"
#include <iostream>

namespace piel { namespace lib { namespace logger_out {
using namespace std;

const char* ConsoleLogger::SFX = "_console";

ConsoleLogger::ConsoleLogger(const string& name)
    : BaseLogger(name, SFX)
{}

ConsoleLogger::~ConsoleLogger()
{}

void ConsoleLogger::print(const string& str)
{
    cout << str;
    cout.flush();
}

void ConsoleLogger::printErr(const std::string& str)
{
    cerr << str;
    cerr.flush();
}

} } } // namespace piel::lib::logger_out
