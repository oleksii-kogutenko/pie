#pragma once

#include <string>
#include <list>
#include <boost/shared_ptr.hpp>
#include "logmessage.h"

namespace piel { namespace lib { namespace logger {

class Log;
typedef boost::shared_ptr<Log> LogPtr;
typedef std::list<LogPtr> LogPtrList;

class Log {
public:
    virtual bool isDebugEnabled() = 0;
    virtual bool isErrorEnabled() = 0;
    virtual bool isFatalEnabled() = 0;
    virtual bool isInfoEnabled() = 0;
    virtual bool isTraceEnabled() = 0;
    virtual bool isWarnEnabled() = 0;

    virtual void trace(const std::string& var1) = 0;
    virtual void debug(const std::string& var1) = 0;
    virtual void info(const std::string& var1) = 0;
    virtual void warn(const std::string& var1) = 0;
    virtual void error(const std::string& var1) = 0;
    virtual void fatal(const std::string& var1) = 0;

    virtual void printMessage(const LogMessage & m) = 0;
    virtual std::string getName() = 0;

    Log() {};
    virtual ~Log() {};

protected:
    virtual void print(const std::string& var1) = 0;
    virtual void printWarn(const std::string& var1) = 0;
    virtual void printErr(const std::string& var1) = 0;

protected:
    std::string name;
};

} } } // namespace piel::lib::logger
