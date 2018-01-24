#include "baselogger.h"
#include "../env.h"
#include "../loggerutils.h"
#include "../utils.h"
#include <stdarg.h>

#define DEF_PLUGINS_LOGGER  "plugins_logger"

#define DBG(x)
#define DBG1(x)
#define DBG2(x)

namespace piel { namespace lib { namespace logger_out {

using namespace logger;
using namespace std;

const std::string BaseLogger::PLUGINS_LOGGER          = DEF_PLUGINS_LOGGER;
char const * const BaseLogger::PLUGINS_LOGGER_FILENAME = DEF_PLUGINS_LOGGER"_filename";

char const * const BaseLogger::TIMESTAMPS_SFX   = "_timestamps";

char const * const BaseLogger::LEVEL_SFX = "_level";
char const * const BaseLogger::TRACE_SFX = "_trace";
char const * const BaseLogger::DEBUG_SFX = "_debug";
char const * const BaseLogger::INFO_SFX = "_info";
char const * const BaseLogger::WARN_SFX = "_warn";
char const * const BaseLogger::ERROR_SFX = "_error";
char const * const BaseLogger::FATAL_SFX = "_fatal";

const int BaseLogger::FATAL_LVL = 0;
const int BaseLogger::ERROR_LVL = 1;
const int BaseLogger::WARN_LVL = 2;
const int BaseLogger::INFO_LVL = 3;
const int BaseLogger::DEBUG_LVL = 4;
const int BaseLogger::TRACE_LVL = 5;

char const * const BaseLogger::TRACE_M = "T";
char const * const BaseLogger::DEBUG_M = "D";
char const * const BaseLogger::INFO_M = "I";
char const * const BaseLogger::WARN_M = "W";
char const * const BaseLogger::ERROR_M = "E";
char const * const BaseLogger::FATAL_M = "F";
char const * const BaseLogger::NULL_M  = "?";

BaseLogger::BaseLogger(const std::string &_name, const std::string &sfx) : LogBase(_name)
{
    DBG(std::cout << __PRETTY_FUNCTION__ << ":" << _name << " sfx:" << sfx << std::endl;)

    int logLevelAll = Env::getEnv(PLUGINS_LOGGER + LEVEL_SFX, INFO_LVL);

    bool isTraceEnabledAll = Env::getEnv(PLUGINS_LOGGER + TRACE_SFX, false);
    bool isDebugEnabledAll = Env::getEnv(PLUGINS_LOGGER + DEBUG_SFX, false);
    bool isInfoEnabledAll = Env::getEnv(PLUGINS_LOGGER + INFO_SFX, false);
    bool isWarnEnabledAll = Env::getEnv(PLUGINS_LOGGER + WARN_SFX, false);
    bool isErrorEnabledAll = Env::getEnv(PLUGINS_LOGGER + ERROR_SFX, false);
    bool isFatalEnabledAll = Env::getEnv(PLUGINS_LOGGER + FATAL_SFX, false);

    DBG(std::cout << __PRETTY_FUNCTION__ << " === " << __LINE__ <<  std::endl;)
    DBG2(std::cout << __PRETTY_FUNCTION__ << " " << name <<  " " << __LINE__ <<  " " << \
         " logLevelAll:" << logLevelAll << \
         " isTraceEnabledAll:" << isTraceEnabledAll << \
         " isDebugEnabledAll:" << isDebugEnabledAll << \
         " isInfoEnabledAll:" << isInfoEnabledAll << \
         " isWarnEnabledAll:" << isWarnEnabledAll << \
         " isErrorEnabledAll:" << isErrorEnabledAll << \
         " isFatalEnabledAll:" << isFatalEnabledAll << \
         std::endl;)

    std::string const pName = PLUGINS_LOGGER + sfx;
    logLevelAll = Env::getEnv(pName + LEVEL_SFX, logLevelAll);

    isTraceEnabledAll = Env::getEnv(pName + TRACE_SFX, isTraceEnabledAll);
    isDebugEnabledAll = Env::getEnv(pName + DEBUG_SFX, isDebugEnabledAll);
    isInfoEnabledAll = Env::getEnv(pName + INFO_SFX, isInfoEnabledAll);
    isWarnEnabledAll = Env::getEnv(pName + WARN_SFX, isWarnEnabledAll);
    isErrorEnabledAll = Env::getEnv(pName + ERROR_SFX, isErrorEnabledAll);
    isFatalEnabledAll = Env::getEnv(pName + FATAL_SFX, isFatalEnabledAll);

    DBG(std::cout << __PRETTY_FUNCTION__ << " === " << __LINE__ <<  std::endl;)
    DBG2(std::cout << __PRETTY_FUNCTION__ << " " << name <<  " " << __LINE__ <<  " " << \
         " pName:" << pName << \
         " logLevelAll:" << logLevelAll << \
         " isTraceEnabledAll:" << isTraceEnabledAll << \
         " isDebugEnabledAll:" << isDebugEnabledAll << \
         " isInfoEnabledAll:" << isInfoEnabledAll << \
         " isWarnEnabledAll:" << isWarnEnabledAll << \
         " isErrorEnabledAll:" << isErrorEnabledAll << \
         " isFatalEnabledAll:" << isFatalEnabledAll << \
         std::endl;)

    std::string const nName = LoggerUtils::normalize(name) + sfx;
    logLevel = Env::getEnv(nName + LEVEL_SFX, logLevelAll);

    DBG(std::cout << __PRETTY_FUNCTION__ << " === " << __LINE__ <<  std::endl;)
    _isTraceEnabled = Env::getEnv(nName + TRACE_SFX, isTraceEnabledAll);
    _isDebugEnabled = Env::getEnv(nName + DEBUG_SFX, isDebugEnabledAll);
    _isInfoEnabled = Env::getEnv(nName + INFO_SFX, isInfoEnabledAll);
    _isWarnEnabled = Env::getEnv(nName + WARN_SFX, isWarnEnabledAll);
    _isErrorEnabled = Env::getEnv(nName + ERROR_SFX, isErrorEnabledAll);
    _isFatalEnabled = Env::getEnv(nName + FATAL_SFX, isFatalEnabledAll);

    DBG2(std::cout << __PRETTY_FUNCTION__ << " " << name <<  " " << __LINE__ <<  " " << \
         " nName:" << nName << \
         " logLevelAll:" << logLevel << \
         " _isTraceEnabled:" << _isTraceEnabled << \
         " _isDebugEnabled:" << _isDebugEnabled << \
         " _isInfoEnabled:" << _isInfoEnabled << \
         " _isWarnEnabled:" << _isWarnEnabled << \
         " _isErrorEnabled:" << _isErrorEnabled << \
         " _isFatalEnabled:" << _isFatalEnabled << \
         std::endl;)

    DBG2(std::cout << __PRETTY_FUNCTION__ << ":" << toString() << std::endl;)
}

BaseLogger::~BaseLogger()
{
    DBG(std::cout << __PRETTY_FUNCTION__ << ":" << name << std::endl;)
}

bool BaseLogger::isFatalEnabled() { return _isFatalEnabled || _isTraceEnabled || logLevel >= FATAL_LVL; }
bool BaseLogger::isErrorEnabled() { return _isErrorEnabled || _isTraceEnabled || logLevel >= ERROR_LVL; }
bool BaseLogger::isWarnEnabled()  { return _isWarnEnabled || _isTraceEnabled || logLevel >= WARN_LVL; }
bool BaseLogger::isInfoEnabled()  { return _isInfoEnabled || _isTraceEnabled || logLevel >= INFO_LVL; }
bool BaseLogger::isDebugEnabled() { return _isDebugEnabled || _isTraceEnabled || logLevel >= DEBUG_LVL; }
bool BaseLogger::isTraceEnabled() { return _isTraceEnabled || logLevel >= TRACE_LVL; }

std::string BaseLogger::toString()
{
    return Utils::format("%s (%s) levels:%d F:%d E:%d W:%d i:%d D:%d T:%d", name.c_str(), LoggerUtils::normalize(name).c_str(),
                logLevel,
                _isFatalEnabled, _isErrorEnabled, _isWarnEnabled,
                _isInfoEnabled, _isDebugEnabled, _isTraceEnabled);
}

void BaseLogger::enable(const logger::log_type& type)
{
    switch (type) {
    case logger::TRACE: _isTraceEnabled = true; break;
    case logger::DEBUG: _isDebugEnabled = true; break;
    case logger::INFO:  _isInfoEnabled  = true; break;
    case logger::WARN:  _isWarnEnabled  = true; break;
    case logger::ERROR: _isErrorEnabled = true; break;
    case logger::FATAL: _isFatalEnabled = true; break;
    }
}

void BaseLogger::disable(const logger::log_type& type)
{
    switch (type) {
    case logger::TRACE: _isTraceEnabled = false; break;
    case logger::DEBUG: _isDebugEnabled = false; break;
    case logger::INFO:  _isInfoEnabled  = false; break;
    case logger::WARN:  _isWarnEnabled  = false; break;
    case logger::ERROR: _isErrorEnabled = false; break;
    case logger::FATAL: _isFatalEnabled = false; break;
    }
}

void BaseLogger::print(const logger::log_type& type, const std::string& var1)
{
    DBG(std::cout << __PRETTY_FUNCTION__ << ":" << type << "," << var1 << std::endl;)
    switch (type) {
    case logger::TRACE: trace(var1); break;
    case logger::DEBUG: debug(var1); break;
    case logger::INFO:  info(var1);  break;
    case logger::WARN:  warn(var1);  break;
    case logger::ERROR: error(var1); break;
    case logger::FATAL: fatal(var1); break;
    }
}

void BaseLogger::printMessage(const logger::LogMessage& m)
{
    DBG(std::cout << __PRETTY_FUNCTION__ << ":" << m.toString() << std::endl;)
    print(m.type, m.message);
}

const char *BaseLogger::getTypeLogStr(const logger::log_type& type)
{
    switch (type) {
    case logger::TRACE: return TRACE_M;
    case logger::DEBUG: return DEBUG_M;
    case logger::INFO:  return INFO_M;
    case logger::WARN:  return WARN_M;
    case logger::ERROR: return ERROR_M;
    case logger::FATAL: return FATAL_M;
    }
    return NULL_M;
}


} } } // namespace piel::lib::logger_out
