#include <string>
#include "../dispatcher/logdispatcher_types.h"
#include "../dispatcher/logdispatcher.h"
#include "../app/logapp_types.h"
#include "../app/logapp.h"
#include "../out/loggerout_types.h"
#include "../out/loggerout.h"
#include "logging.h"

namespace piel { namespace lib { namespace logger_app {

Logging::Logging()
{

}

Logging::~Logging()
{

}

logger_app::LogAppPtr Logging::create_logger(std::string name)
{

    RawPtr p = getInstance();
    logger::LogMessage m(name, logger::CREATE_LOG);

    if (!p->loggerOutThread) {
        p->loggerOutThread.reset(new logger_out::LoggerOut());
    }

    logger_dispatcher::LogDispatcherPtr disp(new logger_dispatcher::LogDispatcher());
    disp->connect(boost::bind(&logger_out::LoggerOut::enqueue, p->loggerOutThread.get(), _1));
    disp->enqueue(m);

    logger_app::LogAppPtr res(new LogApp(name, disp));
    return res;
}

} } } // namespace piel::lib::logger_out

