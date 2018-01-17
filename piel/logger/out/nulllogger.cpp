#include "nulllogger.h"
#include <iostream>

#define DBG(x)
#define DBG1(x)

namespace piel { namespace lib { namespace logger_out {
using namespace std;
NullLogger::NullLogger(const std::string &_name) : LogBase(_name+"_null")
{
    DBG(cout << __PRETTY_FUNCTION__ << " name:" <<  _name << endl;)
}

NullLogger::~NullLogger()
{
    DBG(cout << __PRETTY_FUNCTION__ << endl;)
}

void NullLogger::print(const std::string& str) {
}

} } } // namespace piel::lib::logger_out
