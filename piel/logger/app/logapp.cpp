#include "logapp.h"
#include "../env.h"
#include "../loggerutils.h"
#include "../utils.h"
#include <stdarg.h>
#include "../dispatcher/logdispatcher.h"

namespace piel { namespace lib { namespace logger_app {

using namespace logger;

void LogApp::trace(const std::string& var1){ dispatcherPtr->enqueue(LogMessage(name, TRACE, var1)); }
void LogApp::debug(const std::string& var1){ dispatcherPtr->enqueue(LogMessage(name, DEBUG, var1)); }
void LogApp::info(const std::string& var1) { dispatcherPtr->enqueue(LogMessage(name, INFO,  var1)); }
void LogApp::warn(const std::string& var1) { dispatcherPtr->enqueue(LogMessage(name, WARN,  var1)); }
void LogApp::error(const std::string& var1){ dispatcherPtr->enqueue(LogMessage(name, ERROR, var1)); }
void LogApp::fatal(const std::string& var1){ dispatcherPtr->enqueue(LogMessage(name, FATAL, var1)); }

LogApp::LogApp(const std::string& _name, logger_dispatcher::LogDispatcherPtr d) : name(_name)
{
    dispatcherPtr = d;
}

LogApp::~LogApp()
{
    dispatcherPtr->enqueue(LogMessage(name, REMOVE_LOG));
}

} } } // namespace piel::lib::logger_out

/*
 * TODO:
 * Logger log = Logger::get_logger("name");
 *
 * log(Logger::trace)("Some info")(5)(6).endl
 * ==> list_messages.add(message_type:trace, log_message:"Some info 5 6")
 *
 * thread::
 * {null_log|console_log|file_log}
 * ==> get log_message from list_messages: if (message_type.allowed) print(log_message)
 *
*/


/*
template<typename T> void insert_into_stream(std::ostream& ostream, const T& val)
{
    ostream << val;
}

struct Logger {

    struct Level {};

    template<typename T>
    const Logger& operator()(const T& val) const
    {
        insert_into_stream((*osp_), val);
        (*osp_) << " ";
    }

    Level trace;
    Level debug;
    Level info;

private:
    boost::shared_ptr<std::ostream> osp_;

};
*/
