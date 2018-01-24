#include "logbase.h"
#include <iostream>

namespace piel { namespace lib { namespace logger {

LogBase::LogBase(const std::string& n)
    : name(n)
{}

LogBase::~LogBase()
{}

} } } // namespace piel::lib::logger
