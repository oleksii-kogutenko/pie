#pragma once

#include <string>
#include <queue>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include "../log.h"
#include "../singletone.h"
#include "commonlogger.h"
#include "loggerout_types.h"
#include "logmessagequeue.h"
#include "../../queuedthread.h"

namespace piel { namespace lib { namespace logger_out {

LogMessageQueueWeakPtr createOutputLogger();

class LoggerOut {
protected:
    class checkEqualLog : public std::binary_function<logger::LogPtr, std::string, bool>
    {
        public:
            bool operator()(const logger::LogPtr &log, const std::string &name) const
            {   return log->getName() == name;}
    };

protected:
    static const char*      TIME_FORMAT;
    logger::LogPtrList      logPtrList;
    ThreadPtr               threadPtr;
    bool                    enableLogsTimestamps;
    LogMessageQueuePtr      queuePtr;

    void create_log(const std::string& name);
    void remove_log(const std::string& name);
    void print(const logger::LogMessage& m);
    bool process_queue();
    void genMessage(logger::LogMessage& m);        

public:
    LoggerOut();
    virtual ~LoggerOut();

    int on_message(const logger::LogMessage& m);
    void enqueue(const logger::LogMessage& v);

    static logger::LogPtr getCommonLogger();

private:
    QueuedThread<logger::LogMessage>::ThisPtr qthread_;
    static logger::LogPtr commonLogger;
};

} } } // namespace piel::lib::logger_app
