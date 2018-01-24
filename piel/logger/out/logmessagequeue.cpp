#include "logmessagequeue.h"

namespace piel { namespace lib { namespace logger_out {
using namespace logger_out;

LogMessageQueue::LogMessageQueue()
{}

LogMessageQueue::~LogMessageQueue()
{}

void LogMessageQueue::push(const logger::LogMessage& m)
{
    boost::unique_lock<boost::mutex> lock{queueMutex};
    queue.push(m);
    queueCond.notify_all();
}

void LogMessageQueue::pop(logger::LogMessage& m)
{
    bool isEmpty;
    {
        boost::unique_lock<boost::mutex> lock{queueMutex};
        isEmpty = queue.empty();
    }
    if (isEmpty)
    {
        boost::unique_lock<boost::mutex> lock{queueMutex};
        queueCond.wait(lock);
    }
    {
        boost::unique_lock<boost::mutex> lock{queueMutex};
        m = queue.front();
        queue.pop();
    }
}

} } } // namespace piel::lib::logger_dispatcher
