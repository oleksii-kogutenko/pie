#pragma once

#include <string>

namespace piel { namespace lib { namespace logger {

class LoggerUtils {
private:
    static std::string const  DEFAULT_EMAIL_FROM;
    static std::string const  DEFAULT_EMAIL_TO;
    static std::string const  DEFAULT_EMAIL_MESSAGE;
    static std::string const  DEFAULT_EMAIL_SUBJECT;
    static std::string const  DEFAULT_EMAIL_HOSTNAME;

public:

    static std::string replace(std::string str, std::string from, std::string to);
    static std::string normalize(std::string loggerName);
    static void sendEmail(std::string to, std::string from, std::string subject, std::string message);
    static void sendEmail(std::string subject, std::string message);
};

} } } // namespace piel::lib::logger
