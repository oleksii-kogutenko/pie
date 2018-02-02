#pragma once

#include <string>
#include <boost/shared_ptr.hpp>
#include "log.h"

namespace piel { namespace lib { namespace logger {

class LogBase : public Log {
public:
    virtual bool isDebugEnabled() { return _isDebugEnabled; }
    virtual bool isErrorEnabled() { return _isErrorEnabled; }
    virtual bool isFatalEnabled() { return _isFatalEnabled; }
    virtual bool isInfoEnabled()  { return _isInfoEnabled; }
    virtual bool isTraceEnabled() { return _isTraceEnabled; }
    virtual bool isWarnEnabled()  { return _isWarnEnabled; }

    virtual void trace(const std::string& var1) { if (isTraceEnabled()) print(var1); }
    virtual void debug(const std::string& var1) { if (isDebugEnabled()) print(var1); }
    virtual void info(const std::string& var1) { if (isInfoEnabled()) print(var1); }
    virtual void warn(const std::string& var1) { if (isWarnEnabled()) printWarn(var1); }
    virtual void error(const std::string& var1) { if (isErrorEnabled()) printErr(var1); }
    virtual void fatal(const std::string& var1) { if (isFatalEnabled()) printErr(var1); }

    virtual void enable(const log_type& type) {}
    virtual void disable(const log_type& type) {}

    virtual void printMessage(const LogMessage & m) { print(m.message); }

    virtual std::string getName() { return name; }

    LogBase(const std::string& n);// : name(n) {}
    virtual ~LogBase();// {}
protected:
    virtual void print(const std::string& var1) {}
    virtual void printWarn(const std::string& var1) {print(var1);}
    virtual void printErr(const std::string& var1) {print(var1);}

protected:
    std::string name;
    bool _isFatalEnabled; // 0
    bool _isErrorEnabled; // 1
    bool _isWarnEnabled;  // 2
    bool _isInfoEnabled;  // 3
    bool _isDebugEnabled; // 4
    bool _isTraceEnabled; // 5
};

} } } // namespace piel::lib::logger
