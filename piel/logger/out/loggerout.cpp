#include <list>
#include <string>
#include <iostream>
#include <boost/lambda/bind.hpp>
#include "loggerout.h"
#include "baselogger.h"
#include "filelogger.h"
#include "consolelogger.h"
#include "commonlogger.h"
#include "nulllogger.h"
#include "../dispatcher/logdispatcher.h"
#include "../utils.h"

namespace piel { namespace lib { namespace logger_out {
using namespace piel::lib::logger;
using namespace piel::lib::utils;
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
        os << "[" << timeToStr(TIME_FORMAT) << "]";
    }
    os << "[" << m.name << "][" << BaseLogger::getLogTypeStr(m.type) << "] " << m.message;

    m.message = os.str();
}

void LoggerOut::create_log(const std::string& _name)
{
    std::string nName = normalize(_name);
    {
        LogPtr log {new NullLogger(_name)};
        logPtrList.push_back(log);
    }

    if (!getEnv(nName + ConsoleLogger::SFX, false) ||
         getEnv(BaseLogger::PLUGINS_LOGGER + ConsoleLogger::SFX, true) ) {
        LogPtr log {new ConsoleLogger(_name)};
        logPtrList.push_back(log);
    }

    if (!getEnv(FileLogger::PLUGINS_LOGGER_FILENAME, std::string("")).empty()) {
        if (getEnv(nName + FileLogger::SFX, false) ||
            getEnv(BaseLogger::PLUGINS_LOGGER + FileLogger::SFX, false)) {
            LogPtr log {new FileLogger(_name)};
            logPtrList.push_back(log);

        }
    }

    enableLogsTimestamps = getEnv(BaseLogger::PLUGINS_LOGGER + BaseLogger::TIMESTAMPS_SFX, false);
}

void LoggerOut::remove_log(const std::string& _name)
{
    logPtrList.remove_if(bind2nd(checkEqualLog(), _name));
    logPtrList.remove_if(bind2nd(checkEqualLog(), _name+"_null"));
}

void LoggerOut::print(const logger::LogMessage& m)
{
    logger::LogMessage new_m(m);
    genMessage(new_m);

    for(LogPtrList::const_iterator i=logPtrList.begin(), end = logPtrList.end(); i!=end; ++i) {
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
