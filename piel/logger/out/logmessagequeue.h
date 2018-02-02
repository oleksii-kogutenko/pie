#pragma once

#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include "../logmessage.h"

namespace piel { namespace lib { namespace logger_out {

class LogMessageQueue {
protected:
    logger::LogMessagesQueue    queue;
    boost::mutex                queueMutex;
    boost::condition_variable   queueCond;

public:
    LogMessageQueue();
    ~LogMessageQueue();

    void push(const logger::LogMessage& m);
    void pop(logger::LogMessage& m); //locked function (waiting new message)
};

} } } // namespace piel::lib::logger_app
