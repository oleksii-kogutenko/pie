#include "logbase.h"
#include <iostream>

#define DBG(x)
#define DBG1(x)

namespace piel { namespace lib { namespace logger {

LogBase::LogBase(const std::string& n): name(n)
{
    DBG(std::cout << __PRETTY_FUNCTION__ << ":" << name << std::endl;)
}

LogBase::~LogBase()
{
    DBG(std::cout << __PRETTY_FUNCTION__ << ":" << name << std::endl;)
}

} } } // namespace piel::lib::logger
