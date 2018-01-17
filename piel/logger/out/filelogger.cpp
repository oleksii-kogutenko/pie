#include "filelogger.h"
#include "../env.h"
#include "../utils.h"
#include "loggerout.h"
#include <iostream>
#include <locale>
#include <ctime>

#define DBG(x) x
#define DBG1(x)
#define DBG2(x)

namespace piel { namespace lib { namespace logger_out {
using namespace logger;

const char* FileLogger::SFX = "_file";
const char* FileLogger::COMMONS_LOGGER = "FileLogger";
const char* FileLogger::TIME_FORMAT = "%Y-%m-%d %I-%M-%S";
std::ofstream    FileLogger::writer;

FileLogger::FileLogger(const std::string& name):BaseLogger(name, SFX)
{
    initLogFile();
}

logger::Log& FileLogger::commonsLog()
{
    //return LoggerOut::get_common_logger();
}

void FileLogger::initLogFile()
{
    if (!writer.is_open()) {
        //final SimpleDateFormat sdf = new SimpleDateFormat();
        //final String logFileName = EnvUtils.safeGetEnvString(PLUGINS_LOGGER_FILENAME, "");
        std::string logFileName = Env::getEnv(PLUGINS_LOGGER_FILENAME, std::string(""));
        if (!logFileName.empty()) {
            writer.open(logFileName);
            if (!writer.is_open()) {
               //commonsLog().error(std::string("Can't create log file: ") + logFileName);
            } else {
                writer << "--------------------------------------------------------------------------------\n";
                writer << " Log started at: " << Utils::timeToStr(TIME_FORMAT, std::time(nullptr)) << "\n";
                writer << "--------------------------------------------------------------------------------\n";
                writer << "\n";
                writer.flush();
            }
        }
    }
}

void FileLogger::print(const std::string& str) {
    if (writer.is_open()){
        writer << str;
        writer.flush();
    }
}

} } } // namespace piel::lib::logger_out
