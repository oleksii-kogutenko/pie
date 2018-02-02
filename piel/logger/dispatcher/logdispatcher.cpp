#include "logdispatcher.h"
#include <iostream>
#include "../out/loggerout.h"
#include "../app/logapp.h"

namespace piel { namespace lib { namespace logger_dispatcher {
using namespace logger;

LogDispatcher::LogDispatcher()
{}

LogDispatcher::~LogDispatcher()
{}

void LogDispatcher::enqueue(const LogMessage& m)
{
    enqueue_signal(m);
}

} } } // namespace piel::lib::logger_dispatcher
