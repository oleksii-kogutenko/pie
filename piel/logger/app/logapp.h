#pragma once

#include <string>
#include "../dispatcher/logdispatcher_types.h"

namespace piel { namespace lib { namespace logger_app {

class LogApp {
public:
    virtual void trace(const std::string& var1);
    virtual void debug(const std::string& var1);
    virtual void info(const std::string& var1);
    virtual void warn(const std::string& var1);
    virtual void error(const std::string& var1);
    virtual void fatal(const std::string& var1);

    LogApp(const std::string& _name, logger_dispatcher::LogDispatcherPtr d);
    virtual ~LogApp();

protected:
    logger_dispatcher::LogDispatcherPtr dispatcherPtr;
    std::string              name;
};

} } } // namespace piel::lib::logger_out
