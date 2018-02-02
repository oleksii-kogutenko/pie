#pragma once

#include <string>
#include "../dispatcher/logdispatcher_types.h"
#include "../app/logapp_types.h"
#include "../singletone.h"
#include "../out/loggerout_types.h"


namespace piel { namespace lib { namespace logger_app {

class Logging : public SingleTon<Logging>
{
private:
    logger_out::LoggerOutPtr         loggerOutThread;
public:
    typedef Logging* RawPtr;

    Logging();
    ~Logging();
    static logger_app::LogAppPtr create_logger(std::string name);
};

} } } // namespace piel::lib::logger_out

