#pragma once

#include "../logbase.h"
#include <string>

namespace errors {
    struct not_implemented {};
}

namespace piel { namespace lib { namespace logger_out {

class BaseLogger : public logger::LogBase {
public:
    static const std::string PLUGINS_LOGGER;
    static char const * const PLUGINS_LOGGER_FILENAME;
    static char const * const TIMESTAMPS_SFX;
private:
    static char const * const LEVEL_SFX;
    static char const * const TRACE_SFX;
    static char const * const DEBUG_SFX;
    static char const * const INFO_SFX;
    static char const * const WARN_SFX;
    static char const * const ERROR_SFX;
    static char const * const FATAL_SFX;

    static const int FATAL_LVL;
    static const int ERROR_LVL;
    static const int WARN_LVL;
    static const int INFO_LVL;
    static const int DEBUG_LVL;
    static const int TRACE_LVL;

    static char const * const TRACE_M;
    static char const * const DEBUG_M;
    static char const * const INFO_M;
    static char const * const WARN_M;
    static char const * const ERROR_M;
    static char const * const FATAL_M;
    static char const * const NULL_M;

    int logLevel;
public:
    virtual void enable(const logger::log_type &type);// = 0;
    virtual void disable(const logger::log_type &type);// = 0;
    virtual void print(const logger::log_type& type, const std::string& var1);// = 0;
    virtual void printMessage(const logger::LogMessage &m);// = 0;

    virtual bool isDebugEnabled();
    virtual bool isErrorEnabled();
    virtual bool isFatalEnabled();
    virtual bool isInfoEnabled();
    virtual bool isTraceEnabled();
    virtual bool isWarnEnabled();

    static const char *getLogTypeStr(const logger::log_type& type);

    BaseLogger(const std::string& name, const std::string& sfx);
    virtual ~BaseLogger();

    std::string toString();
};

} } } // namespace piel::lib::logger_out
