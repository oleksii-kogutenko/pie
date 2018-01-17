#include "consolelogger.h"
#include <iostream>

#define DBG(x)
#define DBG1(x)
namespace piel { namespace lib { namespace logger_out {
using namespace std;

const char* ConsoleLogger::SFX = "_console";

ConsoleLogger::ConsoleLogger(const string& name):BaseLogger(name, SFX)
{
    DBG(cout << __PRETTY_FUNCTION__ << endl;)
}

ConsoleLogger::~ConsoleLogger()
{
    DBG(cout << __PRETTY_FUNCTION__ << "+++" << endl; )
}

void ConsoleLogger::print(const string& str) {

    DBG1(cout << __PRETTY_FUNCTION__ << ":" << str << endl;)
    cout << str;
    cout.flush();
}

void ConsoleLogger::printErr(const std::string& str) {
    DBG1(cout << __PRETTY_FUNCTION__ << ":" << str << endl;)
    cerr << str;
    cerr.flush();
}

} } } // namespace piel::lib::logger_out
