#include <list>
#include <string>
#include <iostream>
#include <boost/lambda/bind.hpp>
#include "loggerout.h"
#include "../loggerutils.h"
#include "../env.h"
#include "../utils.h"
#include "baselogger.h"
#include "filelogger.h"
#include "consolelogger.h"
#include "commonlogger.h"
#include "nulllogger.h"
#include "../dispatcher/logdispatcher.h"

namespace piel { namespace lib { namespace logger_out {
using namespace piel::lib::logger;
using namespace std;

const char* LoggerOut::TIME_FORMAT = "%Y-%m-%d %I:%M:%S";
LogPtr LoggerOut::commonLogger(new CommonLogger());

LoggerOut::LoggerOut()
    : qthread_(QueuedThread<logger::LogMessage>::start())
{
    qthread_->on_message.connect(boost::bind(&LoggerOut::on_message, this, _1));
}

LoggerOut::~LoggerOut()
{
    qthread_->enqueue(logger::LogMessage(logger::QUIT));
    qthread_->join();
}

void LoggerOut::enqueue(const logger::LogMessage& v)
{
    qthread_->enqueue(v);
}

void LoggerOut::genMessage(logger::LogMessage& m)
{
    std::stringstream os;

    if (enableLogsTimestamps) {
        os << "[" << Utils::timeToStr(TIME_FORMAT) << "]";
    }
    os << "[" << m.name << "][" << BaseLogger::getTypeLogStr(m.type) << "] " << m.message;

    m.message = os.str();
}

void LoggerOut::create_log(const std::string& _name)
{
    std::string nName = LoggerUtils::normalize(_name);
    {
        LogPtr log {new NullLogger(_name)};
        logs.push_back(log);
    }

    if (!Env::getEnv(nName + ConsoleLogger::SFX, false) ||
         Env::getEnv(BaseLogger::PLUGINS_LOGGER + ConsoleLogger::SFX, true) ) {
        LogPtr log {new ConsoleLogger(_name)};
        logs.push_back(log);
    }
    if (!Env::getEnv(FileLogger::PLUGINS_LOGGER_FILENAME, std::string("")).empty()) {
        if (Env::getEnv(nName + FileLogger::SFX, false) ||
            Env::getEnv(BaseLogger::PLUGINS_LOGGER + FileLogger::SFX, false)) {
            LogPtr log {new FileLogger(_name)};
            logs.push_back(log);

        }
    }
    enableLogsTimestamps = Env::getEnv(BaseLogger::PLUGINS_LOGGER + BaseLogger::TIMESTAMPS_SFX, false);
}

void LoggerOut::remove_log(const std::string& _name)
{
    logs.remove_if(bind2nd(checkEqualLog(), _name));
    logs.remove_if(bind2nd(checkEqualLog(), _name+"_null"));
}

void LoggerOut::print(const logger::LogMessage& m)
{
    logger::LogMessage new_m(m);
    genMessage(new_m);

    for(LogPtrList::const_iterator i=logs.begin(), end = logs.end(); i!=end; ++i) {
        if ( (*i)->getName() != m.name ) continue;
        (*i)->printMessage(new_m);
    }
}

int LoggerOut::on_message(const logger::LogMessage& m)
{
    int retVal = 0;
    switch (m.cmd) {
    case logger::LOG:
        print(m);
        break;
    case logger::CREATE_LOG:
        create_log(m.name);
        break;
    case logger::REMOVE_LOG:
        remove_log(m.name);
        break;
    case logger::QUIT:
        retVal = -1;
        break;
    default:
        break;
    }
    return retVal;
}

logger::LogPtr LoggerOut::getCommonLogger()
{
    return commonLogger;
}

} } } // namespace piel::lib::logger_out
