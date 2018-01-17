#pragma once

#include "baselogger.h"

namespace piel { namespace lib { namespace logger_out {

class ConsoleLogger : public BaseLogger {
public:
    static const char* SFX;

    ConsoleLogger(const std::string& name);
    virtual ~ConsoleLogger();
protected:
    virtual void print(const std::string& str);
    virtual void printErr(const std::string& str);
};

} } } // namespace piel::lib::logger_out
