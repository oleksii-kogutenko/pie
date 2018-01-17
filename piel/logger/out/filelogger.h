#pragma once

#include "baselogger.h"
#include <fstream>

namespace piel { namespace lib { namespace logger_out {

class FileLogger : public BaseLogger {
private:
    static const char*      COMMONS_LOGGER;
    static const char*      TIME_FORMAT;
    static std::ofstream    writer;

    static Log &commonsLog();
    static void initLogFile();

protected:
    virtual void print(const std::string& str);

public:
    static const char* SFX;

    FileLogger(const std::string& name);
    virtual ~FileLogger() {}
};

} } } // namespace piel::lib::logger_out
