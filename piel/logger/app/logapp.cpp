#include "logapp.h"
#include "../env.h"
#include "../loggerutils.h"
#include "../utils.h"
#include <stdarg.h>
#include "../dispatcher/logdispatcher.h"

namespace piel { namespace lib { namespace logger_app {

using namespace logger;
using namespace logger_dispatcher;
using namespace std;

const LogAppPtr& operator<< (const LogAppPtr& p, LogAppManipulator manipulator)
{
    manipulator(*p.get());
    return p;
}

void LogApp::trace(const std::string& var1){ dispatcherPtr->enqueue(LogMessage(name, TRACE, var1)); }
void LogApp::debug(const std::string& var1){ dispatcherPtr->enqueue(LogMessage(name, DEBUG, var1)); }
void LogApp::info(const std::string& var1) { dispatcherPtr->enqueue(LogMessage(name, INFO,  var1)); }
void LogApp::warn(const std::string& var1) { dispatcherPtr->enqueue(LogMessage(name, WARN,  var1)); }
void LogApp::error(const std::string& var1){ dispatcherPtr->enqueue(LogMessage(name, ERROR, var1)); }
void LogApp::fatal(const std::string& var1){ dispatcherPtr->enqueue(LogMessage(name, FATAL, var1)); }

LogApp& trace(LogApp& val) { val.trace(val.logStream.str()); val.clear(); return val; }
LogApp& debug(LogApp& val) { val.debug(val.logStream.str()); val.clear(); return val; }
LogApp& info(LogApp& val)  { val.info (val.logStream.str()); val.clear(); return val; }
LogApp& warn(LogApp& val)  { val.warn (val.logStream.str()); val.clear(); return val; }
LogApp& error(LogApp& val) { val.error(val.logStream.str()); val.clear(); return val; }
LogApp& fatal(LogApp& val) { val.fatal(val.logStream.str()); val.clear(); return val; }

LogApp::LogApp(const string& _name, LogDispatcherPtr d)
    : name(_name)
    , logStream("")
{ dispatcherPtr = d; }

LogApp::~LogApp()
{
    dispatcherPtr->enqueue(LogMessage(name, REMOVE_LOG));
}

LogApp& LogApp::operator<< (LogAppManipulator manipulator)
{
    return manipulator(*this);
}

void LogApp::clear()
{
    logStream.str("");
}

} } } // namespace piel::lib::logger_out

