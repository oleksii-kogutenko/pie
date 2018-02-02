#pragma once

#include <string>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include "logapp_types.h"
#include "../dispatcher/logdispatcher_types.h"

namespace piel { namespace lib { namespace logger_app {
class LogApp;

typedef LogApp& (*LogAppManipulator)(LogApp&);

LogApp& trace(LogApp& val);
LogApp& debug(LogApp& val);
LogApp& info(LogApp& val);
LogApp& warn(LogApp& val);
LogApp& error(LogApp& val);
LogApp& fatal(LogApp& val);

class LogApp {
    //friend function info;
    friend LogApp& trace(LogApp& val);
    friend LogApp& debug(LogApp& val);
    friend LogApp& info(LogApp& val);
    friend LogApp& warn(LogApp& val);
    friend LogApp& error(LogApp& val);
    friend LogApp& fatal(LogApp& val);
protected:
    void clear();
public:
    void trace(const std::string& var1);
    void debug(const std::string& var1);
    void info(const std::string& var1);
    void warn(const std::string& var1);
    void error(const std::string& var1);
    void fatal(const std::string& var1);

    LogApp(const LogApp& l) : dispatcherPtr(l.dispatcherPtr), name(l.name) {
        logStream << l.logStream.str();
    }
    LogApp(const std::string& _name, logger_dispatcher::LogDispatcherPtr d);
    ~LogApp();

    template<typename T>
    LogApp& operator<<(T val)
    {
        this->logStream << val;// << std::string(" ");
        return *this;
    }

    LogApp& operator<< (LogAppManipulator manipulator);
protected:
    std::stringstream                   logStream;
    logger_dispatcher::LogDispatcherPtr dispatcherPtr;
    std::string                         name;
};

template<typename T>
const LogAppPtr& operator<<(const LogAppPtr& p, T val)
{
    p->operator <<(val);
    return p;
}

const LogAppPtr& operator<< (const LogAppPtr& p, LogAppManipulator manipulator);

} } } // namespace piel::lib::logger_out
